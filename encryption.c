/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: encryption.c
 * Authors: Wally (design), Georgia
 *
 * ChaCha20 encryption (RFC 8439) with a key derived from the file's
 * password.
 *
 * Encrypted file layout:
 *   12 bytes   nonce (random per file, stored in the clear)
 *   rest       ChaCha20-encrypted stream of: 8-byte check value, then the
 *              file
 * The check value lets decrypt_file tell a wrong password from a right one.
 */
#include <stdio.h>  /* fopen, fclose, fread, fwrite, ferror, remove */
#include <stdlib.h> /* rand */
#include <string.h> /* strlen, memcpy, memset, memcmp */
#include <time.h>   /* time, clock */
#include "encryption.h"

#define CHECK_LENGTH 8
#define KDF_ROUNDS 1000

static const unsigned char CHECK_VALUE[CHECK_LENGTH] = {
    '4', '8', '4', '3', '0', 'O', 'K', '!'
};

/* ANSI C has no fixed-size types, so keep 32-bit values in unsigned long
 * and mask */
#define U32(x) ((x) & 0xffffffffUL)
#define ROTL(v, n) U32(((v) << (n)) | ((v) >> (32 - (n))))
#define QUARTER_ROUND(a, b, c, d) \
    a = U32(a + b); d = ROTL(d ^ a, 16); \
    c = U32(c + d); b = ROTL(b ^ c, 12); \
    a = U32(a + b); d = ROTL(d ^ a, 8);  \
    c = U32(c + d); b = ROTL(b ^ c, 7)

/* Keeps track of where we are in the keystream while encrypting a file */
typedef struct {
    unsigned char key[CHACHA_KEY_LENGTH];
    unsigned char nonce[CHACHA_NONCE_LENGTH];
    unsigned long counter;
    unsigned char block[CHACHA_BLOCK_SIZE];
    int position; /* next unused byte in block */
} key_stream_t;

/*
 * Reads 4 little-endian bytes as a 32-bit number.
 * Parameters: pointer to the bytes
 * Returns: The number
 * Author: Georgia
 */
static unsigned long load32(const unsigned char *p) {
    return (unsigned long)p[0] | ((unsigned long)p[1] << 8) |
           ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);
}

/*
 * Produces one 64-byte ChaCha20 keystream block (RFC 8439 section 2.3).
 * Parameters: key, nonce, block counter, output block
 * Returns: None
 * Author: Georgia
 */
static void chacha20_block(const unsigned char key[CHACHA_KEY_LENGTH],
                           const unsigned char nonce[CHACHA_NONCE_LENGTH],
                           unsigned long counter,
                           unsigned char out[CHACHA_BLOCK_SIZE]) {
    unsigned long state[16];
    unsigned long x[16];
    unsigned long v;
    int i;

    /* "expand 32-byte k" */
    state[0] = 0x61707865UL;
    state[1] = 0x3320646eUL;
    state[2] = 0x79622d32UL;
    state[3] = 0x6b206574UL;
    for (i = 0; i < 8; i++) {
        state[4 + i] = load32(key + 4 * i);
    }
    state[12] = U32(counter);
    for (i = 0; i < 3; i++) {
        state[13 + i] = load32(nonce + 4 * i);
    }

    for (i = 0; i < 16; i++) {
        x[i] = state[i];
    }
    /* 20 rounds: 10 x (column round + diagonal round) */
    for (i = 0; i < 10; i++) {
        QUARTER_ROUND(x[0], x[4], x[8], x[12]);
        QUARTER_ROUND(x[1], x[5], x[9], x[13]);
        QUARTER_ROUND(x[2], x[6], x[10], x[14]);
        QUARTER_ROUND(x[3], x[7], x[11], x[15]);
        QUARTER_ROUND(x[0], x[5], x[10], x[15]);
        QUARTER_ROUND(x[1], x[6], x[11], x[12]);
        QUARTER_ROUND(x[2], x[7], x[8], x[13]);
        QUARTER_ROUND(x[3], x[4], x[9], x[14]);
    }

    for (i = 0; i < 16; i++) {
        v = U32(x[i] + state[i]);
        out[4 * i] = (unsigned char)(v & 0xff);
        out[4 * i + 1] = (unsigned char)((v >> 8) & 0xff);
        out[4 * i + 2] = (unsigned char)((v >> 16) & 0xff);
        out[4 * i + 3] = (unsigned char)((v >> 24) & 0xff);
    }
}

