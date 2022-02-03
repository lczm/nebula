#include <stdlib.h>

#include "array.h"

#define ALLOCATE(type, count) \
    (type*)malloc(sizeof(type) * count)

void init_token_array(TokenArray* arr) {
    arr->count = 0;
    arr->capacity = 1;
    arr->tokens = ALLOCATE(Token, 1);
}

void push_token_array(TokenArray* arr, Token token) {
    // If there is no space to push a new token on
    // then expand the size
    if (arr->capacity < arr->count + 1) {
        int new_capacity = arr->capacity * 2;
        arr->tokens = (Token*)realloc(
                arr->tokens, sizeof(Token) * new_capacity);
        arr->capacity = new_capacity;
    }

    // Push the token onto the array
    arr->tokens[arr->count] = token;
    arr->count++;
}

void free_token_array(TokenArray* arr) {
    arr->count = 0;
    arr->capacity = 0;
    free(arr->tokens);
}

void init_op_array(OpArray* arr) {
    arr->count = 0;
    arr->capacity = 1;
    arr->ops = ALLOCATE(OpCode, 1);
}

void push_op_array(OpArray* arr, OpCode op) {
    if (arr->capacity < arr->count + 1) {
        int new_capacity = arr->capacity * 2;
        arr->ops = (OpCode*)realloc(
                arr->ops, sizeof(OpCode) * new_capacity);
        arr->capacity = new_capacity;
    }
    arr->ops[arr->count] = op;
    arr->count++;
}

void free_op_array(OpArray* arr) {
    arr->count = 0;
    arr->capacity = 0;
    free(arr->ops);
}

void init_value_array(ValueArray* arr) {
    arr->count = 0;
    arr->capacity = 1;
    arr->values = ALLOCATE(Value, 1);
}

void push_value_array(ValueArray* arr, Value value) {
    if (arr->capacity < arr->count + 1) {
        int new_capacity = arr->capacity * 2;
        arr->values = (Value*)realloc(
                arr->values, sizeof(Value) * new_capacity);
        arr->capacity = new_capacity;
    }
    arr->values[arr->count] = value;
    arr->count++;
}

void free_value_array(ValueArray* arr) {
    arr->count = 0;
    arr->capacity = 0;
    free(arr->values);
}
