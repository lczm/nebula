#include "object.h"

#include <stdio.h>
#include <string.h>
#include "macros.h"
#include "hash.h"

ObjString* make_obj_string(const char* chars, int length) {
    ObjString* obj_string = ALLOCATE(ObjString, 1);

    // Create a new string, add c string delimiter
    char* new_string = ALLOCATE(char, length + 1);
    strncpy(new_string, chars, length);
    new_string[length] = '\0';

    obj_string->length = length;
    obj_string->chars = new_string;
    // TODO : Set proper hash
    obj_string->hash = fnv_hash32(new_string, length);

    return obj_string;
}

void print_obj_string(ObjString* obj_string) {
    printf("%s\n", obj_string->chars);
}
