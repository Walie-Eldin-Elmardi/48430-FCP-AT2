/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: custom_library.c
 * Authors: Wally, Georgia
 *
 * The data structures used by Huffman compression: a min-priority queue
 * (binary heap) to build the tree, and a stack to walk it. See
 * custom_library.h for what each public function does.
 */
#include <stdio.h>  /* fprintf, vfprintf */
#include <stdarg.h> /* va_list, va_start, va_end */
#include <stdlib.h> /* malloc, free */
#include "custom_library.h"

priority_queue_t *init_priority_queue(int capacity) {
    priority_queue_t *pq;

    if (capacity <= 0) {
        return NULL;
    }

    pq = malloc(sizeof(priority_queue_t));
    if (pq == NULL) {
        return NULL;
    }

    pq->elements = malloc(capacity * sizeof(huffman_node_t *));
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
 * Author: Georgia
 */
static int higher_priority(const huffman_node_t *a, const huffman_node_t *b) {
    if (a->frequency != b->frequency) {
        return a->frequency < b->frequency;
    }
    return a->symbol < b->symbol;
}

status_t enqueue(priority_queue_t *pq, huffman_node_t *node) {
    int i;
    int parent;
    huffman_node_t *temp;

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

huffman_node_t *dequeue(priority_queue_t *pq) {
    huffman_node_t *top;
    huffman_node_t *temp;
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

        if (left < pq->size &&
            higher_priority(pq->elements[left], pq->elements[smallest])) {
            smallest = left;
        }
        if (right < pq->size &&
            higher_priority(pq->elements[right], pq->elements[smallest])) {
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

void free_priority_queue(priority_queue_t *pq) {
    if (pq == NULL) {
        return;
    }
    free(pq->elements);
    free(pq);
}

node_stack_t *init_stack(int capacity) {
    node_stack_t *stack;

    if (capacity <= 0) {
        return NULL;
    }

    stack = malloc(sizeof(node_stack_t));
    if (stack == NULL) {
        return NULL;
    }

    stack->entries = malloc(capacity * sizeof(stack_entry_t));
    if (stack->entries == NULL) {
        free(stack);
        return NULL;
    }

    stack->size = 0;
    stack->capacity = capacity;
    return stack;
}

status_t push_node(node_stack_t *stack, huffman_node_t *node, int depth,
                   char bit) {
    stack_entry_t *entry;

    if (stack == NULL || node == NULL) {
        return INVALID_ARGUMENT;
    }
    if (stack->size >= stack->capacity) {
        return MEMORY_ALLOCATION_FAILED;
    }

    entry = &stack->entries[stack->size++];
    entry->node = node;
    entry->depth = depth;
    entry->bit = bit;
    return SUCCESS;
}

int pop_node(node_stack_t *stack, stack_entry_t *entry) {
    if (stack == NULL || entry == NULL || stack->size == 0) {
        return 0;
    }
    *entry = stack->entries[--stack->size];
    return 1;
}

void free_stack(node_stack_t *stack) {
    if (stack == NULL) {
        return;
    }
    free(stack->entries);
    free(stack);
}

void count_freq(const unsigned char *data, long size,
                unsigned long freq[HUFFMAN_SYMBOLS]) {
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
 * Author: Georgia
 */
static huffman_node_t *new_node(unsigned char symbol, unsigned long frequency,
                                huffman_node_t *left, huffman_node_t *right) {
    huffman_node_t *node = malloc(sizeof(huffman_node_t));

    if (node == NULL) {
        return NULL;
    }
    node->symbol = symbol;
    node->frequency = frequency;
    node->left = left;
    node->right = right;
    return node;
}

void free_huffman_tree(huffman_node_t *root) {
    if (root == NULL) {
        return;
    }
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}

/*
 * Adds a leaf node to the queue for every byte value that appears.
 * Parameters: priority queue, frequency array
 * Returns: SUCCESS, or MEMORY_ALLOCATION_FAILED
 * Author: Georgia
 */
static status_t enqueue_leaves(priority_queue_t *pq,
                               const unsigned long freq[HUFFMAN_SYMBOLS]) {
    huffman_node_t *node;
    int i;

    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        if (freq[i] > 0) {
            node = new_node((unsigned char)i, freq[i], NULL, NULL);
            if (node == NULL) {
                return MEMORY_ALLOCATION_FAILED;
            }
            enqueue(pq, node);
        }
    }
    return SUCCESS;
}

/*
 * Repeatedly joins the two least frequent nodes in the queue until one
 * tree is left.
 * Parameters: priority queue holding the leaves
 * Returns: Root of the tree, or NULL if the queue was empty or allocation
 *          failed (nodes left in the queue are for the caller to free)
 * Author: Georgia
 */
static huffman_node_t *join_nodes(priority_queue_t *pq) {
    huffman_node_t *a;
    huffman_node_t *b;
    huffman_node_t *node;

    DEBUG_PRINT("building Huffman tree from %d distinct byte values",
                pq->size);

    if (pq->size == 0) {
        return NULL;
    }

    /* A file with only one distinct byte still needs a 1-bit code */
    if (pq->size == 1) {
        a = dequeue(pq);
        node = new_node(a->symbol, a->frequency, a, NULL);
        if (node == NULL) {
            free(a);
        }
        return node;
    }

    while (pq->size > 1) {
        a = dequeue(pq);
        b = dequeue(pq);
        node = new_node(a->symbol < b->symbol ? a->symbol : b->symbol,
                        a->frequency + b->frequency, a, b);
        if (node == NULL) {
            free_huffman_tree(a);
            free_huffman_tree(b);
            return NULL;
        }
        enqueue(pq, node);
    }
    return dequeue(pq);
}

huffman_node_t *build_huffman_tree(const unsigned long freq[HUFFMAN_SYMBOLS]) {
    priority_queue_t *pq;
    huffman_node_t *root = NULL;

    pq = init_priority_queue(HUFFMAN_SYMBOLS);
    if (pq == NULL) {
        return NULL;
    }

    if (enqueue_leaves(pq, freq) == SUCCESS) {
        root = join_nodes(pq);
    }

    /* Free anything left in the queue if building stopped part way */
    while (pq->size > 0) {
        free_huffman_tree(dequeue(pq));
    }
    free_priority_queue(pq);
    return root;
}

status_t generate_huffman_codes(huffman_node_t *root,
                                huffman_code_t codes[HUFFMAN_SYMBOLS]) {
    char path[HUFFMAN_SYMBOLS];
    node_stack_t *stack;
    stack_entry_t entry;
    huffman_node_t *node;
    status_t status = SUCCESS;
    int i;

    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        codes[i].length = 0;
    }
    if (root == NULL) {
        return SUCCESS;
    }

    /* A tree with n leaves has 2n - 1 nodes, so this is always enough */
    stack = init_stack(2 * HUFFMAN_SYMBOLS);
    if (stack == NULL) {
        return MEMORY_ALLOCATION_FAILED;
    }
    push_node(stack, root, 0, '\0');

    /* Depth-first walk. Each node writes its bit at path[depth - 1]. The
     * nodes visited between a node's parent and the node itself only
     * write at that depth or deeper, so path[0..depth-1] is always the
     * path from the root to the node being visited. */
    while (status == SUCCESS && pop_node(stack, &entry)) {
        node = entry.node;
        if (entry.depth > 0) {
            path[entry.depth - 1] = entry.bit;
        }

        if (node->left == NULL && node->right == NULL) {
            for (i = 0; i < entry.depth; i++) {
                codes[node->symbol].bits[i] = path[i];
            }
            codes[node->symbol].length = entry.depth;
        } else if (entry.depth < HUFFMAN_SYMBOLS) {
            /* Push right first so the left branch is visited first */
            if (node->right != NULL) {
                status = push_node(stack, node->right, entry.depth + 1, '1');
            }
            if (status == SUCCESS && node->left != NULL) {
                status = push_node(stack, node->left, entry.depth + 1, '0');
            }
        } else {
            /* Codes cannot be longer than the bits array */
            status = INVALID_ARGUMENT;
        }
    }

    free_stack(stack);
    return status;
}

const char *status_message(status_t status) {
    switch (status) {
        case SUCCESS: return "success";
        case FILE_NOT_FOUND: return "file could not be opened or written";
        case MEMORY_ALLOCATION_FAILED: return "out of memory";
        case INVALID_ARGUMENT: return "invalid input or file format";
        case RECORD_NOT_FOUND: return "record not found";
        case INVALID_AUTHENTICATION: return "wrong password";
    }
    return "unknown error";
}

void debug_print(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "DEBUG: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}
