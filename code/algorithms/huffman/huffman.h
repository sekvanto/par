#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

#include "../../common.h"
#include "../../data.h"

int huffman_archive(Data* data, FILE* in, FILE* out);
int huffman_unarchive(Data* data, FILE* in, FILE* out);

#endif