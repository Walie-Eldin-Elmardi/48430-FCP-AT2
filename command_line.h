/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: command_line.h
 * Author: Georgia
 *
 * The command line modes, which compress, decompress, encrypt or decrypt
 * a single file without logging in. Run "./main --help" to list them.
 */
#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

/*
 * Runs the mode chosen by the command line arguments, for example
 * "./main --compress notes.txt notes.huf". Only call it when there is at
 * least one argument; with none, the program runs interactively.
 * Parameters: argument count and arguments, as passed to main
 * Returns: Exit code for main: 0 on success, 1 on failure
 * Author: Georgia
 */
int run_command_line(int argc, char *argv[]);

#endif /* COMMAND_LINE_H */
