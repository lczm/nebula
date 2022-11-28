#include "vm.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "debugging.h"
#include "macros.h"
#include "object.h"
#include "op.h"

static Vm* vm;

void init_vm(Vm* v) {
  vm = v;
  init_hashmap(&v->variables);
  init_value_array(&v->vm_stack);
  reserve_value_array(&vm->vm_stack, MAX_STACK);
  v->stack_top = &v->vm_stack.values[0];
}

void free_vm(Vm* v) {
  v->stack_top = 0;
  free_hashmap(&v->variables);
  free_value_array(&v->vm_stack);
}

static void print_value(Value value) {
  if (IS_NUMBER(value)) {
    printf("%f\n", AS_NUMBER(value));
  } else if (IS_BOOLEAN(value) && AS_BOOLEAN(value) == true) {
    printf("true\n");
  } else if (IS_BOOLEAN(value) && AS_BOOLEAN(value) == false) {
    printf("false\n");
  } else if (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_FUNC) {
    ObjFunc* func = (ObjFunc*)AS_OBJ(value);
    printf("func arity: %d\n", func->arity);
    print_obj_string(func->name);
  } else if (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_STRING) {
    print_obj_string((ObjString*)AS_OBJ(value));
  } else {
    printf("this value is not anything\n");
  }
}

static void push(Value value) {
  // If there is still space
  // note to use count and not capacity
  // if (vm->stack_top < vm->vm_stack.count) {
  //   vm->vm_stack.values[vm->stack_top] = value;
  // } else {
  //   push_value_array(&vm->vm_stack, value);
  // }

  // if (vm->stack_top < vm->vm_stack.count) {
  //   *vm->stack_top = value;
  //   vm->stack_top++;
  //   // vm->vm_stack.values[vm->stack_top] = value;
  // } else {
  //   push_value_array(&vm->vm_stack, value);
  // }

  // printf("---\n");
  // for (int i = 0; i < vm->vm_stack.count; i++) {
  //   print_value(vm->vm_stack.values[i]);
  // }
  // printf("---\n");

  *vm->stack_top = value;
  vm->stack_top++;
}

static Value pop() {
  // Decrement stack_top
  // vm->stack_top--;
  // TODO: Debug flag for this to check if its in range

  // it has to be more than 0 to be able to return anything
  // if (vm->stack_top > 0) {
  //   Value value = vm->vm_stack.values[vm->stack_top - 1];
  //   // print_value(value);
  //   vm->stack_top--;
  //   return value;
  // } else {
  //   printf("returning nil value as there is nothing to pop()\n");
  //   return NIL_VAL;
  // }

  vm->stack_top--;
  Value value = *vm->stack_top;
  // vm->stack_top++;
  return value;

  if (vm->stack_top > 0) {
    vm->stack_top--;
    Value value = *vm->stack_top;
    vm->stack_top++;
    // Value value = vm->vm_stack.values[vm->stack_top - 1];
    // print_value(value);
    // vm->stack_top--;
    return value;
  } else {
    printf("returning nil value as there is nothing to pop()\n");
    return NIL_VAL;
  }
}

static Value peek(int index) {
  // Value value = vm->vm_stack.values[vm->stack_top - 1 - index];
  // Value value = vm->stack_top[-1 - index];
  Value value = vm->stack_top[-1 - index];
  // Value value = vm->vm_stack.values[vm->stack_top - 1 - index];
  return value;
}

// Both of these debugging functions are unused now
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

static void debug_value_array(ValueArray* value_array) {
  printf("value_array->count: %d\n", value_array->count);
  for (int i = 0; i < value_array->count; i++) {
    print_value(value_array->values[i]);
  }
}

// static uint16_t read_short() {
//   // move the vm instruction pointer up by two
//   vm->ip += 2;
//   return (uint16_t)((op_array->ops[vm->ip - 2] << 8) |
//                     (op_array->ops[vm->ip - 1]));
// }

static bool is_falsey(Value value) {
  if (IS_NIL(value))
    return true;
  if (IS_BOOLEAN(value) && AS_BOOLEAN(value) == false)
    return true;

  return false;
}

