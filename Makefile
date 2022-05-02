# source and object directories
SRC := .
OBJ := .

# Can use clang as well
CC      = gcc
CCFLAGS = -g -Wall -Wextra -Wpedantic -Wfloat-equal -O0

SRC_FILES := $(wildcard *.c)
OBJ_FILES := $(patsubst %.c, %.o, $(SRC_FILES))

OBJ_FILES_MAIN := $(filter-out test.o, $(OBJ_FILES))
OBJ_FILES_TEST := $(filter-out main.o, $(OBJ_FILES))

DEPENDS := $(patsubst %.c,%.d,$(SRC_FILES))

.PHONY: all nebula test clean

all: nebula

nebula: $(OBJ_FILES_MAIN)
	$(CC) -o $@ $^

test: $(OBJ_FILES_TEST)
	$(CC) -o $@ $^ && ./test

-include $(DEPENDS)

%.o: %.c Makefile
	$(CC) $(CCFLAGS) -MMD -MP -c $< -o $@
 
clean:
	$(RM) $(DEPENDS) $(OBJ_FILES) nebula test
