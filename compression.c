#include <stdio.h>
#include <stdlib.h>
#include "compression.h"

/*
 * Compressed file layout:
 *   4 bytes   original file size
 *   256 x 4   frequency of each byte value (used to rebuild the tree)
 *   rest      Huffman codes packed into bytes, most significant bit first
 * All numbers are stored little-endian so the format is the same on any machine.
 */

/*
 * Writes a 32-bit number to a file as 4 little-endian bytes.
 * Parameters: file, value
 * Returns: 1 on success, 0 on write error
 * Author: Wally
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
 * Author: Wally
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
 * Parameters: filename, pointer to store the buffer, pointer to store the size
 * Returns: Status code indicating success or failure
 * Author: Wally
 */
static Status read_whole_file(const char *filename, unsigned char **data, long *size) {
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        return FILE_NOT_FOUND;
    }
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

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

Status compress_file(const char *input_filename, const char *output_filename) {
    unsigned long freq[HUFFMAN_SYMBOLS];
    HuffmanCode *codes;
    HuffmanNode *root;
    unsigned char *data;
    unsigned char current = 0;
    int bit_count = 0;
    long size;
    long i;
    int j;
    FILE *out;
    Status status;

    if (input_filename == NULL || output_filename == NULL) {
        return INVALID_ARGUMENT;
    }

    status = read_whole_file(input_filename, &data, &size);
    if (status != SUCCESS) {
        return status;
    }

    count_freq(data, size, freq);
    root = build_huffman_tree(freq);
    if (root == NULL && size > 0) {
        free(data);
        return MEMORY_ALLOCATION_FAILED;
    }

    /* 256 codes of 256 chars each is too big for the stack on some systems */
    codes = malloc(HUFFMAN_SYMBOLS * sizeof(HuffmanCode));
    if (codes == NULL) {
        free_huffman_tree(root);
        free(data);
        return MEMORY_ALLOCATION_FAILED;
    }
    generate_huffman_codes(root, codes);

    out = fopen(output_filename, "wb");
    if (out == NULL) {
        free(codes);
        free_huffman_tree(root);
        free(data);
        return FILE_NOT_FOUND;
    }

    write_u32(out, (unsigned long)size);
    for (j = 0; j < HUFFMAN_SYMBOLS; j++) {
        write_u32(out, freq[j]);
    }

    for (i = 0; i < size; i++) {
        const HuffmanCode *code = &codes[data[i]];

        for (j = 0; j < code->length; j++) {
            current = (unsigned char)((current << 1) | (code->bits[j] == '1'));
            if (++bit_count == 8) {
                fputc(current, out);
                current = 0;
                bit_count = 0;
            }
        }
    }
    /* Pad the last byte with zeros */
    if (bit_count > 0) {
        fputc(current << (8 - bit_count), out);
    }

    status = ferror(out) ? FILE_NOT_FOUND : SUCCESS;
    fclose(out);
    free(codes);
    free_huffman_tree(root);
    free(data);

    DEBUG_PRINT("compressed %s (%ld bytes) to %s", input_filename, size, output_filename);
    return status;
}

Status decompress_file(const char *input_filename, const char *output_filename) {
    unsigned long freq[HUFFMAN_SYMBOLS];
    unsigned long size;
    unsigned long total = 0;
    unsigned long written = 0;
    const HuffmanNode *node;
    HuffmanNode *root;
    FILE *in;
    FILE *out;
    int c = 0;
    int bit = -1;
    int i;
    Status status = SUCCESS;

    if (input_filename == NULL || output_filename == NULL) {
        return INVALID_ARGUMENT;
    }

    in = fopen(input_filename, "rb");
    if (in == NULL) {
        return FILE_NOT_FOUND;
    }

    if (!read_u32(in, &size)) {
        fclose(in);
        return INVALID_ARGUMENT;
    }
    for (i = 0; i < HUFFMAN_SYMBOLS; i++) {
        if (!read_u32(in, &freq[i])) {
            fclose(in);
            return INVALID_ARGUMENT;
        }
        total += freq[i];
    }
    /* The frequencies must add up to the file size, or this isn't our format */
    if (total != size) {
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

    /* Follow bits down the tree from the root; each leaf is one output byte */
    node = root;
    while (written < size) {
        if (bit < 0) {
            c = fgetc(in);
            if (c == EOF) {
                status = INVALID_ARGUMENT;
                break;
            }
            bit = 7;
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
    fclose(out);
    fclose(in);
    free_huffman_tree(root);

    DEBUG_PRINT("decompressed %s to %s (%lu bytes)", input_filename, output_filename, written);
    return status;
}
