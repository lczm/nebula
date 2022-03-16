#include <stdio.h>
#include <stdbool.h>

#include "op.h"
#include "vm.h"

static Vm* vm;
static OpArray* op_array;
static ValueArray* value_array;

void init_vm(Vm* v) {
    vm = v;
    v->ip = 0;
    v->stack_top = 0;
    init_hashmap(&v->variables);
    init_value_array(&v->vm_stack);
}

void free_vm(Vm* v) {
    v->ip = 0;
    v->stack_top = 0;
    free_hashmap(&v->variables);
    free_value_array(&v->vm_stack);
}

static void push(Value value) {
    // If there is still space
    if (vm->vm_stack.capacity == 1) {
        push_value_array(&vm->vm_stack, value);
    } else if (vm->stack_top < vm->vm_stack.capacity) {
        vm->vm_stack.values[vm->stack_top] = value;
    } else {
        printf("push_value_array\n");
        push_value_array(&vm->vm_stack, value);
    }
    vm->stack_top++;
}

static Value pop() {
    // Decrement stack_top 
    vm->stack_top--;
    // TODO: Debug flag for this to check if its in range
    Value value = vm->vm_stack.values[vm->stack_top];
    return value;
}

static Value peek(int index) {
    Value value = vm->vm_stack.values[vm->vm_stack.count - 1 - index];
    return value;
}

static void print_value(Value value) {
    if (value.type == VAL_NUMBER) {
        printf("%f\n", AS_NUMBER(value));
    } else if (value.type == VAL_BOOLEAN) {
        if (AS_BOOLEAN(value)) {
            printf("true\n");
        } else {
            printf("false\n");
        }
    }
}

static bool values_equal(Value value1, Value value2) {
    if (value1.type == VAL_NUMBER && 
        value2.type == VAL_NUMBER) {
        return AS_NUMBER(value1) == AS_NUMBER(value2);
    } else if (value1.type == VAL_BOOLEAN &&
               value2.type == VAL_BOOLEAN) {
        return AS_BOOLEAN(value1) == AS_BOOLEAN(value2);
    } else { // TODO : This should have a proper return
        return false;
    }
}

static void debug_vm_stack_top() {
    printf("vm->stack_top: %d\n", vm->stack_top);
    for (int i = 0; i < vm->stack_top + 1; i++) {
        print_value(vm->vm_stack.values[i]);
    }
}

static void debug_vm_stack() {
    printf("vm->vm_stack.count: %d\n", vm->vm_stack.count);
    for (int i = 0; i < vm->vm_stack.count; i++) {
        print_value(vm->vm_stack.values[i]);
    }
}

void run(Vm* vm, OpArray* op_arr, ValueArray* value_arr) {
    // Set these to the static variables for convenience
    op_array = op_arr;
    value_array = value_arr;
    // Temporary ip pointer
    OpCode instruction;

    // Mostly for debugging purposes
    printf("--- VM Output --- \n");

    for (;;) {
        instruction = op_array->ops[vm->ip];
        vm->ip++;
        switch (instruction) {
            case OP_CONSTANT: {
                OpCode constant_index = op_array->ops[vm->ip];
                vm->ip++;
                double number = AS_NUMBER(value_arr->values[constant_index]);
                push(NUMBER_VAL(number));
                break;
            }
            case OP_TRUE:
                push(BOOLEAN_VAL(true));
                break;
            case OP_FALSE:
                push(BOOLEAN_VAL(false));
                break;
            case OP_ADD: {
                // TODO : This can be optimized, does not need so many local variables
                Value value1 = pop();
                Value value2 = pop();
                double number1 = AS_NUMBER(value1);
                double number2 = AS_NUMBER(value2);
                // Note that it is 2 + 1, not 1 + 2 as it is poped in
                // the reverse order from when it is pushed in
                double number3 = number2 + number1;
                push(NUMBER_VAL(number3));
                break;
            }
            case OP_SUBTRACT: {
                // TODO : This can be optimized, does not need so many local variables
                Value value1 = pop();
                Value value2 = pop();
                double number1 = AS_NUMBER(value1);
                double number2 = AS_NUMBER(value2);
                // Note that it is 2 + 1, not 1 + 2 as it is poped in
                // the reverse order from when it is pushed in
                double number3 = number2 - number1;
                push(NUMBER_VAL(number3));
                break;
            }
            case OP_MULTIPLY: {
                Value value1 = pop(); 
                Value value2 = pop();
                double number1 = AS_NUMBER(value1);
                double number2 = AS_NUMBER(value2);
                double number3 = number2 * number1;
                push(NUMBER_VAL(number3));
                break;
            }
            case OP_DIVIDE: {
                Value value1 = pop(); 
                Value value2 = pop();
                double number1 = AS_NUMBER(value1);
                double number2 = AS_NUMBER(value2);
                double number3 = number2 / number1;
                push(NUMBER_VAL(number3));
                break;
            }
            case OP_EQUAL: {
                Value value1 = pop();
                Value value2 = pop();
                push(BOOLEAN_VAL(values_equal(value2, value1)));
                break;
            }
            case OP_RETURN: {
                printf("op_return %f\n", AS_NUMBER(pop()));
                return;
            }
            case OP_PRINT: {
                print_value(pop());
                break;
            }
            case OP_SET_GLOBAL: {
                printf("@@@ OP_SET_GLOBAL\n");
                OpCode name_constant_index = op_array->ops[vm->ip];
                vm->ip++;
                Obj* obj = AS_OBJ(value_arr->values[name_constant_index]);
                ObjString* obj_string = (ObjString*)obj;
                print_obj_string(obj_string);

                OpCode value_constant_index = op_array->ops[vm->ip];
                vm->ip++;
                Value value = value_arr->values[value_constant_index];
                double number = AS_NUMBER(value);
                printf("number: %f\n", number);

                // Add to the variables hashmap
                push_hashmap(&vm->variables, obj_string, value);
                break;
            }
            case OP_GET_GLOBAL: {
                printf("@@@ OP_GET_GLOBAL\n");
                break;
            }
            default: // Just break out of those that are not handled yet
                return;
        }
    }
}
