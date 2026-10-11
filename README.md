# Accountant-Client Records Sharing Platform

Group project for 48430 Fundamentals of C Programming (Assessment Task 3), Team 31, Thursday 4pm lab.

A command-line C program that lets accountants and their clients share sensitive documents (bank statements, payroll records, ID documents, etc.) securely. Files are compressed with Huffman coding and encrypted with ChaCha before they are stored, and only the users a file is shared with can retrieve it.

## Team

| Name | Responsible for |
|---|---|
| Wally Elmardi | Compression, encryption, Huffman helpers |
| Georgia Gamble | `main`, user interface (menu), sorting and searching records |
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

## Project structure

| File | Purpose | Owner |
|---|---|---|
| `main.c` | Program entry point | Georgia |
| `user_interface.c/.h` | `print_menu`, `read_choice` | Georgia |
| `interface_management.c/.h` | `load_users`, `save_users`, `create_user`, `login`, `logout` | Minh |
| `data_management.c/.h` | `load_records`, `save_records`, `add_record`, `display_records` | Minh |
| | `sort_records`, `search_record` | Georgia |
| `compression.c/.h` | `compress_file`, `decompress_file` | Wally |
| `encryption.c/.h` | `derive_key`, `generate_nonce`, `encrypt_file`, `decrypt_file` | Wally |
| `custom_library.c/.h` | Shared types (`Status`, Huffman structs), priority queue and Huffman tree helpers, `DEBUG_PRINT` | Wally |

Every function returns a `Status` code from `custom_library.h` where it can fail; check it rather than assuming success.

## Data files

| File | Contents |
|---|---|
| `Users.txt` | One line per user account. Format TBD. |
| `FilePaths.txt` | One line per stored record (owner, shared-with user, filename, size, timestamp, path). Format TBD. |

## Scope

**In scope:** creating accounts, uploading and retrieving documents, encrypting files before storage, and sharing documents between accountants and clients.

**Out of scope:** accounting calculations, revoking access, and real network transfer. Everything runs locally; "sending" a file is simulated by letting a different user log in and access files assigned to their user ID.

## Status

Work in progress. Headers are in place; implementations are being filled in per module. `sort_records` and `search_record` are implemented.
