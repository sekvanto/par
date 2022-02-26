#include <stdio.h>
#include <stdlib.h>

#include "stats.h"

const char* FSIZE_UNITS[] = {
    "B",
    "KB",
    "MB",
    "GB"
};

/*
 * Given file size in **bytes**, determines optimal format for it, generates
 * a formatted string and returns it (e.g. "51 KB" is more optimal than "0.05 МB")
 */
static char* format_file_size(long fileSize) {
    double floatFileSize = (double) fileSize;
    const int size = 100;
    char* result = malloc(size);
    int numOfUnits = sizeof(FSIZE_UNITS) / sizeof (char*);

    for (int i = 0; i < numOfUnits; i++) {
        if (floatFileSize < 1024 || i + 1 == numOfUnits) {
            i == 0 ?
                snprintf(result, size, "%ld %s", fileSize, FSIZE_UNITS[i]) : /* Bytes number will always be an integer */
                snprintf(result, size, "%.2f %s", floatFileSize, FSIZE_UNITS[i]);
            break;
        }
        floatFileSize /= 1024;
    }
    return result;
}

void output_stats(Data* data) {
    char* text = format_file_size(data->fileInSize);
    printf("Input file size:   %s\n", text);
    free(text);

    text = format_file_size(data->fileOutSize);
    printf("Output file size:  %s\n", text);
    free(text);
    
    printf("Compression ratio: %.2f%% (less percents - higher compression, 100%% - no compression)\n",
            data->efficiency);
    printf("Operation took: %.4f seconds\n", data->time);
}