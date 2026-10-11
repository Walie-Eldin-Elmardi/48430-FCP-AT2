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
