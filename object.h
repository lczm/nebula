#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "token.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

typedef enum {
  TYPE_FUNCTION,
  TYPE_SCRIPT,
} FunctionType;

typedef enum {
  OBJ_STRING,
  OBJ_FUNC,
} ObjType;

// No need to type define this again, as value as done
struct Obj {
  ObjType type;
};

typedef struct {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;
} ObjString;

typedef struct {
  Obj obj;
  int arity;
  ObjString* name;
} ObjFunc;

#define AS_OBJ_STRING(value) (ObjString*)AS_OBJ(value)
#define AS_OBJ_FUNC(value) (ObjFunc*)AS_OBJ(value);

// Takes a pointer to the character and builds a new string from that
// converts it into an ObjString
// This is basically the init method equilvalent for objs
ObjString* make_obj_string(const char* chars, int length);
ObjString* make_obj_string_sl(const char* chars);
void print_obj_string(ObjString* obj_string);
void print_obj_string_without_quotes(ObjString* obj_string);
bool token_value_equals(Token token, Value value);
bool obj_string_equals(ObjString* obj1, ObjString* obj2);
ObjString* concatenate_obj_string(ObjString* obj1, ObjString* obj2);

ObjFunc* make_obj_func(int arity, ObjString* name);
