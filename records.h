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
#define MAX_DATE_LEN  11    /* "YYYY-MM-DD" plus the terminating '\0' */
#define MAX_PATH_LEN  256   /* longest file name handled by the program */

/* User accounts */
#define MAX_USERS         50
#define MAX_USERNAME_LEN  30   /* includes the terminating '\0' */
#define MAX_PASSWORD_LEN  32   /* includes the terminating '\0' */
#define ROLE_ACCOUNTANT   1
#define ROLE_CLIENT       2

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
 * Example line in a records file:  101,Jane Smith,Invoice,1250.50,2026-09-30
 */
typedef struct
{
    int    id;                       /* unique client record ID          */
    char   name[MAX_NAME_LEN];       /* client name                      */
    char   type[MAX_TYPE_LEN];       /* e.g. Invoice, Receipt, Tax       */
    double amount;                   /* dollar amount                    */
    char   date[MAX_DATE_LEN];       /* YYYY-MM-DD, sorts in date order  */
} record_t;

/*
 * One user account. The password itself is never stored, only a hash of
 * the username and password (see hash_password).
 * Example line in Users.txt:  1,jane,2087654321,1
 */
typedef struct
{
    int           id;                         /* unique user ID, from 1    */
    char          username[MAX_USERNAME_LEN]; /* login name                */
    unsigned long password_hash;              /* 32-bit hash of user+pass  */
    int           role;                       /* ROLE_ACCOUNTANT / _CLIENT */
} user_t;

/* ------------------------------------------------------------------ */
/* User interface and data management  (main.c / records.c)            */
/* Author: Minh Huy Pham                                               */
/* ------------------------------------------------------------------ */

/*
 * print_menu
 * Author: Minh Huy Pham
 * Purpose: print the list of menu options to the terminal.
 */
void print_menu(void);

/*
 * read_choice
 * Author: Minh Huy Pham
 * Purpose: read the user's menu choice.
 * Returns: the number chosen, or -1 if the input was not a valid number.
 */
int read_choice(void);

/*
 * load_records
 * Author: Minh Huy Pham
 * Purpose: read records from a text file into an array.
 * Inputs : filename    - file to read
 *          records     - destination array
 *          max_records - size of the array
 * Returns: number of records loaded, or -1 on error.
 */
int load_records(const char filename[], record_t records[], int max_records);

/*
 * save_records
 * Author: Minh Huy Pham
 * Purpose: write records to a text file.
 * Inputs : filename - file to write
 *          records  - array of records
 *          count    - number of records in the array
 * Returns: SUCCESS or FAILURE.
 */
int save_records(const char filename[], const record_t records[], int count);

/*
 * display_records
 * Author: Minh Huy Pham
 * Purpose: print all records to the terminal as a table.
 */
void display_records(const record_t records[], int count);

/* ------------------------------------------------------------------ */
/* Compression  (compression.c)                                        */
/* Author: Georgia Gamble                                              */
/* ------------------------------------------------------------------ */

/*
 * compress_rle
 * Author: Georgia Gamble
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
 * Author: Georgia Gamble
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
/* Author: Wally Elmardi                                               */
/* ------------------------------------------------------------------ */

/*
 * encrypt_file
 * Author: Wally Elmardi
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
 * Author: Wally Elmardi
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
/* Author: Wally Elmardi                                               */
/* ------------------------------------------------------------------ */

/*
 * search_record
 * Author: Wally Elmardi
 * Purpose: find a record by its ID.
 * Returns: index of the record in the array, or -1 if not found.
 */
int search_record(const record_t records[], int count, int id);

/*
 * sort_records
 * Author: Wally Elmardi
 * Purpose: sort the records into ascending order of ID (in place).
 */
void sort_records(record_t records[], int count);

/*
 * sort_records_by_name
 * Author: Wally Elmardi
 * Purpose: sort the records into alphabetical order of client name
 *          (in place, using strcmp so upper case sorts before lower case).
 */
void sort_records_by_name(record_t records[], int count);

/*
 * sort_records_by_date
 * Author: Wally Elmardi
 * Purpose: sort the records into chronological order, oldest first
 *          (in place). Dates are YYYY-MM-DD, so comparing them as strings
 *          gives date order.
 */
void sort_records_by_date(record_t records[], int count);

/* ------------------------------------------------------------------ */
/* User accounts  (users.c)                                           */
/* Author: Georgia Gamble                                             */
/* ------------------------------------------------------------------ */

/*
 * hash_password
 * Author: Georgia Gamble
 * Purpose: turn a username and password into a 32-bit hash so passwords
 *          are not stored as plain text. Simple djb2-style hash, good
 *          enough for this assignment but NOT secure for real use.
 * Returns: the hash. The same inputs always give the same hash.
 */
unsigned long hash_password(const char username[], const char password[]);

