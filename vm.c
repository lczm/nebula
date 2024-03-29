#include "vm.h"

#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "debugging.h"
#include "macros.h"
#include "object.h"
#include "op.h"

static Vm* vm;

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
    // printf("<func>
    print_obj_string(func->name);
  } else if (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_STRING) {
    print_obj_string((ObjString*)AS_OBJ(value));
  } else {
    printf("this value is not anything\n");
  }
}

static void push(Value value) {
  *vm->stack_top = value;
  vm->stack_top++;
}

static Value pop() {
  // Decrement stack_top
  // vm->stack_top--;
  // TODO: Debug flag for this to check if its in range

  vm->stack_top--;
  Value value = *vm->stack_top;
  // vm->stack_top++;
  return value;
}

static Value peek(int index) {
  Value value = vm->stack_top[-1 - index];
  return value;
}

// Define native functions
static void define_native_func(const char* name, NativeFunc native_func) {
  ObjString* func_name = make_obj_string_sl(name);
  ObjNative* obj_native_func = make_obj_native_func(native_func);

  Value value_func_name = OBJ_VAL(func_name);
  Value value_native_func = OBJ_VAL(obj_native_func);

  // The push and pop here is for garbage collection
  push(value_func_name);
  push(value_native_func);

  push_hashmap(&vm->variables, AS_OBJ_STRING(vm->vm_stack.values[0]),
               vm->vm_stack.values[1]);

  pop();
  pop();
}

// VM native functions
static Value clock_native(int argument_count, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value die(int argument_count, Value* args) {
  exit(0);
}

static Value assert(int argument_count, Value* args) {
  if (argument_count != 2)
    return BOOLEAN_VAL(false);

  Value first = args[0];
  Value second = args[1];

  // print_value(first);
  // print_value(second);

  if (IS_NUMBER(first) && IS_NUMBER(second)) {
    double first_number = AS_NUMBER(first);
    double second_number = AS_NUMBER(second);

    // If not equal
    if ((first_number - second_number) > DBL_EPSILON) {
      printf("assert: %f != %f\n", first_number, second_number);
      exit(1);
    }
  }

  if (IS_BOOLEAN(first) && IS_BOOLEAN(second)) {
    bool first_bool = AS_BOOLEAN(first);
    bool second_bool = AS_BOOLEAN(second);

    if (first_bool != second_bool) {
      char* first_string = first_bool ? "true" : "false";
      char* second_string = second_bool ? "true" : "false";

      printf("assert: %s != %s\n", first_string, second_string);
      exit(1);
    }
  }

  return BOOLEAN_VAL(false);
}

static Value print_v(int argument_count, Value* args) {
  for (int i = 0; i < argument_count; i++) {
    print_value(*(args + i));
  }
  return NIL_VAL;
}

void init_vm(Vm* v) {
  vm = v;
  v->frame_count = 0;
  init_hashmap(&v->variables);
  init_value_array(&v->vm_stack);
  reserve_value_array(&vm->vm_stack, MAX_STACK);
  v->stack_top = &v->vm_stack.values[0];

  // Just update the native_function_count when adding more
  define_native_func("clock", clock_native);
  define_native_func("assert", assert);
  define_native_func("die", die);
  define_native_func("print_v", print_v);
  v->native_function_count = 4;
}

void free_vm(Vm* v) {
  v->stack_top = 0;
  free_hashmap(&v->variables);
  free_value_array(&v->vm_stack);
}

static void inspect_stack(int up_to, const char* from) {
  printf("Inspecting stack from %s START\n", from);
  for (int i = 0; i < up_to; i++) {
    print_value(vm->vm_stack.values[i]);
  }
  printf("Inspecting stack from %s END\n", from);
}

static bool is_falsey(Value value) {
  if (IS_NIL(value))
    return true;
  if (IS_BOOLEAN(value) && AS_BOOLEAN(value) == false)
    return true;

  return false;
}

static bool call(ObjFunc* func, int argument_count) {
  if (argument_count != func->arity) {
    printf("Arity count and function argument_count differs\n");
    return false;
  }

  CallFrame* frame = &vm->frames[vm->frame_count++];
  frame->func = func;
  frame->ip = func->chunk.code.ops;
  frame->slots = vm->stack_top - func->arity - 1;
  return true;
}

static bool call_value(Value callee, int argument_count) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_FUNC: {
        return call(AS_OBJ_FUNC(callee), argument_count);
      }
      case OBJ_NATIVE_FUNC: {
        NativeFunc native_func = AS_OBJ_NATIVE_FUNC(callee);
        Value result =
            native_func(argument_count, vm->stack_top - argument_count);
        vm->stack_top -= argument_count + 1;
        push(result);
        return true;
      }
      default: {
        break;
      }
    }
  }
  return false;
}

