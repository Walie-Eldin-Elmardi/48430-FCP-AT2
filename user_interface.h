#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "custom_library.h"
#include "interface_management.h"
#include "data_management.h"

/*
 * Prints the main menu options. Shows login/create account/quit when no
 * user is logged in, otherwise the options for the logged-in user's role.
 * Parameters: current_user (NULL if no one is logged in)
 * Returns: None
 * Author: Georgia
 */
void print_menu(const User *current_user);
/*
 * Reads the user's choice from the console and carries it out.
 * Parameters: records (head of records list, may be changed), current_user (set on login, NULL on logout)
 * Returns: 0 if the user chose to quit, otherwise 1
 * Author: Georgia
 */
int read_choice(Record **records, User **current_user);

#endif
