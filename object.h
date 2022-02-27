#pragma once
#include <stdint.h>

typedef enum {
    OBJ_STRING,
} ObjType;

typedef struct {
    int length;
    char* chars;
    uint32_t hash;
} ObjString;

// Takes a pointer to the character and builds a new string from that
// converts it into an ObjString
// This is basically the init method equilvalent for objs
ObjString* make_obj_string(const char* chars, int length);