void run(bool arguments[const], Vm* vm, ObjFunc* main_func) {
#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8 | frame->ip[-1])))
#define READ_CONSTANT() (frame->func->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_OBJ_STRING(READ_CONSTANT())

  call(main_func, 0);

  CallFrame* frame = &vm->frames[vm->frame_count - 1];

  OpCode instruction;
  for (;;) {
    instruction = READ_BYTE();
    switch (instruction) {
      case OP_CONSTANT: {
        OpCode constant_index = READ_BYTE();

        Value constant =
            frame->func->chunk.constants.values[constant_index - 1];

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
        // inspect_stack(8, "OP_RETURN");

        Value result = pop();

        vm->frame_count--;
        if (vm->frame_count == 0) {
          pop();
          // printf("Done interpreting all code\n");
          return;
        }

        vm->stack_top = frame->slots;

        push(result);
        frame = &vm->frames[vm->frame_count - 1];
        break;
      }
      case OP_PRINT: {
        print_value(pop());
        break;
      }
      case OP_SET_GLOBAL: {
        // Get the variable_name from the constants_array
        OpCode name_constant_index = READ_BYTE();
        Obj* obj =
            AS_OBJ(frame->func->chunk.constants.values[name_constant_index]);
        ObjString* obj_string = (ObjString*)obj;

        // Get the value from the top of the stack
        Value value = peek(0);

        // Add to the variables hashmap
        push_hashmap(&vm->variables, obj_string, value);
        break;
      }
      case OP_SET_LOCAL: {
        OpCode index = READ_BYTE();
        Value value = peek(0);
        frame->slots[index] = value;
        break;
      }
      case OP_GET_GLOBAL: {
        OpCode name_constant_index = READ_BYTE();

        Obj* obj =
            AS_OBJ(frame->func->chunk.constants.values[name_constant_index]);
        ObjString* obj_string = (ObjString*)obj;

        // ObjString* obj_string = READ_STRING();

        Value value = get_hashmap(&vm->variables, obj_string);

        push(value);
        break;
      }
      case OP_GET_LOCAL: {
        // Take the index from the OpCode array
        // And push it onto the value stack, from
        // wherever the old local is.
        OpCode index = READ_BYTE();
        push(frame->slots[index]);
        break;
      }
      case OP_DEFINE_GLOBAL: {
        OpCode op_constant = READ_BYTE();
        OpCode constant_index = READ_BYTE();
        Value constant =
            frame->func->chunk.constants.values[constant_index - 1];
        Obj* obj = AS_OBJ(constant);
        ObjString* obj_string = (ObjString*)obj;

        // Will return the function
        Value p = peek(0);

        // inspect_stack(8, "OP_DEFINE_GLOBAL");

        push_hashmap(&vm->variables, obj_string, p);

        // pop the function off the stack
        pop();

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
        // Argument count, from codegen, not the parser
        // int argument_count = READ_BYTE();
        // printf("Argument count: %d\n", argument_count);

        Value func_name_obj = frame->func->chunk.constants.values[index2 - 1];
        ObjString* func_name = AS_OBJ_STRING(func_name_obj);
        Value func_obj = get_hashmap(&vm->variables, func_name);

        int argument_count = READ_BYTE();

        // inspect_stack(8, "OP_CALL");

        if (!call_value(func_obj, argument_count)) {
          // if (!call_value(func_obj, func->arity)) {
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
