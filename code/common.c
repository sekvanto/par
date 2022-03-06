#include <stdio.h>
#include <stdlib.h>

#include "common.h"

uint8_t bufferIn[BLOCK_SIZE];
uint8_t bufferOut[BLOCK_SIZE];

FILE* fileIn;
FILE* fileOut;

bool at_end(FILE* file) {
    off_t current = ftell(file);
    fseek(file, 0, SEEK_END);
    off_t end = ftell(file);
    fseek(file, current, SEEK_SET);  /* Back to where we started */
    return current == end;
}

/*
 * Refreshes input buffer, returns the number of bytes read into it
 */
size_t update_buffer() {
    return fread(bufferIn, sizeof(uint8_t), BLOCK_SIZE, fileIn);
}

/*
 * Writes bytes from output buffer to the file
 */
void flush_buffer(size_t size) {
    fwrite(bufferOut, sizeof(uint8_t), size, fileOut);
}

/*
 * Returns the next index of the output buffer
 */
size_t output_byte(uint8_t byte, size_t bufferIndex) {
    if (bufferIndex >= BLOCK_SIZE) {
        flush_buffer(bufferIndex);
        bufferIndex = 0;
    }
    bufferOut[bufferIndex] = byte;
    return bufferIndex + 1;
}

/*
 * Determines the number of unique bytes in file
 * Returns true if the file has 2+ unique bytes
 */
bool is_file_correct(FILE* file) {
    uint8_t firstByte;
    fread(&firstByte, sizeof(uint8_t), 1, fileIn);
    uint8_t secondByte = firstByte;
    while (fread(&secondByte, sizeof(uint8_t), 1, fileIn) != 0) {
        if (firstByte != secondByte) {
#ifdef DEBUG
            printf("First two bytes: %c, %c\n\n", firstByte, secondByte);
#endif
            return true;
        }
    }
    return false;
}