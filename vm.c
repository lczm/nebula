#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "op.h"
#include "vm.h"
#include "macros.h"

static Vm* vm;
static OpArray* op_array;
static ValueArray* ast_value_array;

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

static void print_value(Value value) {
    if (value.type == VAL_NUMBER) {
        printf("%f\n", AS_NUMBER(value));
    } else if (value.type == VAL_BOOLEAN) {
        if (AS_BOOLEAN(value)) {
            printf("true\n");
        } else {
            printf("false\n");
        }
    } else if (value.type == VAL_OBJ) {
        printf("Obj\n");
    }
}

static void push(Value value) {
    // If there is still space
    // note to use count and not capacity
    if (vm->stack_top < vm->vm_stack.count) {
        vm->vm_stack.values[vm->stack_top] = value;
    } else {
        push_value_array(&vm->vm_stack, value);
    }

    vm->stack_top++;
}

static Value pop() {
    // Decrement stack_top 
    // vm->stack_top--;
    // TODO: Debug flag for this to check if its in range

    // it has to be more than 0 to be able to return anything
    if (vm->stack_top > 0) {
        Value value = vm->vm_stack.values[vm->stack_top - 1];
        // print_value(value);
        vm->stack_top--;
        return value;
    } else {
        printf("returning nil value as there is nothing to pop()\n");
        return NIL_VAL;
    }
}

static Value peek(int index) {
    Value value = vm->vm_stack.values[vm->stack_top - 1 - index];
    return value;
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

static uint16_t read_short() {
    // move the vm instruction pointer up by two
    vm->ip += 2;
    return (uint16_t)((op_array->ops[vm->ip - 2] << 8) |
                      (op_array->ops[vm->ip - 1]));
}

static bool is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOLEAN(value) && !AS_BOOLEAN(value));
}

void run(Vm* vm, OpArray* op_arr, ValueArray* ast_value_arr) {
    // Set these to the static variables for convenience
    op_array = op_arr;
    ast_value_array = ast_value_arr;
    // Temporary ip pointer
    OpCode instruction;

    // Mostly for debugging purposes
#ifdef DEBUGGING
    printf("--- VM Output --- \n");
#endif

    for (;;) {
        instruction = op_array->ops[vm->ip];
        vm->ip++;
        switch (instruction) {
            case OP_CONSTANT: {
                // Get the constant_index, and take the number from the ast_value_arr
                OpCode constant_index = op_array->ops[vm->ip];
                vm->ip++;
                double number = AS_NUMBER(ast_value_arr->values[constant_index]);
                push(NUMBER_VAL(number));
                break;
            }
            case OP_POP: {
                pop();
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
// #if DEBUGGING
                printf("op_return %f\n", AS_NUMBER(pop()));
// #endif
                return;
            }
            case OP_PRINT: {
                // Value value = pop();
// #if DEBUGGING
                print_value(peek(0));
// #endif
                break;
            }
            case OP_SET_GLOBAL: {
                // Get the variable_name from the constants_array
                OpCode name_constant_index = op_array->ops[vm->ip];
                // Increment it as it has 'eaten' this op
                vm->ip++;
                Obj* obj = AS_OBJ(ast_value_arr->values[name_constant_index]);
                // The variable_name representation is an ObjString*
                ObjString* obj_string = (ObjString*)obj;

                // Get the value from the top of the stack
                Value value = peek(0);
                double number = AS_NUMBER(value);

                // Add to the variables hashmap
                push_hashmap(&vm->variables, obj_string, value);
                break;
            }
            case OP_GET_GLOBAL: {
                // printf("@@@ OP_GET_GLOBAL\n");
                OpCode name_constant_index = op_array->ops[vm->ip];
                vm->ip++;

                Obj* obj = AS_OBJ(ast_value_arr->values[name_constant_index]);
                ObjString* obj_string = (ObjString*)obj;
                // printf("variable name from OP_GET_GLOBAL\n");
                // print_obj_string(obj_string);

                Value value = get_hashmap(&vm->variables, obj_string);
                push(value);
                break;
            }
            case OP_JUMP: {
                uint16_t jump_index = read_short();
                // Jump to the jump_index
                vm->ip = jump_index;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t jump_index_if_false = read_short();
                // printf("jump index: %d\n", jump_index_if_false);

                Value condition_expr = peek(0);
                // if false, jump to the jump_index, otherwise, continue
                // executing the program.
                if (is_falsey(condition_expr)) {
                    vm->ip = jump_index_if_false;
                }

                break;
            }
            case OP_NIL: {
                break;
            }
            default: // Just break out of those that are not handled yet
                return;
        }
    }
}