/*
 * XORs a buffer with the next bytes of the keystream (encrypts or
 * decrypts).
 * Parameters: keystream, buffer, length of buffer
 * Returns: None
 * Author: Georgia
 */
static void keystream_xor(key_stream_t *ks, unsigned char *buffer,
                          size_t length) {
    size_t i;

    for (i = 0; i < length; i++) {
        if (ks->position == CHACHA_BLOCK_SIZE) {
            chacha20_block(ks->key, ks->nonce, ks->counter++, ks->block);
            ks->position = 0;
        }
        buffer[i] ^= ks->block[ks->position++];
    }
}

/*
 * Sets up a keystream from a password and nonce. The counter starts at 1
 * as in RFC 8439.
 * Parameters: keystream to set up, password, nonce
 * Returns: None
 * Author: Georgia
 */
static void keystream_init(key_stream_t *ks, const char *password,
                           const unsigned char nonce[CHACHA_NONCE_LENGTH]) {
    derive_key(password, ks->key);
    memcpy(ks->nonce, nonce, CHACHA_NONCE_LENGTH);
    ks->counter = 1;
    ks->position = CHACHA_BLOCK_SIZE;
}

/*
 * Note: a real program would use a proper key derivation function such as
 * PBKDF2 or Argon2. Without a crypto library we mix the password into the
 * key with the ChaCha20 block function and repeat it to slow down guessing.
 */
void derive_key(const char *password, unsigned char key[CHACHA_KEY_LENGTH]) {
    unsigned char nonce[CHACHA_NONCE_LENGTH];
    unsigned char block[CHACHA_BLOCK_SIZE];
    size_t length = strlen(password);
    size_t i;
    int round;

    memset(key, 0, CHACHA_KEY_LENGTH);
    memset(nonce, 0, CHACHA_NONCE_LENGTH);

    /* Absorb the password 32 bytes at a time, plus its length */
    for (i = 0; i < length; i++) {
        key[i % CHACHA_KEY_LENGTH] ^= (unsigned char)password[i];
        if (i % CHACHA_KEY_LENGTH == CHACHA_KEY_LENGTH - 1 ||
            i == length - 1) {
            chacha20_block(key, nonce, (unsigned long)(i / CHACHA_KEY_LENGTH),
                           block);
            memcpy(key, block, CHACHA_KEY_LENGTH);
        }
    }
    nonce[0] = (unsigned char)(length & 0xff);
    nonce[1] = (unsigned char)((length >> 8) & 0xff);

    for (round = 0; round < KDF_ROUNDS; round++) {
        chacha20_block(key, nonce, (unsigned long)round, block);
        memcpy(key, block, CHACHA_KEY_LENGTH);
    }
    memset(block, 0, sizeof(block));
}

/*
 * The nonce only has to be different for every file encrypted with the
 * same password, so it combines the seed, a count of nonces made this run,
 * and random bytes. rand() is seeded once in main, not here, so that
 * seeding it again cannot repeat an earlier nonce.
 */
void generate_nonce(unsigned char nonce[CHACHA_NONCE_LENGTH],
                    unsigned int seed) {
    static unsigned long calls = 0;
    int i;

    calls++;
    for (i = 0; i < 4; i++) {
        nonce[i] = (unsigned char)((seed >> (8 * i)) & 0xff);
        nonce[4 + i] = (unsigned char)((calls >> (8 * i)) & 0xff);
    }
    for (i = 8; i < CHACHA_NONCE_LENGTH; i++) {
        nonce[i] = (unsigned char)(rand() & 0xff);
    }
}

/*
 * Closes both files and works out the final status. If anything failed,
 * the output file is deleted so no half-written file is left behind.
 * Parameters: input file, output file, output filename, status so far
 * Returns: Final status
 * Author: Georgia
 */
