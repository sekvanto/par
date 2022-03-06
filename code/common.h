#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FAILURE -1
//#define DEBUG

#define UINT8_COUNT (UINT8_MAX + 1)
#define BYTE_SIZE 8

/* Signatures */
#define SIG_HUFFMAN 0x3a

#define BLOCK_SIZE 65536

extern uint8_t bufferIn[BLOCK_SIZE];
extern uint8_t bufferOut[BLOCK_SIZE];

extern FILE* fileIn;
extern FILE* fileOut;

size_t update_buffer();
void flush_buffer(size_t size);
size_t output_byte(uint8_t byte, size_t bufferIndex);

bool at_end(FILE* file);
bool is_file_correct(FILE* file);

#endif