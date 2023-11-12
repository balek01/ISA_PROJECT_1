# Compiler and flags
CC = gcc
CFLAGS = -Wall -g 

# List of source files
SRC = utils.c bind.c search.c ldap.c tcp.c 
# Generate a list of object files from source files
OBJ = $(SRC:.c=.o)

# Target executable
TARGET = isa-ldapserver

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	rm -f *.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)
