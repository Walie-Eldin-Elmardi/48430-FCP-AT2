/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: input.h
 * Author: Georgia
 *
 * Reads and checks everything the user types at the console. Every prompt
 * reads one whole line, so leftover input never spills into the next
 * prompt.
 */
#ifndef INPUT_H
#define INPUT_H

/* Size of the buffers used for typed input, including the '\0' */
#define INPUT_LENGTH 256

/*
 * Discards everything left on the current line of standard input.
 * Parameters: None
 * Returns: None
 * Author: Georgia
 */
void discard_line(void);

/*
 * Prints a prompt and reads one non-empty line from standard input,
 * without the trailing newline. Prints a message if the line is empty or
 * does not fit in the buffer.
 * Parameters: prompt to print, buffer to fill, size of buffer
 * Returns: 1 if a line was read, otherwise 0
 * Author: Georgia
 */
int prompt_line(const char *prompt, char *buffer, int size);

/*
 * Prints a prompt and reads a whole number. Rejects anything that is not
 * a number, has extra characters after it, or does not fit in an int.
 * Parameters: prompt to print, pointer to store the number
 * Returns: 1 if a valid number was read, otherwise 0
 * Author: Georgia
 */
int prompt_int(const char *prompt, int *value);

/*
 * Prints a prompt and reads the answer to a yes or no question.
 * Parameters: prompt to print
 * Returns: 1 if the answer starts with y or Y, otherwise 0
 * Author: Georgia
 */
int prompt_yes_no(const char *prompt);

/*
 * Checks whether a string contains any whitespace. Names saved to the
 * data files are separated by spaces, so they cannot contain any.
 * Parameters: string to check
 * Returns: 1 if the string contains whitespace, otherwise 0
 * Author: Georgia
 */
int has_whitespace(const char *str);

/*
 * Checks whether standard input has ended (Ctrl-D, or the end of a file
 * piped into the program).
 * Parameters: None
 * Returns: 1 if there is no more input, otherwise 0
 * Author: Georgia
 */
int input_ended(void);

#endif /* INPUT_H */
