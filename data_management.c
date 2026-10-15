/******************************************************************************
 * 48430 Fundamentals of C Programming
 * Assessment Task 3 - Group Project
 *
 * File: data_management.c
 * Author: Huy
 *
 * This file contains functions used to manage document records.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_management.h"

#define INPUT_BUFFER_SIZE 1000

/******************************************************************************
 * Loads records from the records database into a linked list.
 *
 * Author: Huy
 ******************************************************************************/
Status load_records(const char *filename,
                    Record **head)
{
    FILE *file;

    Record *new_record;
    Record *current;

    int result;

    int id;
    int owner_id;
    int shared_id;
    int timestamp;

    long size;

    char document_name[INPUT_BUFFER_SIZE];
    char path[INPUT_BUFFER_SIZE];

    if (head == NULL || *head != NULL)
    {
        return 1;
    }

    file = fopen(filename, "r");

    if (file == NULL)
    {
        return 1;
    }

    while (1)
    {
        result = fscanf(file,
                        "%d %d %d %999s %ld %d %999s",
                        &id,
                        &owner_id,
                        &shared_id,
                        document_name,
                        &size,
                        &timestamp,
                        path);

        if (result == EOF)
        {
            break;
        }

        if (result != 7)
        {
            fclose(file);
            return 1;
        }

        if (strlen(document_name) >= MAX_FILENAME_LENGTH ||
            strlen(path) >= MAX_PATH_LENGTH)
        {
            fclose(file);
            return 1;
        }

        new_record = (Record *)malloc(sizeof(Record));

        if (new_record == NULL)
        {
            fclose(file);
            return 1;
        }

        new_record->id = id;
        new_record->owner_id = owner_id;
        new_record->shared_with_user_id = shared_id;

        strcpy(new_record->filename, document_name);

        new_record->size = size;
        new_record->timestamp = timestamp;

        strcpy(new_record->path, path);

        new_record->next = NULL;

        if (*head == NULL)
        {
            *head = new_record;
        }
        else
        {
            current = *head;

            while (current->next != NULL)
            {
                current = current->next;
            }

            current->next = new_record;
        }
    }

    fclose(file);

    return 0;
}

/******************************************************************************
 * Saves all records from the linked list to the database.
 *
 * Author: Huy
 ******************************************************************************/
Status save_records(const char *filename,
                    const Record *head)
{
    FILE *file;

    const Record *current;

    file = fopen(filename, "w");

    if (file == NULL)
    {
        return 1;
    }

    current = head;

    while (current != NULL)
    {
        if (fprintf(file,
                    "%d %d %d %s %ld %d %s\n",
                    current->id,
                    current->owner_id,
                    current->shared_with_user_id,
                    current->filename,
                    current->size,
                    current->timestamp,
                    current->path) < 0)
        {
            fclose(file);
            return 1;
        }

        current = current->next;
    }

    if (fclose(file) != 0)
    {
        return 1;
    }

    return 0;
}

/******************************************************************************
 * Adds a new document record to the linked list.
 *
 * Author: Huy
 ******************************************************************************/
Status add_record(Record **head,
                  int owner_id,
                  int shared_with_user_id,
                  const char *filename,
                  long size,
                  const char *path)
{
    Record *new_record;
    Record *current;

    int next_id;
    int next_timestamp;

    if (head == NULL ||
        filename == NULL ||
        path == NULL)
    {
        return 1;
    }

    if (owner_id <= 0 ||
        shared_with_user_id <= 0 ||
        size < 0)
    {
        return 1;
    }

    if (strlen(filename) == 0 ||
        strlen(filename) >= MAX_FILENAME_LENGTH)
    {
        return 1;
    }

    if (strlen(path) == 0 ||
        strlen(path) >= MAX_PATH_LENGTH)
    {
        return 1;
    }

    next_id = 1;
    next_timestamp = 1;

    current = *head;

    while (current != NULL)
    {
        if (current->id >= next_id)
        {
            next_id = current->id + 1;
        }

        if (current->timestamp >= next_timestamp)
        {
            next_timestamp = current->timestamp + 1;
        }

        current = current->next;
    }

    new_record = (Record *)malloc(sizeof(Record));

    if (new_record == NULL)
    {
        return 1;
    }

    new_record->id = next_id;
    new_record->owner_id = owner_id;
    new_record->shared_with_user_id =
        shared_with_user_id;

    strcpy(new_record->filename, filename);

    new_record->size = size;
    new_record->timestamp = next_timestamp;

    strcpy(new_record->path, path);

    new_record->next = NULL;

    if (*head == NULL)
    {
        *head = new_record;
    }
    else
    {
        current = *head;

        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = new_record;
    }

    return 0;
}

/******************************************************************************
 * Displays records owned by or shared with the current user.
 *
 * Author: Huy
 ******************************************************************************/
void display_records(const Record *head,
                     int user_id,
                     int numOfRecords)
{
    const Record *current;

    int displayed;

    current = head;
    displayed = 0;

    printf("\nStored Documents\n");
    printf("ID\tFilename\tSize\tOwner\tShared With\n");

    while (current != NULL)
    {
        if (current->owner_id == user_id ||
            current->shared_with_user_id == user_id)
        {
            printf("%d\t%s\t%ld\t%d\t%d\n",
                   current->id,
                   current->filename,
                   current->size,
                   current->owner_id,
                   current->shared_with_user_id);

            displayed++;

            if (numOfRecords > 0 &&
                displayed >= numOfRecords)
            {
                break;
            }
        }

        current = current->next;
    }

    if (displayed == 0)
    {
        printf("No documents found.\n");
    }
}
#include <stdlib.h>
#include <string.h>
#include "data_management.h"

/*
 * Compares two records according to the sort option.
 * Parameters: two records to compare, sort option
 * Returns: negative if a comes before b, 0 if equal, positive if a comes after b
 * Author: Georgia
 */
static int compare_records(const Record *a, const Record *b, SortOption sort_option) {
    if (sort_option == SORT_BY_DATE) {
        if (a->timestamp < b->timestamp) return -1;
        if (a->timestamp > b->timestamp) return 1;
        return 0;
    }
    return strcmp(a->filename, b->filename);
}

/*
 * Splits a linked list into two halves using slow and fast pointers.
 * Parameters: head of list to split, pointer to store head of second half
 * Returns: None (the first half starts at head)
 * Author: Georgia
 */
static void split_list(Record *head, Record **second_half) {
    Record *slow = head;
    Record *fast = head->next;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }

    *second_half = slow->next;
    slow->next = NULL;
}

/*
 * Merges two sorted linked lists into one sorted list.
 * Equal records keep their original order (stable sort).
 * Parameters: heads of the two sorted lists, sort option
 * Returns: Head of the merged list
 * Author: Georgia
 */
static Record *merge_lists(Record *a, Record *b, SortOption sort_option) {
    Record dummy;
    Record *tail = &dummy;

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

void sort_records(Record **head, SortOption sort_option) {
    Record *second_half;

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

const Record *search_record(const Record *head, int user_id, const char *filename) {
    const Record *current;

    if (filename == NULL) {
        return NULL;
    }

    for (current = head; current != NULL; current = current->next) {
        if ((current->owner_id == user_id || current->shared_with_user_id == user_id)
            && strcmp(current->filename, filename) == 0) {
            return current;
        }
    }
    return NULL;
}

void free_records(Record **head) {
    Record *current;
    Record *next;

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
