#ifndef INTERFACE_MANAGEMENT_H
#define INTERFACE_MANAGEMENT_H

#include "custom_library.h"


typedef enum {
    ACCOUNTANT = 1, /* Accountant role for user */
    CLIENT = 2, /* Client role for user */
    AUDITOR = 3 /* Auditor role for user */
} UserRole;

typedef struct User {
    int id; /* Unique identifier for the user */
    char username[MAX_FILENAME_LENGTH]; /* Username for the user */
    char password[MAX_FILENAME_LENGTH]; /* Password for the user */
    UserRole role; /* Role for the user */
} User;
/* 
 * Loads users from a file into the users list.
 * Parameters: filename of users database
 * Returns: SUCCESS, FILE_NOT_FOUND if the file does not exist yet, or another Status on failure
 * Author: Huy (updated by Georgia)
 */
Status load_users(const char *filename);
/* 
 * Saves the users list to a file. Passwords are saved hashed, never in plain text.
 * Parameters: filename of user database to save users to
 * Returns: Status code indicating success or failure
 * Author: Huy (updated by Georgia)
 */
Status save_users(const char *filename);
/* 
 * Logs out the current user.
 * Parameters: current_user
 * Returns: Status code indicating success or failure
 * Author: Huy (updated by Georgia)
 */
Status logout(User *current_user);
/* 
 * Asks for a username, password and role and creates a new user with a
 * hashed password. Reads each answer from its own line.
 * Parameters: None
 * Returns: Status code indicating success or failure
 * Author: Huy (updated by Georgia)
 */
Status create_user(void);
/*
 * Asks for a username and password and logs in that user. Reads each
 * answer from its own line.
 * Parameters: None
 * Returns: User structure if login is successful, otherwise a User with id -1
 * Author: Huy (updated by Georgia)
 */
User login(void);

#endif
