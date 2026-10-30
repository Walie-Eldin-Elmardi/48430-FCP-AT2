/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: main.c
 * Author: Georgia
 *
 * Entry point. With no arguments, runs the interactive records program.
 * With arguments, runs one of the command line modes (see --help).
 */
#include <stdio.h>  /* printf, fprintf */
#include <stdlib.h> /* srand */
#include <time.h>   /* time, clock */
#include "custom_library.h"
#include "data_management.h"
#include "interface_management.h"
#include "user_interface.h"
#include "command_line.h"

/*
 * Loads the users and records, runs the menu until the user quits, then
 * saves both and frees the records list.
 * Parameters: None
 * Returns: Exit code for main: 0 on success, 1 if loading or saving
 *          failed
 * Author: Georgia
 */
static int run_interactive(void) {
    user_list_t users;
    record_t *records = NULL;
    user_t current_user = no_user();
    status_t status;
    int running = 1;
    int exit_code = 0;

    /* FILE_NOT_FOUND is expected on the first run, before anything is
     * saved */
    status = load_users(USERS_FILE, &users);
    if (status != SUCCESS && status != FILE_NOT_FOUND) {
        fprintf(stderr, "Error: could not load users from %s\n", USERS_FILE);
        return 1;
    }
    status = load_records(RECORDS_FILE, &records);
    if (status != SUCCESS && status != FILE_NOT_FOUND) {
        fprintf(stderr, "Error: could not load records from %s\n",
                RECORDS_FILE);
        free_records(&records);
        return 1;
    }

    printf("Accountant-Client Records Sharing Platform\n");
    if (users.count == 0) {
        printf("No accounts yet. Choose 2 to create one.\n");
    }

    while (running) {
        print_menu(&current_user);
        running = read_choice(&records, &users, &current_user);
    }
    logout(&current_user);

    if (save_users(USERS_FILE, &users) != SUCCESS) {
        fprintf(stderr, "Error: could not save users to %s\n", USERS_FILE);
        exit_code = 1;
    }
    if (save_records(RECORDS_FILE, records) != SUCCESS) {
        fprintf(stderr, "Error: could not save records to %s\n",
                RECORDS_FILE);
        exit_code = 1;
    }

    free_records(&records);
    return exit_code;
}

int main(int argc, char *argv[]) {
    /* Seed rand() once, for password salts and encryption nonces */
    srand((unsigned int)time(NULL) ^ (unsigned int)clock());
    DEBUG_PRINT("debug build, %d command line argument(s)", argc - 1);

    if (argc > 1) {
        return run_command_line(argc, argv);
    }
    return run_interactive();
}
