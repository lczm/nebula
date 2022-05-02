# source and object directories
SRC := .
OBJ := .

# Can use clang as well
CC      = gcc
CCFLAGS = -g -Wall -Wextra -Wpedantic -Wfloat-equal -MMD -O0

SRC_FILES := $(wildcard *.c)
OBJ_FILES := $(patsubst %.c, %.o, $(SRC_FILES))

OBJ_FILES_MAIN := $(filter-out test.o, $(OBJ_FILES))
OBJ_FILES_TEST := $(filter-out main.o, $(OBJ_FILES))

.PHONY: all nebula test clean

all: nebula test

nebula: $(OBJ_FILES_MAIN)
	$(CC) -o $@ $^

test: $(OBJ_FILES_TEST)
	$(CC) -o $@ $^ && ./test

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -I$(SRC) -c $< -o $@
 
clean:
	rm *.o && rm nebula test
