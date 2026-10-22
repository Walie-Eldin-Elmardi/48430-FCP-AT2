#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include "custom_library.h"

#define CHACHA_KEY_LENGTH 32
#define CHACHA_NONCE_LENGTH 12
#define CHACHA_BLOCK_SIZE 64

/* 
 * Derives a key from a password.
 * Parameters: password , key
 * Returns: None
 * Author: Wally (implemented by Georgia)
 */
void derive_key(const char *password, unsigned char key[CHACHA_KEY_LENGTH]);
/* 
 * Generates a nonce that is different for every call. rand() must have been
 * seeded with srand first (main does this).
 * Parameters: nonce, seed
 * Returns: None
 * Author: Wally (implemented by Georgia)
 */
void generate_nonce(unsigned char nonce[CHACHA_NONCE_LENGTH], unsigned int seed);
/* 
 * Encrypts a file.
 * Parameters: input_filename, output_filename, password
 * Returns: Status code indicating success or failure
 * Author: Wally (implemented by Georgia)
 */
Status encrypt_file(const char *input_filename, const char *output_filename, const char *password);
/* 
 * Decrypts a file. If decrypting fails, no output file is left behind.
 * Parameters: input_filename, output_filename (const char *), password (const char *)
 * Returns: SUCCESS, INVALID_AUTHENTICATION if the password is wrong, or another Status on failure
 * Author: Wally (implemented by Georgia)
 */
Status decrypt_file(const char *input_filename, const char *output_filename, const char *password);

#endif