static bool call(ObjFunc* func, int argument_count) {
  CallFrame* frame = &vm->frames[vm->frame_count++];
  frame->func = func;
  frame->ip = func->chunk.code.ops;
  // frame->slots = vm->stack_top - argument_count - 1;
  // frame->slots = &vm->vm_stack.values[vm->stack_top - argument_count - 1];
  // frame->slots = &vm->vm_stack.values[vm->stack_top];
  frame->slots = vm->stack_top;

  // Value value = func->chunk.constants.values[0];
  // printf("From call START\n");
  // print_value(value);
  // printf("From call END\n");
  return true;
}

static bool call_value(Value callee, int argument_count) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_FUNC: {
        return call(AS_OBJ_FUNC(callee), argument_count);
      }
      default: {
        break;
      }
    }
  }
  return false;
}

void run(bool arguments[const],
         Vm* vm,
         OpArray* op_arr,
         ValueArray* value_arr,
         ObjFunc* main_func) {
  // Set these to the static variables for convenience
  // op_array = op_arr;
  // value_array = value_arr;
#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8 | frame->ip[-1])))
#define READ_CONSTANT() (frame->func->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_OBJ_STRING(READ_CONSTANT())

  call(main_func, 0);

  // Temporary : TODO! This can probably be placed a lot better
  // CallFrame* frame = &vm->frames[vm->frame_count++];
  // frame->func = main_func;
  // frame->ip = main_func->chunk.code;
  // frame->slots = vm->stack_top;

  // printf("value_arr count: %d\n", value_arr->count);
  // debug_value_array(value_arr);

  // printf("vm->frame_count: %d\n", vm->frame_count);

  // CallFrame* frame = &vm->frames[vm->frame_count - 1];
  CallFrame* frame = &vm->frames[vm->frame_count - 1];

  // printf("LISTING OUT ALL CHUNK CONSTANTS VALUE -- START\n");
  // for (int i = 0; i < frame->func->chunk.constants.count; i++) {
  //   print_value(frame->func->chunk.constants.values[i]);
  // }
  // printf("LISTING OUT ALL CHUNK CONSTANTS VALUE -- END\n");

  // printf("LISTING OUT ALL CHUNK OP_CODE -- START\n");
  // for (int i = 0; i < frame->func->chunk.code.count; i++) {
  //   printf("OP %d : %d\n", i, frame->func->chunk.code.ops[i]);
  // }
  // printf("LISTING OUT ALL CHUNK OP_CODE -- END\n");

  // printf("LISTING OUT ALL CHUNK OP_CODE2 -- START\n");
  // for (int i = 0; i < frame->func->chunk.code.count; i++) {
  //   printf("OP %d : %d\n", i, frame->func->chunk.code.ops[*frame->ip]);
  //   // *frame->ip++;
  //   *frame->ip = *frame->ip + 1;
  // }
  // printf("LISTING OUT ALL CHUNK OP_CODE2 -- END\n");

  // OpCode* t = frame->func->chunk.code.ops;

  // for (int i = 0; i < frame->func->chunk.count; i++) {
  //   OpCode a = *t++;
  //   OpCode b = READ_BYTE();
  //   printf("%d\n", a);
  // }

  // printf("%d\n", a);
  // printf("%d\n", b);
  // printf("%d\n", c);

  OpCode instruction;
  for (;;) {
    // instruction = op_array->ops[vm->ip];
    // vm->ip++;
    // instruction = frame->func->chunk.code.ops[*frame->ip++];
    instruction = READ_BYTE();
    // *frame->ip = *frame->ip + 1;
    switch (instruction) {
      case OP_CONSTANT: {
        // Get the constant_index, and take the number from the
        // ast_value_arr
        // OpCode constant_index = op_array->ops[vm->ip];
        // vm->ip++;

        // Value value = value_arr->values[constant_index];
        // push(value);

        // OpCode instruction2 = READ_BYTE();
        // printf("instruction index: %d\n", instruction2);
        // return;

        // OpCode constant_index = frame->func->chunk.code.ops[*frame->ip];
        // *frame->ip = *frame->ip + 1;

        // OpCode constant_index = frame->func->chunk.code.ops[*frame->ip++];
        OpCode constant_index = READ_BYTE();

        Value constant =
            frame->func->chunk.constants.values[constant_index - 1];
        // *frame->ip++;

        // Value constant = READ_CONSTANT();
        // printf("@@@ ---FROM OP_CONSTANT--- START\n");
        // print_value(constant);
        // printf("@@@ ---FROM OP_CONSTANT--- END\n");
        push(constant);
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
        // TODO : This can be optimized, does not need so many local
        // variables
        Value value1 = pop();
        Value value2 = pop();
        if (IS_NUMBER(value1) && IS_NUMBER(value2)) {
          double number1 = AS_NUMBER(value1);
          double number2 = AS_NUMBER(value2);
          // Note that it is 2 + 1, not 1 + 2 as it is poped in
          // the reverse order from when it is pushed in
          double number3 = number2 + number1;
          push(NUMBER_VAL(number3));
        } else if ((IS_OBJ(value1) && ((OBJ_TYPE(value1) == OBJ_STRING))) &&
                   (IS_OBJ(value2) && ((OBJ_TYPE(value2) == OBJ_STRING)))) {
          ObjString* obj_string1 = AS_OBJ_STRING(value1);
          ObjString* obj_string2 = AS_OBJ_STRING(value2);
          ObjString* obj_string3 =
              concatenate_obj_string(obj_string2, obj_string1);
          push(OBJ_VAL(obj_string3));
        } else {
          printf(
              "Error: Tried to add two values that cannot be added together\n");
        }
        break;
      }
      case OP_SUBTRACT: {
        // TODO : This can be optimized, does not need so many local
        // variables
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
      case OP_NEGATE: {
        Value value = pop();
        if (IS_NUMBER(value)) {
          push(NUMBER_VAL(-(AS_NUMBER(value))));
        }
        break;
      }
      case OP_GREATER: {
        Value value1 = pop();
        Value value2 = pop();
        double number1 = AS_NUMBER(value1);
        double number2 = AS_NUMBER(value2);
        bool greater = number2 > number1;
        push(BOOLEAN_VAL(greater));
        break;
      }
      case OP_LESS: {
        Value value1 = pop();
        Value value2 = pop();
        double number1 = AS_NUMBER(value1);
        double number2 = AS_NUMBER(value2);
        bool greater = number2 < number1;
        push(BOOLEAN_VAL(greater));
        break;
      }
      case OP_NOT: {
        Value value = pop();
        if (AS_BOOLEAN(value) == true) {
          push(BOOLEAN_VAL(false));
        } else {
          push(BOOLEAN_VAL(true));
        }
        break;
      }
      case OP_EQUAL: {
        Value value1 = pop();
        Value value2 = pop();
        push(BOOLEAN_VAL(values_equal(value2, value1)));
        break;
      }
      case OP_RETURN: {
        Value result = pop();
        vm->frame_count--;
        if (vm->frame_count == 0) {
          pop();
          printf("Done interpreting all code\n");
          return;
        }

        // TODO : This is the issue
        vm->stack_top = frame->slots;
        // vm->stack_top = frame->stack_top;
        push(result);
        frame = &vm->frames[vm->frame_count - 1];
        break;

        // if (arguments[VM_OUTPUT])
        //   printf("op_return %f\n", AS_NUMBER(pop()));
        // return;
      }
      case OP_PRINT: {
        // Value value = pop();
        if (arguments[VM_OUTPUT]) {
          // printf("@@@\n");
          // debug_vm_stack();
          // printf("vm_stack_top:%d\n", vm->stack_top);
          // printf("@@@\n");
          // print_value(peek(0));
        }

        // debug_vm_stack_top();

        print_value(pop());
        break;
      }
      case OP_SET_GLOBAL: {
        // Get the variable_name from the constants_array
        // OpCode name_constant_index = op_array->ops[vm->ip];
        // Increment it as it has 'eaten' this op
        // vm->ip++;

        // Obj* obj = AS_OBJ(value_arr->values[name_constant_index]);
        // The variable_name representation is an ObjString*
        // ObjString* obj_string = (ObjString*)obj;

        // OpCode name_constant_index = frame->func->chunk.code.ops[*frame->ip];
        // *frame->ip = *frame->ip + 1;
        // OpCode name_constant_index =
        // frame->func->chunk.code.ops[*frame->ip++];
        OpCode name_constant_index = READ_BYTE();

        Obj* obj =
            AS_OBJ(frame->func->chunk.constants.values[name_constant_index]);
        ObjString* obj_string = (ObjString*)obj;

        // ObjString* obj_string = READ_STRING();

        // Get the value from the top of the stack
        Value value = peek(0);

        // Add to the variables hashmap
        push_hashmap(&vm->variables, obj_string, value);
        break;
      }
      case OP_SET_LOCAL: {
        // OpCode index = op_array->ops[vm->ip];
        // vm->ip++;
        // Value value = peek(0);
        // vm->vm_stack.values[index] = value;

        // OpCode index = frame->func->chunk.code.ops[*frame->ip];
        // *frame->ip = *frame->ip + 1;
        // OpCode index = frame->func->chunk.code.ops[*frame->ip++];
        OpCode index = READ_BYTE();

        Value value = peek(0);
        // frame->slots->values[index] = value;
        frame->slots[index] = value;

        // uint8_t slot = READ_BYTE();
        // frame->slots->values[slot] = peek(0);
        break;
      }
      case OP_GET_GLOBAL: {
        // OpCode name_constant_index = op_array->ops[vm->ip];
        // vm->ip++;

        // Obj* obj = AS_OBJ(value_arr->values[name_constant_index]);
        // ObjString* obj_string = (ObjString*)obj;

        // OpCode name_constant_index = frame->func->chunk.code.ops[*frame->ip];
        // *frame->ip = *frame->ip + 1;
        // OpCode name_constant_index =
        // frame->func->chunk.code.ops[*frame->ip++];
        OpCode name_constant_index = READ_BYTE();

        // Obj* obj =
        //     AS_OBJ(frame->func->chunk.constants.values[name_constant_index]);
        Obj* obj =
            AS_OBJ(frame->func->chunk.constants.values[name_constant_index]);
        ObjString* obj_string = (ObjString*)obj;

        // ObjString* obj_string = READ_STRING();

        Value value = get_hashmap(&vm->variables, obj_string);
        // print_value(value);

        push(value);
        break;
      }
      case OP_GET_LOCAL: {
        // Take the index from the OpCode array
        // And push it onto the value stack, from
        // wherever the old local is.
        OpCode index = READ_BYTE();

        // printf("OP_GET_LOCAL index is :%d\n", index);
        push(frame->slots[index]);
        break;
      }
      case OP_JUMP: {
        // printf("OP_JUMP\n");
        uint16_t offset = READ_SHORT();
        frame->ip += offset;
        break;
      }
      case OP_JUMP_IF_FALSE: {
        // printf("OP_JUMP_IF_FALSE\n");
        uint16_t jump_index_if_false = READ_SHORT();

        Value condition_expr = peek(0);
        // if false, jump to the jump_index, otherwise, continue
        // executing the program.
        if (is_falsey(condition_expr)) {
          frame->ip += jump_index_if_false;
        }

        break;
      }
      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->ip -= offset;
        break;
      }
      case OP_CALL: {
        // printf("OP_CALL\n");
        // OP_CONSTANT, just move it over the OP_CONSTANT
        READ_BYTE();
        // OP_CONSTANT INDEX
        OpCode index2 = READ_BYTE();

        Value func_name_obj = frame->func->chunk.constants.values[index2 - 1];
        ObjString* func_name = AS_OBJ_STRING(func_name_obj);
        Value func_obj = get_hashmap(&vm->variables, func_name);

        if (!call_value(func_obj, 0)) {
          printf("Error out here\n");
          break;
        }

        // call_value() if successful, will push a new callframe
        // and the current callframe will need to be updated to it
        frame = &vm->frames[vm->frame_count - 1];
        break;
      }
      case OP_NIL: {
        break;
      }
      default:  // Just break out of those that are not handled yet
        return;
    }
  }
#undef READ_STRING
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
}
