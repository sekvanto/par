#ifndef DATA_H
#define DATA_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"

typedef enum {
    ALG_HUFFMAN,
    ALG_ADAPTIVE_HUFFMAN
} AlgorithmType;

const static struct {
    AlgorithmType val;
    const char    *str;
} conversion [] = {
    {ALG_HUFFMAN,          "huffman"},
    {ALG_ADAPTIVE_HUFFMAN, "adaptive-huffman"},
};

void dataError(const char* message);
AlgorithmType str_to_algorithm_type (const char *str);

typedef struct {
    char* fileIn;
    char* fileOut;
    bool isArchiving;
    AlgorithmType algorithmType;

    double efficiency; /* File compression/decompression ratio (in percents, less is better) */
    double time;       /* How much time operation took (in seconds) */
    long fileInSize;   /* Size of input file (in bytes) */
    long fileOutSize;  /* Size of output file (in bytes) */
} Data;

void initData(Data* data);

#endif