/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: user_interface.h
 * Author: Georgia
 *
 * The interactive menu: logging in, and uploading, retrieving, listing
 * and searching files.
 */
#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "custom_library.h"
#include "interface_management.h"
#include "data_management.h"

/*
 * Prints the main menu. Shows log in, create account and quit when no
 * user is logged in, otherwise the options for the logged-in user's role.
 * Parameters: current user (id NO_USER_ID if nobody is logged in)
 * Returns: None
 * Author: Georgia
 */
void print_menu(const user_t *current_user);
/*
 * Reads the user's menu choice from the console and carries it out.
 * Parameters: pointer to head of records list (may be changed), user
 *             list, current user (set on login, cleared on logout)
 * Returns: 0 if the user chose to quit or input ended, otherwise 1
 * Author: Georgia
 */
int read_choice(record_t **records, user_list_t *users,
                user_t *current_user);

#endif /* USER_INTERFACE_H */
