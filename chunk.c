#include "chunk.h"
#include <stdlib.h>

void init_chunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  // chunk->code = NULL;
  // chunk->lines = NULL;
  init_op_array(&chunk->code);
  init_int_array(&chunk->lines);
  init_value_array(&chunk->constants);
}

void write_chunk(Chunk* chunk, uint8_t byte, int line) {
  // if (chunk->capacity < chunk->count + 1) {
  //   int old_capacity = chunk->capacity;
  //   chunk->capacity = chunk->capacity * 2;
  //   chunk->code =
  //       (uint8_t*)realloc(chunk->code, sizeof(uint8_t) * chunk->capacity);
  //   chunk->lines = (int*)realloc(chunk->lines, sizeof(int) *
  //   chunk->capacity);
  // }

  // chunk->code[chunk->count] = byte;
  // chunk->lines[chunk->count] = line;
  // chunk->count++;

  push_op_array(&chunk->code, byte);
  push_int_array(&chunk->lines, line);
  chunk->count++;
}

void free_chunk(Chunk* chunk) {
  free_value_array(&chunk->constants);
  free_int_array(&chunk->lines);
  free_op_array(&chunk->code);
  init_chunk(chunk);
}

