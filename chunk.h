#pragma once

#include <stdint.h>

#include "array.h"

typedef struct {
  int count;
  int capacity;
  OpArray code;
  IntArray lines;
  ValueArray constants;
} Chunk;

void init_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte, int line);
void free_chunk(Chunk* chunk);

