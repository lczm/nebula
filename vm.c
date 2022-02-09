#include <stdio.h>
#include "vm.h"

static Vm* vm;
static OpArray* op_array;
static ValueArray* value_array;

void init_vm(Vm* v) {
    vm = v;
    v->ip = 0;
    init_value_array(&v->vm_stack);
}

void free_vm(Vm* v) {
    v->ip = 0;
    free_value_array(&v->vm_stack);
}

static void push(Value value) {
    push_value_array(&vm->vm_stack, value);
}

static Value pop() {
    // TODO: Debug flag for this to check if its in range
    return vm->vm_stack.values[vm->vm_stack.count - 1];
}

static Value pop_peek(int index) {
    // TODO: Debug flag for this to check if its in range
    return vm->vm_stack.values[vm->vm_stack.count - 1 - index];
}

void run(Vm* vm, OpArray* op_arr, ValueArray* value_arr) {
    // Set these to the static variables for convenience
    op_array = op_arr;
    value_array = value_arr;
    // Temporary ip pointer
    OpCode instruction;
    for (;;) {
        instruction = op_array->ops[vm->ip];
        vm->ip++;
        switch (instruction) {
            case OP_ADD: {
                // pop them in the reverse order, as it got 
                // pushed onto the stack in reverse order as well
                Value value1 = pop_peek(1);
                Value value2 = pop_peek(0);
                double number1 = AS_NUMBER(value1);
                double number2 = AS_NUMBER(value2);
                double number3 = number1 + number2;
                push(NUMBER_VAL(number3));
                break;
            }
            case OP_SUBTRACT:
                break;
            case OP_CONSTANT: {
                OpCode constant_index = op_array->ops[vm->ip];
                vm->ip++;
                double number = AS_NUMBER(value_arr->values[constant_index]);
                push(NUMBER_VAL(number));
                break;
            }
            case OP_RETURN: {
                printf("op_return %f\n", AS_NUMBER(pop()));
                for (int i = 0; i < vm->vm_stack.count; i++) {
                    printf("%f\n", AS_NUMBER(vm->vm_stack.values[i]));
                }
                return;
            }
        }
    }
}
