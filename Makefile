CC=gcc
CFLAGS=-Wall -Wextra -g

all: clean user AS

user: user.c aux.c
	$(CC) $(CFLAGS) $^ -o $@

AS: server_jony.c aux.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f user AS

