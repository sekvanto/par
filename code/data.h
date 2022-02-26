#ifndef DATA_H
#define DATA_H

#include "common.h"

typedef enum {
    ALG_HUFFMAN,
    ALG_ADAPTIVE_HUFFMAN
} AlgorithmType;

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