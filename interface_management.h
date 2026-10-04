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
 * Loads users from a file into a linked list.
 * Parameters: filename of users database
 * Returns: Status code indicating success or failure
 * Author: Minh
 */
Status load_users(const char *filename);
/* 
 * Saves users from a linked list to a file.
 * Parameters: filename of user database to save users to, head of linked list of users
 * Returns: Status code indicating success or failure
 * Author: Minh
 */
Status save_users(const char *filename);
/* 
 * Logs out the current user.
 * Parameters: current_user
 * Returns: Status code indicating success or failure
 * Author: Minh
 */
Status logout(User *current_user);
/* 
 * Creates a new user.
 * Parameters: None
 * Returns: Status code indicating success or failure
 * Author: Minh
 */
Status create_user();
/* 
 * Logs in a user.
 * Parameters: None
 * Returns: User structure if login is successful, otherwise NULL
 * Author: Minh
 */
User login();

#endif