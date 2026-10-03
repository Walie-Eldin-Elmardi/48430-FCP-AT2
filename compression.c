/*
 * compression.c
 *
 * 48430 Fundamentals of C Programming - Assessment Task 3 (Group Project)
 * Group: Team 31 (Thursday 4pm)
 *
 * Author: Georgia Gamble
 *
 * Run-Length Encoding (RLE) compression and decompression of files.
 * Works on raw bytes, so it can be used on any file.
 *
 * Compressed file format (planned):
 *   a series of two-byte pairs: [count][value]
 *   count is 1..255 and says how many times value repeats.
 *   A run longer than 255 is written as several pairs.
 */

#include <stdio.h>
#include "records.h"

/*
 * compress_rle
 * Reads input_filename byte by byte, groups repeated bytes into runs and
 * writes a [count][value] pair for each run to output_filename.
 * Returns SUCCESS, or FAILURE on any file error.
 */
int compress_rle(const char input_filename[], const char output_filename[])
{
    (void)input_filename;
    (void)output_filename;

    /* TODO: implement */
    return FAILURE;
}

/*
 * decompress_rle
 * Reads [count][value] pairs from input_filename and writes each value
 * count times to output_filename, restoring the original file.
 * Returns SUCCESS, or FAILURE on any file error or an invalid file
 * (for example an odd number of bytes).
 */
int decompress_rle(const char input_filename[], const char output_filename[])
{
    (void)input_filename;
    (void)output_filename;

    /* TODO: implement */
    return FAILURE;
}
