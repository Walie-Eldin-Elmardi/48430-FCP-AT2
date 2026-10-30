/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: data_management.c
 * Authors: Huy, Georgia
 *
 * Loads, saves, adds, displays, sorts and searches the records of stored
 * files. See data_management.h for what each public function does.
 *
 * The records file has one record per line:
 *     id owner_id shared_with_user_id filename size timestamp path
 */
#include <stdio.h>  /* fopen, fclose, fgets, fgetc, feof, fprintf, printf,
                       sscanf */
#include <stdlib.h> /* malloc, free */
#include <string.h> /* strlen, strcpy, strcmp, strchr */
#include <time.h>   /* time, localtime, strftime */
#include "data_management.h"

/* Longest line the records file can hold: filename, path and 5 numbers */
#define LINE_LENGTH (MAX_FILENAME_LENGTH + MAX_PATH_LENGTH + 100)

/*
 * Discards the rest of a line that was too long for the line buffer.
 * Parameters: file being read
 * Returns: None
 * Author: Georgia
 */
static void skip_rest_of_line(FILE *file) {
    int c;

    while ((c = fgetc(file)) != '\n' && c != EOF) {
        /* discard */
    }
}

/*
 * Reads one record from a line of the records file.
 * Parameters: line of text, record to fill in
 * Returns: 1 if the line holds a valid record, otherwise 0
 * Author: Huy (updated by Georgia)
 */
static int parse_record(const char *line, record_t *record) {
    char filename[LINE_LENGTH];
    char path[LINE_LENGTH];
    char extra;

    /* Reading one more character catches extra fields on the line */
    if (sscanf(line, "%d %d %d %s %ld %ld %s %c", &record->id,
               &record->owner_id, &record->shared_with_user_id, filename,
               &record->size, &record->timestamp, path, &extra) != 7) {
        return 0;
    }
    if (record->id <= 0 || record->owner_id <= 0 ||
        record->shared_with_user_id <= 0 || record->size < 0 ||
        strlen(filename) >= MAX_FILENAME_LENGTH ||
        strlen(path) >= MAX_PATH_LENGTH) {
        return 0;
    }

    strcpy(record->filename, filename);
    strcpy(record->path, path);
    record->next = NULL;
    return 1;
}

status_t load_records(const char *filename, record_t **head) {
    FILE *file;
    record_t *new_record;
    record_t *tail = NULL;
    record_t parsed;
    char line[LINE_LENGTH];
    char first;
    int line_number = 0;
    int loaded = 0;

    if (filename == NULL || head == NULL || *head != NULL) {
        return INVALID_ARGUMENT;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        return FILE_NOT_FOUND;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;

        if (strchr(line, '\n') == NULL && !feof(file)) {
            skip_rest_of_line(file);
            fprintf(stderr, "Warning: skipped line %d of %s (too long).\n",
                    line_number, filename);
            continue;
        }
        /* Skip blank lines */
        if (sscanf(line, " %c", &first) != 1) {
            continue;
        }
        if (!parse_record(line, &parsed)) {
            fprintf(stderr, "Warning: skipped line %d of %s (badly "
                    "formatted).\n", line_number, filename);
            continue;
        }

        new_record = malloc(sizeof(record_t));
        if (new_record == NULL) {
            fclose(file);
            free_records(head);
            return MEMORY_ALLOCATION_FAILED;
        }
        *new_record = parsed;

        /* Keep track of the tail so each record is added in constant time */
        if (tail == NULL) {
            *head = new_record;
        } else {
            tail->next = new_record;
        }
        tail = new_record;
        loaded++;
    }

    fclose(file);
    DEBUG_PRINT("loaded %d record(s) from %s", loaded, filename);
    return SUCCESS;
}

status_t save_records(const char *filename, const record_t *head) {
    FILE *file;
    const record_t *current;
    int saved = 0;

    if (filename == NULL) {
        return INVALID_ARGUMENT;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        return FILE_NOT_FOUND;
    }

    for (current = head; current != NULL; current = current->next) {
        if (fprintf(file, "%d %d %d %s %ld %ld %s\n", current->id,
                    current->owner_id, current->shared_with_user_id,
                    current->filename, current->size, current->timestamp,
                    current->path) < 0) {
            fclose(file);
            return FILE_NOT_FOUND;
        }
        saved++;
    }

    if (fclose(file) != 0) {
        return FILE_NOT_FOUND;
    }
    DEBUG_PRINT("saved %d record(s) to %s", saved, filename);
    return SUCCESS;
}

status_t add_record(record_t **head, int owner_id, int shared_with_user_id,
                    const char *filename, long size, const char *path) {
    record_t *new_record;
    record_t *current;
    int next_id = 1;

    if (head == NULL || filename == NULL || path == NULL) {
        return INVALID_ARGUMENT;
    }
    if (owner_id <= 0 || shared_with_user_id <= 0 || size < 0) {
        return INVALID_ARGUMENT;
    }
    if (strlen(filename) == 0 || strlen(filename) >= MAX_FILENAME_LENGTH ||
        strlen(path) == 0 || strlen(path) >= MAX_PATH_LENGTH) {
        return INVALID_ARGUMENT;
    }

    for (current = *head; current != NULL; current = current->next) {
        if (current->id >= next_id) {
            next_id = current->id + 1;
        }
    }

    new_record = malloc(sizeof(record_t));
    if (new_record == NULL) {
        return MEMORY_ALLOCATION_FAILED;
    }

    new_record->id = next_id;
    new_record->owner_id = owner_id;
    new_record->shared_with_user_id = shared_with_user_id;
    strcpy(new_record->filename, filename);
    new_record->size = size;
    new_record->timestamp = (long)time(NULL);
    strcpy(new_record->path, path);
    new_record->next = NULL;

    if (*head == NULL) {
        *head = new_record;
    } else {
        current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_record;
    }

    DEBUG_PRINT("added record %d: %s from user %d, shared with user %d",
                next_id, filename, owner_id, shared_with_user_id);
    return SUCCESS;
}

