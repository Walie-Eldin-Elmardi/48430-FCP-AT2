/******************************************************************************
 * 48430 Fundamentals of C Programming
 * Assessment Task 3 - Group Project
 *
 * File: interface_management.c
 * Authors: Huy, Georgia
 *
 * This file contains functions used to manage user accounts.
 *
 * Passwords are never stored in plain text. Each one is stored as
 *     $sha256$<salt>$<hash>
 * where salt is 16 random hex digits and hash is SHA-256 applied
 * HASH_ITERATIONS times to the salt and password. Accounts saved before
 * hashing was added still log in, and are upgraded to a hash when they do.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface_management.h"

#define MAX_USERS 100
#define INPUT_BUFFER_SIZE 1000

#define HASH_PREFIX "$sha256$"
#define HASH_PREFIX_LENGTH 8
#define SALT_BYTES 8
#define SALT_HEX_LENGTH (SALT_BYTES * 2)
#define DIGEST_BYTES 32
#define DIGEST_HEX_LENGTH (DIGEST_BYTES * 2)
#define HASH_ITERATIONS 10000

/* SHA-256 works on 32-bit words. unsigned long is at least 32 bits, so
 * every result is masked back down to 32 bits. */
#define MASK32 0xFFFFFFFFUL
#define ROTR(x, n) ((((x) >> (n)) | ((x) << (32 - (n)))) & MASK32)
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static User users[MAX_USERS];
static int user_count = 0;

/******************************************************************************
 * Discards the rest of the current input line, so that each prompt reads
 * its own line and leftover input is not read by the next prompt.
 *
 * Author: Georgia
 ******************************************************************************/
static void discard_line(void)
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF)
    {
        /* discard */
    }
}

/******************************************************************************
 * Processes one 64-byte block of a SHA-256 message, updating the hash state.
 *
 * Author: Georgia
 ******************************************************************************/
