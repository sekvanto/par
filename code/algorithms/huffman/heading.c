#include "heading.h"

void init_huffman_heading(HuffmanHeading* heading) {
    heading->ignoreBits = 0;
    heading->signature = SIG_HUFFMAN;
    heading->treeShapeSize = 0;
    heading->treeLeavesSize = 0;
    heading->treeShape = makelist();
    heading->treeLeaves = makelist();
}

HuffmanTreeNode* build_huffman_tree(FILE* fileIn) {
    return NULL;
}

void free_huffman_tree(HuffmanTreeNode* tree) {
    //
}