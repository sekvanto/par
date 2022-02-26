#include "huffman.h"
#include "heading.h"
#include "../../archiver.h"

static FILE* fileIn;
static FILE* fileOut;
static HuffmanHeading heading;

uint8_t buffer[BLOCK_SIZE];

static void post() {
    destroy(heading.treeLeaves);
    destroy(heading.treeShape);
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
    //flatten tree

    free_huffman_tree(tree);
    post();
    return 0;
}