static status_t finish_files(FILE *in, FILE *out, const char *output_filename,
                             status_t status) {
    if (status == SUCCESS && (ferror(in) || ferror(out))) {
        status = FILE_NOT_FOUND;
    }
    fclose(in);
    if (fclose(out) != 0 && status == SUCCESS) {
        status = FILE_NOT_FOUND;
    }
    if (status != SUCCESS) {
        remove(output_filename);
    }
    return status;
}

status_t encrypt_file(const char *input_filename, const char *output_filename,
                      const char *password) {
    unsigned char nonce[CHACHA_NONCE_LENGTH];
    unsigned char buffer[CHACHA_BLOCK_SIZE];
    key_stream_t ks;
    FILE *in;
    FILE *out;
    size_t n;
    status_t status;

    if (input_filename == NULL || output_filename == NULL ||
        password == NULL) {
        return INVALID_ARGUMENT;
    }

    in = fopen(input_filename, "rb");
    if (in == NULL) {
        return FILE_NOT_FOUND;
    }
    out = fopen(output_filename, "wb");
    if (out == NULL) {
        fclose(in);
        return FILE_NOT_FOUND;
    }

    /* A fresh nonce per file means the same password never reuses a
     * keystream */
    generate_nonce(nonce, (unsigned int)time(NULL) ^ (unsigned int)clock());
    if (fwrite(nonce, 1, CHACHA_NONCE_LENGTH, out) != CHACHA_NONCE_LENGTH) {
        return finish_files(in, out, output_filename, FILE_NOT_FOUND);
    }
    keystream_init(&ks, password, nonce);

    memcpy(buffer, CHECK_VALUE, CHECK_LENGTH);
    keystream_xor(&ks, buffer, CHECK_LENGTH);
    fwrite(buffer, 1, CHECK_LENGTH, out);

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        keystream_xor(&ks, buffer, n);
        fwrite(buffer, 1, n, out);
    }

    status = finish_files(in, out, output_filename, SUCCESS);
    DEBUG_PRINT("encrypted %s to %s using %lu keystream block(s)",
                input_filename, output_filename, ks.counter - 1);
    memset(&ks, 0, sizeof(ks));
    return status;
}

status_t decrypt_file(const char *input_filename, const char *output_filename,
                      const char *password) {
    unsigned char nonce[CHACHA_NONCE_LENGTH];
    unsigned char buffer[CHACHA_BLOCK_SIZE];
    key_stream_t ks;
    FILE *in;
    FILE *out;
    size_t n;
    status_t status;

    if (input_filename == NULL || output_filename == NULL ||
        password == NULL) {
        return INVALID_ARGUMENT;
    }

    in = fopen(input_filename, "rb");
    if (in == NULL) {
        return FILE_NOT_FOUND;
    }

    if (fread(nonce, 1, CHACHA_NONCE_LENGTH, in) != CHACHA_NONCE_LENGTH ||
        fread(buffer, 1, CHECK_LENGTH, in) != CHECK_LENGTH) {
        DEBUG_PRINT("%s is too short to be an encrypted file",
                    input_filename);
        fclose(in);
        return INVALID_ARGUMENT;
    }

    /* Check the password before creating the output file */
    keystream_init(&ks, password, nonce);
    keystream_xor(&ks, buffer, CHECK_LENGTH);
    if (memcmp(buffer, CHECK_VALUE, CHECK_LENGTH) != 0) {
        DEBUG_PRINT("check value of %s did not match", input_filename);
        memset(&ks, 0, sizeof(ks));
        fclose(in);
        return INVALID_AUTHENTICATION;
    }

    out = fopen(output_filename, "wb");
    if (out == NULL) {
        memset(&ks, 0, sizeof(ks));
        fclose(in);
        return FILE_NOT_FOUND;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        keystream_xor(&ks, buffer, n);
        fwrite(buffer, 1, n, out);
    }

    status = finish_files(in, out, output_filename, SUCCESS);
    memset(&ks, 0, sizeof(ks));

    DEBUG_PRINT("decrypted %s to %s", input_filename, output_filename);
    return status;
}
