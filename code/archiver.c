#include <stdio.h>
#include <stdlib.h>

#include "archiver.h"
#include "algorithms/huffman/huffman.h"

typedef int (*ArchiveFn)(Data* data, FILE* in, FILE* out);

typedef struct {
    ArchiveFn archiveFunction;
    ArchiveFn unarchiveFunction;
} Operations;

FILE* fileIn;
FILE* fileOut;

int archiveError(const char* message) {
    printf("Archive error: %s\n", message);
}

static int init(Data* data) {
    fileIn = fopen(data->fileIn, "rb");
    fileOut = fopen(data->fileOut, "wb");
    if (fileIn == NULL || fileOut == NULL) {
        archiveError("can't open file(-s).");
        return FAILURE;
    }
    return 0;
}

static void post(Data* data) {
    fclose(fileIn);
    fclose(fileOut);
}

Operations operations[] = {
    [ALG_HUFFMAN]          = {huffman_archive, NULL},
    [ALG_ADAPTIVE_HUFFMAN] = {NULL,            NULL},
};

int archive(Data* data) {
    if (init(data) != 0) {
        return FAILURE;
    }

    int success = operations[data->algorithmType].archiveFunction(data, fileIn, fileOut);

    post(data);
    return success;
}

int unarchive(Data* data) {
    if (init(data) != 0) {
        return FAILURE;
    }

    int success = operations[data->algorithmType].unarchiveFunction(data, fileIn, fileOut);

    post(data);
    return success;
}