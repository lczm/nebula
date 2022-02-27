#pragma once

#include "op.h"
#include "token.h"
#include "value.h"

// This file will contain all the arrays used internally in this program
typedef struct {
    int count;
    int capacity;
    Token* tokens;
} TokenArray;

void init_token_array(TokenArray* arr);
void push_token_array(TokenArray* arr, Token token);
void free_token_array(TokenArray* arr);

typedef struct {
    int count;
    int capacity;
    OpCode* ops;
} OpArray;

void init_op_array(OpArray* arr);
void push_op_array(OpArray* arr, OpCode op);
void free_op_array(OpArray* arr);

typedef struct {
    int count;
    int capacity;
    Value* values;
} ValueArray;

void init_value_array(ValueArray* arr);
void push_value_array(ValueArray* arr, Value value);
void free_value_array(ValueArray* arr);
