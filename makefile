CC = gcc
CFLAGS = -Wall -Werror -ansi
SRC = main.c command_line.c user_interface.c interface_management.c \
      data_management.c compression.c encryption.c input.c custom_library.c
HDR = command_line.h user_interface.h interface_management.h \
      data_management.h compression.h encryption.h input.h custom_library.h

# Normal build: ./main
main: $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o main

# Debug build: ./debug prints DEBUG_PRINT messages to stderr
debug: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -DDEBUG $(SRC) -o debug

# Remove the built programs and any leftover temporary file
clean:
	rm -f main debug transfer.tmp

.PHONY: clean