static void sha256_block(unsigned long state[8],
                         const unsigned char block[64])
{
    static const unsigned long k[64] = {
        0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
        0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
        0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
        0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
        0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
        0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
        0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
        0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
        0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
        0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
        0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
        0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
        0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
        0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
        0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
        0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
    };

    unsigned long w[64];
    unsigned long a, b, c, d, e, f, g, h;
    unsigned long t1, t2;
    int i;

    for (i = 0; i < 16; i++)
    {
        w[i] = ((unsigned long)block[i * 4] << 24) |
               ((unsigned long)block[i * 4 + 1] << 16) |
               ((unsigned long)block[i * 4 + 2] << 8) |
               (unsigned long)block[i * 4 + 3];
    }

    for (i = 16; i < 64; i++)
    {
        w[i] = (SSIG1(w[i - 2]) + w[i - 7] +
                SSIG0(w[i - 15]) + w[i - 16]) & MASK32;
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    f = state[5];
    g = state[6];
    h = state[7];

    for (i = 0; i < 64; i++)
    {
        t1 = (h + BSIG1(e) + CH(e, f, g) + k[i] + w[i]) & MASK32;
        t2 = (BSIG0(a) + MAJ(a, b, c)) & MASK32;
        h = g;
        g = f;
        f = e;
        e = (d + t1) & MASK32;
        d = c;
        c = b;
        b = a;
        a = (t1 + t2) & MASK32;
    }

    state[0] = (state[0] + a) & MASK32;
    state[1] = (state[1] + b) & MASK32;
    state[2] = (state[2] + c) & MASK32;
    state[3] = (state[3] + d) & MASK32;
    state[4] = (state[4] + e) & MASK32;
    state[5] = (state[5] + f) & MASK32;
    state[6] = (state[6] + g) & MASK32;
    state[7] = (state[7] + h) & MASK32;
}

/******************************************************************************
 * Computes the SHA-256 digest of a message. The message is padded one
 * block at a time, so it does not need any spare room after it.
 * Only the low 32 bits of the message length in bits are encoded, which
 * is plenty for the short messages hashed here.
 *
 * Author: Georgia
 ******************************************************************************/
static void sha256(const unsigned char *message,
                   unsigned long length,
                   unsigned char digest[DIGEST_BYTES])
{
    unsigned long state[8];
    unsigned long padded_length;
    unsigned long bit_length;
    unsigned long position;
    unsigned long from_end;
    unsigned char block[64];
    int i;

    state[0] = 0x6a09e667UL;
    state[1] = 0xbb67ae85UL;
    state[2] = 0x3c6ef372UL;
    state[3] = 0xa54ff53aUL;
    state[4] = 0x510e527fUL;
    state[5] = 0x9b05688cUL;
    state[6] = 0x1f83d9abUL;
    state[7] = 0x5be0cd19UL;

    /* Room for the message, the 0x80 byte and the 8-byte length */
    padded_length = ((length + 9 + 63) / 64) * 64;
    bit_length = (length * 8) & MASK32;

    for (position = 0; position < padded_length; position++)
    {
        if (position < length)
        {
            block[position % 64] = message[position];
        }
        else if (position == length)
        {
            block[position % 64] = 0x80;
        }
        else if (position >= padded_length - 8)
        {
            from_end = padded_length - 1 - position;
            block[position % 64] = (from_end < 4)
                ? (unsigned char)((bit_length >> (8 * from_end)) & 0xFF)
                : 0;
        }
        else
        {
            block[position % 64] = 0;
        }

        if (position % 64 == 63)
        {
            sha256_block(state, block);
        }
    }

    for (i = 0; i < 8; i++)
    {
        digest[i * 4] = (unsigned char)((state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = (unsigned char)((state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = (unsigned char)((state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = (unsigned char)(state[i] & 0xFF);
    }
}

/******************************************************************************
 * Writes bytes as lowercase hex digits followed by a null terminator.
 * The output must have room for 2 * length + 1 characters.
 *
 * Author: Georgia
 ******************************************************************************/
static void to_hex(const unsigned char *bytes,
                   int length,
                   char *output)
{
    static const char digits[] = "0123456789abcdef";
    int i;

    for (i = 0; i < length; i++)
    {
        output[i * 2] = digits[bytes[i] >> 4];
        output[i * 2 + 1] = digits[bytes[i] & 0x0F];
    }

    output[length * 2] = '\0';
}

/******************************************************************************
 * Generates a random salt as hex digits. The salt does not need to be
 * secret, only different for each account, so rand() is good enough.
 *
 * Author: Georgia
 ******************************************************************************/
static void generate_salt(char salt_hex[SALT_HEX_LENGTH + 1])
{
    unsigned char salt[SALT_BYTES];
    int i;

    /* rand() is seeded once in main */
    for (i = 0; i < SALT_BYTES; i++)
    {
        salt[i] = (unsigned char)(rand() & 0xFF);
    }

    to_hex(salt, SALT_BYTES, salt_hex);
}

/******************************************************************************
 * Hashes a password with the given salt and writes it in the stored
 * format $sha256$<salt>$<hash>. The output must hold MAX_FILENAME_LENGTH
 * characters.
 *
 * Author: Georgia
 ******************************************************************************/
static void hash_password(const char *password,
                          const char *salt_hex,
                          char *output)
{
    unsigned char buffer[DIGEST_BYTES + SALT_HEX_LENGTH + MAX_FILENAME_LENGTH];
    unsigned char digest[DIGEST_BYTES];
    char digest_hex[DIGEST_HEX_LENGTH + 1];
    unsigned long password_length;
    int i;

    password_length = (unsigned long)strlen(password);

    /* First round hashes salt + password */
    memcpy(buffer, salt_hex, SALT_HEX_LENGTH);
    memcpy(buffer + SALT_HEX_LENGTH, password, password_length);
    sha256(buffer, SALT_HEX_LENGTH + password_length, digest);

    /* Each later round hashes previous digest + salt + password, which
     * makes guessing passwords much slower */
    for (i = 1; i < HASH_ITERATIONS; i++)
    {
        memcpy(buffer, digest, DIGEST_BYTES);
        memcpy(buffer + DIGEST_BYTES, salt_hex, SALT_HEX_LENGTH);
        memcpy(buffer + DIGEST_BYTES + SALT_HEX_LENGTH,
               password, password_length);
        sha256(buffer,
               DIGEST_BYTES + SALT_HEX_LENGTH + password_length,
               digest);
    }

    to_hex(digest, DIGEST_BYTES, digest_hex);
    sprintf(output, "%s%s$%s", HASH_PREFIX, salt_hex, digest_hex);
}

/******************************************************************************
 * Checks whether a stored password is in the hashed format.
 *
 * Author: Georgia
 ******************************************************************************/
static int is_hashed(const char *stored)
{
    return strncmp(stored, HASH_PREFIX, HASH_PREFIX_LENGTH) == 0 &&
           strlen(stored) == HASH_PREFIX_LENGTH + SALT_HEX_LENGTH + 1 +
                             DIGEST_HEX_LENGTH;
}

/******************************************************************************
 * Checks a password against a stored password. Accounts saved before
 * hashing was added are compared as plain text.
 *
 * Author: Georgia
 ******************************************************************************/
static int check_password(const char *stored,
                          const char *password)
{
    char salt_hex[SALT_HEX_LENGTH + 1];
    char computed[MAX_FILENAME_LENGTH];

    if (!is_hashed(stored))
    {
        return strcmp(stored, password) == 0;
    }

    memcpy(salt_hex, stored + HASH_PREFIX_LENGTH, SALT_HEX_LENGTH);
    salt_hex[SALT_HEX_LENGTH] = '\0';

    hash_password(password, salt_hex, computed);

    return strcmp(stored, computed) == 0;
}

/******************************************************************************
 * Builds the User returned by login when logging in fails.
 *
 * Author: Georgia
 ******************************************************************************/
static User failed_login(void)
{
    User failed_user;

    failed_user.id = -1;
    failed_user.username[0] = '\0';
    failed_user.password[0] = '\0';
    failed_user.role = CLIENT;

    return failed_user;
}

/******************************************************************************
 * Loads users from the users database.
 *
 * Author: Huy (status codes and password hashing: Georgia)
 ******************************************************************************/
Status load_users(const char *filename)
{
    FILE *file;

    int id;
    int role_number;
    int result;

    char username[INPUT_BUFFER_SIZE];
    char password[INPUT_BUFFER_SIZE];

    if (filename == NULL)
    {
        return INVALID_ARGUMENT;
    }

    user_count = 0;

    file = fopen(filename, "r");

    if (file == NULL)
    {
        return FILE_NOT_FOUND;
    }

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

        if (result != 4 ||
            id <= 0 ||
            strlen(username) >= MAX_FILENAME_LENGTH ||
            strlen(password) >= MAX_FILENAME_LENGTH ||
            role_number < ACCOUNTANT ||
            role_number > AUDITOR ||
            user_count >= MAX_USERS)
        {
            user_count = 0;
            fclose(file);
            return INVALID_ARGUMENT;
        }

        users[user_count].id = id;

        strcpy(users[user_count].username, username);
        strcpy(users[user_count].password, password);

        users[user_count].role = (UserRole)role_number;

        user_count++;
    }

    fclose(file);

    return SUCCESS;
}

/******************************************************************************
 * Saves users to the users database.
 *
 * Author: Huy (status codes and password hashing: Georgia)
 ******************************************************************************/
Status save_users(const char *filename)
{
    FILE *file;
    int i;

    if (filename == NULL)
    {
        return INVALID_ARGUMENT;
    }

    file = fopen(filename, "w");

    if (file == NULL)
    {
        return FILE_NOT_FOUND;
    }

    for (i = 0; i < user_count; i++)
    {
        if (fprintf(file,
                    "%d %s %s %d\n",
                    users[i].id,
                    users[i].username,
                    users[i].password,
                    (int)users[i].role) < 0)
        {
            fclose(file);
            return FILE_NOT_FOUND;
        }
    }

    if (fclose(file) != 0)
    {
        return FILE_NOT_FOUND;
    }

    return SUCCESS;
}

/******************************************************************************
 * Logs out the current user.
 *
 * Author: Huy (status codes and password hashing: Georgia)
 ******************************************************************************/
Status logout(User *current_user)
{
    if (current_user == NULL)
    {
        return INVALID_ARGUMENT;
    }

    current_user->id = -1;
    current_user->username[0] = '\0';
    current_user->password[0] = '\0';

    return SUCCESS;
}

/******************************************************************************
 * Creates a new user account. Each answer is read from its own line.
 *
 * Author: Huy (status codes and password hashing: Georgia)
 ******************************************************************************/
Status create_user(void)
{
    User new_user;

    int role_number;
    int next_id;
    int i;

    char username[INPUT_BUFFER_SIZE];
    char password[INPUT_BUFFER_SIZE];
    char salt_hex[SALT_HEX_LENGTH + 1];

    if (user_count >= MAX_USERS)
    {
        printf("User database is full.\n");
        return MEMORY_ALLOCATION_FAILED;
    }

    printf("Enter username: ");

    if (scanf("%999s", username) != 1)
    {
        return INVALID_ARGUMENT;
    }

    discard_line();

    if (strlen(username) >= MAX_FILENAME_LENGTH)
    {
        printf("Username is too long.\n");
        return INVALID_ARGUMENT;
    }

    next_id = 1;

    for (i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            printf("Username already exists.\n");
            return INVALID_ARGUMENT;
        }

        if (users[i].id >= next_id)
        {
            next_id = users[i].id + 1;
        }
    }

    printf("Enter password: ");

    if (scanf("%999s", password) != 1)
    {
        return INVALID_ARGUMENT;
    }

    discard_line();

    if (strlen(password) >= MAX_FILENAME_LENGTH)
    {
        printf("Password is too long.\n");
        return INVALID_ARGUMENT;
    }

    printf("Select role:\n");
    printf("1. Accountant\n");
    printf("2. Client\n");
    printf("3. Auditor\n");
    printf("Choice: ");

    if (scanf("%d", &role_number) != 1)
    {
        discard_line();
        printf("Invalid role.\n");
        return INVALID_ARGUMENT;
    }

    discard_line();

    if (role_number < ACCOUNTANT || role_number > AUDITOR)
    {
        printf("Invalid role.\n");
        return INVALID_ARGUMENT;
    }

    new_user.id = next_id;

    strcpy(new_user.username, username);

    generate_salt(salt_hex);
    hash_password(password, salt_hex, new_user.password);

    new_user.role = (UserRole)role_number;

    users[user_count] = new_user;

    user_count++;

    printf("Account created. Your user ID is %d.\n", new_user.id);

    return SUCCESS;
}

/******************************************************************************
 * Logs a user into the program. Each answer is read from its own line.
 *
 * Author: Huy (status codes and password hashing: Georgia)
 ******************************************************************************/
User login(void)
{
    char username[INPUT_BUFFER_SIZE];
    char password[INPUT_BUFFER_SIZE];
    char salt_hex[SALT_HEX_LENGTH + 1];

    int i;

    printf("Username: ");

    if (scanf("%999s", username) != 1)
    {
        return failed_login();
    }

    discard_line();

    printf("Password: ");

    if (scanf("%999s", password) != 1)
    {
        return failed_login();
    }

    discard_line();

    if (strlen(username) < MAX_FILENAME_LENGTH &&
        strlen(password) < MAX_FILENAME_LENGTH)
    {
        for (i = 0; i < user_count; i++)
        {
            if (strcmp(users[i].username, username) == 0 &&
                check_password(users[i].password, password))
            {
                /* Upgrade an old plain text password to a hash */
                if (!is_hashed(users[i].password))
                {
                    generate_salt(salt_hex);
                    hash_password(password, salt_hex, users[i].password);
                }

                return users[i];
            }
        }
    }

    printf("Invalid username or password.\n");

    return failed_login();
}
