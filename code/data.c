#include "data.h"

void initData(Data* data) {
    data->fileIn = "";
    data->fileOut = "";
    data->isArchiving = false;
    data->algorithmType = ALG_HUFFMAN;

    data->efficiency = 0;
    data->time = 0;
    data->fileInSize = 0;
    data->fileOutSize = 0;
}