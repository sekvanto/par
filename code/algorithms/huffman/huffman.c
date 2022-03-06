#include <stdlib.h>

#include "huffman.h"
#include "heading.h"
#include "../../archiver.h"
#include "../../utils/linkedlist.h"

/**
 * This structure represents a byte sequence.
 * It's basically the same as **int** but with an additional size field
 * (for knowing which bits of the value field are included into the sequence)
 * (e.g. val = "0b00000000000000001", size = "3" => sequence = "001")
 */
typedef struct {
    uint32_t value; /* Value of byte sequence */
    size_t size;    /* Size of value in bits */
} Sequence;

static HuffmanHeading heading;

static void post() {
    destroylist(heading.treeLeaves);
    destroylist(heading.treeShape);
}

/*
 * Flattens tree structure
 */
static void flatten_tree_shape(HuffmanTreeNode* tree) {
    if (tree->hasValue) { /* Tree leaf */
        list_add(false, heading.treeShape);
        return;
    }
    /* Otherwise, it's a parent node */
    list_add(true, heading.treeShape);
    flatten_tree_shape(tree->left);
    flatten_tree_shape(tree->right);
}

/*
 * Flattens tree leaves
 */
static void flatten_tree_leaves(HuffmanTreeNode* tree) {
    if (tree->hasValue) {
        list_add(tree->uniqueByte, heading.treeLeaves);
        return;
    }
    /* Otherwise, it's a parent node */
    flatten_tree_leaves(tree->left);
    flatten_tree_leaves(tree->right);
}

/*
 * Given a huffman encoding tree, flattens it and initializes
 * heading.treeShape, treeLeaves, and their sizes
 */
static void flatten_tree(HuffmanTreeNode* tree) {
    flatten_tree_shape(tree);
    flatten_tree_leaves(tree);

    /* Initialize tree shape size in bytes */
    heading.treeShapeSize = list_size(heading.treeShape) / BYTE_SIZE;
    if (list_size(heading.treeShape) % BYTE_SIZE != 0) { /* Round up if there's a remainder */
        heading.treeShapeSize++;
    }
    /* Initialize tree leaves size in bytes */
    heading.treeLeavesSize = list_size(heading.treeLeaves) - 1;

#ifdef DEBUG
    printf("Flat tree shape:\n");
    list_display(heading.treeShape);
    printf("Flat tree leaves:\n");
    list_display(heading.treeLeaves);
    printf("Tree shape size (in bytes): %d\n", heading.treeShapeSize);
    printf("Tree leaves size (in bytes): %d\n\n", heading.treeLeavesSize + 1);
#endif
}

static void write_heading() {
    /* One empty byte, will be overwritten in the end of the program */
    uint8_t byte = 0;
    fwrite(&byte, sizeof(uint8_t), 1, fileOut);

    /* Other fields */
    fwrite(&heading.signature, sizeof(uint8_t), 1, fileOut);
    fwrite(&heading.treeShapeSize, sizeof(uint8_t), 1, fileOut);
    fwrite(&heading.treeLeavesSize, sizeof(uint8_t), 1, fileOut);

    /* Writing tree shape bytes */
    for (size_t i = 0; i < heading.treeShapeSize; i++) {
        byte = 0;
        /* Generating each byte from 8 bits */
        for (size_t j = 0; j < BYTE_SIZE; j++) {
            bool bit = list_poll(heading.treeShape);
            if (bit) {
                byte |= (1 << (BYTE_SIZE - 1 - j));
            }
        }
        /* Write current byte */
        fwrite(&byte, sizeof(uint8_t), 1, fileOut);
    }
    
    /* Writing tree leaves bytes */
    while (list_size(heading.treeLeaves) != 0) {
        byte = list_poll(heading.treeLeaves);
        fwrite(&byte, sizeof(uint8_t), 1, fileOut);
    }
}

/*
 * Given a huffman encoding tree, builds a map of all leaves and their values.
 * An array of ByteSequences (size = 256) represents the map. The index of each element
 * is a unique byte value. The value of each element is its encoded sequence of bits
 *
 * currentSeq - current sequence, which we are forming for the next leaf
 * size       - current sequence size
 */
static void build_map(HuffmanTreeNode* tree, Sequence* map, int currentSeq, int size) {
    if (tree->hasValue) { /* We reached a tree leaf */
        Sequence seq = {currentSeq, size};
        map[tree->uniqueByte] = seq;
        return;
    }
    /* Otherwise, we have a parent node */
    currentSeq <<= 1; /* Shifting current path sequence one cell left */
    size++;           /* Incrementing size of sequence */

    /* First, moving left */
    build_map(tree->left, map, currentSeq, size);

    /* Then, moving right */
    currentSeq |= 1;   /* Right => sequence has "1" added */
    build_map(tree->right, map, currentSeq, size);
}