/*
 * load_users
 * Author: Georgia Gamble
 * Purpose: read user accounts from a text file (one "id,username,hash,role"
 *          line per user, blank lines ignored).
 * Inputs : filename - file to read (Users.txt)
 *          users - destination array
 *          max_users - size of the array
 * Returns: number of users loaded, or -1 if the file cannot be opened,
 *          holds more than max_users users or contains a bad line.
 */
int load_users(const char filename[], user_t users[], int max_users);

/*
 * save_users
 * Author: Georgia Gamble
 * Purpose: write user accounts to a text file in the format above.
 * Inputs : filename - file to write (Users.txt)
 *          users - array of users
 *          count - number of users in the array
 * Returns: SUCCESS or FAILURE.
 */
int save_users(const char filename[], const user_t users[], int count);

/*
 * create_user
 * Author: Georgia Gamble
 * Purpose: add a new account to the end of the array and give it the next
 *          free ID. The username must be 1 to MAX_USERNAME_LEN-1 characters
 *          with no spaces, commas or control characters, the password 1 to
 *          MAX_PASSWORD_LEN-1 characters, and the role ROLE_ACCOUNTANT or
 *          ROLE_CLIENT. Usernames are case sensitive.
 * Inputs : users - array of users
 *          count - number of users now in the array (increased on success)
 *          max_users - size of the array
 *          username, password, role - details of the new account
 * Returns: SUCCESS, or FAILURE (nothing is changed) if any detail is
 *          invalid, the username is taken or the array is full.
 */
int create_user(user_t users[], int *count, int max_users,
                const char username[], const char password[], int role);

/*
 * login_user
 * Author: Georgia Gamble
 * Purpose: check a username and password against the stored accounts.
 * Returns: index of the matching user in the array, or -1 if the username
 *          does not exist or the password is wrong.
 */
int login_user(const user_t users[], int count,
               const char username[], const char password[]);

/* ------------------------------------------------------------------ */
/* Document store and retrieve  (pipeline.c)                          */
/* Author: Georgia Gamble                                             */
/* ------------------------------------------------------------------ */

/*
 * store_document
 * Author: Georgia Gamble
 * Purpose: protect a file for storage or sharing: compress it with
 *          compress_rle(), then encrypt the result with encrypt_file().
 *          (Compress first: encrypted data looks random and does not
 *          compress.) The unencrypted temporary file is always deleted.
 * Inputs : input_filename - original file
 *          store_filename - compressed and encrypted file to create
 *          key - password used to encrypt, must not be empty
 * Returns: SUCCESS or FAILURE (a file error, an empty key, or a file name
 *          too long for the temporary file).
 */
int store_document(const char input_filename[], const char store_filename[],
                   const char key[]);

/*
 * retrieve_document
 * Author: Georgia Gamble
 * Purpose: reverse store_document(): decrypt with decrypt_file(), then
 *          decompress with decompress_rle(). The temporary file is always
 *          deleted.
 * Inputs : store_filename - file made by store_document()
 *          output_filename - restored file to create
 *          key - password used to store the file
 * Returns: SUCCESS or FAILURE. A wrong key is not detected directly: the
 *          decrypted data is garbage, which usually (not always) fails to
 *          decompress and so returns FAILURE.
 */
int retrieve_document(const char store_filename[],
                      const char output_filename[], const char key[]);

/* ------------------------------------------------------------------ */
/* Sharing stored files with users  (filelist.c)                      */
/* Author: Minh Huy Pham                                              */
/* ------------------------------------------------------------------ */

/*
 * assign_file
 * Author: Minh Huy Pham
 * Purpose: record that a stored file belongs to a user, so that user can
 *          find it after logging in. This is how sharing between an
 *          accountant and a client is simulated: the file is assigned to
 *          the client's user ID. Appends the line "user_id,stored_filename"
 *          to the registry file (FilePaths.txt).
 * Inputs : registry_filename - registry file to add to (FilePaths.txt)
 *          user_id - ID of the user who can access the file
 *          stored_filename - file made by store_document(); must not
 *                            contain a comma or be longer than MAX_PATH_LEN-1
 * Returns: SUCCESS or FAILURE.
 */
int assign_file(const char registry_filename[], int user_id,
                const char stored_filename[]);

/*
 * list_user_files
 * Author: Minh Huy Pham
 * Purpose: find every stored file assigned to a user.
 * Inputs : registry_filename - registry file to read (FilePaths.txt)
 *          user_id - ID of the user
 *          filenames - destination array of file names
 *          max_files - size of the array
 * Returns: number of file names found (0 if none), or -1 if the registry
 *          cannot be opened.
 */
int list_user_files(const char registry_filename[], int user_id,
                    char filenames[][MAX_PATH_LEN], int max_files);

#endif /* RECORDS_H */
