#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "custom_library.h"
#include "data_management.h"
#include "interface_management.h"
#include "user_interface.h"

#define USERS_FILE "Users.txt"
#define RECORDS_FILE "FilePaths.txt"

/*
 * Loads the user and record databases, runs the menu loop until the user
 * quits, then saves both databases and frees the records list.
 * Author: Georgia
 */
int main(void) {
    Record *records = NULL;
    User *current_user = NULL;
    Status status;
    int running = 1;
    int exit_code = 0;

    /* Seed rand() once, for password salts and encryption nonces */
    srand((unsigned int)time(NULL) ^ (unsigned int)clock());

    /* FILE_NOT_FOUND is expected on the first run, before any users are saved */
    status = load_users(USERS_FILE);
    if (status != SUCCESS && status != FILE_NOT_FOUND) {
        fprintf(stderr, "Error: could not load users from %s\n", USERS_FILE);
        return 1;
    }

    status = load_records(RECORDS_FILE, &records);
    if (status != SUCCESS && status != FILE_NOT_FOUND) {
        fprintf(stderr, "Error: could not load records from %s\n", RECORDS_FILE);
        free_records(&records);
        return 1;
    }

    while (running) {
        print_menu(current_user);
        running = read_choice(&records, &current_user);
    }

    if (current_user != NULL) {
        logout(current_user);
        current_user = NULL;
    }

    if (save_users(USERS_FILE) != SUCCESS) {
        fprintf(stderr, "Error: could not save users to %s\n", USERS_FILE);
        exit_code = 1;
    }
    if (save_records(RECORDS_FILE, records) != SUCCESS) {
        fprintf(stderr, "Error: could not save records to %s\n", RECORDS_FILE);
        exit_code = 1;
    }

    free_records(&records);
    return exit_code;
}
