#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

#include "../../common.h"
#include "../../data.h"

#define BLOCK_SIZE 65536

extern uint8_t buffer[BLOCK_SIZE];

int huffman_archive(Data* data, FILE* in, FILE* out);
int huffman_unarchive(Data* data, FILE* in, FILE* out);

#endif