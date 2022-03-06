#include "adaptive_huffman.h"

static void initialize_model() {
    //
}

static void update_model(uint8_t c) {
    //
}

static void encode(uint8_t c) {
    //
}

int adaptive_huffman_archive(Data* data) {
    int c;
    initialize_model();
    while ((c = getc(fileIn)) != EOF) {
        encode(c);
        update_model(c);
    }
    return 0;
}

static int8_t decode() {
    static Sequence byte;
    return 0;
}

int adaptive_huffman_unarchive(Data* data) {
    int c;
    initialize_model();
    while((c = decode()) != EOF) {
        output_byte(c);
        update_model(c);
    }
    return 0;
}