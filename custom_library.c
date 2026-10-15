#include <stdarg.h>
#include <stdlib.h>
#include "custom_library.h"

PriorityQueue *init_priority_queue(int capacity) {
    PriorityQueue *pq;

    if (capacity <= 0) {
        return NULL;
    }

    pq = malloc(sizeof(PriorityQueue));
    if (pq == NULL) {
        return NULL;
    }

    pq->elements = malloc(capacity * sizeof(HuffmanNode *));
    if (pq->elements == NULL) {
        free(pq);
        return NULL;
    }

    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

/*
 * Returns 1 if node a should come out of the queue before node b.
 * Lower frequency comes first; ties are broken by symbol so the tree is
 * always built the same way for the same frequencies.
 * Parameters: two nodes to compare
 * Returns: 1 if a has higher priority than b, otherwise 0
 * Author: Wally
 */
static int higher_priority(const HuffmanNode *a, const HuffmanNode *b) {
    if (a->frequency != b->frequency) {
        return a->frequency < b->frequency;
    }
    return a->symbol < b->symbol;
}

Status enqueue(PriorityQueue *pq, HuffmanNode *node) {
    int i;
    int parent;
    HuffmanNode *temp;

    if (pq == NULL || node == NULL) {
        return INVALID_ARGUMENT;
    }
    if (pq->size >= pq->capacity) {
        return MEMORY_ALLOCATION_FAILED;
    }

    /* Add at the bottom of the heap, then move up while smaller than parent */
    i = pq->size++;
    pq->elements[i] = node;
    while (i > 0) {
        parent = (i - 1) / 2;
        if (!higher_priority(pq->elements[i], pq->elements[parent])) {
            break;
        }
        temp = pq->elements[i];
        pq->elements[i] = pq->elements[parent];
        pq->elements[parent] = temp;
        i = parent;
    }
    return SUCCESS;
}

HuffmanNode *dequeue(PriorityQueue *pq) {
    HuffmanNode *top;
    HuffmanNode *temp;
    int i = 0;
    int left;
    int right;
    int smallest;

    if (pq == NULL || pq->size == 0) {
        return NULL;
    }

    /* Take the root, move the last node to the root, then move it down */
    top = pq->elements[0];
    pq->elements[0] = pq->elements[--pq->size];

    while (1) {
        left = 2 * i + 1;
        right = 2 * i + 2;
        smallest = i;

        if (left < pq->size && higher_priority(pq->elements[left], pq->elements[smallest])) {
            smallest = left;
        }
        if (right < pq->size && higher_priority(pq->elements[right], pq->elements[smallest])) {
            smallest = right;
        }
        if (smallest == i) {
            break;
        }
        temp = pq->elements[i];
        pq->elements[i] = pq->elements[smallest];
        pq->elements[smallest] = temp;
        i = smallest;
    }
    return top;
}

void count_freq(const unsigned char *data, long size, unsigned long freq[HUFFMAN_SYMBOLS]) {
    long i;

    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        freq[i] = 0;
    }
    for (i = 0; i < size; i++) {
        freq[data[i]]++;
    }
}

/*
 * Allocates a Huffman tree node.
 * Parameters: symbol, frequency, left and right children
 * Returns: Pointer to the new node, or NULL if allocation failed
 * Author: Wally
 */
static HuffmanNode *new_node(unsigned char symbol, unsigned long frequency,
                             HuffmanNode *left, HuffmanNode *right) {
    HuffmanNode *node = malloc(sizeof(HuffmanNode));

    if (node == NULL) {
        return NULL;
    }
    node->symbol = symbol;
    node->frequency = frequency;
    node->left = left;
    node->right = right;
    return node;
}

void free_huffman_tree(HuffmanNode *root) {
    if (root == NULL) {
        return;
    }
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}

HuffmanNode *build_huffman_tree(const unsigned long freq[HUFFMAN_SYMBOLS]) {
    PriorityQueue *pq;
    HuffmanNode *a;
    HuffmanNode *b;
    HuffmanNode *node;
    HuffmanNode *root = NULL;
    int i;

    pq = init_priority_queue(HUFFMAN_SYMBOLS);
    if (pq == NULL) {
        return NULL;
    }

    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        if (freq[i] > 0) {
            node = new_node((unsigned char)i, freq[i], NULL, NULL);
            if (node == NULL) {
                goto cleanup;
            }
            enqueue(pq, node);
        }
    }

    /* A file with only one distinct byte still needs a 1-bit code */
    if (pq->size == 1) {
        a = dequeue(pq);
        root = new_node(a->symbol, a->frequency, a, NULL);
        if (root == NULL) {
            free(a);
        }
        goto cleanup;
    }

    /* Repeatedly join the two least frequent nodes until one tree is left.
     * The joined node takes the smaller symbol so ties break consistently. */
    while (pq->size > 1) {
        a = dequeue(pq);
        b = dequeue(pq);
        node = new_node(a->symbol < b->symbol ? a->symbol : b->symbol,
                        a->frequency + b->frequency, a, b);
        if (node == NULL) {
            free_huffman_tree(a);
            free_huffman_tree(b);
            goto cleanup;
        }
        enqueue(pq, node);
    }
    root = dequeue(pq);

cleanup:
    while (pq->size > 0) {
        free_huffman_tree(dequeue(pq));
    }
    free(pq->elements);
    free(pq);
    return root;
}

/*
 * Walks the tree, recording the path to each leaf as its code.
 * Parameters: current node, path so far, length of path, codes array
 * Returns: None
 * Author: Wally
 */
static void assign_codes(const HuffmanNode *node, char *path, int depth,
                         HuffmanCode codes[HUFFMAN_SYMBOLS]) {
    int i;

    if (node == NULL) {
        return;
    }
    if (node->left == NULL && node->right == NULL) {
        for (i = 0; i < depth; i++) {
            codes[node->symbol].bits[i] = path[i];
        }
        codes[node->symbol].length = depth;
        return;
    }
    if (depth >= HUFFMAN_SYMBOLS) {
        return;
    }
    path[depth] = '0';
    assign_codes(node->left, path, depth + 1, codes);
    path[depth] = '1';
    assign_codes(node->right, path, depth + 1, codes);
}

void generate_huffman_codes(HuffmanNode *root, HuffmanCode codes[HUFFMAN_SYMBOLS]) {
    char path[HUFFMAN_SYMBOLS];
    int i;

    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        codes[i].length = 0;
    }
    assign_codes(root, path, 0, codes);
}

void debug_print(const char *fmt, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "DEBUG: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
#else
    (void)fmt; /* No debug output in release mode */
#endif
}
