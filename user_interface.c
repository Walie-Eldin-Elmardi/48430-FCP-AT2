/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: user_interface.c
 * Author: Georgia
 *
 * The main menu, and the upload, retrieve, display and search options it
 * offers. See user_interface.h for the public functions.
 */
#include <stdio.h>  /* printf, fopen, fclose, fgetc, ferror, fseek, ftell,
                       sprintf, remove */
#include <string.h> /* strrchr, strlen, strcmp, memset */
#include "user_interface.h"
#include "input.h"
#include "compression.h"
#include "encryption.h"

/* Holds the compressed file between compressing and encrypting it */
#define TEMP_FILE "transfer.tmp"

/* Options on the menu shown when nobody is logged in */
typedef enum {
    START_QUIT = 0,
    START_LOGIN = 1,
    START_CREATE_ACCOUNT = 2
} start_option_t;

/* Options on the menu shown to a logged-in user. Auditors cannot upload,
 * so their menu starts at MENU_RETRIEVE, and each number shown to them is
 * one less than the option (see menu_offset). */
typedef enum {
    MENU_QUIT = 0,
    MENU_UPLOAD,
    MENU_RETRIEVE,
    MENU_LIST_BY_NAME,
    MENU_LIST_BY_DATE,
    MENU_SEARCH,
    MENU_LOGOUT
} menu_option_t;

/* Text shown for each menu_option_t, in the same order */
static const char *const MENU_LABELS[] = {
    "Quit",
    "Upload a file",
    "Retrieve a file",
    "Display records sorted by filename",
    "Display records sorted by upload date",
    "Search records by filename",
    "Log out"
};

/*
 * Gets how much smaller the numbers shown on the menu are than the menu
 * options, since auditors have no upload option.
 * Parameters: current user
 * Returns: 1 for auditors, otherwise 0
 * Author: Georgia
 */
