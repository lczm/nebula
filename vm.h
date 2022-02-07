#pragma once

#include "array.h"

typedef struct {
    int ip;
} Vm;

void init_vm(Vm* vm);
void free_vm(Vm* vm);

//TODO : Return and report run errors from here.
void run(Vm* vm, OpArray* op_arr, ValueArray* value_arr);
