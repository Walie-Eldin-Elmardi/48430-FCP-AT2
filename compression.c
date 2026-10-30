/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: compression.c
 * Authors: Wally (design), Georgia
 *
 * Huffman compression: each byte value gets a bit code whose length
 * depends on how often it appears, so common bytes take fewer bits.
 *
 * Compressed file layout:
 *   4 bytes   original file size
 *   256 x 4   frequency of each byte value (used to rebuild the tree)
 *   rest      Huffman codes packed into bytes, most significant bit first
 * All numbers are stored little-endian so the format is the same on any
 * machine.
 */
#include <stdio.h>  /* fopen, fclose, fseek, ftell, fread, fgetc, fputc,
                       ferror, remove */
#include <stdlib.h> /* malloc, free */
#include "compression.h"

#define BITS_PER_BYTE 8
/* The header holds the file size and one frequency per byte value */
#define HEADER_LENGTH (4 + 4 * HUFFMAN_SYMBOLS)

/*
 * Writes a 32-bit number to a file as 4 little-endian bytes.
 * Parameters: file, value
 * Returns: 1 on success, 0 on write error
 * Author: Georgia
 */
static int write_u32(FILE *fp, unsigned long value) {
    int i;

    for (i = 0; i < 4; i++) {
        if (fputc((int)((value >> (8 * i)) & 0xff), fp) == EOF) {
            return 0;
        }
    }
    return 1;
}

/*
 * Reads a 32-bit number stored as 4 little-endian bytes.
 * Parameters: file, pointer to store the value
 * Returns: 1 on success, 0 if the file ended early
 * Author: Georgia
 */
static int read_u32(FILE *fp, unsigned long *value) {
    int i;
    int c;

    *value = 0;
    for (i = 0; i < 4; i++) {
        c = fgetc(fp);
        if (c == EOF) {
            return 0;
        }
        *value |= (unsigned long)c << (8 * i);
    }
    return 1;
}

/*
 * Reads a whole file into memory.
 * Parameters: filename, pointer to store the buffer, pointer to store the
 *             size in bytes
 * Returns: SUCCESS, or a status code saying why it failed
 * Author: Georgia
 */
static status_t read_whole_file(const char *filename, unsigned char **data,
                                long *size) {
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        return FILE_NOT_FOUND;
    }
    if (fseek(fp, 0, SEEK_END) != 0 || (*size = ftell(fp)) < 0 ||
        fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return FILE_NOT_FOUND;
    }

    /* malloc(0) may return NULL, so always ask for at least one byte */
    *data = malloc(*size > 0 ? (size_t)*size : 1);
    if (*data == NULL) {
        fclose(fp);
        return MEMORY_ALLOCATION_FAILED;
    }
    if (fread(*data, 1, (size_t)*size, fp) != (size_t)*size) {
        free(*data);
        fclose(fp);
        return FILE_NOT_FOUND;
    }
    fclose(fp);
    return SUCCESS;
}

/*
 * Writes the data as packed Huffman codes, padding the last byte with
 * zeros.
 * Parameters: output file, data, size of data, code for each byte value
 * Returns: None (write errors are picked up with ferror by the caller)
 * Author: Georgia
 */
static void write_codes(FILE *out, const unsigned char *data, long size,
                        const huffman_code_t codes[HUFFMAN_SYMBOLS]) {
    const huffman_code_t *code;
    unsigned char current = 0;
    int bit_count = 0;
    long i;
    int j;

    for (i = 0; i < size; i++) {
        code = &codes[data[i]];
        for (j = 0; j < code->length; j++) {
            current = (unsigned char)((current << 1) | (code->bits[j] == '1'));
            if (++bit_count == BITS_PER_BYTE) {
                fputc(current, out);
                current = 0;
                bit_count = 0;
            }
        }
    }
    if (bit_count > 0) {
        fputc(current << (BITS_PER_BYTE - bit_count), out);
    }
}

