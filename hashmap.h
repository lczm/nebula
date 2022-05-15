#pragma once

#include "object.h"
#include "value.h"

typedef struct {
  ObjString* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} HashMap;

void init_hashmap(HashMap* hashmap);
void push_hashmap(HashMap* hashmap, ObjString* key, Value value);
void free_hashmap(HashMap* hashmap);

Value get_hashmap(HashMap* hashmap, ObjString* key);
