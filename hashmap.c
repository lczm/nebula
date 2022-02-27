#include "hashmap.h"
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
    // Set all entries to null
    for (int i = 0; i < resized_capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    hashmap->count = 0;
    hashmap->capacity = resized_capacity;

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
}

void free_hashmap(HashMap* hashmap) {

}