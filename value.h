#pragma once

#include <stdio.h>
#include <stdbool.h>

// Forward declare this obj, 
// as "object.h" will include "value.h", this will prevent further
// cyclic dependencies
typedef struct Obj Obj;

typedef enum {
    VAL_BOOLEAN,
    VAL_NUMBER,
    VAL_OBJ,
    VAL_NIL,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool b;
        double number;
        Obj* obj;
    } as;
} Value;

#define IS_BOOLEAN(value)  ((value).type == VAL_BOOLEAN)
#define IS_NUMBER(value)   ((value).type == VAL_NUMBER)
#define IS_OBJ(value)      ((value).type == VAL_OBJ)
#define IS_NIL(value)      ((value).type == VAL_NIL)

#define AS_BOOLEAN(value)  ((value).as.b)
#define AS_NUMBER(value)   ((value).as.number)
#define AS_OBJ(value)      ((value).as.obj)

#define BOOLEAN_VAL(value) ((Value){VAL_BOOLEAN, {.b = value}})
#define NUMBER_VAL(value)  ((Value){VAL_NUMBER, {.number = value}})
// Cannot name this obj, as the macro will substitute .obj and obj together
// Base cast the object explicitly as well
#define OBJ_VAL(object)    ((Value){VAL_OBJ, {.obj = (Obj*)object}})
// Set nil values to be false booleans
#define NIL_VAL            ((Value){VAL_NIL, {.b = false}})
