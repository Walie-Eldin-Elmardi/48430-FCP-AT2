#include <stdio.h>
#include <string.h>
#include "user_interface.h"
#include "compression.h"
#include "encryption.h"

#define INPUT_LENGTH 256
#define TEMP_FILE "transfer.tmp"

/* login returns the user by value, so current_user points here while logged in */
static User logged_in_user;

/*
 * Discards everything left on the current line of stdin. Needed after
 * login and create_user, which read with scanf and leave the newline behind.
 * Parameters: None
 * Returns: None
 * Author: Georgia
 */
static void discard_line(void) {
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

/*
 * Reads one line from stdin into buffer, without the trailing newline.
 * If the line is longer than the buffer, the rest of it is discarded.
 * Parameters: buffer to fill, size of buffer
 * Returns: 1 if a line was read, 0 on end of input
 * Author: Georgia
 */
static int read_line(char *buffer, int size) {
    char *newline;

    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }

    newline = strchr(buffer, '\n');
    if (newline != NULL) {
        *newline = '\0';
    } else {
        discard_line();
    }
    return 1;
}

/*
 * Prints a prompt and reads a non-empty line from stdin.
 * Parameters: prompt to print, buffer to fill, size of buffer
 * Returns: 1 if a non-empty line was read, 0 otherwise
 * Author: Georgia
 */
static int prompt_line(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    if (!read_line(buffer, size) || buffer[0] == '\0') {
        printf("No input given.\n");
        return 0;
    }
    return 1;
}

/*
 * Prints a prompt and reads an integer from stdin.
 * Parameters: prompt to print, pointer to store the integer
 * Returns: 1 if a valid integer was read, 0 otherwise
 * Author: Georgia
 */
static int prompt_int(const char *prompt, int *value) {
    char buffer[INPUT_LENGTH];

    if (!prompt_line(prompt, buffer, sizeof(buffer))) {
        return 0;
    }
    if (sscanf(buffer, "%d", value) != 1) {
        printf("Please enter a number.\n");
        return 0;
    }
    return 1;
}

/*
 * Returns the filename part of a path (everything after the last '/').
 * Parameters: path
 * Returns: Pointer into path where the filename starts
 * Author: Georgia
 */
static const char *base_name(const char *path) {
    const char *slash = strrchr(path, '/');
    return (slash != NULL) ? slash + 1 : path;
}

/*
 * Gets the size of a file in bytes.
 * Parameters: filename
 * Returns: Size in bytes, or -1 if the file could not be opened
 * Author: Georgia
 */
