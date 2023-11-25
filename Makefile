CC=gcc
CFLAGS=-Wall -Wextra -g

all: run

run: main.c aux.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f run
