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