#pragma once

#include <stdbool.h>

#include "array.h"
#include "hashmap.h"

typedef struct {
  int ip;
  int stack_top;
  HashMap variables;

  // total amount of frames in use at any moment
  int frame_count;
  CallFrameArray frames;

  ValueArray vm_stack;
} Vm;

void init_vm(Vm* vm);
void free_vm(Vm* vm);

// TODO : Return and report run errors from here.
void run(bool arguments[const],
         Vm* vm,
         OpArray* op_arr,
         ValueArray* ast_value_arr);

