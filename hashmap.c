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

static Entry* find_entry(Entry* entries, int capacity, ObjString* key) {
  int index = key->hash % capacity;
  // printf("find_entry : key : %s | hash : %d | bucket : %d | \n", key->chars,
  //        key->hash, index);
  // return &entries[index];
  Entry* tombstone = NULL;

  for (;;) {
    Entry* entry = &entries[index];
    // Check if the entry is null first
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL)
          tombstone = entry;
      }
      return entry;
    }

    // Otherwise, check that the key already exists, and it is the same key
    // Note that we cannot do entry->key == key->chars as a check here
    // because string equality is not the same.
    // Note to compare with key->length and not entry->key->length
    // e.g. if entry->key is "data", and key is "data2"
    // comparing data and data2 with a length of 4 will be true, when it's not
    if (strncmp(entry->key->chars, key->chars,
                MAX(entry->key->length, key->length)) == 0) {
      // if (strncmp(key->chars, entry->key->chars, key->length) == 0) {
      return entry;
    }

    // there exists a key but it does not match the same key,
    // then we give it the next bucket, incrementing it by 1
    // for linear probing, modulo so that it will wrap to the
    // beginning if it is at the end
    index = (index + 1) % capacity;
  }
}

static void add_all_hashmap(HashMap* from, HashMap* to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (entry->key != NULL) {
      push_hashmap(to, entry->key, entry->value);
    }
  }
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

    Entry* destination = find_entry(entries, resized_capacity, entry->key);
    destination->key = entry->key;
    destination->value = entry->value;

    // int hash_bucket = entry->key->hash % resized_capacity;
    // entries[hash_bucket].key = entry->key;
    // entries[hash_bucket].value = entry->value;

    // Reset them just in case
    // entry->key = NULL;
    // entry->value = NIL_VAL;
    // Increment count
    hashmap->count++;
  }

  // printf("@@@@@\n");
  // printf("adjusted capacity | old : %d | new : %d | \n", hashmap->capacity,
  //        resized_capacity);
  // printf("@@@@@\n");

  hashmap->capacity = resized_capacity;

  // Free the old hashmap array
  // free(hashmap->entries);
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
  if (hashmap->count + 1 > hashmap->capacity * LOAD_FACTOR) {
    adjust_capacity(hashmap);
  }

  // Previous implementation that does not consider
  // collision and linear probing
  // int hash_bucket = key->hash % hashmap->capacity;
  // Entry* entry = &hashmap->entries[hash_bucket];

  Entry* entry = find_entry(hashmap->entries, hashmap->capacity, key);

  // Only if the hashmap entry is null
  // then the count should be incremented
  // the entry is new if there was no previous keys
  bool new_key = false;
  if (entry->key == NULL)
    new_key = true;

  if (new_key && IS_NIL(entry->value))
    hashmap->count++;
  // if (entry->key == NULL)
  //   hashmap->count++;

  // Set the key and value in that hash_bucket
  entry->key = key;
  entry->value = value;
}

// TODO : Consider whether to return a boolean
void remove_hashmap(HashMap* hashmap, ObjString* key) {
  // If there is nothing to remove, end the function
  if (hashmap->count == 0)
    return;

  // Find the entry
  Entry* entry = find_entry(hashmap->entries, hashmap->capacity, key);
  if (entry->key == NULL)
    return;

  // Place a tombstone in the entry
  // (NULL + true) is considered a tombstone here
  entry->key = NULL;
  entry->value = BOOLEAN_VAL(true);
}

// TODO: Being able to free this will probably free up a lot of the
// memory leakage issues
void free_hashmap(HashMap* hashmap) {
  hashmap->count = 0;
  hashmap->capacity = 0;
  free(hashmap->entries);
}

Value get_hashmap(HashMap* hashmap, ObjString* key) {
  if (hashmap->count == 0)
    return NIL_VAL;

  // Previous implementation that did not consider collision
  // int hash_bucket = key->hash % hashmap->capacity;
  // Entry* entry = &hashmap->entries[hash_bucket];

  Entry* entry = find_entry(hashmap->entries, hashmap->capacity, key);
  if (entry->key == NULL)
    return NIL_VAL;

  // Debugging whether the key (ObjString) gives the same (Entry) value.
  // printf("get_hashmap: %s | %d | hash_bucket: %d| entry: %p \n", key->chars,
  //     key->hash, hash_bucket, entry);

  // Value value = entry->value;
  // double number = AS_NUMBER(value);
  // printf("key: %s | value number: %f\n", entry->key->chars, number);

  return entry->value;
}
