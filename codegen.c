#include "codegen.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "debugging.h"
#include "macros.h"
#include "object.h"
#include "op.h"

static OpArray* op_array;
static ValueArray* constants_array;
static AstArray* ast_array;
static LocalArray* local_array;
static Compiler* c;

static void emit_byte(OpCode op) {
  // switch (op) {
  //   case OP_POP:
  //     printf("[%d] [%-20s]\n", 0, "@@@ OP_POP");
  //     break;
  // }
  push_op_array(op_array, op);
  // printf("Current op_array after pushing : %d\n", op_array->count);
}

static void emit_constant(Value value) {
  emit_byte(OP_CONSTANT);
  // Add to value_array
  push_value_array(constants_array, value);
  // Minus 1 of the current count as it is 0-indexed
  emit_byte((OpCode)(constants_array->count - 1));
}

static int make_constant(Value value) {
  push_value_array(constants_array, value);
  int constant_index = constants_array->count - 1;
  // if (constant_index == 1) {
  //   printf("### emitting pop from here\n");
  // }
  emit_byte((OpCode)constant_index);
  return constant_index;
}

static bool identifier_equal(Token* a, Token* b) {
  if (a->length != b->length)
    return false;

  return memcmp(a->start, b->start, a->length) == 0;
}

// Given the token name, and which compiler,
// look through the local arrays, to see if it exists
static int resolve_local(Compiler* c, Token* name) {
  for (int i = local_array->count - 1; i >= 0; i--) {
    Local* local = &local_array->locals[i];
    if (identifier_equal(name, &local->name)) {
      return i;
    }
  }
  return -1;
}

