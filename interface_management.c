/*
 * 48430 Fundamentals of C Programming - Assessment Task 3, Team 31
 * File: interface_management.c
 * Authors: Huy, Georgia
 *
 * Creates, loads, saves, logs in and logs out user accounts. See
 * interface_management.h for what each public function does.
 *
 * The users file has one user per line:
 *     id username password_hash role
 * Passwords are never stored in plain text. Each one is stored as
 *     $sha256$<salt>$<hash>
 * where salt is 16 random hex digits and hash is SHA-256 applied
 * HASH_ITERATIONS times to the salt and password. Accounts saved before
 * hashing was added still log in, and are upgraded to a hash when they do.
 */
#include <stdio.h>  /* fopen, fclose, fgets, fgetc, feof, fprintf, printf,
                       sprintf, sscanf */
#include <stdlib.h> /* rand */
#include <string.h> /* strlen, strcpy, strcmp, strncmp, strchr, memcpy,
                       memset */
#include <limits.h> /* INT_MAX */
#include "interface_management.h"
#include "input.h"

/* Longest line the users file can hold */
#define LINE_LENGTH (MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 40)

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

/*
 * Processes one 64-byte block of a SHA-256 message, updating the hash
 * state (FIPS 180-4 section 6.2.2).
 * Parameters: hash state, block
 * Returns: None
 * Author: Georgia
 */
