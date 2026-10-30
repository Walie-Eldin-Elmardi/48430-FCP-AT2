/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: interface_management.h
 * Authors: Huy, Georgia
 *
 * User accounts: creating them, logging in and out, and loading and
 * saving them to the users file.
 */
#ifndef INTERFACE_MANAGEMENT_H
#define INTERFACE_MANAGEMENT_H

#include "custom_library.h"

/* File the user accounts are saved to */
#define USERS_FILE "Users.txt"

#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 64 /* including the '\0' */
#define MAX_PASSWORD_LENGTH 128 /* stored hash, including the '\0' */

/* ID of the user returned by no_user, meaning nobody is logged in */
#define NO_USER_ID (-1)

typedef enum {
    ACCOUNTANT = 1, /* Accountant role for user */
    CLIENT = 2, /* Client role for user */
    AUDITOR = 3 /* Auditor role for user */
} user_role_t;

typedef struct {
    int id; /* Unique identifier for the user */
    char username[MAX_USERNAME_LENGTH]; /* Username for the user */
    char password_hash[MAX_PASSWORD_LENGTH]; /* $sha256$<salt>$<hash> */
    user_role_t role; /* Role for the user */
} user_t;

/* Every user account, loaded from and saved to the users file */
typedef struct {
    user_t users[MAX_USERS];
    int count;
} user_list_t;

/*
 * Makes the blank user that stands for nobody being logged in.
 * Parameters: None
 * Returns: A user with id NO_USER_ID
 * Author: Georgia
 */
user_t no_user(void);
/*
 * Loads users from a file into the user list. Badly formatted lines and
 * duplicate usernames are skipped with a warning.
 * Parameters: filename of users file, user list to fill in
 * Returns: SUCCESS, FILE_NOT_FOUND if the file does not exist yet (the
 *          list is then empty), or another status code on failure
 * Author: Huy (updated by Georgia)
 */
status_t load_users(const char *filename, user_list_t *users);
/*
 * Saves the user list to a file. Passwords are saved hashed, never in
 * plain text.
 * Parameters: filename of users file, user list
 * Returns: SUCCESS, or FILE_NOT_FOUND if the file could not be written
 * Author: Huy (updated by Georgia)
 */
status_t save_users(const char *filename, const user_list_t *users);
/*
 * Asks for a username, password (twice) and role, and adds a new user
 * with a hashed password to the list.
 * Parameters: user list
 * Returns: SUCCESS, INVALID_ARGUMENT if an answer was not valid, or
 *          MEMORY_ALLOCATION_FAILED if the list is full
 * Author: Huy (updated by Georgia)
 */
status_t create_user(user_list_t *users);
/*
 * Asks for a username and password and checks them against the list.
 * Old plain text passwords are upgraded to a hash when they log in.
 * Parameters: user list
 * Returns: The logged-in user, or no_user() if logging in failed
 * Author: Huy (updated by Georgia)
 */
user_t login(user_list_t *users);
/*
 * Logs out the current user, clearing their details.
 * Parameters: current user
 * Returns: SUCCESS, or INVALID_ARGUMENT if current_user is NULL
 * Author: Huy (updated by Georgia)
 */
status_t logout(user_t *current_user);
/*
 * Finds a user by username.
 * Parameters: user list, username
 * Returns: Pointer to the user if found, otherwise NULL
 * Author: Georgia
 */
const user_t *find_user_by_name(const user_list_t *users,
                                const char *username);
/*
 * Finds a user by user ID.
 * Parameters: user list, user ID
 * Returns: Pointer to the user if found, otherwise NULL
 * Author: Georgia
 */
const user_t *find_user_by_id(const user_list_t *users, int id);

#endif /* INTERFACE_MANAGEMENT_H */
