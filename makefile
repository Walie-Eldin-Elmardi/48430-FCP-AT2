CC = gcc
CFLAGS = -Wall -Werror -ansi -o -lm
SRC = main.c interface_management.c user_interface.c data_management.c encryption.c compression.c

main: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o main

debug: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -DDEBUG -o debug