/*
 * Given a buffer of bytes with a specified block size, compresses info there according to the
 * association table and the seqSize and writes it into the output stream
 *
 * map - Huffman encoding tree map
 * returns - Number of additional bits added to the archive
 */
static uint8_t compress_in_blocks(Sequence* map) {
    uint8_t current;                                    /* Current byte to be written */
    size_t replaceValIndex = 0;                         /* Index of current bit in replace value */
    size_t bufferIndexIn   = 0;                         /* Index of current byte in the input buffer */
    size_t bufferIndexOut  = 0;                         /* Index of current byte in the output buffer */
    size_t size = update_buffer();                      /* Update buffer and get the size of bytes read from file */
    Sequence currentReplaceVal = map[bufferIn[0] & 0xff]; /* Replace bit combination of current byte in the buffer */

    int i = 0;

    /*
     * Each iteration:
     * 1. Form byte
     * 2. Write byte to output file
     */
    while (size > 0) {
        current = 0;

        /* Forming a byte */
        for (i = 0; i < BYTE_SIZE; i++) {
            /* Adding one bit a time */
            int mask = 1 << (currentReplaceVal.size - 1 - replaceValIndex);
            int currentBit = currentReplaceVal.value & mask;

            /* Adding bit to the byte, which is being archived currently */
            current |= ((currentBit >> (currentReplaceVal.size - 1 - replaceValIndex)) << (BYTE_SIZE - 1 - i));

            /* Moving to the next byte in the buffer of initial bytes */
            if (currentReplaceVal.size - 1 - replaceValIndex == 0) {
                bufferIndexIn++;
                replaceValIndex = 0;

                /* Update buffer if it ended */
                if (bufferIndexIn == size) {
                    size = update_buffer();
                    if (size <= 0) { /* End of the file */
                        break;
                    }
                    bufferIndexIn = 0;
                }
                currentReplaceVal = map[bufferIn[bufferIndexIn] & 0xff];
            }
            /* Otherwise, moving to the next bit in current value associated bit sequence */
            else {
                replaceValIndex++;
            }
        }
        bufferIndexOut = output_byte(current, bufferIndexOut);
    }
    flush_buffer(bufferIndexOut);
    return (BYTE_SIZE - (i + 1)); /* Number of additional zeros in the end of the file */
}

/*
 * Rewrites the first byte in the archive file with ignoreBits
 */
static void overwrite_ignore_bits(FILE* file) {
    fseek(file, 0, SEEK_SET);
    fwrite(&heading.ignoreBits, sizeof(uint8_t), 1, file);
}

int huffman_archive(Data* data, FILE* in, FILE* out) {
    fileIn = in;
    fileOut = out;
    init_huffman_heading(&heading);

    if (!is_file_correct(fileIn)) {
        archiveError("incorrect file: contains less than 2 unique bytes");
        return FAILURE;
    }
    fseek(fileIn, 0, SEEK_SET);

    HuffmanTreeNode* tree = build_huffman_tree(fileIn);
    flatten_tree(tree);
    write_heading();
    Sequence map[UINT8_COUNT] = {0};
    build_map(tree, map, 0, 0);

#ifdef DEBUG
    printf("Generated tree map:\n");
    for (size_t i = 0; i < UINT8_COUNT; i++) {
        if (i != 0 && i % 16 == 0) {
            printf("\n");
        }
        printf("(%x %ld)\t", map[i].value, map[i].size);
    }
    printf("\n\n");
#endif
    fseek(fileIn, 0, SEEK_SET);
    heading.ignoreBits = compress_in_blocks(map);

#ifdef DEBUG
    printf("ignoreBits: %d\n\n", heading.ignoreBits);
#endif
    free_huffman_tree(tree);
    post();
    overwrite_ignore_bits(fileOut);
    return 0;
}

/*
 * Given tree shape and leaves, unflattens tree and returns it
 */
static HuffmanTreeNode* unflatten_tree(List* treeShape, List* treeLeaves) {
    if (list_size(treeShape) == 0) {
        return NULL;
    }
    bool bit = list_poll(treeShape);

    /* Current bit indicates it's a leaf node */
    if (!bit) {
        HuffmanTreeNode* leaf = malloc(sizeof(HuffmanTreeNode));
        leaf->uniqueByte = list_poll(treeLeaves);
        leaf->hasValue = true;
        leaf->left = NULL;
        leaf->right = NULL;
        return leaf;
    }

    /* Otherwise, we have a parent node */
    HuffmanTreeNode* parent = malloc(sizeof(HuffmanTreeNode));
    parent->hasValue = false;
    parent->left = unflatten_tree(treeShape, treeLeaves);
    parent->right = unflatten_tree(treeShape, treeLeaves);
    return parent;
}

/*
 * Given the encoding tree shape and leaves size, read it from the file,
 * unflattens and returns the result
 */
