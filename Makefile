.PHONY: all clean

# C Compiler
CC = gcc
CFLAGS = -D_POSIX_SOURCE -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE -pthread

###################################

# C Files
SOURCE = $(wildcard *.c) # All .c files
HEADER = $(SOURCE:.c=.h)
OBJS = $(SOURCE:.c=.o)
#TARGET = client

###################################

# Standard Rules
all: server

server: server.o
	$(CC) -pthread -o $@ server.o

clean:
	rm -f $(OBJS) server

###################################

# Overwrite suffix rules to enforce our rules
.SUFFIXES:

%.o: %.c
	$(CC) -c $(CFLAGS) $<

#EOF