static int menu_offset(const user_t *current_user) {
    return (current_user->role == AUDITOR) ? 1 : 0;
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
 * Returns: Size in bytes, or -1 if the file could not be opened or read
 * Author: Georgia
 */
static long file_size(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    long size;

    if (fp == NULL) {
        return -1;
    }
    /* Some systems let fopen open a folder; reading from it then fails */
    if (fgetc(fp) == EOF && ferror(fp)) {
        fclose(fp);
        return -1;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    size = ftell(fp);
    fclose(fp);
    return size;
}

/*
 * Gets the user ID to search records with. Auditors can see every record;
 * everyone else only sees records they own or that are shared with them.
 * Parameters: current user
 * Returns: ANY_USER for auditors, otherwise the user's ID
 * Author: Georgia
 */
static int visible_user_id(const user_t *current_user) {
    return (current_user->role == AUDITOR) ? ANY_USER : current_user->id;
}

/*
 * Returns a printable name for a user role.
 * Parameters: role
 * Returns: Name of the role, or "Unknown" if the role is not recognised
 * Author: Georgia
 */
static const char *role_name(user_role_t role) {
    switch (role) {
        case ACCOUNTANT: return "Accountant";
        case CLIENT: return "Client";
        case AUDITOR: return "Auditor";
    }
    return "Unknown";
}

/*
 * Checks whether a user has already uploaded a file with this name. Each
 * user's files are stored as <user ID>_<filename>.enc, so the name must be
 * unique among their own uploads.
 * Parameters: records list, owner's user ID, filename
 * Returns: 1 if they have, otherwise 0
 * Author: Georgia
 */
static int has_uploaded(const record_t *records, int owner_id,
                        const char *filename) {
    const record_t *current;

    for (current = records; current != NULL; current = current->next) {
        if (current->owner_id == owner_id &&
            strcmp(current->filename, filename) == 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * Checks that retrieving a file to this path will not overwrite one of
 * the program's own files: the users file, the records file, the
 * temporary file, or a stored encrypted file. Only the filename part is
 * compared, so "./1_notes.txt.enc" is caught too.
 * Parameters: records list, path the user wants to save to
 * Returns: 1 if the path is safe to write, otherwise 0
 * Author: Georgia
 */
static int is_safe_output_path(const record_t *records, const char *path) {
    const char *name = base_name(path);
    const record_t *current;
    int reserved;

    if (name[0] == '\0') {
        printf("Please give a file name, not a folder.\n");
        return 0;
    }

    reserved = strcmp(name, USERS_FILE) == 0 ||
               strcmp(name, RECORDS_FILE) == 0 ||
               strcmp(name, TEMP_FILE) == 0;
    for (current = records; current != NULL && !reserved;
         current = current->next) {
        reserved = strcmp(name, base_name(current->path)) == 0;
    }

    if (reserved) {
        printf("%s is used by this program to store data. "
               "Please choose another name.\n", name);
        return 0;
    }
    return 1;
}

/*
 * Checks whether a file exists (can be opened for reading).
 * Parameters: filename
 * Returns: 1 if it exists, otherwise 0
 * Author: Georgia
 */
static int file_exists(const char *filename) {
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

/*
 * Asks for a filename and finds the matching record the current user can
 * see. Different users can upload files with the same name, so if more
 * than one matches, lists them and asks for the record ID.
 * Parameters: records list, current user, prompt for the filename
 * Returns: The chosen record, or NULL if there is none or input was
 *          invalid
 * Author: Georgia
 */
static const record_t *choose_record(const record_t *records,
                                     const user_t *current_user,
                                     const char *prompt) {
    char filename[MAX_FILENAME_LENGTH];
    const record_t *record;
    int user_id = visible_user_id(current_user);
    int matches;
    int record_id;

    if (!prompt_line(prompt, filename, sizeof(filename))) {
        return NULL;
    }

    matches = count_matching_records(records, user_id, filename);
    DEBUG_PRINT("%d record(s) called %s visible to user %d", matches,
                filename, current_user->id);
    if (matches == 0) {
        printf("No file called %s is shared with you.\n", filename);
        return NULL;
    }
    if (matches == 1) {
        return search_record(records, user_id, filename);
    }

    printf("There are %d files called %s:\n", matches, filename);
    display_records(records, user_id, filename);
    if (!prompt_int("ID of the file you want: ", &record_id)) {
        return NULL;
    }
    record = find_record_by_id(records, user_id, record_id);
    if (record == NULL || strcmp(record->filename, filename) != 0) {
        printf("None of the files called %s has ID %d.\n", filename,
               record_id);
        return NULL;
    }
    return record;
}

/*
 * Asks for a password for a file twice and checks both match.
 * Parameters: buffer of INPUT_LENGTH characters to store the password
 * Returns: 1 if a password was entered and confirmed, otherwise 0
 * Author: Georgia
 */
static int prompt_file_password(char password[INPUT_LENGTH]) {
    char confirm[INPUT_LENGTH];
    int matches;

    if (!prompt_line("Password for this file: ", password, INPUT_LENGTH) ||
        !prompt_line("Confirm password: ", confirm, INPUT_LENGTH)) {
        return 0;
    }
    matches = strcmp(password, confirm) == 0;
    memset(confirm, 0, sizeof(confirm));
    if (!matches) {
        printf("Passwords do not match.\n");
    }
    return matches;
}

/*
 * Asks for a file to upload and who to share it with, then compresses and
 * encrypts it into storage, adds a record for it and saves the records.
 * Parameters: pointer to head of records list, user list, current user
 * Returns: None
 * Author: Georgia
 */
static void handle_upload(record_t **records, const user_list_t *users,
                          const user_t *current_user) {
    char input_path[MAX_PATH_LENGTH];
    char stored_path[MAX_PATH_LENGTH];
    char username[INPUT_LENGTH];
    char password[INPUT_LENGTH];
    const char *filename;
    const user_t *share_with;
    long size;
    status_t status;

    if (!prompt_line("Path of file to upload: ", input_path,
                     sizeof(input_path))) {
        return;
    }
    filename = base_name(input_path);
    if (filename[0] == '\0') {
        printf("Please give the path of a file, not a folder.\n");
        return;
    }
    if (strlen(filename) >= MAX_FILENAME_LENGTH) {
        printf("Filename is too long.\n");
        return;
    }
    if (has_whitespace(filename)) {
        printf("Filenames cannot contain spaces.\n");
        return;
    }
    size = file_size(input_path);
    if (size < 0) {
        printf("Could not open %s.\n", input_path);
        return;
    }
    if (has_uploaded(*records, current_user->id, filename)) {
        printf("You have already uploaded a file called %s.\n", filename);
        return;
    }

    if (!prompt_line("Username to share with: ", username,
                     sizeof(username))) {
        return;
    }
    share_with = find_user_by_name(users, username);
    if (share_with == NULL) {
        printf("There is no user called %s.\n", username);
        return;
    }

    printf("The file is locked with its own password. Give it to %s so "
           "they can open it.\n", share_with->username);
    if (!prompt_file_password(password)) {
        return;
    }

    sprintf(stored_path, "%d_%s.enc", current_user->id, filename);

    status = compress_file(input_path, TEMP_FILE);
    if (status == SUCCESS) {
        status = encrypt_file(TEMP_FILE, stored_path, password);
    }
    remove(TEMP_FILE);
    memset(password, 0, sizeof(password));
    if (status == SUCCESS) {
        status = add_record(records, current_user->id, share_with->id,
                            filename, size, stored_path);
    }

    if (status != SUCCESS) {
        printf("Upload failed: %s.\n", status_message(status));
        return;
    }
    printf("Uploaded %s and shared it with %s.\n", filename,
           share_with->username);

    /* Save now, so the record is not lost if the program is killed */
    if (save_records(RECORDS_FILE, *records) != SUCCESS) {
        printf("Warning: could not save records to %s.\n", RECORDS_FILE);
    }
}

/*
 * Asks which file to retrieve and where to save it, then decrypts and
 * decompresses it if the current user is allowed to access it.
 * Parameters: records list, current user
 * Returns: None
 * Author: Georgia
 */
static void handle_retrieve(const record_t *records,
                            const user_t *current_user) {
    char output_path[MAX_PATH_LENGTH];
    char password[INPUT_LENGTH];
    const record_t *record;
    status_t status;

    record = choose_record(records, current_user, "Filename to retrieve: ");
    if (record == NULL) {
        return;
    }
    if (!prompt_line("Save as: ", output_path, sizeof(output_path))) {
        return;
    }
    if (!is_safe_output_path(records, output_path)) {
        return;
    }
    if (file_exists(output_path) &&
        !prompt_yes_no("That file already exists. Overwrite it? (y/n): ")) {
        printf("Retrieve cancelled.\n");
        return;
    }
    if (!prompt_line("Password for this file: ", password,
                     sizeof(password))) {
        return;
    }

    status = decrypt_file(record->path, TEMP_FILE, password);
    memset(password, 0, sizeof(password));
    if (status == SUCCESS) {
        status = decompress_file(TEMP_FILE, output_path);
    }
    remove(TEMP_FILE);

    if (status == SUCCESS) {
        printf("Saved %s to %s.\n", record->filename, output_path);
    } else {
        printf("Retrieve failed: %s.\n", status_message(status));
    }
}

/*
 * Asks for a filename and prints the details of that record if the
 * current user can access it.
 * Parameters: records list, user list, current user
 * Returns: None
 * Author: Georgia
 */
static void handle_search(const record_t *records, const user_list_t *users,
                          const user_t *current_user) {
    char uploaded[TIMESTAMP_LENGTH];
    const record_t *record;
    const user_t *owner;
    const user_t *shared_with;

    record = choose_record(records, current_user, "Filename to search for: ");
    if (record == NULL) {
        return;
    }

    owner = find_user_by_id(users, record->owner_id);
    shared_with = find_user_by_id(users, record->shared_with_user_id);
    format_timestamp(record->timestamp, uploaded);

    printf("Record ID:   %d\n", record->id);
    printf("Filename:    %s\n", record->filename);
    printf("Size:        %ld bytes\n", record->size);
    printf("Owner:       %s (user ID %d)\n",
           owner != NULL ? owner->username : "unknown", record->owner_id);
    printf("Shared with: %s (user ID %d)\n",
           shared_with != NULL ? shared_with->username : "unknown",
           record->shared_with_user_id);
    printf("Uploaded:    %s\n", uploaded);
}

/*
 * Sorts the records and shows the ones the current user can see.
 * Parameters: pointer to head of records list, current user, sort option
 * Returns: None
 * Author: Georgia
 */
static void handle_display(record_t **records, const user_t *current_user,
                           sort_option_t sort_option) {
    sort_records(records, sort_option);
    DEBUG_PRINT("sorted records by %s",
                sort_option == SORT_BY_NAME ? "filename" : "upload date");
    display_records(*records, visible_user_id(current_user), NULL);
}

/*
 * Carries out a choice from the menu shown when nobody is logged in.
 * Parameters: choice, user list, current user (set on login)
 * Returns: None
 * Author: Georgia
 */
static void handle_start_choice(int choice, user_list_t *users,
                                user_t *current_user) {
    switch (choice) {
        case START_LOGIN:
            *current_user = login(users);
            if (current_user->id == NO_USER_ID) {
                return;
            }
            break;
        case START_CREATE_ACCOUNT:
            if (create_user(users) != SUCCESS) {
                printf("Could not create account.\n");
                return;
            }
            break;
        default:
            printf("Invalid choice.\n");
            return;
    }

    /* Save now, so a new account or upgraded password is not lost if the
     * program is killed */
    if (save_users(USERS_FILE, users) != SUCCESS) {
        printf("Warning: could not save users to %s.\n", USERS_FILE);
    }
}

void print_menu(const user_t *current_user) {
    int option;
    int offset;

    printf("\n");

    if (current_user == NULL || current_user->id == NO_USER_ID) {
        printf("%d. Log in\n", START_LOGIN);
        printf("%d. Create account\n", START_CREATE_ACCOUNT);
        printf("%d. Quit\n", START_QUIT);
        printf("Enter choice: ");
        return;
    }

    printf("Logged in as %s (%s, user ID %d)\n", current_user->username,
           role_name(current_user->role), current_user->id);

    offset = menu_offset(current_user);
    for (option = MENU_UPLOAD + offset; option <= MENU_LOGOUT; option++) {
        printf("%d. %s\n", option - offset, MENU_LABELS[option]);
    }
    printf("%d. %s\n", MENU_QUIT, MENU_LABELS[MENU_QUIT]);
    printf("Enter choice: ");
}

int read_choice(record_t **records, user_list_t *users,
                user_t *current_user) {
    int choice;

    if (!prompt_int("", &choice)) {
        /* Quit at the end of input, otherwise show the menu again */
        return !input_ended();
    }
    DEBUG_PRINT("menu choice %d", choice);

    if (choice == MENU_QUIT) {
        return 0;
    }
    if (current_user->id == NO_USER_ID) {
        handle_start_choice(choice, users, current_user);
        return 1;
    }

    /* Turn the number shown into a menu option. Choices outside the
     * menu, such as an auditor's 0 + 1 = MENU_UPLOAD, are caught first. */
    if (choice < 1 || choice > MENU_LOGOUT - menu_offset(current_user)) {
        printf("Invalid choice.\n");
        return 1;
    }
    choice += menu_offset(current_user);

    switch (choice) {
        case MENU_UPLOAD:
            handle_upload(records, users, current_user);
            break;
        case MENU_RETRIEVE:
            handle_retrieve(*records, current_user);
            break;
        case MENU_LIST_BY_NAME:
            handle_display(records, current_user, SORT_BY_NAME);
            break;
        case MENU_LIST_BY_DATE:
            handle_display(records, current_user, SORT_BY_DATE);
            break;
        case MENU_SEARCH:
            handle_search(*records, users, current_user);
            break;
        case MENU_LOGOUT:
            logout(current_user);
            printf("Logged out.\n");
            break;
        default:
            printf("Invalid choice.\n");
    }
    return 1;
}