static void gen(Ast* ast) {
  if (ast == NULL)
    return;

  switch (ast->type) {
    case AST_NONE:
      break;
    case AST_PRINT: {
      PrintStmt* print_stmt = (PrintStmt*)ast->as;
      // Emit nested statement, then emit print
      gen(print_stmt->expr);
      emit_byte(OP_PRINT);
      break;
    }
    case AST_IF: {
      IfStmt* if_stmt = (IfStmt*)ast->as;
      gen(if_stmt->condition_expr);

      emit_byte(OP_JUMP_IF_FALSE);
      // Placeholder, this placeholder is two bytes, uint16_t
      // This is so that it can jump (65536 - 1) times.
      emit_byte((OpCode)0xff);
      emit_byte((OpCode)0xff);
      // minus two because the jump position is in two counts, add one
      // becuase this is zero indexed
      int jump_if_false_index = op_array->count - 2;

      // If the condition goes through to the then_expression, it will first
      // run OP_POP to keep the stack clean. This clears the if (condition)
      // codegen
      emit_byte(OP_POP);

      gen(if_stmt->then_stmt);
      // patch the jump index
      // minus one because this is zero indexed
      // tried setting it to 100 to test if
      int jump_position = op_array->count;

      op_array->ops[jump_if_false_index] =
          (OpCode)((jump_position >> 8) & 0xff);
      op_array->ops[jump_if_false_index + 1] = (OpCode)(jump_position & 0xff);

      emit_byte(OP_POP);

      // Generate the else branch
      gen(if_stmt->else_stmt);
      break;
    }
    case AST_WHILE: {
      WhileStmt* while_stmt = (WhileStmt*)ast->as;

      // Need to keep track of this index, as it will jump back here
      int while_start_index = op_array->count;

      // Generate the condition for the while loop
      gen(while_stmt->condition_expr);

      // Create a placeholder for the jump
      emit_byte(OP_JUMP_IF_FALSE);
      emit_byte((OpCode)0xff);
      emit_byte((OpCode)0xff);

      // Minus two because the jump position is in two counts
      int jump_if_false_index = op_array->count - 2;
      emit_byte(OP_POP);

      // Generate the block statement for the while loop
      gen(while_stmt->block_stmt);

      emit_byte(OP_JUMP);
      emit_byte((OpCode)((while_start_index >> 8) & 0xff));
      emit_byte((OpCode)(while_start_index & 0xff));

      // jump to this after it is done
      int jump_position = op_array->count;
      // do the byte to integer conversion
      op_array->ops[jump_if_false_index] =
          (OpCode)((jump_position >> 8) & 0xff);
      op_array->ops[jump_if_false_index + 1] = (OpCode)(jump_position & 0xff);

      emit_byte(OP_POP);

      break;
    }
    case AST_FOR: {
      // TODO : For some reason using 'i' as the assignment
      // and using increment 'a' using 'i' will cause
      // 'a' and 'i' to be looked at as the same variable.
      // e.g.
      // let a = 0;
      // for (let i = 0; i < 10; i += 1) {
      //   a = a + i;
      // }
      // this will cause it to only loop 5 times, as
      // incrementing 'a' seems to be incrementing 'i'
      ForStmt* for_stmt = (ForStmt*)ast->as;

      // Generate the assignment, assuming it is a completely
      // new initialization
      gen(for_stmt->assignment_stmt);

      // Need to keep track of this index, as it will jump back here
      int while_start_index = op_array->count;

      // Generate the condition for the while loop
      gen(for_stmt->condition_expr);

      // Create a placeholder for the jump
      emit_byte(OP_JUMP_IF_FALSE);
      emit_byte((OpCode)0xff);
      emit_byte((OpCode)0xff);

      // Minus two because the jump position is in two counts
      int jump_if_false_index = op_array->count - 2;
      emit_byte(OP_POP);

      // Generate the block statement for the while loop
      gen(for_stmt->block_stmt);

      // Add the then expression at the end of evaluation
      gen(for_stmt->then_expr);

      emit_byte(OP_JUMP);
      emit_byte((OpCode)((while_start_index >> 8) & 0xff));
      emit_byte((OpCode)(while_start_index & 0xff));

      // jump to this after it is done
      int jump_position = op_array->count;
      // do the byte to integer conversion
      op_array->ops[jump_if_false_index] =
          (OpCode)((jump_position >> 8) & 0xff);
      op_array->ops[jump_if_false_index + 1] = (OpCode)(jump_position & 0xff);

      emit_byte(OP_POP);

      break;
    }
    case AST_BLOCK: {
      begin_scope(c);
      BlockStmt* block_stmt = (BlockStmt*)ast->as;
      // For every statement inside the block, do the codegen
      for (int i = 0; i < block_stmt->ast_array.count; i++) {
        gen(block_stmt->ast_array.ast[i]);
      }
      close_scope(c);
      while (c->local_array.count > 0 &&
             c->local_array.locals[c->local_array.count - 1].depth >
                 c->scope_depth) {
        // Emit byte OP_POP
        emit_byte(OP_POP);
        c->local_array.count--;
      }
      break;
    }
    case AST_VARIABLE_STMT: {
      VariableStmt* variable_stmt = (VariableStmt*)ast->as;
      Token name = variable_stmt->name;

      // If this variable is a local variable
      // scope_depth 0 is the global scope
      if (c->scope_depth != 0) {
        // Check whether there is a variable of the same name
        // in the same local scope
        for (int i = local_array->count - 1; i >= 0; i--) {
          Local* local = &local_array->locals[i];
          if (local->depth != -1 && local->depth < c->scope_depth) {
            break;
          }

          if (identifier_equal(&name, &local->name)) {
            PRINT_TOKEN_STRING(local->name);
            PRINT_TOKEN_STRING(name);
            printf(
                "There already exists a variable of this name in this scope\n");
            return;
          }
        }

        if (local_array->count == UINT8_MAX + 1) {
          printf("Tried to add more than 256 locals while codegen\n");
          return;
        }

        // Using the token, add to the local array
        Local* local = &local_array->locals[local_array->count++];
        local->name = name;
        local->depth = c->scope_depth;
        // printf("Creating a local object\n");
      }

      if (variable_stmt->initializer_expr->type != AST_NONE)
        gen(variable_stmt->initializer_expr);

      int variable_scope = resolve_local(c, &name);
      if (variable_scope == -1) {
        emit_byte(OP_SET_GLOBAL);
      } else if (variable_stmt->initialized) {
        // only if the variable has been initialized before then it should be set again \
        // i.e. \
        // let a = 10; (does not emit OP_SET_LOCAL) \
        // a = 20;
        // printf("emitting op_set_local\n");
        emit_byte(OP_SET_LOCAL);
      }

      if (variable_stmt->initializer_expr->type != AST_NONE &&
          variable_stmt->initialized == false) {
        variable_stmt->initialized = true;
        // printf("setting initialized to be true for\n");
        // PRINT_TOKEN_STRING(variable_stmt->name);
      }

      ObjString* variable_name = make_obj_string(variable_stmt->name.start,
                                                 variable_stmt->name.length);

      // printf("printing variable name\n");
      // print_obj_string(variable_name);
      // printf("hash: %d\n", variable_name->hash);
      // printf("printing variable name\n");

      // TODO : Figure out what this is actually for
      // works for test-lang/numbers.neb
      // but will break for test-lang/locals.neb

      // Only make constant for when its in the global scope
      if (c->scope_depth == 0) {
        Value variable_name_value = OBJ_VAL(variable_name);
        make_constant(variable_name_value);
      }
      break;
    }
    case AST_NUMBER: {  // emit a constant
      NumberExpr* number_expr = (NumberExpr*)ast->as;
      // printf("emitting number\n");
      emit_constant(NUMBER_VAL(number_expr->value));
      break;
    }
    case AST_BINARY: {
      BinaryExpr* binary_expr = (BinaryExpr*)ast->as;
      gen(binary_expr->left_expr);
      gen(binary_expr->right_expr);
      // TODO : Handle the rest of the switch cases
      switch (binary_expr->op.type) {
        case TOKEN_PLUS:
        case TOKEN_PLUS_EQUAL:
          emit_byte(OP_ADD);
          break;
        case TOKEN_MINUS:
        case TOKEN_MINUS_EQUAL:
          emit_byte(OP_SUBTRACT);
          break;
        case TOKEN_STAR:
        case TOKEN_STAR_EQUAL:
          emit_byte(OP_MULTIPLY);
          break;
        case TOKEN_SLASH:
        case TOKEN_SLASH_EQUAL:
          emit_byte(OP_DIVIDE);
          break;
        case TOKEN_EQUAL_EQUAL:
          emit_byte(OP_EQUAL);
          break;
        case TOKEN_BANG_EQUAL:
          emit_byte(OP_EQUAL);
          emit_byte(OP_NOT);
          break;
        case TOKEN_LESS:
          emit_byte(OP_LESS);
          break;
        case TOKEN_LESS_EQUAL:
          emit_byte(OP_GREATER);
          emit_byte(OP_NOT);
          break;
        case TOKEN_GREATER:
          emit_byte(OP_GREATER);
          break;
        case TOKEN_GREATER_EQUAL:
          emit_byte(OP_LESS);
          emit_byte(OP_NOT);
          break;
        default:
          break;
      }
      break;
    }
    case AST_UNARY: {
      UnaryExpr* unary_expr = (UnaryExpr*)ast->as;
      gen(unary_expr->right_expr);
      switch (unary_expr->op.type) {
        case TOKEN_BANG:
          emit_byte(OP_NOT);
          break;
        case TOKEN_MINUS:
          emit_byte(OP_NEGATE);
          break;
        default:
          break;
      }
      break;
    }
    case AST_BOOL: {
      BoolExpr* bool_expr = (BoolExpr*)ast->as;
      if (bool_expr->value == true) {
        emit_byte(OP_TRUE);
      } else {
        emit_byte(OP_FALSE);
      }
      break;
    }
      // This retrieves a variable, i.e. print a;
      // variable_expr == a
    case AST_VARIABLE_EXPR: {
      VariableExpr* variable_expr = (VariableExpr*)ast->as;
      Token name = variable_expr->name;

      // Check if this variable is a local or global variable
      int variable_scope = resolve_local(c, &name);
      if (variable_scope == -1) {
        emit_byte(OP_GET_GLOBAL);
      } else {
        // printf("emitting OP_GET_LOCAL from AST_VARIABLE_EXPR\n");
        emit_byte(OP_GET_LOCAL);
        // emit_byte((OpCode)0);
      }

      // Find the constant
      // For global scope
      if (variable_scope == -1) {
        bool found_variable = false;
        for (int i = 0; i < constants_array->count; i++) {
          Value value = constants_array->values[i];
          // If it is an object and is an ObjString*
          if (IS_OBJ(value) && OBJ_TYPE(value) == OBJ_STRING) {
            // this obj_string can be used to debug string variables
            // ObjString* obj_string = AS_OBJ_STRING(value);
            // print_obj_string(obj_string);
            if (token_value_equals(name, value)) {
              // printf("found variable name at : %d\n", i);
              emit_byte((OpCode)i);
              found_variable = true;
              break;
            }
          }
        }
        // If it did not find the variable
        if (!found_variable)
          printf("Error: Could not find token\n");
      } else {  // For local scope
        for (int i = local_array->count - 1; i >= 0; i--) {
          Local* local = &local_array->locals[i];
          if (identifier_equal(&name, &local->name)) {
            emit_byte((OpCode)i);
            break;
          }
        }
      }
      break;
    }
    case AST_GROUP: {
      GroupExpr* group_expr = (GroupExpr*)ast->as;
      gen(group_expr->expr);
      break;
    }
    case AST_ASSIGNMENT_EXPR: {
      AssignmentExpr* assignment_expr = (AssignmentExpr*)ast->as;
      gen(assignment_expr->expr);
      emit_byte(OP_SET_GLOBAL);

      ObjString* variable_name = make_obj_string(assignment_expr->name.start,
                                                 assignment_expr->name.length);
      Value variable_name_value = OBJ_VAL(variable_name);
      make_constant(variable_name_value);
      break;
    }
    case AST_STRING: {
      StringExpr* string_expr = (StringExpr*)ast->as;
      ObjString* string =
          make_obj_string(string_expr->start, string_expr->length);
      Value string_value = OBJ_VAL(string);
      emit_constant(string_value);
      break;
    }
  }
}

