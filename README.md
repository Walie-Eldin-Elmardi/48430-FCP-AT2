# Accountant-Client Records Sharing Platform

Group project for 48430 Fundamentals of C Programming (Assessment Task 3), Team 31, Thursday 4pm lab.

A command-line C program that lets accountants and their clients share sensitive documents (bank statements, payroll records, ID documents, etc.) securely. Files are compressed with Huffman coding and encrypted with ChaCha20 before they are stored, and only the users a file is shared with can retrieve it.

## Team

| Name | Responsible for |
|---|---|
| Wally Elmardi | Compression and encryption design (header prototypes), shared types, `DEBUG_PRINT` |
| Georgia Gamble | `main`, command line modes, user interface (menu), input handling, sorting and searching records, Huffman compression, priority queue and stack, ChaCha20 encryption, password hashing, auditor access, status codes and debug output |
| Minh Huy Pham | User accounts, loading/saving/adding/displaying records |

## Building and running

Requires `gcc` and `make`.

```sh
make          # builds ./main
make debug    # builds ./debug, which prints DEBUG_PRINT messages to stderr
make clean    # removes ./main, ./debug and any leftover temporary file
./main
```

The code is compiled with `-Wall -Werror -ansi`, so it must be strict ANSI C (C89): use `/* */` comments only and declare variables at the top of each block.

### Command line modes

With no arguments, `./main` starts the interactive program. With an option, it works on a single file without logging in:

```sh
./main --help                        # list the options
./main --compress notes.txt notes.huf
./main --decompress notes.huf notes.txt
./main --encrypt notes.txt notes.enc # asks for the password twice
./main --decrypt notes.enc notes.txt # asks for the password
```

Each option also has a short form (`-h`, `-c`, `-x`, `-e`, `-d`). Passwords are asked for rather than passed as arguments, so they are not saved in your shell history. If the output file already exists, you are asked before it is overwritten.

### Coding conventions

- Every `#include` of a standard header lists the library functions used from it.
- Every `typedef` name ends in `_t` (for example `record_t`, `status_t`).
- Lines are at most 80 characters, indented with 4 spaces, with K&R braces.
- Public functions are described in the header file; static functions are described above their definition.
- No global variables: the user list and records list belong to `main` and are passed to the functions that need them.

## Features

- Create an account as an accountant, client or auditor, then log in and out
- Upload a file and share it with another user by username: it is compressed, encrypted with its own password and recorded against the owner and the user it is shared with
- Retrieve a file: it is decrypted and decompressed for authorised users. If two files share a name, you choose by record ID. Retrieving will not overwrite the program's own data files, and asks before overwriting any other file
- Display your records, sorted alphabetically by filename or by upload date and time
- Search your records by filename
- Auditors can view, search and retrieve every record, but cannot upload
- Compress, decompress, encrypt and decrypt single files from the command line
- A debug build (`make debug`) that prints what the program is doing to stderr

## Project structure

| File | Purpose | Owner |
|---|---|---|
| `main.c` | Program entry point: interactive or command line mode | Georgia |
| `command_line.c/.h` | `run_command_line` (the `--compress`, `--encrypt`, ... modes) | Georgia |
| `user_interface.c/.h` | `print_menu`, `read_choice` | Georgia |
| `input.c/.h` | `prompt_line`, `prompt_int` and other console input checks | Georgia |
| `interface_management.c/.h` | `load_users`, `save_users`, `create_user`, `login`, `logout` | Minh |
| | Password hashing (SHA-256 with salt), `find_user_by_name`, `find_user_by_id` | Georgia |
| `data_management.c/.h` | `load_records`, `save_records`, `add_record`, `display_records` | Minh |
| | `sort_records` (merge sort), `search_record`, `count_matching_records`, `find_record_by_id`, `free_records` | Georgia |
| `compression.c/.h` | `compress_file`, `decompress_file` (Huffman coding) | Georgia |
| `encryption.c/.h` | `derive_key`, `generate_nonce`, `encrypt_file`, `decrypt_file` (ChaCha20) | Georgia |
| `custom_library.c/.h` | Shared types (`status_t`, Huffman structs), `DEBUG_PRINT` | Wally |
| | Priority queue (binary heap), node stack and Huffman tree helpers | Georgia |

Every function returns a `status_t` code from `custom_library.h` where it can fail; check it rather than assuming success.

## Data files

| File | Contents |
|---|---|
| `Users.txt` | One line per user: `id username password_hash role`, where role is 1 (accountant), 2 (client) or 3 (auditor). Usernames can be up to 63 characters. |
| `FilePaths.txt` | One line per record: `id owner_id shared_with_user_id filename size timestamp path`, where timestamp is the upload time in seconds since 1970. |

Both files are created on the first run, and are saved again after every new account, login and upload. Fields are separated by spaces, so usernames and filenames cannot contain spaces (passwords can, since only their hash is saved). Badly formatted lines are skipped with a warning when the files are loaded.

Passwords are stored as `$sha256$<salt>$<hash>`: a random 16-hex-digit salt, and SHA-256 applied 10,000 times to the salt and password. Accounts saved with a plain text password still log in, and are upgraded to a hash when they do.

## Scope

**In scope:** creating accounts, uploading and retrieving documents, encrypting files before storage, and sharing documents between accountants and clients.

**Out of scope:** accounting calculations, revoking access, and real network transfer. Everything runs locally; "sending" a file is simulated by letting a different user log in and access files assigned to their user ID.

## Status

All features are implemented: the menu, command line modes, user accounts with hashed passwords, records, sorting, searching, Huffman compression, ChaCha20 encryption and the debug build.

Known limitations: encrypted files are not protected against tampering (there is no MAC), and every compressed file has a 1 KB header, so very small files get bigger.
