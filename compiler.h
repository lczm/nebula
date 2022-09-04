#pragma once

#include "array.h"
#include "local.h"

typedef struct {
  LocalArray local_array;
  int local_depth;
  int scope_depth;
} Compiler;

void init_compiler(Compiler* compiler);
void free_compiler(Compiler* compiler);

void begin_scope(Compiler* compiler);
void close_scope(Compiler* compiler);
