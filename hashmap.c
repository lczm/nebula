#include "hashmap.h"

#include <stdint.h>

#include "macros.h"

// Default load factor that is well tested
#define LOAD_FACTOR 0.75

static int resize_capacity(int capacity) {
  // Default capacity, start off with 4 a
  // total capacity of 4
  if (capacity == 0)
    return 4;
  // Expand twice as big otherwise
  return capacity * 2;
}

// This function gets called when the load factor gets hit
static void adjust_capacity(HashMap* hashmap) {
  int resized_capacity = resize_capacity(hashmap->capacity);
  Entry* entries = ALLOCATE(Entry, resized_capacity);
  // Set all new entries to null
  for (int i = 0; i < resized_capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  hashmap->count = 0;
  for (int i = 0; i < hashmap->capacity; i++) {
    // Get the entry
    Entry* entry = &hashmap->entries[i];
    // Ignore if empty
    if (entry->key == NULL)
      continue;
    int hash_bucket = entry->key->hash % resized_capacity;
    entries[hash_bucket].key = entry->key;
    entries[hash_bucket].value = entry->value;
    // Reset them just in case
    entry->key = NULL;
    entry->value = NIL_VAL;
    // Increment count
    hashmap->count++;
  }
  hashmap->capacity = resized_capacity;

  // Free the old hashmap array
  free(hashmap->entries);
  // Set the new hashmap array
  hashmap->entries = entries;
}

void init_hashmap(HashMap* hashmap) {
  hashmap->count = 0;
  hashmap->capacity = 0;
  hashmap->entries = NULL;
}

void push_hashmap(HashMap* hashmap, ObjString* key, Value value) {
  // Check if there is a need to adjust_capacity based off
  // the load factor
  if (hashmap->count >= hashmap->capacity * LOAD_FACTOR) {
    printf("Before adjusting capacity: %d\n", hashmap->capacity);
    adjust_capacity(hashmap);
    printf("After adjusting capacity: %d\n", hashmap->capacity);
  }

  int hash_bucket = key->hash % hashmap->capacity;

  // Only if the hashmap entry is null
  // then the count should be incremented
  if (hashmap->entries[hash_bucket].key == NULL)
    hashmap->count++;

  // Set the key and value in that hash_bucket
  hashmap->entries[hash_bucket].key = key;
  hashmap->entries[hash_bucket].value = value;
}

void free_hashmap(HashMap* hashmap) {}

Value get_hashmap(HashMap* hashmap, ObjString* key) {
  if (hashmap->count == 0)
    return NIL_VAL;

  int hash_bucket = key->hash % hashmap->capacity;
  Entry* entry = &hashmap->entries[hash_bucket];

  // Debugging whether the key (ObjString) gives the same (Entry) value.
  printf("get_hashmap: %s | %d | hash_bucket: %d| entry: %p \n", key->chars,
         key->hash, hash_bucket, entry);

  return entry->value;
}