void codegen(OpArray* op_arr,
             ValueArray* constants_arr,
             AstArray* ast_arr,
             LocalArray* local_arr,
             Compiler* compiler) {
  op_array = op_arr;
  constants_array = constants_arr;
  ast_array = ast_arr;
  local_array = local_arr;
  c = compiler;

  for (int i = 0; i < ast_array->count; i++) {
    gen(ast_array->ast[i]);
  }
}

void disassemble_opcode_values(OpArray* op_arr, ValueArray* value_arr) {
  printf("-----%s-----\n", "Codegen Disassembly");
  for (int i = 0; i < op_arr->count; i++) {
    switch (op_arr->ops[i]) {
      case OP_POP:
        printf("[%d] [%-20s]\n", i, "OP_POP");
        break;
      case OP_ADD:
        printf("[%d] [%-20s]\n", i, "OP_ADD");
        break;
      case OP_TRUE:
        printf("[%d] [%-20s]\n", i, "OP_TRUE");
        break;
      case OP_FALSE:
        printf("[%d] [%-20s]\n", i, "OP_FALSE");
        break;
      case OP_SUBTRACT:
        printf("[%d] [%-20s]\n", i, "OP_SUBTRACT");
        break;
      case OP_MULTIPLY:
        printf("[%d] [%-20s]\n", i, "OP_MULTIPLY");
        break;
      case OP_DIVIDE:
        printf("[%d] [%-20s]\n", i, "OP_DIVIDE");
        break;
      case OP_NEGATE:
        printf("[%d] [%-20s]\n", i, "OP_NEGATE");
        break;
      case OP_GREATER:
        printf("[%d] [%-20s]\n", i, "OP_GREATER");
        break;
      case OP_LESS:
        printf("[%d] [%-20s]\n", i, "OP_LESS");
        break;
      case OP_NOT:
        printf("[%d] [%-20s]\n", i, "OP_NOT");
        break;
      case OP_EQUAL:
        printf("[%d] [%-20s]\n", i, "OP_EQUAL");
        break;
      case OP_CONSTANT:
        i++;
        printf("[%d-%d] [%-20s] at %d: %f\n", i - 1, i, "OP_CONSTANT", i,
               AS_NUMBER(value_arr->values[op_arr->ops[i]]));
        break;
      case OP_RETURN:
        printf("[%d] [%-20s]\n", i, "OP_RETURN");
        break;
      case OP_PRINT:
        printf("[%d] [%-20s]\n", i, "OP_PRINT");
        break;
      case OP_SET_GLOBAL:
        printf("[%d-%d] [%-20s]\n", i, i + 1, "OP_SET_GLOBAL");
        // TODO : these values should be printed out and not
        // just skipped over
        i++;  // name index
        // i++; // value index
        break;
      case OP_SET_LOCAL:
        printf("[%d-%d] [%-20s]\n", i, i + 1, "OP_SET_LOCAL");
        i++;
        break;
      case OP_GET_GLOBAL:
        i++;  //
        printf("[%d-%d] [%-20s] at constants_array: %d\n", i - 1, i,
               "OP_GET_GLOBAL", op_arr->ops[i]);
        break;
      case OP_GET_LOCAL:
        i++;  //
        printf("[%d-%d] [%-20s] at constants_array: %d\n", i - 1, i,
               "OP_GET_LOCAL", op_arr->ops[i]);
        break;
      case OP_JUMP:
        printf("[%d] [%-20s]\n", i, "OP_JUMP");
        break;
      case OP_JUMP_IF_FALSE: {
        // i+=2; // get the index of the jump, if false
        uint16_t number =
            (uint16_t)((op_arr->ops[i + 1] << 8) | (op_arr->ops[i + 2]));
        i += 2;
        printf("[%d-%d] [%-20s] jump if false to : %d\n", i - 2, i,
               "OP_JUMP_IF_FALSE", number);
        break;
      }
      case OP_NIL:
        printf("[%d] [%-20s]\n", i, "OP_NIL");
        break;
    }
  }
}
