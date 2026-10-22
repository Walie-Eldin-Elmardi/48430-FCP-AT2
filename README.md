# Accountant-Client Records Sharing Platform

Group project for 48430 Fundamentals of C Programming (Assessment Task 3), Team 31, Thursday 4pm lab.

A command-line C program that lets accountants and their clients share sensitive documents (bank statements, payroll records, ID documents, etc.) securely. Files are compressed with Huffman coding and encrypted with ChaCha20 before they are stored, and only the users a file is shared with can retrieve it.

## Team

| Name | Responsible for |
|---|---|
| Wally Elmardi | Compression and encryption design (header prototypes), shared types, `DEBUG_PRINT` |
| Georgia Gamble | `main`, user interface (menu), sorting and searching records, Huffman compression and helpers, ChaCha20 encryption, password hashing, auditor access, status codes and input handling in the user and record functions |
| Minh Huy Pham | User accounts, loading/saving/adding/displaying records |

## Building and running

Requires `gcc` and `make`.

```sh
make          # builds ./main
make debug    # builds ./debug, which prints DEBUG_PRINT messages to stderr
./main
```

The code is compiled with `-Wall -Werror -ansi`, so it must be strict ANSI C (C89): use `/* */` comments only and declare variables at the top of each block.

## Features

- Create an account as an accountant, client or auditor, then log in and out
- Upload a file: it is compressed, encrypted and recorded against the owner and the user it is shared with
- Retrieve a file: it is decrypted and decompressed for authorised users
- Display your records, sorted alphabetically by filename or chronologically by upload date
- Search your records by filename
- Auditors can view, search and retrieve every record, but cannot upload

## Project structure

| File | Purpose | Owner |
|---|---|---|
| `main.c` | Program entry point | Georgia |
| `user_interface.c/.h` | `print_menu`, `read_choice` | Georgia |
| `interface_management.c/.h` | `load_users`, `save_users`, `create_user`, `login`, `logout` | Minh |
| | Password hashing (SHA-256 with salt) | Georgia |
| `data_management.c/.h` | `load_records`, `save_records`, `add_record`, `display_records` | Minh |
| | `sort_records`, `search_record`, `free_records` | Georgia |
| `compression.c/.h` | `compress_file`, `decompress_file` (Huffman coding) | Georgia |
| `encryption.c/.h` | `derive_key`, `generate_nonce`, `encrypt_file`, `decrypt_file` (ChaCha20) | Georgia |
| `custom_library.c/.h` | Shared types (`Status`, Huffman structs), `DEBUG_PRINT` | Wally |
| | Priority queue and Huffman tree helpers | Georgia |

Every function returns a `Status` code from `custom_library.h` where it can fail; check it rather than assuming success.

## Data files

| File | Contents |
|---|---|
| `Users.txt` | One line per user: `id username password_hash role`, where role is 1 (accountant), 2 (client) or 3 (auditor). |
| `FilePaths.txt` | One line per record: `id owner_id shared_with_user_id filename size timestamp path`. |

Both files are created on the first run, and fields are separated by spaces, so usernames, passwords and filenames cannot contain spaces.

Passwords are stored as `$sha256$<salt>$<hash>`: a random 16-hex-digit salt, and SHA-256 applied 10,000 times to the salt and password. Accounts saved with a plain text password still log in, and are upgraded to a hash when they do.

## Scope

**In scope:** creating accounts, uploading and retrieving documents, encrypting files before storage, and sharing documents between accountants and clients.

**Out of scope:** accounting calculations, revoking access, and real network transfer. Everything runs locally; "sending" a file is simulated by letting a different user log in and access files assigned to their user ID.

## Status

Work in progress. All features are implemented: `main`, the menu, user accounts with hashed passwords, records, sorting, searching, Huffman compression and ChaCha20 encryption.

Known limitations: encrypted files are not protected against tampering (there is no MAC), and every compressed file has a 1 KB header, so very small files get bigger.