static void sha256_block(unsigned long state[8],
                         const unsigned char block[64]) {
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

    for (i = 0; i < 16; i++) {
        w[i] = ((unsigned long)block[i * 4] << 24) |
               ((unsigned long)block[i * 4 + 1] << 16) |
               ((unsigned long)block[i * 4 + 2] << 8) |
               (unsigned long)block[i * 4 + 3];
    }
    for (i = 16; i < 64; i++) {
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

    for (i = 0; i < 64; i++) {
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

/*
 * Computes the SHA-256 digest of a message. The message is padded one
 * block at a time, so it does not need any spare room after it. Only the
 * low 32 bits of the message length in bits are encoded, which is plenty
 * for the short messages hashed here.
 * Parameters: message, length in bytes, digest to fill in
 * Returns: None
 * Author: Georgia
 */
static void sha256(const unsigned char *message, unsigned long length,
                   unsigned char digest[DIGEST_BYTES]) {
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

    for (position = 0; position < padded_length; position++) {
        if (position < length) {
            block[position % 64] = message[position];
        } else if (position == length) {
            block[position % 64] = 0x80;
        } else if (position >= padded_length - 8) {
            from_end = padded_length - 1 - position;
            block[position % 64] = (from_end < 4)
                ? (unsigned char)((bit_length >> (8 * from_end)) & 0xFF)
                : 0;
        } else {
            block[position % 64] = 0;
        }

        if (position % 64 == 63) {
            sha256_block(state, block);
        }
    }

    for (i = 0; i < 8; i++) {
        digest[i * 4] = (unsigned char)((state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = (unsigned char)((state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = (unsigned char)((state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = (unsigned char)(state[i] & 0xFF);
    }
}

/*
 * Writes bytes as lowercase hex digits followed by a '\0'.
 * Parameters: bytes, number of bytes, output with room for 2 * length + 1
 *             characters
 * Returns: None
 * Author: Georgia
 */
static void to_hex(const unsigned char *bytes, int length, char *output) {
    static const char digits[] = "0123456789abcdef";
    int i;

    for (i = 0; i < length; i++) {
        output[i * 2] = digits[bytes[i] >> 4];
        output[i * 2 + 1] = digits[bytes[i] & 0x0F];
    }
    output[length * 2] = '\0';
}

/*
 * Generates a random salt as hex digits. The salt does not need to be
 * secret, only different for each account, so rand() is good enough.
 * Parameters: salt to fill in
 * Returns: None
 * Author: Georgia
 */
static void generate_salt(char salt_hex[SALT_HEX_LENGTH + 1]) {
    unsigned char salt[SALT_BYTES];
    int i;

    /* rand() is seeded once in main */
    for (i = 0; i < SALT_BYTES; i++) {
        salt[i] = (unsigned char)(rand() & 0xFF);
    }
    to_hex(salt, SALT_BYTES, salt_hex);
}

/*
 * Hashes a password with the given salt and writes it in the stored
 * format $sha256$<salt>$<hash>.
 * Parameters: password (shorter than INPUT_LENGTH), salt, output of
 *             MAX_PASSWORD_LENGTH characters
 * Returns: None
 * Author: Georgia
 */
static void hash_password(const char *password, const char *salt_hex,
                          char output[MAX_PASSWORD_LENGTH]) {
    unsigned char buffer[DIGEST_BYTES + SALT_HEX_LENGTH + INPUT_LENGTH];
    unsigned char digest[DIGEST_BYTES];
    char digest_hex[DIGEST_HEX_LENGTH + 1];
    unsigned long password_length;
    int i;

    password_length = (unsigned long)strlen(password);
    if (password_length >= INPUT_LENGTH) {
        password_length = INPUT_LENGTH - 1;
    }

    /* First round hashes salt + password */
    memcpy(buffer, salt_hex, SALT_HEX_LENGTH);
    memcpy(buffer + SALT_HEX_LENGTH, password, password_length);
    sha256(buffer, SALT_HEX_LENGTH + password_length, digest);

    /* Each later round hashes previous digest + salt + password, which
     * makes guessing passwords much slower */
    for (i = 1; i < HASH_ITERATIONS; i++) {
        memcpy(buffer, digest, DIGEST_BYTES);
        memcpy(buffer + DIGEST_BYTES, salt_hex, SALT_HEX_LENGTH);
        memcpy(buffer + DIGEST_BYTES + SALT_HEX_LENGTH, password,
               password_length);
        sha256(buffer, DIGEST_BYTES + SALT_HEX_LENGTH + password_length,
               digest);
    }

    to_hex(digest, DIGEST_BYTES, digest_hex);
    sprintf(output, "%s%s$%s", HASH_PREFIX, salt_hex, digest_hex);
    memset(buffer, 0, sizeof(buffer));
}

/*
 * Checks whether a stored password is in the hashed format.
 * Parameters: stored password
 * Returns: 1 if it is hashed, 0 if it is old plain text
 * Author: Georgia
 */
static int is_hashed(const char *stored) {
    return strncmp(stored, HASH_PREFIX, HASH_PREFIX_LENGTH) == 0 &&
           strlen(stored) == HASH_PREFIX_LENGTH + SALT_HEX_LENGTH + 1 +
                             DIGEST_HEX_LENGTH;
}

/*
 * Checks a password against a stored password. Accounts saved before
 * hashing was added are compared as plain text.
 * Parameters: stored password, password typed by the user
 * Returns: 1 if the password is right, otherwise 0
 * Author: Georgia
 */
static int check_password(const char *stored, const char *password) {
    char salt_hex[SALT_HEX_LENGTH + 1];
    char computed[MAX_PASSWORD_LENGTH];

    if (!is_hashed(stored)) {
        return strcmp(stored, password) == 0;
    }

    memcpy(salt_hex, stored + HASH_PREFIX_LENGTH, SALT_HEX_LENGTH);
    salt_hex[SALT_HEX_LENGTH] = '\0';
    hash_password(password, salt_hex, computed);

    return strcmp(stored, computed) == 0;
}

/*
 * Reads one user from a line of the users file.
 * Parameters: line of text, user to fill in
 * Returns: 1 if the line holds a valid user, otherwise 0
 * Author: Huy (updated by Georgia)
 */
static int parse_user(const char *line, user_t *user) {
    char username[LINE_LENGTH];
    char password_hash[LINE_LENGTH];
    int role_number;
    char extra;

    /* Reading one more character catches extra fields on the line */
    if (sscanf(line, "%d %s %s %d %c", &user->id, username, password_hash,
               &role_number, &extra) != 4) {
        return 0;
    }
    if (user->id <= 0 || strlen(username) >= MAX_USERNAME_LENGTH ||
        strlen(password_hash) >= MAX_PASSWORD_LENGTH ||
        role_number < ACCOUNTANT || role_number > AUDITOR) {
        return 0;
    }

    strcpy(user->username, username);
    strcpy(user->password_hash, password_hash);
    user->role = (user_role_t)role_number;
    return 1;
}

user_t no_user(void) {
    user_t user;

    user.id = NO_USER_ID;
    user.username[0] = '\0';
    user.password_hash[0] = '\0';
    user.role = CLIENT;
    return user;
}

status_t load_users(const char *filename, user_list_t *users) {
    FILE *file;
    user_t parsed;
    char line[LINE_LENGTH];
    char first;
    int line_number = 0;
    int c;

    if (filename == NULL || users == NULL) {
        return INVALID_ARGUMENT;
    }
    users->count = 0;

    file = fopen(filename, "r");
    if (file == NULL) {
        return FILE_NOT_FOUND;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;

        if (strchr(line, '\n') == NULL && !feof(file)) {
            while ((c = fgetc(file)) != '\n' && c != EOF) {
                /* discard the rest of the line */
            }
            fprintf(stderr, "Warning: skipped line %d of %s (too long).\n",
                    line_number, filename);
            continue;
        }
        /* Skip blank lines */
        if (sscanf(line, " %c", &first) != 1) {
            continue;
        }
        if (!parse_user(line, &parsed)) {
            fprintf(stderr, "Warning: skipped line %d of %s (badly "
                    "formatted).\n", line_number, filename);
            continue;
        }
        if (find_user_by_name(users, parsed.username) != NULL ||
            find_user_by_id(users, parsed.id) != NULL) {
            fprintf(stderr, "Warning: skipped line %d of %s (duplicate "
                    "user).\n", line_number, filename);
            continue;
        }
        if (users->count >= MAX_USERS) {
            fprintf(stderr, "Warning: only the first %d users in %s were "
                    "loaded.\n", MAX_USERS, filename);
            break;
        }

        users->users[users->count++] = parsed;
    }

    fclose(file);
    DEBUG_PRINT("loaded %d user(s) from %s", users->count, filename);
    return SUCCESS;
}

status_t save_users(const char *filename, const user_list_t *users) {
    FILE *file;
    const user_t *user;
    int i;

    if (filename == NULL || users == NULL) {
        return INVALID_ARGUMENT;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        return FILE_NOT_FOUND;
    }

    for (i = 0; i < users->count; i++) {
        user = &users->users[i];
        if (fprintf(file, "%d %s %s %d\n", user->id, user->username,
                    user->password_hash, (int)user->role) < 0) {
            fclose(file);
            return FILE_NOT_FOUND;
        }
    }

    if (fclose(file) != 0) {
        return FILE_NOT_FOUND;
    }
    DEBUG_PRINT("saved %d user(s) to %s", users->count, filename);
    return SUCCESS;
}

status_t logout(user_t *current_user) {
    if (current_user == NULL) {
        return INVALID_ARGUMENT;
    }
    DEBUG_PRINT("user %d logged out", current_user->id);
    *current_user = no_user();
    return SUCCESS;
}

/*
 * Asks for a new password twice and checks both match.
 * Parameters: buffer of INPUT_LENGTH characters to store the password
 * Returns: 1 if a password was entered and confirmed, otherwise 0
 * Author: Georgia
 */
static int prompt_new_password(char password[INPUT_LENGTH]) {
    char confirm[INPUT_LENGTH];
    int matches;

    if (!prompt_line("Enter password: ", password, INPUT_LENGTH) ||
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

status_t create_user(user_list_t *users) {
    user_t new_user;
    char username[INPUT_LENGTH];
    char password[INPUT_LENGTH];
    char salt_hex[SALT_HEX_LENGTH + 1];
    int role_number;
    int i;

    if (users == NULL) {
        return INVALID_ARGUMENT;
    }
    if (users->count >= MAX_USERS) {
        printf("The user database is full.\n");
        return MEMORY_ALLOCATION_FAILED;
    }

    if (!prompt_line("Enter username: ", username, sizeof(username))) {
        return INVALID_ARGUMENT;
    }
    if (strlen(username) >= MAX_USERNAME_LENGTH) {
        printf("Usernames can be at most %d characters.\n",
               MAX_USERNAME_LENGTH - 1);
        return INVALID_ARGUMENT;
    }
    if (has_whitespace(username)) {
        printf("Usernames cannot contain spaces.\n");
        return INVALID_ARGUMENT;
    }
    if (find_user_by_name(users, username) != NULL) {
        printf("Username already exists.\n");
        return INVALID_ARGUMENT;
    }

    if (!prompt_new_password(password)) {
        return INVALID_ARGUMENT;
    }

    printf("Select role:\n");
    printf("%d. Accountant\n", ACCOUNTANT);
    printf("%d. Client\n", CLIENT);
    printf("%d. Auditor\n", AUDITOR);
    if (!prompt_int("Choice: ", &role_number)) {
        return INVALID_ARGUMENT;
    }
    if (role_number < ACCOUNTANT || role_number > AUDITOR) {
        printf("Invalid role.\n");
        return INVALID_ARGUMENT;
    }

    new_user.id = 1;
    for (i = 0; i < users->count; i++) {
        /* Adding one to INT_MAX would overflow */
        if (users->users[i].id == INT_MAX) {
            printf("No more user IDs are available.\n");
            return MEMORY_ALLOCATION_FAILED;
        }
        if (users->users[i].id >= new_user.id) {
            new_user.id = users->users[i].id + 1;
        }
    }
    strcpy(new_user.username, username);
    generate_salt(salt_hex);
    hash_password(password, salt_hex, new_user.password_hash);
    memset(password, 0, sizeof(password));
    new_user.role = (user_role_t)role_number;

    users->users[users->count++] = new_user;
    printf("Account created. Your user ID is %d.\n", new_user.id);
    DEBUG_PRINT("created user %d (%s) with role %d", new_user.id,
                new_user.username, role_number);
    return SUCCESS;
}

user_t login(user_list_t *users) {
    char username[INPUT_LENGTH];
    char password[INPUT_LENGTH];
    char salt_hex[SALT_HEX_LENGTH + 1];
    user_t *user;
    int i;

    if (users == NULL) {
        return no_user();
    }
    if (!prompt_line("Username: ", username, sizeof(username)) ||
        !prompt_line("Password: ", password, sizeof(password))) {
        return no_user();
    }

    for (i = 0; i < users->count; i++) {
        user = &users->users[i];
        if (strcmp(user->username, username) == 0 &&
            check_password(user->password_hash, password)) {
            /* Upgrade an old plain text password to a hash */
            if (!is_hashed(user->password_hash)) {
                generate_salt(salt_hex);
                hash_password(password, salt_hex, user->password_hash);
                DEBUG_PRINT("upgraded user %d to a hashed password",
                            user->id);
            }
            memset(password, 0, sizeof(password));
            DEBUG_PRINT("user %d (%s) logged in", user->id, user->username);
            return *user;
        }
    }

    memset(password, 0, sizeof(password));
    DEBUG_PRINT("login failed for username %s", username);
    printf("Invalid username or password.\n");
    return no_user();
}

const user_t *find_user_by_name(const user_list_t *users,
                                const char *username) {
    int i;

    if (users == NULL || username == NULL) {
        return NULL;
    }
    for (i = 0; i < users->count; i++) {
        if (strcmp(users->users[i].username, username) == 0) {
            return &users->users[i];
        }
    }
    return NULL;
}

const user_t *find_user_by_id(const user_list_t *users, int id) {
    int i;

    if (users == NULL) {
        return NULL;
    }
    for (i = 0; i < users->count; i++) {
        if (users->users[i].id == id) {
            return &users->users[i];
        }
    }
    return NULL;
}
