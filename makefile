CC = gcc
CFLAGS = -Wall -Werror -ansi
LDLIBS = -lm
SRC = main.c custom_library.c interface_management.c user_interface.c data_management.c encryption.c compression.c

main: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o main $(LDLIBS)

debug: $(SRC)
	$(CC) $(CFLAGS) -DDEBUG $(SRC) -o debug $(LDLIBS)
