/*
 * records.h
 *
 * 48430 Fundamentals of C Programming - Assessment Task 3 (Group Project)
 * Group: Team 31 (Thursday 4pm)
 * Project: Accountant-client records sharing platform
 *
 * Shared header: common constants, the record structure and the prototypes
 * for every module of the program. Do not change anything in this file
 * without telling the group first.
 *
 * Rules agreed for all modules:
 *  - Only stdio.h, stdlib.h, string.h and math.h may be used.
 *  - Functions that can fail return SUCCESS (0) or FAILURE (1).
 *  - Library functions do not print menus or messages; main.c decides what
 *    to show the user.
 *
 * DRAFT: names, fields and owners are proposals until the group agrees.
 */

#ifndef RECORDS_H
#define RECORDS_H

#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Constants                                                           */
/* ------------------------------------------------------------------ */

#define SUCCESS 0
#define FAILURE 1

#define MAX_RECORDS   100
#define MAX_NAME_LEN  50
#define MAX_TYPE_LEN  20
#define MAX_KEY_LEN   32

/* Debug mode: compile with -DDEBUG to print extra information. */
#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

/* ------------------------------------------------------------------ */
/* Data structure                                                      */
/* ------------------------------------------------------------------ */

/*
 * One client record held by the accountant.
 * Example line in a records file:  101,Jane Smith,Invoice,1250.50
 */
typedef struct
{
    int    id;                       /* unique client record ID          */
    char   name[MAX_NAME_LEN];       /* client name                      */
    char   type[MAX_TYPE_LEN];       /* e.g. Invoice, Receipt, Tax       */
    double amount;                   /* dollar amount                    */
} record_t;

/* ------------------------------------------------------------------ */
/* User interface and data management  (main.c / records.c)            */
/* Author: TBC (proposed: Huy)                                         */
/* ------------------------------------------------------------------ */

/*
 * print_menu
 * Purpose: print the list of menu options to the terminal.
 */
void print_menu(void);

/*
 * read_choice
 * Purpose: read the user's menu choice.
 * Returns: the number chosen, or -1 if the input was not a valid number.
 */
int read_choice(void);

/*
 * load_records
 * Purpose: read records from a text file into an array.
 * Inputs : filename    - file to read
 *          records     - destination array
 *          max_records - size of the array
 * Returns: number of records loaded, or -1 on error.
 */
int load_records(const char filename[], record_t records[], int max_records);

/*
 * save_records
 * Purpose: write records to a text file.
 * Inputs : filename - file to write
 *          records  - array of records
 *          count    - number of records in the array
 * Returns: SUCCESS or FAILURE.
 */
int save_records(const char filename[], const record_t records[], int count);

/*
 * display_records
 * Purpose: print all records to the terminal as a table.
 */
void display_records(const record_t records[], int count);

/* ------------------------------------------------------------------ */
/* Compression  (compression.c)                                        */
/* Author: Georgia Gamble                                              */
/* ------------------------------------------------------------------ */

/*
 * compress_rle
 * Purpose: compress a file using Run-Length Encoding (RLE). Works on raw
 *          bytes, so it does not depend on record_t and can compress any
 *          file. The output is a series of [count][value] byte pairs.
 * Inputs : input_filename  - original file
 *          output_filename - compressed file to create
 * Returns: SUCCESS, or FAILURE if a file cannot be opened, read or written.
 * Note   : an empty input file gives an empty output file.
 */
int compress_rle(const char input_filename[], const char output_filename[]);

/*
 * decompress_rle
 * Purpose: restore a file that was created by compress_rle(). The result
 *          must match the original file byte for byte (lossless).
 * Inputs : input_filename  - compressed file
 *          output_filename - restored file to create
 * Returns: SUCCESS, or FAILURE if a file cannot be opened, read or written,
 *          or the compressed file is not a valid RLE file.
 */
int decompress_rle(const char input_filename[], const char output_filename[]);

/* ------------------------------------------------------------------ */
/* Encryption  (encryption.c)                                          */
/* Author: TBC (proposed: Wally)                                       */
/* ------------------------------------------------------------------ */

/*
 * encrypt_file
 * Purpose: encrypt a file using a repeating-key XOR cipher.
 * Inputs : input_filename  - plain file
 *          output_filename - encrypted file to create
 *          key             - password, must not be empty
 * Returns: SUCCESS or FAILURE (including an empty key).
 */
int encrypt_file(const char input_filename[], const char output_filename[],
                 const char key[]);

/*
 * decrypt_file
 * Purpose: reverse encrypt_file() using the same key.
 * Inputs : input_filename  - encrypted file
 *          output_filename - decrypted file to create
 *          key             - password used to encrypt
 * Returns: SUCCESS or FAILURE.
 */
int decrypt_file(const char input_filename[], const char output_filename[],
                 const char key[]);

/* ------------------------------------------------------------------ */
/* Searching and sorting  (search_sort.c)                              */
/* Author: TBC (proposed: Wally)                                       */
/* ------------------------------------------------------------------ */

/*
 * search_record
 * Purpose: find a record by its ID.
 * Returns: index of the record in the array, or -1 if not found.
 */
int search_record(const record_t records[], int count, int id);

/*
 * sort_records
 * Purpose: sort the records into ascending order of ID (in place).
 */
void sort_records(record_t records[], int count);

#endif /* RECORDS_H */