static HuffmanTreeNode* get_tree(uint16_t shapeSize, uint16_t leavesSize) {
    List* shape = makelist();
    List* leaves = makelist();

    /* Read tree shape */
    for (size_t i = 0; i < shapeSize; i++) {
        uint8_t byte;
        fread(&byte, sizeof(uint8_t), 1, fileIn);

        /* Generate 8 bits and put them to shape array */
        for (size_t j = 0; j < BYTE_SIZE; j++) {
            uint8_t bit = (byte >> (BYTE_SIZE - j - 1)) & 1;
            if (bit == 1) {
                list_add(true, shape);
            } else {
                list_add(false, shape);
            }
        }
    }

    /* Read tree leaves */
    for (size_t i = 0; i < leavesSize; i++) {
        uint8_t byte;
        fread(&byte, sizeof(uint8_t), 1, fileIn);
        list_add(byte, leaves);
    }
#ifdef DEBUG
    printf("Read flat tree shape from file:\n");
    list_display(shape);
    printf("Read flat tree leaves from file:\n");
    list_display(leaves);
    printf("Tree shape size (in bytes): %d\n", shapeSize);
    printf("Tree leaves size (in bytes): %d\n\n", leavesSize);
#endif
    return unflatten_tree(shape, leaves);
}

/*
 * Given a buffer of bytes with a specified blocks size, decompresses info there according to the
 * huffman encoding tree and writes it into the output stream
 *
 * tree - Huffman encoding tree
 * ignoreBits - Bits to ignore in the last element of buffer (to cut redundant bytes in the end of file)
 */
static void decompress_in_blocks(HuffmanTreeNode* tree, uint8_t ignoreBits) {
    uint8_t bitIndex = 0;           /* Index of current bit in buffer's byte (0-7) */
    size_t  bufferIndexIn = 0;      /* Index of current byte in the input buffer */
    size_t  bufferIndexOut = 0;     /* Index of current byte in the output buffer */
    size_t  size = update_buffer(); /* Update buffer and get the size of bytes read from file */

    /*
     * Each iteration:
     * 1. Going through Huffman tree until reaching a leaf
     * 2. Get leaf's value
     * 3. Write it to output stream
     */
    while (size != 0) {
        HuffmanTreeNode* currentNode = tree;

        /* Forming a unique combination and fetching its value from the tree */
        while (!currentNode->hasValue) {
            /* Fetching current bit from the buffer */
            uint32_t bit = ((0b10000000 >> bitIndex) & bufferIn[bufferIndexIn]) >> (BYTE_SIZE - 1 - bitIndex);

            /* Check the value (either 1 or 0) */
            if (bit == 0) {
                currentNode = currentNode->left;
            } else {
                currentNode = currentNode->right;
            }

            /* Check if we should ignore all the next bits in this byte */
            if (bufferIndexIn == size - 1 && at_end(fileIn) && bitIndex == BYTE_SIZE - 1 - ignoreBits) {
                size = 0;
                break;
            }

            /* Moving to the next byte in buffer */
            if (bitIndex == BYTE_SIZE - 1) {
                bufferIndexIn++;
                bitIndex = 0;

                /* Update buffer if it ended */
                if (bufferIndexIn >= size) {
                    size = update_buffer();
                    bufferIndexIn = 0;
                }
            }
            /* Otherwise, moving to the next bit in current buffer's byte */
            else {
                bitIndex++;
            }
        }
        /* We've found the value of current bit combination, writing it to the buffer */
        bufferIndexOut = output_byte(currentNode->uniqueByte, bufferIndexOut);
    }
    /* Flush the rest of the output buffer */
    flush_buffer(bufferIndexOut);
}

int huffman_unarchive(Data* data, FILE* in, FILE* out) {
    fileIn = in;
    fileOut = out;

    /* Read first two heading fields */
    uint8_t ignoreBits, signature;
    fread(&ignoreBits, sizeof(uint8_t), 1, fileIn);
    fread(&signature, sizeof(uint8_t), 1, fileIn);
#ifdef DEBUG
    printf("Ignore bits: %d\n", ignoreBits);
    printf("Signature: 0x%x\n", signature);
#endif
    /* Check signature */
    if (signature != SIG_HUFFMAN) {
        archiveError("invalid archive");
        return FAILURE;
    }

    /* Read last four heading fields */
    uint16_t treeShapeSize = 0, treeLeavesSize = 0;
    fread(&treeShapeSize, sizeof(uint8_t), 1, fileIn);
    fread(&treeLeavesSize, sizeof(uint8_t), 1, fileIn);
    treeLeavesSize++;
    HuffmanTreeNode* tree = get_tree(treeShapeSize, treeLeavesSize);

    /* Decompress file */
    decompress_in_blocks(tree, ignoreBits);
    free_huffman_tree(tree);
}