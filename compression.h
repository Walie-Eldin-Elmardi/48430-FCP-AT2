/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: compression.h
 * Authors: Wally (design), Georgia
 *
 * Compresses and decompresses whole files with Huffman coding.
 */
#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "custom_library.h"

/*
 * Reads a file, compresses it with Huffman coding, and writes a header
 * followed by the compressed data to the output file. If compressing
 * fails, no output file is left behind.
 * Parameters: input filename, output filename
 * Returns: SUCCESS, or a status code saying why it failed
 * Author: Wally (implemented by Georgia)
 */
status_t compress_file(const char *input_filename,
                       const char *output_filename);
/*
 * Reads a file written by compress_file, decompresses it, and writes the
 * original data to the output file. If decompressing fails, no output
 * file is left behind.
 * Parameters: input filename, output filename
 * Returns: SUCCESS, INVALID_ARGUMENT if the input is not a compressed
 *          file, or another status code saying why it failed
 * Author: Wally (implemented by Georgia)
 */
status_t decompress_file(const char *input_filename,
                         const char *output_filename);

#endif /* COMPRESSION_H */
