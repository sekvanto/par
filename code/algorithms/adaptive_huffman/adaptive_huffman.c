#include "adaptive_huffman.h"

/* Not yet transmitted */
#define NYT 0

typedef struct TreeNode {
    uint16_t uniqueByte; /* Range: 0-256 */
    uint16_t number;
    uint64_t weight;
    bool hasValue;

    struct TreeNode* parent;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

static TreeNode* tree;
static TreeNode* map[UINT8_COUNT + 1] = {NULL}; /* When encounters a new symbol, adds its pointer here */

static void set_node(TreeNode* node, uint16_t uniqueByte, uint16_t number, uint64_t weight,
                     bool hasValue, TreeNode* parent, TreeNode* left, TreeNode* right) {
    if (node == NULL) {
        return;
    }
    node->uniqueByte = uniqueByte;
    node->number = number;
    node->weight = weight;
    node->hasValue = hasValue;
    node->parent = parent;
    node->left = left;
    node->right = right;
}

static void initialize_model() {
    tree = malloc(sizeof(TreeNode));
    set_node(tree, NYT, ((UINT8_COUNT * 2) + 1), 0, true, NULL, NULL, NULL);
    map[NYT] = tree;
}

static void free_huffman_tree(TreeNode* tree) {
    if (!tree) {
        return;
    }
    if (!tree->left && !tree->right) { /* Tree leaf */
        free(tree);
        return;
    }
    /* Otherwise, it's a parent node */
    free_huffman_tree(tree->left);
    free_huffman_tree(tree->right);
}

static void init_symbol(uint8_t c) {
    TreeNode* nyt = malloc(sizeof(TreeNode));
    TreeNode* external = malloc(sizeof(TreeNode));
    TreeNode* oldNyt = map[NYT];

    set_node(nyt, NYT, (oldNyt->number - 2), 0, true, oldNyt, NULL, NULL);
    set_node(external, c, (oldNyt->number - 1), 1, true, oldNyt, NULL, NULL);

    oldNyt->weight++;
    oldNyt->hasValue = false;
    oldNyt->left = nyt;
    oldNyt->right = external;

    map[NYT] = nyt;
    map[c+1] = external;
}

static void update_model(uint8_t c) {
    TreeNode* current;
    if (map[c+1] == NULL) {
        current = map[NYT];
        init_symbol(c);
        if (current->parent == NULL) {
            return;
        }
        current = current->parent;
    } else {
        current = map[c+1];
    }
    while (current->parent != NULL) {
        /* Node number max in block? */
        if (current->number < current->parent->right->number &&
            current->weight == current->parent->right->weight) {
            /* Swapping */
            current->parent->left = current->parent->right;
            current->parent->right = current;
            /* Preserve number */
            uint16_t temp = current->parent->left->number;
            current->parent->left->number = current->parent->right->number;
            current->parent->right->number = temp;
        }
        current->weight++;
        current = current->parent;
    }
    current->weight++; /* Root node */
}

/*
 * Takes an already existing byte, converts
 * it into a replace sequence
 */
static Sequence replace_byte(uint8_t c, Sequence seq, TreeNode* tree) {
    if (tree->hasValue) {
        if (tree->uniqueByte == c) {
            return seq;
        }
        seq.value = 0;
        seq.size = 0;
        return seq;
    }
    seq.size++;
    seq.value <<= 1;
    Sequence furtherSeq = replace_byte(c, seq, tree->left);
    if (furtherSeq.size != 0) {
        return furtherSeq;
    }
    seq.value |= 1;
    return replace_byte(c, seq, tree->right);
}

static void encode(uint8_t c) {
    if (map[c+1] != NULL) {
        Sequence seq = {0, 0};
        seq = replace_byte(c + 1, seq, tree);
        output_bit_sequence(seq);
        return;
    }
    Sequence nyt = {0, 0};
    nyt = replace_byte(NYT, nyt, tree);
    output_bit_sequence(nyt);
    Sequence code = {c, BYTE_SIZE};
    output_bit_sequence(code);
}

int adaptive_huffman_archive(Data* data) {
    int c;
    initialize_model();
    while ((c = fgetc(fileIn)) != EOF) {
        encode(c);
        update_model(c);
    }
    flush_incomplete_bytes();
    free_huffman_tree(tree);
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
    flush_incomplete_bytes();
    free_huffman_tree(tree);
    return 0;
}