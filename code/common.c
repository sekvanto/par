#include <stdio.h>
#include <stdlib.h>

#include "common.h"

Sequence outByte = {0};

uint8_t bufferIn[BLOCK_SIZE];
uint8_t bufferOut[BLOCK_SIZE];
size_t  bufferIndexIn = 0;
size_t  bufferIndexOut = 0;

FILE* fileIn;
FILE* fileOut;

size_t update_buffer() {
    return fread(bufferIn, sizeof(uint8_t), BLOCK_SIZE, fileIn);
}

void flush_buffer() {
    fwrite(bufferOut, sizeof(uint8_t), bufferIndexOut, fileOut);
    bufferIndexOut = 0;
}

/*
 * Generates and outputs the bytes from the incomplete byte
 * until its size becomes less than 8 bits
 */
static void reduce_seq() {
    if (outByte.size < BYTE_SIZE) {
        return;
    }
    while (outByte.size >= BYTE_SIZE) {
        if (outByte.size == BYTE_SIZE) {
            output_byte(outByte.value);
            outByte.size = 0;
            outByte.value = 0;
            return;
        }
        /* Constructing the byte */
        uint8_t byte = outByte.value >> (outByte.size - BYTE_SIZE);
        output_byte(byte);
        outByte.size -= BYTE_SIZE;
        outByte.value &= ((1 << outByte.size) - 1); /* Nullify unused bits */
    }
}

/*
 * Flush last incomplete byte and return the number of
 * extra bits added to the end of file
 */
int flush_incomplete_byte() {
    reduce_seq();
    flush_buffer();

    if (outByte.size == 0) {
        return 0;
    }

    /* Processing the rest of the sequence */
    uint8_t extraBits = (BYTE_SIZE - outByte.size);
    uint8_t byte = outByte.value << extraBits;
    fputc(byte, fileOut);
    outByte.size = 0;
    outByte.value = 0;
    return extraBits;
}

void output_byte(uint8_t byte) {
    if (bufferIndexOut >= BLOCK_SIZE) {
        flush_buffer(bufferIndexOut);
    }
    bufferOut[bufferIndexOut] = byte;
    bufferIndexOut++;
}

void output_bit_sequence(Sequence seq) {
    /* Zero out garbage bits */
    uint32_t mask = (1 << seq.size) - 1;
    seq.value &= mask;

    /* If current sequence is already longer than 8 bits */
    reduce_seq();

    /* It's possible to output a full byte */
    if (seq.size + outByte.size >= BYTE_SIZE) {
        /* Generating the byte to output */
        uint8_t byte = outByte.value << (BYTE_SIZE - outByte.size); /* First half */
        int bitsLeft = BYTE_SIZE - outByte.size; /* Bits left to complete the byte */
        byte |= (seq.value >> (seq.size - bitsLeft)); /* Second half */
        output_byte(byte);

        /* Updating outByte */
        outByte.size = seq.size - bitsLeft;
        outByte.value = seq.value &= ((1 << outByte.size) - 1); /* Nullify unused bits */
        return;
    }

    /* Can't output a byte yet, not enough bits accumulated */
    outByte.value <<= seq.size;
    outByte.value |= seq.value;
    outByte.size += seq.size;
}

bool at_end(FILE* file) {
    off_t current = ftell(file);
    fseek(file, 0, SEEK_END);
    off_t end = ftell(file);
    fseek(file, current, SEEK_SET);  /* Back to where we started */
    return current == end;
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