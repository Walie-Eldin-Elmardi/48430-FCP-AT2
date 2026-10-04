#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "custom_library.h"

/* 
 * This function exists to read in a file, compress it using Huffman coding
 * and write the header to the output file, followed by the compressed data.
 * Parameters: input filename, output filename
 * Returns: This function returns a status code indicating success or failure
 * Author: Wally
 */
Status compress_file(const char *input_filename, const char *output_filename);
/* 
 * This function exists to read in a compressed file, decompress it using Huffman coding
 * and write the decompressed data to the output file.
 * Parameters: input filename, output filename
 * Returns: This function returns a status code indicating success or failure
 * Author: Wally
 */
Status decompress_file(const char *input_filename, const char *output_filename);

#endif