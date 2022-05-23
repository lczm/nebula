#include "object.h"

#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "macros.h"

// The length is needed here because for a lot of the program internals
// there is the start of the characters, 'chars' in this case
// can refer to a random middle point of a very long string,
// in which case, the length will determine when it will be terminated
// in the cases where the length is very obvious (such as in test.c)
// another function can be used for it.
ObjString* make_obj_string(const char* chars, int length) {
  ObjString* obj_string = ALLOCATE(ObjString, 1);

  // Create a new string, add c string delimiter
  char* new_string = ALLOCATE(char, length + 1);
  strncpy(new_string, chars, length);
  new_string[length] = '\0';

  obj_string->obj.type = OBJ_STRING;
  obj_string->length = length;
  obj_string->chars = new_string;
  obj_string->hash = fnv_hash32(new_string, length);

  return obj_string;
}

// This is for when the length can be determined using strlen()
ObjString* make_obj_string_sl(const char* chars) {
  ObjString* obj_string = ALLOCATE(ObjString, 1);

  int length = strlen(chars);

  // Create a new string, add c string delimiter
  char* new_string = ALLOCATE(char, length + 1);
  strncpy(new_string, chars, length);
  new_string[length] = '\0';

  obj_string->obj.type = OBJ_STRING;
  obj_string->length = length;
  obj_string->chars = new_string;
  obj_string->hash = fnv_hash32(new_string, length);

  return obj_string;
}

void print_obj_string(ObjString* obj_string) {
  printf("%s\n", obj_string->chars);
}

// Token is a string, Value is a ObjString
bool token_value_equals(Token token, Value value) {
  if (!IS_OBJ(value))
    return false;
  if (!(OBJ_TYPE(value) == OBJ_STRING))
    return false;

  // This obj_string comparison only checks for
  // the length being the same, it does not check
  // that they belong to the same start
  ObjString* obj_string = AS_OBJ_STRING(value);
  if (token.length != obj_string->length)
    return false;

  int length = token.length;
  for (int i = 0; i < length; i++) {
    if (token.start[i] != obj_string->chars[i]) {
      return false;
    }
  }

  return true;
}

bool obj_string_equals(ObjString* obj1, ObjString* obj2) {
  // They must be of the same length at the minimum
  if (obj1->length != obj2->length)
    return false;
  // They must also be referring to the same char pointer
  if (obj1->chars != obj2->chars)
    return false;
  // if the above checks all pass, then it can only be true
  return true;
}

ObjString* concatenate_obj_string(ObjString* obj1, ObjString* obj2) {
  ObjString* obj_string = ALLOCATE(ObjString, 1);

  int length = obj1->length + obj2->length;

  // Create a new string
  char* new_string = ALLOCATE(char, length + 1);
  // Add the first obj characters
  for (int i = 0; i < obj1->length; i++) {
    new_string[i] = obj1->chars[i];
  }
  // Add the second obj characters
  for (int i = obj1->length; i < length; i++) {
    new_string[i] = obj2->chars[i - obj1->length];
  }
  // Add the delimiter
  new_string[length] = '\0';

  obj_string->obj.type = OBJ_STRING;
  obj_string->length = length;
  obj_string->chars = new_string;
  obj_string->hash = fnv_hash32(new_string, length);

  return obj_string;
}