/*
 * Checks whether a record is owned by or shared with a user.
 * Parameters: record, user ID (ANY_USER matches every record)
 * Returns: 1 if the user can see the record, otherwise 0
 * Author: Georgia
 */
static int is_visible(const record_t *record, int user_id) {
    return user_id == ANY_USER || record->owner_id == user_id ||
           record->shared_with_user_id == user_id;
}

void format_timestamp(long timestamp, char buffer[TIMESTAMP_LENGTH]) {
    time_t time_value = (time_t)timestamp;
    struct tm *local = localtime(&time_value);

    if (local == NULL ||
        strftime(buffer, TIMESTAMP_LENGTH, "%Y-%m-%d %H:%M", local) == 0) {
        strcpy(buffer, "unknown");
    }
}

void display_records(const record_t *head, int user_id,
                     const char *filename) {
    const record_t *current;
    char uploaded[TIMESTAMP_LENGTH];
    int displayed = 0;

    printf("\n%-4s %-24s %12s %6s %7s  %s\n", "ID", "Filename",
           "Size (bytes)", "Owner", "Shared", "Uploaded");

    for (current = head; current != NULL; current = current->next) {
        if (!is_visible(current, user_id) ||
            (filename != NULL && strcmp(current->filename, filename) != 0)) {
            continue;
        }
        format_timestamp(current->timestamp, uploaded);
        printf("%-4d %-24s %12ld %6d %7d  %s\n", current->id,
               current->filename, current->size, current->owner_id,
               current->shared_with_user_id, uploaded);
        displayed++;
    }

    if (displayed == 0) {
        printf("No documents found.\n");
    }
    DEBUG_PRINT("displayed %d record(s) for user %d", displayed, user_id);
}

/*
 * Compares two records according to the sort option. Records uploaded in
 * the same second are ordered by record ID.
 * Parameters: two records to compare, sort option
 * Returns: Negative if a comes before b, 0 if equal, positive if a comes
 *          after b
 * Author: Georgia
 */
static int compare_records(const record_t *a, const record_t *b,
                           sort_option_t sort_option) {
    if (sort_option == SORT_BY_DATE) {
        if (a->timestamp != b->timestamp) {
            return (a->timestamp < b->timestamp) ? -1 : 1;
        }
        return a->id - b->id;
    }
    return strcmp(a->filename, b->filename);
}

/*
 * Splits a linked list into two halves using slow and fast pointers.
 * Parameters: head of list to split, pointer to store head of second half
 * Returns: None (the first half starts at head)
 * Author: Georgia
 */
static void split_list(record_t *head, record_t **second_half) {
    record_t *slow = head;
    record_t *fast = head->next;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }

    *second_half = slow->next;
    slow->next = NULL;
}

/*
 * Merges two sorted linked lists into one sorted list. Equal records keep
 * their original order (stable sort).
 * Parameters: heads of the two sorted lists, sort option
 * Returns: Head of the merged list
 * Author: Georgia
 */
static record_t *merge_lists(record_t *a, record_t *b,
                             sort_option_t sort_option) {
    record_t dummy;
    record_t *tail = &dummy;

    dummy.next = NULL;

    while (a != NULL && b != NULL) {
        if (compare_records(a, b, sort_option) <= 0) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }

    tail->next = (a != NULL) ? a : b;
    return dummy.next;
}

void sort_records(record_t **head, sort_option_t sort_option) {
    record_t *second_half;

    if (head == NULL || *head == NULL || (*head)->next == NULL) {
        return;
    }
    if (sort_option != SORT_BY_NAME && sort_option != SORT_BY_DATE) {
        return;
    }

    split_list(*head, &second_half);
    sort_records(head, sort_option);
    sort_records(&second_half, sort_option);
    *head = merge_lists(*head, second_half, sort_option);
}

const record_t *search_record(const record_t *head, int user_id,
                              const char *filename) {
    const record_t *current;

    if (filename == NULL) {
        return NULL;
    }

    for (current = head; current != NULL; current = current->next) {
        if (is_visible(current, user_id) &&
            strcmp(current->filename, filename) == 0) {
            return current;
        }
    }
    return NULL;
}

int count_matching_records(const record_t *head, int user_id,
                           const char *filename) {
    const record_t *current;
    int count = 0;

    if (filename == NULL) {
        return 0;
    }

    for (current = head; current != NULL; current = current->next) {
        if (is_visible(current, user_id) &&
            strcmp(current->filename, filename) == 0) {
            count++;
        }
    }
    return count;
}

const record_t *find_record_by_id(const record_t *head, int user_id,
                                  int record_id) {
    const record_t *current;

    for (current = head; current != NULL; current = current->next) {
        if (current->id == record_id && is_visible(current, user_id)) {
            return current;
        }
    }
    return NULL;
}

void free_records(record_t **head) {
    record_t *current;
    record_t *next;

    if (head == NULL) {
        return;
    }

    current = *head;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}
