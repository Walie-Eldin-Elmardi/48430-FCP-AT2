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
 * Compressed file format:
 *   a series of two-byte pairs: [count][value]
 *   count is 1..255 and says how many times value repeats.
 *   A run longer than 255 is written as several pairs.
 *
 * Example: "AAAABBC" is stored as 4,'A', 2,'B', 1,'C' (6 bytes for 7).
 * Note: data with few repeated bytes gets BIGGER (up to twice the size),
 * so RLE suits repetitive data such as padded fields or bitmap images.
 *
 * Only stdio.h and string.h are used. No messages are printed except
 * debug output when compiled with -DDEBUG.
 */

#include <stdio.h>
#include <string.h>
#include "records.h"

#define RLE_MAX_RUN 255

/*
 * write_pair
 * Writes one [count][value] pair to out.
 * Returns SUCCESS, or FAILURE if either byte could not be written.
 */
static int write_pair(FILE *out, int count, int value)
{
    if (fputc(count, out) == EOF)
    {
        return FAILURE;
    }
    if (fputc(value, out) == EOF)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/*
 * compress_rle
 * Reads input_filename byte by byte, groups repeated bytes into runs and
 * writes a [count][value] pair for each run to output_filename.
 * Returns SUCCESS, or FAILURE on any file error. If the output file was
 * created but the job failed, the partial output file is removed.
 * The input and output must be different files.
 */
int compress_rle(const char input_filename[], const char output_filename[])
{
    FILE *in;
    FILE *out;
    int current;
    int previous = 0;
    int run_length = 0;
    int status = SUCCESS;
    unsigned long bytes_in = 0;
    unsigned long bytes_out = 0;

    if (strcmp(input_filename, output_filename) == 0)
    {
        return FAILURE;
    }

    in = fopen(input_filename, "rb");
    if (in == NULL)
    {
        return FAILURE;
    }
    out = fopen(output_filename, "wb");
    if (out == NULL)
    {
        fclose(in);
        return FAILURE;
    }

    while (status == SUCCESS && (current = fgetc(in)) != EOF)
    {
        bytes_in++;
        if (run_length > 0 && current == previous && run_length < RLE_MAX_RUN)
        {
            run_length++;
        }
        else
        {
            if (run_length > 0)
            {
                status = write_pair(out, run_length, previous);
                bytes_out += 2;
            }
            previous = current;
            run_length = 1;
        }
    }

    if (status == SUCCESS && ferror(in))
    {
        status = FAILURE;
    }
    if (status == SUCCESS && run_length > 0)
    {
        status = write_pair(out, run_length, previous);
        bytes_out += 2;
    }
    if (fclose(out) != 0)
    {
        status = FAILURE;
    }
    fclose(in);

    if (status == FAILURE)
    {
        remove(output_filename);
    }

    DEBUG_PRINT("compress_rle: %s -> %s, %lu bytes in, %lu bytes out\n",
                input_filename, output_filename, bytes_in, bytes_out);
    (void)bytes_in;   /* only used by DEBUG_PRINT */
    (void)bytes_out;

    return status;
}

/*
 * decompress_rle
 * Reads [count][value] pairs from input_filename and writes each value
 * count times to output_filename, restoring the original file.
 * Returns SUCCESS, or FAILURE on any file error or an invalid file
 * (an odd number of bytes, or a pair with a count of 0). If the output
 * file was created but the job failed, the partial output is removed.
 * The input and output must be different files.
 */
int decompress_rle(const char input_filename[], const char output_filename[])
{
    FILE *in;
    FILE *out;
    int count;
    int value;
    int i;
    int status = SUCCESS;
    unsigned long bytes_out = 0;

    if (strcmp(input_filename, output_filename) == 0)
    {
        return FAILURE;
    }

    in = fopen(input_filename, "rb");
    if (in == NULL)
    {
        return FAILURE;
    }
    out = fopen(output_filename, "wb");
    if (out == NULL)
    {
        fclose(in);
        return FAILURE;
    }

    while (status == SUCCESS && (count = fgetc(in)) != EOF)
    {
        value = fgetc(in);
        if (value == EOF || count == 0)
        {
            status = FAILURE;   /* odd byte count, or an impossible run */
        }
        else
        {
            for (i = 0; i < count && status == SUCCESS; i++)
            {
                if (fputc(value, out) == EOF)
                {
                    status = FAILURE;
                }
                bytes_out++;
            }
        }
    }

    if (status == SUCCESS && ferror(in))
    {
        status = FAILURE;
    }
    if (fclose(out) != 0)
    {
        status = FAILURE;
    }
    fclose(in);

    if (status == FAILURE)
    {
        remove(output_filename);
    }

    DEBUG_PRINT("decompress_rle: %s -> %s, %lu bytes out\n",
                input_filename, output_filename, bytes_out);
    (void)bytes_out;   /* only used by DEBUG_PRINT */

    return status;
}
