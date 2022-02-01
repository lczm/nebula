#pragma once

#include "token.h"

// This file will contain all the arrays used internally in this program

typedef struct {
    int count;
    int capacity;
    Token* tokens;
} TokenArray;

void init_token_array(TokenArray* arr);
void push_token_array(TokenArray* arr, Token token);
void free_token_array(TokenArray* arr);
