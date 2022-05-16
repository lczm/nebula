#pragma once

#include <stdbool.h>

#include "array.h"
#include "hashmap.h"

typedef struct {
  int ip;
  int stack_top;
  HashMap variables;
  ValueArray vm_stack;
  // This is for printing out debugging statements
  // TODO : This can probably be changed into a define macro
  bool debugging;
} Vm;

void init_vm(Vm* vm, bool debugging);
void free_vm(Vm* vm);

// TODO : Return and report run errors from here.
void run(Vm* vm, OpArray* op_arr, ValueArray* ast_value_arr);
