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
#define SIG_HUFFMAN          0x3a
#define SIG_ADAPTIVE_HUFFMAN 0x3b

#define BLOCK_SIZE 65536

/**
 * This structure represents a bit sequence.
 * It's very useful for the algorithms, where codewords are
 * shorter or longer than 8 bits.
 * It's basically the same as **int** but with an additional size field
 * (for knowing which bits of the value field are included into the sequence)
 * (e.g. val = "0b00000000000000001", size = "3" => sequence = "001")
 */
typedef struct {
    uint32_t value; /* Value of byte sequence */
    size_t size;    /* Size of value in bits */
} Sequence;


extern Sequence outByte; /* Incomplete output byte */

extern uint8_t bufferIn[BLOCK_SIZE];
extern uint8_t bufferOut[BLOCK_SIZE];
extern size_t  bufferIndexOut;
extern size_t bufferIndexIn;

extern FILE* fileIn;
extern FILE* fileOut;

size_t update_buffer();
void flush_buffer();
int flush_incomplete_bytes();
void output_byte(uint8_t byte);
void output_bit_sequence(Sequence seq);

bool at_end(FILE* file);
bool is_file_correct(FILE* file);

#endif