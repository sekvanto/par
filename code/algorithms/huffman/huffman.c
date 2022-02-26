#include "huffman.h"
#include "heading.h"
#include "../../archiver.h"
#include "../../linkedlist.h"

static FILE* fileIn;
static FILE* fileOut;
static HuffmanHeading heading;

uint8_t buffer[BLOCK_SIZE];

static void post() {
    destroylist(heading.treeLeaves);
    destroylist(heading.treeShape);
}

/*
 * Determines the number of unique bytes in file
 * Returns true if the file has 2+ unique bytes
 */
static bool is_file_correct(FILE* file) {
    uint8_t firstByte;
    fread(&firstByte, sizeof(uint8_t), 1, fileIn);
    uint8_t secondByte = firstByte;
    while (fread(&secondByte, sizeof(uint8_t), 1, fileIn) != 0) {
        if (firstByte != secondByte) {
#ifdef VERBOSE
            printf("First two bytes: %c, %c\n\n", firstByte, secondByte);
#endif
            return true;
        }
    }
    return false;
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

#ifdef VERBOSE
    printf("Flat tree shape:\n");
    list_display(heading.treeShape);
    printf("Flat tree leaves:\n");
    list_display(heading.treeLeaves);
    printf("Tree shape size (in bytes): %d\n", heading.treeShapeSize);
    printf("Tree leaves size (in bytes): %d\n\n", heading.treeLeavesSize);
#endif
}

int huffman_archive(Data* data, FILE* in, FILE* out) {
    fileIn = in;
    fileOut = out;
    init_huffman_heading(&heading);

    if (!is_file_correct(fileIn)) {
        archiveError("Incorrect file: contains less than 2 unique bytes");
        return FAILURE;
    }
    fseek(fileIn, 0, SEEK_SET);

    HuffmanTreeNode* tree = build_huffman_tree(fileIn);
    flatten_tree(tree);
    //write heading

    free_huffman_tree(tree);
    post();
    return 0;
}