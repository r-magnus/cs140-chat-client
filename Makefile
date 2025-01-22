# Chat Client Makefile for CS140
# @author Ryan Magnuson rmagnuson@westmont.edu

# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g

# Setup
TARGET = chat
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Default arg
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Remove everything
clean:
	rm -f $(TARGET) $(OBJS)
