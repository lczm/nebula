#pragma once

#include <stdbool.h>

#include "array.h"
#include "callframe.h"
#include "hashmap.h"

#define MAX_FRAMES 64
#define MAX_STACK (MAX_FRAMES * UINT8_MAX)

typedef struct {
  HashMap variables;

  // total amount of frames in use at any moment
  int frame_count;
  CallFrame frames[MAX_FRAMES];

  // int stack_top;
  Value* stack_top;
  ValueArray vm_stack;

  // native function count
  int native_function_count;
} Vm;

void init_vm(Vm* vm);
void free_vm(Vm* vm);

// TODO : Return and report run errors from here.
void run(bool arguments[const], Vm* vm, ObjFunc* main_func);
