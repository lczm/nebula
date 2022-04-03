#pragma once

#include "array.h"
#include "hashmap.h"

typedef struct {
    int ip;
    int stack_top;
    HashMap    variables;
    ValueArray vm_stack;
} Vm;

void init_vm(Vm* vm);
void free_vm(Vm* vm);

//TODO : Return and report run errors from here.
void run(Vm* vm, OpArray* op_arr, ValueArray* ast_value_arr);
