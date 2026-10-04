#ifndef DATA_MANAGEMENT_H
#define DATA_MANAGEMENT_H

#include "custom_library.h"

typedef struct Record {
    int id; /* Unique identifier for the record */
    int owner_id; /* user ID of user who uploaded the file */
    int shared_with_user_id; /* user ID of user with whom the file is shared */
    char filename[MAX_FILENAME_LENGTH]; /* Name of the file */
    long size; /* Size of the file in bytes */
    int timestamp; /* Timestamp of when the file was uploaded */
    char path[MAX_PATH_LENGTH]; /* where the file is stored */
    struct Record *next;
} Record;

typedef enum {
    SORT_BY_NAME = 1, /* alphabetically by filename */
    SORT_BY_DATE = 2 /* chronologically by timestamp */
} SortOption;

/* 
 * Loads records from a file into a linked list.
 * Parameters: filename of records database, pointer to head of linked list
 * Returns: Status code indicating success or failure
 * Author: Minh
 */
Status load_records(const char *filename, Record **head);
/* 
 * Saves records from a linked list to a file.
 * Parameters: filename (const char *), head (const Record *)
 * Returns: Status code indicating success or failure
 * Author: Minh
 */
Status save_records(const char *filename, const Record *head);
/* 
 * Adds a new record to the linked list.
 * Parameters: head (Record **), owner_id (int), shared_with_user_id (int), filename (const char *), size (long), path (const char *)
 * Returns: Status code indicating success or failure
 * Author: Minh
 */
Status add_record(Record **head, int owner_id, int shared_with_user_id, const char *filename, long size, const char *path);
/* 
 * Displays records from the linked list.
 * Parameters: head (const Record *), user_id (int), sort_option (SortOption), numOfRecords (int)
 * Returns: Record structure containing the displayed records
 * Author: Minh
 */
Record display_records(const Record *head, int user_id, SortOption sort_option, int numOfRecords);
/* 
 * Sorts the records in the linked list, preferably using merge sort, based on the specified option.
 * Parameters: head (Record **), sort_option (SortOption)
 * Returns: None
 * Author: Georgia
 */
void sort_records(Record **head, SortOption sort_option);
/* 
 * Searches for a record in the linked list.
 * Parameters: head (const Record *), user_id (int), filename (const char *)
 * Returns: Record structure if found, otherwise NULL
 * Author: Georgia
 */
Record search_record(const Record *head, int user_id, const char *filename);

#endif