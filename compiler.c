#include "compiler.h"

void init_compiler(Compiler* compiler) {
  init_local_array(&compiler->local_array);
  compiler->local_depth = 0;
  compiler->scope_depth = 0;
}

void free_compiler(Compiler* compiler) {
  free_local_array(&compiler->local_array);
}

void begin_scope(Compiler* compiler) {
  compiler->scope_depth++;
}

void close_scope(Compiler* compiler) {
  compiler->scope_depth--;
}