status_t compress_file(const char *input_filename,
                       const char *output_filename) {
    unsigned long freq[HUFFMAN_SYMBOLS];
    huffman_code_t *codes;
    huffman_node_t *root;
    unsigned char *data;
    long size;
    long compressed_size;
    int i;
    FILE *out;
    status_t status;

    if (input_filename == NULL || output_filename == NULL) {
        return INVALID_ARGUMENT;
    }

    status = read_whole_file(input_filename, &data, &size);
    if (status != SUCCESS) {
        return status;
    }
    /* The header stores the size in 4 bytes, so larger files cannot be
     * compressed */
    if ((unsigned long)size > 0xFFFFFFFFUL) {
        free(data);
        return INVALID_ARGUMENT;
    }

    count_freq(data, size, freq);
    root = build_huffman_tree(freq);
    if (root == NULL && size > 0) {
        free(data);
        return MEMORY_ALLOCATION_FAILED;
    }

    /* 256 codes of 256 chars each is too big for the stack on some
     * systems */
    codes = malloc(HUFFMAN_SYMBOLS * sizeof(huffman_code_t));
    if (codes == NULL) {
        free_huffman_tree(root);
        free(data);
        return MEMORY_ALLOCATION_FAILED;
    }
    status = generate_huffman_codes(root, codes);
    free_huffman_tree(root);
    if (status != SUCCESS) {
        free(codes);
        free(data);
        return status;
    }

    out = fopen(output_filename, "wb");
    if (out == NULL) {
        free(codes);
        free(data);
        return FILE_NOT_FOUND;
    }

    write_u32(out, (unsigned long)size);
    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        write_u32(out, freq[i]);
    }
    write_codes(out, data, size, codes);

    compressed_size = ftell(out);
    status = ferror(out) ? FILE_NOT_FOUND : SUCCESS;
    if (fclose(out) != 0) {
        status = FILE_NOT_FOUND;
    }
    if (status != SUCCESS) {
        remove(output_filename);
    }
    free(codes);
    free(data);

    DEBUG_PRINT("compressed %s (%ld bytes) to %s (%ld bytes, %d of header)",
                input_filename, size, output_filename, compressed_size,
                HEADER_LENGTH);
    return status;
}

/*
 * Reads the header of a compressed file and checks it is consistent.
 * Parameters: file, pointer to store the original size, frequency array
 * Returns: 1 if the header is valid, otherwise 0
 * Author: Georgia
 */
static int read_header(FILE *in, unsigned long *size,
                       unsigned long freq[HUFFMAN_SYMBOLS]) {
    unsigned long total = 0;
    int i;

    if (!read_u32(in, size)) {
        return 0;
    }
    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        if (!read_u32(in, &freq[i])) {
            return 0;
        }
        total += freq[i];
    }
    /* The frequencies must add up to the file size, or this isn't our
     * format */
    return total == *size;
}

status_t decompress_file(const char *input_filename,
                         const char *output_filename) {
    unsigned long freq[HUFFMAN_SYMBOLS];
    unsigned long size;
    unsigned long written = 0;
    const huffman_node_t *node;
    huffman_node_t *root;
    FILE *in;
    FILE *out;
    int c = 0;
    int bit = -1;
    status_t status = SUCCESS;

    if (input_filename == NULL || output_filename == NULL) {
        return INVALID_ARGUMENT;
    }

    in = fopen(input_filename, "rb");
    if (in == NULL) {
        return FILE_NOT_FOUND;
    }
    if (!read_header(in, &size, freq)) {
        DEBUG_PRINT("%s does not have a valid compressed header",
                    input_filename);
        fclose(in);
        return INVALID_ARGUMENT;
    }

    root = build_huffman_tree(freq);
    if (root == NULL && size > 0) {
        fclose(in);
        return MEMORY_ALLOCATION_FAILED;
    }

    out = fopen(output_filename, "wb");
    if (out == NULL) {
        free_huffman_tree(root);
        fclose(in);
        return FILE_NOT_FOUND;
    }

    /* Follow bits down the tree from the root; each leaf is one output
     * byte */
    node = root;
    while (written < size) {
        if (bit < 0) {
            c = fgetc(in);
            if (c == EOF) {
                DEBUG_PRINT("%s ended after %lu of %lu bytes",
                            input_filename, written, size);
                status = INVALID_ARGUMENT;
                break;
            }
            bit = BITS_PER_BYTE - 1;
        }
        node = ((c >> bit) & 1) ? node->right : node->left;
        bit--;

        if (node == NULL) {
            status = INVALID_ARGUMENT;
            break;
        }
        if (node->left == NULL && node->right == NULL) {
            fputc(node->symbol, out);
            written++;
            node = root;
        }
    }

    if (status == SUCCESS && ferror(out)) {
        status = FILE_NOT_FOUND;
    }
    if (fclose(out) != 0 && status == SUCCESS) {
        status = FILE_NOT_FOUND;
    }
    fclose(in);
    free_huffman_tree(root);

    /* Don't leave a half-written file behind */
    if (status != SUCCESS) {
        remove(output_filename);
    }

    DEBUG_PRINT("decompressed %s to %s (%lu bytes)", input_filename,
                output_filename, written);
    return status;
}
