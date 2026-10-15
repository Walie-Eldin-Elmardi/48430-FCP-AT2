/******************************************************************************
 * 48430 Fundamentals of C Programming
 * Assessment Task 3 - Group Project
 *
 * File: interface_management.c
 * Author: Huy
 *
 * This file contains functions used to manage user accounts.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface_management.h"

#define MAX_USERS 100
#define INPUT_BUFFER_SIZE 1000

static User users[MAX_USERS];
static int user_count = 0;

/******************************************************************************
 * Loads users from the users database.
 *
 * Author: Huy
 ******************************************************************************/
Status load_users(const char *filename)
{
    FILE *file;

    int id;
    int role_number;
    int result;

    char username[INPUT_BUFFER_SIZE];
    char password[INPUT_BUFFER_SIZE];

    file = fopen(filename, "r");

    if (file == NULL)
    {
        return 1;
    }

    user_count = 0;

    while (1)
    {
        result = fscanf(file,
                        "%d %999s %999s %d",
                        &id,
                        username,
                        password,
                        &role_number);

        if (result == EOF)
        {
            break;
        }

        if (result != 4)
        {
            user_count = 0;
            fclose(file);
            return 1;
        }

        if (strlen(username) >= MAX_FILENAME_LENGTH ||
            strlen(password) >= MAX_FILENAME_LENGTH)
        {
            user_count = 0;
            fclose(file);
            return 1;
        }

        if (role_number < 1 || role_number > 3)
        {
            user_count = 0;
            fclose(file);
            return 1;
        }

        if (user_count >= MAX_USERS)
        {
            user_count = 0;
            fclose(file);
            return 1;
        }

        users[user_count].id = id;

        strcpy(users[user_count].username, username);
        strcpy(users[user_count].password, password);

        if (role_number == 1)
        {
            users[user_count].role = ACCOUNTANT;
        }
        else if (role_number == 2)
        {
            users[user_count].role = CLIENT;
        }
        else
        {
            users[user_count].role = AUDITOR;
        }

        user_count++;
    }

    fclose(file);

    return 0;
}

/******************************************************************************
 * Saves users to the users database.
 *
 * Author: Huy
 ******************************************************************************/
Status save_users(const char *filename)
{
    FILE *file;
    int i;

    file = fopen(filename, "w");

    if (file == NULL)
    {
        return 1;
    }

    for (i = 0; i < user_count; i++)
    {
        if (fprintf(file,
                    "%d %s %s %d\n",
                    users[i].id,
                    users[i].username,
                    users[i].password,
                    users[i].role) < 0)
        {
            fclose(file);
            return 1;
        }
    }

    if (fclose(file) != 0)
    {
        return 1;
    }

    return 0;
}

/******************************************************************************
 * Logs out the current user.
 *
 * Author: Huy
 ******************************************************************************/
Status logout(User *current_user)
{
    if (current_user == NULL)
    {
        return 1;
    }

    current_user->id = -1;
    current_user->username[0] = '\0';
    current_user->password[0] = '\0';

    return 0;
}

/******************************************************************************
 * Creates a new user account.
 *
 * Author: Huy
 ******************************************************************************/
Status create_user(void)
{
    User new_user;

    int role_number;
    int next_id;
    int i;

    char username[INPUT_BUFFER_SIZE];
    char password[INPUT_BUFFER_SIZE];

    if (user_count >= MAX_USERS)
    {
        printf("User database is full.\n");
        return 1;
    }

    printf("Enter username: ");

    if (scanf("%999s", username) != 1)
    {
        return 1;
    }

    if (strlen(username) >= MAX_FILENAME_LENGTH)
    {
        printf("Username is too long.\n");
        return 1;
    }

    next_id = 1;

    for (i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            printf("Username already exists.\n");
            return 1;
        }

        if (users[i].id >= next_id)
        {
            next_id = users[i].id + 1;
        }
    }

    printf("Enter password: ");

    if (scanf("%999s", password) != 1)
    {
        return 1;
    }

    if (strlen(password) >= MAX_FILENAME_LENGTH)
    {
        printf("Password is too long.\n");
        return 1;
    }

    printf("Select role:\n");
    printf("1. Accountant\n");
    printf("2. Client\n");
    printf("3. Auditor\n");
    printf("Choice: ");

    if (scanf("%d", &role_number) != 1)
    {
        return 1;
    }

    if (role_number < 1 || role_number > 3)
    {
        printf("Invalid role.\n");
        return 1;
    }

    new_user.id = next_id;

    strcpy(new_user.username, username);
    strcpy(new_user.password, password);

    if (role_number == 1)
    {
        new_user.role = ACCOUNTANT;
    }
    else if (role_number == 2)
    {
        new_user.role = CLIENT;
    }
    else
    {
        new_user.role = AUDITOR;
    }

    users[user_count] = new_user;

    user_count++;

    return 0;
}

/******************************************************************************
 * Logs a user into the program.
 *
 * Author: Huy
 ******************************************************************************/
User login(void)
{
    char username[INPUT_BUFFER_SIZE];
    char password[INPUT_BUFFER_SIZE];

    User failed_user;

    int i;

    printf("Username: ");

    if (scanf("%999s", username) != 1)
    {
        failed_user.id = -1;
        failed_user.username[0] = '\0';
        failed_user.password[0] = '\0';
        failed_user.role = CLIENT;

        return failed_user;
    }

    printf("Password: ");

    if (scanf("%999s", password) != 1)
    {
        failed_user.id = -1;
        failed_user.username[0] = '\0';
        failed_user.password[0] = '\0';
        failed_user.role = CLIENT;

        return failed_user;
    }

    if (strlen(username) < MAX_FILENAME_LENGTH &&
        strlen(password) < MAX_FILENAME_LENGTH)
    {
        for (i = 0; i < user_count; i++)
        {
            if (strcmp(users[i].username, username) == 0 &&
                strcmp(users[i].password, password) == 0)
            {
                return users[i];
            }
        }
    }

    printf("Invalid username or password.\n");

    failed_user.id = -1;
    failed_user.username[0] = '\0';
    failed_user.password[0] = '\0';
    failed_user.role = CLIENT;

    return failed_user;
}