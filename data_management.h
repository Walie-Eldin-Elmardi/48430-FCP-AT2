/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: data_management.h
 * Authors: Huy, Georgia
 *
 * Records of the stored files, kept in a linked list while the program
 * runs and saved to the records file between runs.
 */
#ifndef DATA_MANAGEMENT_H
#define DATA_MANAGEMENT_H

#include "custom_library.h"

/* File the records are saved to */
#define RECORDS_FILE "FilePaths.txt"

/* Pass as user_id to the display and search functions to match every
 * record, whoever owns it. Real user IDs start at 1. Used for auditors. */
#define ANY_USER 0

/* Room for an upload time formatted as "2026-10-30 15:17" */
#define TIMESTAMP_LENGTH 20

typedef struct record {
    int id; /* Unique identifier for the record */
    int owner_id; /* ID of the user who uploaded the file */
    int shared_with_user_id; /* ID of the user the file is shared with */
    char filename[MAX_FILENAME_LENGTH]; /* Name of the file */
    long size; /* Size of the original file in bytes */
    long timestamp; /* Upload time in seconds since 1970, from time() */
    char path[MAX_PATH_LENGTH]; /* Where the encrypted file is stored */
    struct record *next; /* Next record in the list */
} record_t;

typedef enum {
    SORT_BY_NAME = 1, /* Alphabetically by filename */
    SORT_BY_DATE = 2 /* Oldest upload first */
} sort_option_t;

/*
 * Loads records from a file into a linked list, which must start empty.
 * Badly formatted lines are skipped with a warning.
 * Parameters: filename of records file, pointer to head of linked list
 * Returns: SUCCESS, FILE_NOT_FOUND if the file does not exist yet, or
 *          another status code on failure
 * Author: Huy (updated by Georgia)
 */
status_t load_records(const char *filename, record_t **head);
/*
 * Saves every record in the linked list to a file, one per line.
 * Parameters: filename of records file, head of linked list
 * Returns: SUCCESS, or FILE_NOT_FOUND if the file could not be written
 * Author: Huy (updated by Georgia)
 */
status_t save_records(const char *filename, const record_t *head);
/*
 * Adds a new record to the end of the linked list, with the next free ID
 * and the current time as its upload time.
 * Parameters: pointer to head of linked list, owner's user ID, ID of the
 *             user it is shared with, filename, size in bytes, path of
 *             the stored file
 * Returns: SUCCESS, INVALID_ARGUMENT, or MEMORY_ALLOCATION_FAILED
 * Author: Huy (updated by Georgia)
 */
status_t add_record(record_t **head, int owner_id, int shared_with_user_id,
                    const char *filename, long size, const char *path);
/*
 * Prints a table of the records owned by or shared with user_id (every
 * record if ANY_USER), in list order.
 * Parameters: head of linked list, user ID, filename to show only records
 *             with that name (NULL to show all)
 * Returns: None
 * Author: Huy (updated by Georgia)
 */
void display_records(const record_t *head, int user_id,
                     const char *filename);
/*
 * Sorts the linked list with merge sort. Records that compare equal keep
 * their order.
 * Parameters: pointer to head of linked list, sort option
 * Returns: None
 * Author: Georgia
 */
void sort_records(record_t **head, sort_option_t sort_option);
/*
 * Finds the first record with the given filename that is owned by or
 * shared with user_id (any record if ANY_USER).
 * Parameters: head of linked list, user ID, filename
 * Returns: Pointer to the record if found, otherwise NULL
 * Author: Georgia
 */
const record_t *search_record(const record_t *head, int user_id,
                              const char *filename);
/*
 * Counts the records with the given filename that are owned by or shared
 * with user_id (any record if ANY_USER). Different users can upload files
 * with the same name.
 * Parameters: head of linked list, user ID, filename
 * Returns: Number of matching records
 * Author: Georgia
 */
int count_matching_records(const record_t *head, int user_id,
                           const char *filename);
/*
 * Finds the record with the given record ID, if it is owned by or shared
 * with user_id (any record if ANY_USER).
 * Parameters: head of linked list, user ID, record ID
 * Returns: Pointer to the record if found, otherwise NULL
 * Author: Georgia
 */
const record_t *find_record_by_id(const record_t *head, int user_id,
                                  int record_id);
/*
 * Formats an upload time as "YYYY-MM-DD HH:MM" in local time.
 * Parameters: timestamp, buffer of TIMESTAMP_LENGTH characters to fill in
 * Returns: None
 * Author: Georgia
 */
void format_timestamp(long timestamp, char buffer[TIMESTAMP_LENGTH]);
/*
 * Frees every record in the linked list and sets the head to NULL.
 * Parameters: pointer to head of linked list
 * Returns: None
 * Author: Georgia
 */
void free_records(record_t **head);

#endif /* DATA_MANAGEMENT_H */
