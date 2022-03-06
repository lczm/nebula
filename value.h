#pragma once

#include <stdbool.h>

typedef enum {
    VAL_BOOLEAN,
    VAL_NUMBER,
    VAL_NIL,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool b;
        double number;
    } as;
} Value;

#define IS_BOOLEAN(value)  ((value).type == VAL_BOOLEAN)
#define IS_NUMBER(value)   ((value).type == VAL_NUMBER)
#define IS_NIL(value)      ((value).type == VAL_NIL)

#define AS_BOOLEAN(value)  ((value).as.b)
#define AS_NUMBER(value)   ((value).as.number)

#define BOOLEAN_VAL(value) ((Value){VAL_BOOLEAN, {.b = value}})
#define NUMBER_VAL(value)  ((Value){VAL_NUMBER, {.number = value}})
// Set nil values to be false booleans
#define NIL_VAL            ((Value){VAL_NIL, {.b = false}})