#ifndef HUFFMAN_HEADING_H
#define HUFFMAN_HEADING_H

#include <stdio.h>

#include "../../common.h"
#include "../../utils/linkedlist.h"

typedef struct {
    uint8_t ignoreBits;     /* 1 byte  - the number of additional bits to ignore in the end of arch. data */
    uint8_t signature;      /* 1 byte  - signature */
    uint8_t treeShapeSize;  /* 1 byte  - size of tree shape in bytes */
    uint8_t treeLeavesSize; /* 1 byte  - size of tree leaves in bytes MINUS ONE (e.g. if size if 256, then 255 will be written) */
    List*   treeShape;      /* X bytes - huffman tree shape */
    List*   treeLeaves;     /* Y bytes - huffman tree leaves */
    /* ... */               /* Z bytes - archived data (no field) */
} HuffmanHeading;

typedef struct HuffmanTreeNode {
    uint8_t uniqueByte;
    uint64_t weight;
    bool hasValue;

    struct HuffmanTreeNode* left;
    struct HuffmanTreeNode* right;
} HuffmanTreeNode;

void init_huffman_heading(HuffmanHeading* heading);
HuffmanTreeNode* build_huffman_tree(FILE* fileIn);
void free_huffman_tree(HuffmanTreeNode* tree);

#endif