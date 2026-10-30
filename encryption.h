/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: encryption.h
 * Authors: Wally (design), Georgia
 *
 * Encrypts and decrypts whole files with ChaCha20, using a key derived
 * from a password.
 */
#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include "custom_library.h"

#define CHACHA_KEY_LENGTH 32
#define CHACHA_NONCE_LENGTH 12
#define CHACHA_BLOCK_SIZE 64

/*
 * Derives a 256-bit encryption key from a password.
 * Parameters: password, key to fill in
 * Returns: None
 * Author: Wally (implemented by Georgia)
 */
void derive_key(const char *password, unsigned char key[CHACHA_KEY_LENGTH]);
/*
 * Generates a nonce that is different for every call. rand() must have
 * been seeded with srand first (main does this).
 * Parameters: nonce to fill in, seed mixed into the nonce
 * Returns: None
 * Author: Wally (implemented by Georgia)
 */
void generate_nonce(unsigned char nonce[CHACHA_NONCE_LENGTH],
                    unsigned int seed);
/*
 * Encrypts a file with a password. If encrypting fails, no output file is
 * left behind.
 * Parameters: input filename, output filename, password
 * Returns: SUCCESS, or a status code saying why it failed
 * Author: Wally (implemented by Georgia)
 */
status_t encrypt_file(const char *input_filename, const char *output_filename,
                      const char *password);
/*
 * Decrypts a file written by encrypt_file. If decrypting fails, no output
 * file is left behind.
 * Parameters: input filename, output filename, password
 * Returns: SUCCESS, INVALID_AUTHENTICATION if the password is wrong, or
 *          another status code saying why it failed
 * Author: Wally (implemented by Georgia)
 */
status_t decrypt_file(const char *input_filename, const char *output_filename,
                      const char *password);

#endif /* ENCRYPTION_H */
