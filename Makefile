CC=gcc
CFLAGS=-Wall -Wextra -g

user: user.c aux.c
	$(CC) $(CFLAGS) $^ -o $@

AS: server_jony.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f user AS

all: clean user AS