static long file_size(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    long size;

    if (fp == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fclose(fp);
    return size;
}

/*
 * Returns a printable name for a user role.
 * Parameters: role
 * Returns: Name of the role, or "Unknown" if the role is not recognised
 * Author: Georgia
 */
static const char *role_name(UserRole role) {
    switch (role) {
        case ACCOUNTANT: return "Accountant";
        case CLIENT: return "Client";
        case AUDITOR: return "Auditor";
    }
    return "Unknown";
}

void print_menu(const User *current_user) {
    printf("\n");

    if (current_user == NULL) {
        printf("1. Log in\n");
        printf("2. Create account\n");
        printf("0. Quit\n");
        printf("Enter choice: ");
        return;
    }

    printf("Logged in as %s (%s)\n", current_user->username,
           role_name(current_user->role));

    /* Auditors can view records but not upload them */
    if (current_user->role != AUDITOR) {
        printf("1. Upload a file\n");
        printf("2. Retrieve a file\n");
        printf("3. Display records sorted by filename\n");
        printf("4. Display records sorted by upload date\n");
        printf("5. Search records by filename\n");
        printf("6. Log out\n");
        printf("0. Quit\n");
        printf("Enter choice: ");
    }
    printf("1. Retrieve a file\n");
    printf("2. Display records sorted by filename\n");
    printf("3. Display records sorted by upload date\n");
    printf("4. Search records by filename\n");
    printf("5. Log out\n");
    printf("0. Quit\n");
    printf("Enter choice: ");
}

/*
 * Asks for a file to upload and who to share it with, then compresses and
 * encrypts it into storage and adds a record for it.
 * Parameters: records (head of records list), current_user
 * Returns: None
 * Author: Georgia
 */
static void handle_upload(Record **records, const User *current_user) {
    char input_path[MAX_PATH_LENGTH];
    char stored_path[MAX_PATH_LENGTH];
    char password[INPUT_LENGTH];
    const char *filename;
    int shared_with_user_id;
    long size;
    Status status;

    if (!prompt_line("Path of file to upload: ", input_path, sizeof(input_path))) {
        return;
    }
    size = file_size(input_path);
    if (size < 0) {
        printf("Could not open %s.\n", input_path);
        return;
    }
    if (!prompt_int("User ID to share with: ", &shared_with_user_id)) {
        return;
    }
    if (!prompt_line("Password for this file: ", password, sizeof(password))) {
        return;
    }

    filename = base_name(input_path);
    if (strlen(filename) >= MAX_FILENAME_LENGTH) {
        printf("Filename is too long.\n");
        return;
    }
    if (search_record(*records, current_user->id, filename) != NULL) {
        printf("You already have a file called %s.\n", filename);
        return;
    }
    sprintf(stored_path, "%d_%.*s.enc", current_user->id,
            MAX_PATH_LENGTH - 20, filename);

    status = compress_file(input_path, TEMP_FILE);
    if (status == SUCCESS) {
        status = encrypt_file(TEMP_FILE, stored_path, password);
    }
    remove(TEMP_FILE);
    if (status == SUCCESS) {
        status = add_record(records, current_user->id, shared_with_user_id,
                            filename, size, stored_path);
    }

    if (status == SUCCESS) {
        printf("Uploaded %s.\n", filename);
    } else {
        printf("Upload failed (error %d).\n", status);
    }
}

/*
 * Asks for a filename, then decrypts and decompresses that file to an
 * output path if the current user is allowed to access it.
 * Parameters: records (head of records list), current_user
 * Returns: None
 * Author: Georgia
 */
static void handle_retrieve(const Record *records, const User *current_user) {
    char filename[MAX_FILENAME_LENGTH];
    char output_path[MAX_PATH_LENGTH];
    char password[INPUT_LENGTH];
    const Record *record;
    Status status;

    if (!prompt_line("Filename to retrieve: ", filename, sizeof(filename))) {
        return;
    }
    record = search_record(records, current_user->id, filename);
    if (record == NULL) {
        printf("No file called %s is shared with you.\n", filename);
        return;
    }
    if (!prompt_line("Save as: ", output_path, sizeof(output_path))) {
        return;
    }
    if (!prompt_line("Password for this file: ", password, sizeof(password))) {
        return;
    }

    status = decrypt_file(record->path, TEMP_FILE, password);
    if (status == SUCCESS) {
        status = decompress_file(TEMP_FILE, output_path);
    }
    remove(TEMP_FILE);

    if (status == SUCCESS) {
        printf("Saved %s to %s.\n", filename, output_path);
        printf("Retrieve failed (error %d).\n", status);
    }
}

/*
 * Asks for a filename and prints the details of that record if the
 * current user can access it.
 * Parameters: records (head of records list), current_user
 * Returns: None
 * Author: Georgia
 */
static void handle_search(const Record *records, const User *current_user) {
    char filename[MAX_FILENAME_LENGTH];
    const Record *record;

    if (!prompt_line("Filename to search for: ", filename, sizeof(filename))) {
        return;
    }
    record = search_record(records, current_user->id, filename);
    if (record == NULL) {
        printf("No file called %s is shared with you.\n", filename);
        return;
    }

    printf("Filename:    %s\n", record->filename);
    printf("Size:        %ld bytes\n", record->size);
    printf("Owner ID:    %d\n", record->owner_id);
    printf("Shared with: %d\n", record->shared_with_user_id);
    printf("Uploaded:    %d\n", record->timestamp);
}

int read_choice(Record **records, User **current_user) {
    int choice;

    if (!prompt_int("", &choice)) {
        if (feof(stdin)) {
            return 0;
        }
        return 1;
    }

    if (choice == 0) {
        return 0;
    }

    if (*current_user == NULL) {
        switch (choice) {
            case 1:
                logged_in_user = login();
                discard_line();
                if (logged_in_user.id != -1) {
                    *current_user = &logged_in_user;
                }
                break;
            case 2:
                if (create_user() != SUCCESS) {
                    printf("Could not create account.\n");
                }
                discard_line();
                break;
            default:
                printf("Invalid choice.\n");
        }
        return 1;
    }

    switch (choice) {
        case 1:
            if ((*current_user)->role == AUDITOR) {
                printf("Auditors cannot upload files.\n");
            } else {
                handle_upload(records, *current_user);
            }
            break;
        case 2:
            handle_retrieve(*records, *current_user);
            break;
        case 3:
        case 4:
            sort_records(records, (choice == 3) ? SORT_BY_NAME : SORT_BY_DATE);
            display_records(*records, (*current_user)->id,
                            (choice == 3) ? SORT_BY_NAME : SORT_BY_DATE,
                            0);
            break;
        case 5:
            handle_search(*records, *current_user);
            break;
        case 6:
            logout(*current_user);
            *current_user = NULL;
            printf("Logged out.\n");
            break;
        default:
            printf("Invalid choice.\n");
    }
    return 1;
}
