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
