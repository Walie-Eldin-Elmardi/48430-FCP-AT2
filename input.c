/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: input.c
 * Author: Georgia
 *
 * Console input helpers shared by the menu, the account functions and the
 * command line modes. See input.h for what each function does.
 */
#include <stdio.h>  /* printf, fgets, getchar, fflush, feof */
#include <stdlib.h> /* strtol */
#include <string.h> /* strchr, strlen */
#include <ctype.h>  /* isspace, tolower */
#include <errno.h>  /* errno, ERANGE */
#include <limits.h> /* INT_MIN, INT_MAX */
#include "input.h"

void discard_line(void) {
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

int prompt_line(const char *prompt, char *buffer, int size) {
    char *newline;
    size_t length;

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buffer, size, stdin) == NULL) {
        printf("\nNo input given.\n");
        return 0;
    }

    /* No newline means the line did not fit, unless input ended */
    newline = strchr(buffer, '\n');
    if (newline == NULL && !feof(stdin)) {
        discard_line();
        printf("Input is too long (at most %d characters).\n", size - 2);
        return 0;
    }
    if (newline != NULL) {
        *newline = '\0';
    }

    /* Remove a carriage return left by Windows line endings */
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\r') {
        buffer[length - 1] = '\0';
    }

    if (buffer[0] == '\0') {
        printf("No input given.\n");
        return 0;
    }
    return 1;
}

int prompt_int(const char *prompt, int *value) {
    char buffer[INPUT_LENGTH];
    char *end;
    long number;

    if (!prompt_line(prompt, buffer, sizeof(buffer))) {
        return 0;
    }

    /* strtol reports numbers too big for a long through errno, unlike
     * sscanf, where they are undefined behaviour */
    errno = 0;
    number = strtol(buffer, &end, 10);
    while (isspace((unsigned char)*end)) {
        end++;
    }
    if (end == buffer || *end != '\0') {
        printf("Please enter a whole number.\n");
        return 0;
    }
    if (errno == ERANGE || number < INT_MIN || number > INT_MAX) {
        printf("That number is out of range.\n");
        return 0;
    }

    *value = (int)number;
    return 1;
}

int prompt_yes_no(const char *prompt) {
    char buffer[INPUT_LENGTH];

    if (!prompt_line(prompt, buffer, sizeof(buffer))) {
        return 0;
    }
    return tolower((unsigned char)buffer[0]) == 'y';
}

int has_whitespace(const char *str) {
    for (; *str != '\0'; str++) {
        if (isspace((unsigned char)*str)) {
            return 1;
        }
    }
    return 0;
}

int input_ended(void) {
    return feof(stdin) != 0;
}
