/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: custom_library.h
 * Authors: Wally, Georgia
 *
 * Shared by the whole program: status codes, the debug print macro, and
 * the data structures used by Huffman compression (a min-priority queue,
 * a stack and the Huffman tree itself).
 */
#ifndef CUSTOM_LIBRARY_H
#define CUSTOM_LIBRARY_H

/* Maximum values */
#define MAX_FILENAME_LENGTH 256
#define MAX_PATH_LENGTH 512
#define HUFFMAN_SYMBOLS 256

/*
 * DEBUG_PRINT("x = %d", x) prints a message to stderr when the program is
 * built with -DDEBUG (make debug). In a normal build it expands to
 * "1 ? (void)0 : debug_print(...)", so the call is never made. ANSI C has
 * no variadic macros, so this is how the arguments are passed through.
 */
#ifdef DEBUG
#define DEBUG_PRINT debug_print
#else
#define DEBUG_PRINT 1 ? (void)0 : debug_print
#endif

typedef enum {
    SUCCESS = 0, /* Operation succeeded */
    FILE_NOT_FOUND, /* File not found, or could not be read or written */
    MEMORY_ALLOCATION_FAILED, /* Memory allocation failed */
    INVALID_ARGUMENT, /* Invalid argument or badly formatted data */
    RECORD_NOT_FOUND, /* Record not found */
    INVALID_AUTHENTICATION /* Wrong password */
} status_t;

/* Nodes that make up the Huffman tree. Leaves hold a symbol; internal
 * nodes hold the smallest symbol below them, so ties break the same way
 * every time the tree is built. */
typedef struct huffman_node {
    unsigned char symbol;
    unsigned long frequency;
    struct huffman_node *left;
    struct huffman_node *right;
} huffman_node_t;

/* The Huffman code for one symbol, as a string of '0' and '1' chars */
typedef struct {
    char bits[HUFFMAN_SYMBOLS];
    int length;
} huffman_code_t;

/* Min-priority queue of Huffman nodes, ordered by frequency. Stored as a
 * binary heap: the children of elements[i] are elements[2i+1] and
 * elements[2i+2]. */
typedef struct {
    huffman_node_t **elements;
    int size;
    int capacity;
} priority_queue_t;

/* One entry on a node stack: a tree node, its depth in the tree, and the
 * bit ('0' or '1') on the branch leading to it */
typedef struct {
    huffman_node_t *node;
    int depth;
    char bit;
} stack_entry_t;

/* Fixed-capacity stack of tree nodes, used to walk the Huffman tree */
typedef struct {
    stack_entry_t *entries;
    int size;
    int capacity;
} node_stack_t;

/*
 * Creates an empty priority queue.
 * Parameters: maximum number of nodes the queue can hold
 * Returns: Pointer to the new queue, or NULL if allocation failed
 * Author: Wally (implemented by Georgia)
 */
priority_queue_t *init_priority_queue(int capacity);
/*
 * Adds a node to the priority queue.
 * Parameters: priority queue, node to add
 * Returns: SUCCESS, INVALID_ARGUMENT, or MEMORY_ALLOCATION_FAILED if full
 * Author: Wally (implemented by Georgia)
 */
status_t enqueue(priority_queue_t *pq, huffman_node_t *node);
/*
 * Removes the node with the lowest frequency from the priority queue.
 * Parameters: priority queue
 * Returns: The removed node, or NULL if the queue is empty
 * Author: Wally (implemented by Georgia)
 */
huffman_node_t *dequeue(priority_queue_t *pq);
/*
 * Frees a priority queue (but not the nodes still in it).
 * Parameters: priority queue (may be NULL)
 * Returns: None
 * Author: Georgia
 */
void free_priority_queue(priority_queue_t *pq);

/*
 * Creates an empty node stack.
 * Parameters: maximum number of entries the stack can hold
 * Returns: Pointer to the new stack, or NULL if allocation failed
 * Author: Georgia
 */
node_stack_t *init_stack(int capacity);
/*
 * Pushes a tree node onto the stack.
 * Parameters: stack, node, depth of the node, bit on the branch to it
 * Returns: SUCCESS, INVALID_ARGUMENT, or MEMORY_ALLOCATION_FAILED if full
 * Author: Georgia
 */
status_t push_node(node_stack_t *stack, huffman_node_t *node, int depth,
                   char bit);
/*
 * Pops the most recently pushed entry off the stack.
 * Parameters: stack, entry to fill in
 * Returns: 1 if an entry was popped, 0 if the stack was empty
 * Author: Georgia
 */
int pop_node(node_stack_t *stack, stack_entry_t *entry);
/*
 * Frees a node stack (but not the tree nodes it points to).
 * Parameters: stack (may be NULL)
 * Returns: None
 * Author: Georgia
 */
void free_stack(node_stack_t *stack);

/*
 * Counts how often each byte value appears in the data.
 * Parameters: data, size of data in bytes, frequency array to fill in
 * Returns: None
 * Author: Wally (implemented by Georgia)
 */
void count_freq(const unsigned char *data, long size,
                unsigned long freq[HUFFMAN_SYMBOLS]);
/*
 * Builds the Huffman tree for the given byte frequencies.
 * Parameters: frequency array
 * Returns: Root of the tree, or NULL if every frequency is 0 or
 *          allocation failed
 * Author: Wally (implemented by Georgia)
 */
huffman_node_t *build_huffman_tree(const unsigned long freq[HUFFMAN_SYMBOLS]);
/*
 * Works out the Huffman code for every symbol in the tree, by walking the
 * tree depth first with a node stack. Symbols not in the tree get a code
 * of length 0.
 * Parameters: root of the Huffman tree (may be NULL), codes array to fill
 * Returns: SUCCESS, or MEMORY_ALLOCATION_FAILED
 * Author: Wally (implemented by Georgia)
 */
status_t generate_huffman_codes(huffman_node_t *root,
                                huffman_code_t codes[HUFFMAN_SYMBOLS]);
/*
 * Frees every node in a Huffman tree.
 * Parameters: root of the Huffman tree (may be NULL)
 * Returns: None
 * Author: Georgia
 */
void free_huffman_tree(huffman_node_t *root);

/*
 * Describes a status code, for error messages.
 * Parameters: status
 * Returns: Short description of the status
 * Author: Georgia
 */
const char *status_message(status_t status);

/*
 * Prints a printf-style message to stderr, starting with "DEBUG: ".
 * Call it through DEBUG_PRINT, so it is only called in debug builds.
 * Parameters: format string, followed by its arguments
 * Returns: None
 * Author: Wally (updated by Georgia)
 */
void debug_print(const char *fmt, ...);

#endif /* CUSTOM_LIBRARY_H */
