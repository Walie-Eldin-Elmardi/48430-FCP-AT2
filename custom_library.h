#ifndef CUSTOM_LIBRARY_H
#define CUSTOM_LIBRARY_H

#include <stdio.h>

/* Maximum values */
#define MAX_FILENAME_LENGTH 256
#define MAX_PATH_LENGTH 512
#define HUFFMAN_SYMBOLS 256

typedef enum {
    SUCCESS = 0, 
    FILE_NOT_FOUND, /* File not found */
    MEMORY_ALLOCATION_FAILED, /* Memory allocation failed */
    INVALID_ARGUMENT, /* Invalid argument provided */
    RECORD_NOT_FOUND, /* Record not found */
    INVALID_AUTHENTICATION /* Invalid authentication */
} Status;

/* Nodes that make up the Huffman tree. 
 * Each node contains a symbol, its frequency, and pointers to its left and right children.
 * Leaves hold a symbol, however, internal nodes don't
 */
typedef struct HuffmanNode {
    unsigned char symbol;
    unsigned long frequency;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;

/* Represents a Huffman code for a symbol */
typedef struct {
    char bits[HUFFMAN_SYMBOLS];
    int length;
} HuffmanCode;

/* Min-priority queue for managing Huffman nodes, ordered by frequency */
typedef struct {
    HuffmanNode **elements;
    int size;
    int capacity;
} PriorityQueue;

/* 
 * Initializes a priority queue with the given capacity.
 * Parameters: maximum capacity of the priority queue
 * Returns: Pointer to the initialized PriorityQueue
 * Author: Wally
 */
PriorityQueue *init_priority_queue(int capacity);
/* 
 * Adds a node to the priority queue.
 * Parameters: priority queue pointer, node to add
 * Returns: Status indicating success or failure
 * Author: Wally
 */
Status enqueue(PriorityQueue *pq, HuffmanNode *node);
/* 
 * Removes the highest-priority node from the priority queue.
 * Parameters: priority queue pointer
 * Returns: Pointer to the removed node
 * Author: Wally
 */
HuffmanNode *dequeue(PriorityQueue *pq);
/* 
 * Counts the frequency of each symbol in the input data.
 * Parameters: data pointer, size of data, frequency array
 * Returns: None
 * Author: Wally
 */
void count_freq(const unsigned char *data, long size, unsigned long freq[HUFFMAN_SYMBOLS]);
/* 
 * Builds the Huffman tree based on symbol frequencies.
 * Parameters: frequency array
 * Returns: Pointer to the root of the Huffman tree
 * Author: Wally
 */
HuffmanNode *build_huffman_tree(const unsigned long freq[HUFFMAN_SYMBOLS]);
/* 
 * Generates Huffman codes for each symbol in the tree.
 * Parameters: root of the Huffman tree, array to store Huffman codes
 * Returns: None
 * Author: Wally
 */
void generate_huffman_codes(HuffmanNode *root, HuffmanCode codes[HUFFMAN_SYMBOLS]);

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) // No debug output in release mode
#endif

#endif // CUSTOM_LIBRARY_H