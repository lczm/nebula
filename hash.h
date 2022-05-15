#pragma once
#include <stdint.h>

// This header will contain all the hash functions used

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
uint32_t fnv_hash32(const char* key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
uint64_t fnv_hash64(const char* key, int length) {
  uint64_t hash = 14695981039346656037u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint16_t)key[i];
    hash *= 1099511628211;
  }
  return hash;
}
