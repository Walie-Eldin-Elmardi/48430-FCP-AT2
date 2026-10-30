/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: command_line.c
 * Author: Georgia
 *
 * Runs the program in one of its command line modes. See command_line.h.
 */
#include <stdio.h>  /* printf, fprintf, fopen, fclose */
#include <string.h> /* strcmp, memset */
#include "command_line.h"
#include "custom_library.h"
#include "compression.h"
#include "encryption.h"
#include "input.h"

/* What the program does when started with a command line option */
typedef enum {
    MODE_HELP,
    MODE_COMPRESS,
    MODE_DECOMPRESS,
    MODE_ENCRYPT,
    MODE_DECRYPT
} run_mode_t;

/* A command line option, with the arguments and description shown in the
 * help text */
typedef struct {
    const char *short_flag;
    const char *long_flag;
    run_mode_t mode;
    const char *arguments;
    const char *description;
} option_t;

#define OPTION_COUNT 5

/* Every mode takes this many arguments: program, option, input, output */
#define FILE_MODE_ARGC 4

static const option_t OPTIONS[OPTION_COUNT] = {
    {"-h", "--help", MODE_HELP, "", "Show this help"},
    {"-c", "--compress", MODE_COMPRESS, "<input> <output>",
     "Compress a file with Huffman coding"},
    {"-x", "--decompress", MODE_DECOMPRESS, "<input> <output>",
     "Decompress a file made with --compress"},
    {"-e", "--encrypt", MODE_ENCRYPT, "<input> <output>",
     "Encrypt a file with ChaCha20"},
    {"-d", "--decrypt", MODE_DECRYPT, "<input> <output>",
     "Decrypt a file made with --encrypt"}
};

/*
 * Prints how to run the program and every option.
 * Parameters: name the program was run as (argv[0])
 * Returns: None
 * Author: Georgia
 */
static void print_usage(const char *program) {
    int i;

    printf("Usage: %s [option <input> <output>]\n\n", program);
    printf("With no option, starts the interactive records program.\n\n");
    printf("Options:\n");
    for (i = 0; i < OPTION_COUNT; i++) {
        printf("  %s, %-13s %-17s %s\n", OPTIONS[i].short_flag,
               OPTIONS[i].long_flag, OPTIONS[i].arguments,
               OPTIONS[i].description);
    }
    printf("\nEncrypt and decrypt ask for the password, so it is not saved "
           "in your\nshell history. Build with \"make debug\" to print "
           "debug information.\n");
}

/*
 * Finds the option matching a command line flag.
 * Parameters: flag, such as "-c" or "--compress"
 * Returns: Pointer to the option, or NULL if there is none
 * Author: Georgia
 */
static const option_t *find_option(const char *flag) {
    int i;

    for (i = 0; i < OPTION_COUNT; i++) {
        if (strcmp(flag, OPTIONS[i].short_flag) == 0 ||
            strcmp(flag, OPTIONS[i].long_flag) == 0) {
            return &OPTIONS[i];
        }
    }
    return NULL;
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
 * Asks for a password, twice when encrypting so a typo cannot lock the
 * file for good.
 * Parameters: buffer of INPUT_LENGTH characters, 1 to ask twice
 * Returns: 1 if a password was entered (and confirmed), otherwise 0
 * Author: Georgia
 */
static int prompt_password(char password[INPUT_LENGTH], int confirm) {
    char again[INPUT_LENGTH];
    int matches;

    if (!prompt_line("Password: ", password, INPUT_LENGTH)) {
        return 0;
    }
    if (!confirm) {
        return 1;
    }
    if (!prompt_line("Confirm password: ", again, INPUT_LENGTH)) {
        return 0;
    }
    matches = strcmp(password, again) == 0;
    memset(again, 0, sizeof(again));
    if (!matches) {
        printf("Passwords do not match.\n");
    }
    return matches;
}

/*
 * Carries out a file mode.
 * Parameters: mode, input filename, output filename
 * Returns: SUCCESS, or a status code saying why it failed
 * Author: Georgia
 */
static status_t run_mode(run_mode_t mode, const char *input,
                         const char *output) {
    char password[INPUT_LENGTH];
    status_t status;

    switch (mode) {
        case MODE_COMPRESS:
            return compress_file(input, output);
        case MODE_DECOMPRESS:
            return decompress_file(input, output);
        case MODE_ENCRYPT:
        case MODE_DECRYPT:
            if (!prompt_password(password, mode == MODE_ENCRYPT)) {
                return INVALID_ARGUMENT;
            }
            if (mode == MODE_ENCRYPT) {
                status = encrypt_file(input, output, password);
            } else {
                status = decrypt_file(input, output, password);
            }
            memset(password, 0, sizeof(password));
            return status;
        case MODE_HELP:
            break;
    }
    return INVALID_ARGUMENT;
}

int run_command_line(int argc, char *argv[]) {
    const option_t *option;
    status_t status;

    option = find_option(argv[1]);
    if (option == NULL) {
        fprintf(stderr, "Unknown option: %s\n\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
    DEBUG_PRINT("command line mode %s with %d argument(s)",
                option->long_flag, argc - 2);

    if (option->mode == MODE_HELP) {
        print_usage(argv[0]);
        return 0;
    }
    if (argc != FILE_MODE_ARGC) {
        fprintf(stderr, "%s needs an input file and an output file.\n",
                option->long_flag);
        fprintf(stderr, "Usage: %s %s %s\n", argv[0], option->long_flag,
                option->arguments);
        return 1;
    }
    /* Writing the output would wipe the input before it was read */
    if (strcmp(argv[2], argv[3]) == 0) {
        fprintf(stderr, "The input and output files must be different.\n");
        return 1;
    }
    /* Also catches the same file named two ways, such as a.txt and
     * ./a.txt, which ANSI C has no way to compare */
    if (file_exists(argv[3]) &&
        !prompt_yes_no("The output file already exists. Overwrite it? "
                       "(y/n): ")) {
        fprintf(stderr, "Cancelled.\n");
        return 1;
    }

    status = run_mode(option->mode, argv[2], argv[3]);
    if (status != SUCCESS) {
        fprintf(stderr, "%s failed: %s.\n", option->long_flag + 2,
                status_message(status));
        return 1;
    }
    printf("Wrote %s.\n", argv[3]);
    return 0;
}
