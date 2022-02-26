#include "huffman.h"
#include "heading.h"
#include "../../archiver.h"

#define BLOCK_SIZE 65536

static FILE* fileIn;
static FILE* fileOut;
static uint8_t buffer[BLOCK_SIZE];
static HuffmanHeading heading;

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
    //

#ifdef VERBOSE
    printf("First two bytes: %c, %c\n", firstByte, secondByte);
#endif

    return true;//TEMP
}

int huffman_archive(Data* data, FILE* in, FILE* out) {
    fileIn = in;
    fileOut = out;

    init_huffman_heading(&heading);

    if (!is_file_correct(fileIn)) {
        archiveError("Incorrect file: contains less than 2 unique bytes");
        return FAILURE;
    }
    printf("File is correct, we may continue\n");//TEMP

    post();
    return 0;
}