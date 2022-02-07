#include <stdio.h>
#include "vm.h"

static Vm* vm;
static OpArray* op_array;
static ValueArray* value_array;

void init_vm(Vm* v) {
    vm = v;
}

void free_vm(Vm* v) {
}

static void push(Value value) {
    push_value_array(value_array, value);
}

static Value pop() {
    // TODO: Debug flag for this to check if its in range
    return value_array->values[0];
}

static Value pop_peek(int index) {
    // TODO: Debug flag for this to check if its in range
    return value_array->values[0+index];
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
                Value value1 = pop();
                Value value2 = pop_peek(1);
                double number1 = AS_NUMBER(value1);
                double number2 = AS_NUMBER(value2);
                double number3 = number1 + number2;
                printf("op_add %f\n", number3);
                break;
            }
            case OP_SUBTRACT:
                printf("op_subtract\n");
                break;
            case OP_CONSTANT: {
                OpCode constant_index = op_array->ops[vm->ip];
                vm->ip++;
                double number = AS_NUMBER(value_arr->values[constant_index]);
                push(NUMBER_VAL(number));
                printf("op_constant %f\n", number);
                break;
            }
            case OP_RETURN:
                printf("op_return\n");
                return;
        }
    }
}
