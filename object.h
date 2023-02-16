#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "chunk.h"
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
  OBJ_NATIVE_FUNC,
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
  Chunk chunk;
  ObjString* name;
} ObjFunc;

typedef Value (*NativeFunc)(int arg_count, Value* args);
typedef struct {
  Obj obj;
  NativeFunc func;
} ObjNative;

#define IS_STRING(value) (is_obj_type(value, OBJ_STRING))
#define IS_FUNC(value) (is_obj_type(value, OBJ_FUNC))
#define IS_NATIVE_FUNC(value) (is_obj_type(value, OBJ_NATIVE_FUNC))

#define AS_OBJ_STRING(value) (ObjString*)AS_OBJ(value)
#define AS_OBJ_FUNC(value) (ObjFunc*)AS_OBJ(value)
#define AS_OBJ_NATIVE_FUNC(value) (((ObjNative*)AS_OBJ(value))->func)

bool is_obj_type(Value value, ObjType type);

// Takes a pointer to the character and builds a new string from that
// converts it into an ObjString
// This is basically the init method equilvalent for objs
ObjString* make_obj_string(const char* chars, int length);
ObjString* make_obj_string_sl(const char* chars);
ObjString* make_obj_string_from_token(Token token);
void print_obj_string(ObjString* obj_string);
void print_obj_string_without_quotes(ObjString* obj_string);
bool token_value_equals(Token token, Value value);
bool obj_string_equals(ObjString* obj1, ObjString* obj2);
ObjString* concatenate_obj_string(ObjString* obj1, ObjString* obj2);

ObjFunc* make_obj_func(int arity, ObjString* name);
void print_func(ObjFunc* func);

ObjNative* make_obj_native_func(NativeFunc func);
void print_native_func(ObjNative* native_func);
