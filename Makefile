CC=gcc
CFLAGS=-Wall -g
OUTPUT=isa-ldapserver

build: tcp.c tcp.h
	$(CC) $(CFLAGS) tcp.c -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)