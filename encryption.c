#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "encryption.h"

/*
 * Encrypted file layout:
 *   12 bytes   nonce (random per file, stored in the clear)
 *   rest       ChaCha20-encrypted stream of: 8-byte check value, then the file
 * The check value lets decrypt_file tell a wrong password from a right one.
 */

#define CHECK_LENGTH 8
#define KDF_ROUNDS 1000

static const unsigned char CHECK_VALUE[CHECK_LENGTH] = {'4', '8', '4', '3', '0', 'O', 'K', '!'};

/* ANSI C has no fixed-size types, so keep 32-bit values in unsigned long and mask */
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
} KeyStream;

/*
 * Reads 4 little-endian bytes as a 32-bit number.
 * Parameters: pointer to the bytes
 * Returns: The number
 * Author: Wally
 */
static unsigned long load32(const unsigned char *p) {
    return (unsigned long)p[0] | ((unsigned long)p[1] << 8) |
           ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);
}

/*
 * Produces one 64-byte ChaCha20 keystream block (RFC 8439 section 2.3).
 * Parameters: key, nonce, block counter, output block
 * Returns: None
 * Author: Wally
 */
static void chacha20_block(const unsigned char key[CHACHA_KEY_LENGTH],
                           const unsigned char nonce[CHACHA_NONCE_LENGTH],
                           unsigned long counter,
                           unsigned char out[CHACHA_BLOCK_SIZE]) {
    unsigned long state[16];
    unsigned long x[16];
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
        unsigned long v = U32(x[i] + state[i]);

        out[4 * i] = (unsigned char)(v & 0xff);
        out[4 * i + 1] = (unsigned char)((v >> 8) & 0xff);
        out[4 * i + 2] = (unsigned char)((v >> 16) & 0xff);
        out[4 * i + 3] = (unsigned char)((v >> 24) & 0xff);
    }
}

/*
 * XORs a buffer with the next bytes of the keystream (encrypts or decrypts).
 * Parameters: keystream, buffer, length of buffer
 * Returns: None
 * Author: Wally
 */
static void keystream_xor(KeyStream *ks, unsigned char *buffer, size_t length) {
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
 * Sets up a keystream from a password and nonce. Counter starts at 1 as in RFC 8439.
 * Parameters: keystream to set up, password, nonce
 * Returns: None
 * Author: Wally
 */
static void keystream_init(KeyStream *ks, const char *password,
                           const unsigned char nonce[CHACHA_NONCE_LENGTH]) {
    derive_key(password, ks->key);
    memcpy(ks->nonce, nonce, CHACHA_NONCE_LENGTH);
    ks->counter = 1;
    ks->position = CHACHA_BLOCK_SIZE;
}

/*
 * Note: a real program would use a proper key derivation function such as
 * PBKDF2 or Argon2. Without a crypto library we mix the password into the key
 * with the ChaCha20 block function and repeat it to slow down guessing.
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
        if (i % CHACHA_KEY_LENGTH == CHACHA_KEY_LENGTH - 1 || i == length - 1) {
            chacha20_block(key, nonce, (unsigned long)(i / CHACHA_KEY_LENGTH), block);
            memcpy(key, block, CHACHA_KEY_LENGTH);
        }
    }
    nonce[0] = (unsigned char)(length & 0xff);
    nonce[1] = (unsigned char)((length >> 8) & 0xff);

    for (round = 0; round < KDF_ROUNDS; round++) {
        chacha20_block(key, nonce, (unsigned long)round, block);
        memcpy(key, block, CHACHA_KEY_LENGTH);
    }
}

void generate_nonce(unsigned char nonce[CHACHA_NONCE_LENGTH], unsigned int seed) {
    int i;

    srand(seed);
    for (i = 0; i < CHACHA_NONCE_LENGTH; i++) {
        nonce[i] = (unsigned char)(rand() & 0xff);
    }
}

Status encrypt_file(const char *input_filename, const char *output_filename, const char *password) {
    unsigned char nonce[CHACHA_NONCE_LENGTH];
    unsigned char buffer[CHACHA_BLOCK_SIZE];
    KeyStream ks;
    FILE *in;
    FILE *out;
    size_t n;
    Status status;

    if (input_filename == NULL || output_filename == NULL || password == NULL) {
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

    /* A fresh nonce per file means the same password never reuses a keystream */
    generate_nonce(nonce, (unsigned int)time(NULL) ^ (unsigned int)clock());
    fwrite(nonce, 1, CHACHA_NONCE_LENGTH, out);
    keystream_init(&ks, password, nonce);

    memcpy(buffer, CHECK_VALUE, CHECK_LENGTH);
    keystream_xor(&ks, buffer, CHECK_LENGTH);
    fwrite(buffer, 1, CHECK_LENGTH, out);

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        keystream_xor(&ks, buffer, n);
        fwrite(buffer, 1, n, out);
    }

    status = (ferror(in) || ferror(out)) ? FILE_NOT_FOUND : SUCCESS;
    fclose(in);
    fclose(out);
    memset(&ks, 0, sizeof(ks));

    DEBUG_PRINT("encrypted %s to %s", input_filename, output_filename);
    return status;
}

Status decrypt_file(const char *input_filename, const char *output_filename, const char *password) {
    unsigned char nonce[CHACHA_NONCE_LENGTH];
    unsigned char buffer[CHACHA_BLOCK_SIZE];
    KeyStream ks;
    FILE *in;
    FILE *out;
    size_t n;
    Status status;

    if (input_filename == NULL || output_filename == NULL || password == NULL) {
        return INVALID_ARGUMENT;
    }

    in = fopen(input_filename, "rb");
    if (in == NULL) {
        return FILE_NOT_FOUND;
    }

    if (fread(nonce, 1, CHACHA_NONCE_LENGTH, in) != CHACHA_NONCE_LENGTH ||
        fread(buffer, 1, CHECK_LENGTH, in) != CHECK_LENGTH) {
        fclose(in);
        return INVALID_ARGUMENT;
    }

    /* Check the password before creating the output file */
    keystream_init(&ks, password, nonce);
    keystream_xor(&ks, buffer, CHECK_LENGTH);
    if (memcmp(buffer, CHECK_VALUE, CHECK_LENGTH) != 0) {
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

    status = (ferror(in) || ferror(out)) ? FILE_NOT_FOUND : SUCCESS;
    fclose(in);
    fclose(out);
    memset(&ks, 0, sizeof(ks));

    DEBUG_PRINT("decrypted %s to %s", input_filename, output_filename);
    return status;
}
