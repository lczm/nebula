#include "codegen.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "debugging.h"
#include "macros.h"
#include "object.h"
#include "op.h"

typedef struct {
  struct Compiler* enclosing;
  ObjFunc* func;

  FunctionType func_type;

  LocalArray local_array;
  int local_depth;
  int scope_depth;
} Compiler;

static Compiler* current_compiler;

static Chunk* current_chunk() {
  return &current_compiler->func->chunk;
}

static void emit_byte(OpCode op) {
  write_chunk(current_chunk(), op, 0);
}

static void print_value(Value value) {
  if (IS_NUMBER(value)) {
    printf("%f\n", AS_NUMBER(value));
  } else if (IS_BOOLEAN(value) && AS_BOOLEAN(value) == true) {
    printf("true\n");
  } else if (IS_BOOLEAN(value) && AS_BOOLEAN(value) == false) {
    printf("false\n");
  } else if (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_FUNC) {
    printf("func\n");
  } else if (IS_OBJ(value) && AS_OBJ(value)->type == OBJ_STRING) {
    print_obj_string((ObjString*)AS_OBJ(value));
  }
}

static void emit_constant(Value value) {
  emit_byte(OP_CONSTANT);
  // Add to value_array
  // push_value_array(constants_array, value);
  push_value_array(&current_chunk()->constants, value);
  emit_byte((OpCode)(current_chunk()->constants.count));
}

static int make_constant(Value value) {
  push_value_array(&current_chunk()->constants, value);
  int constant_index = current_chunk()->constants.count - 1;
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
static int resolve_local(Token* name) {
  for (int i = current_compiler->local_array.count - 1; i >= 0; i--) {
    Local* local = &current_compiler->local_array.locals[i];
    if (identifier_equal(name, &local->name)) {
      return i;
    }
  }

  return -1;
}

static void init_compiler(Compiler* compiler,
                          FunctionType func_type,
                          Token name) {
  init_local_array(&compiler->local_array);
  reserve_local_array(&compiler->local_array, UINT8_MAX + 1);  // 256

  compiler->enclosing = (struct Compiler*)current_compiler;
  compiler->func = NULL;
  compiler->local_depth = 0;
  compiler->scope_depth = 0;
  compiler->func_type = func_type;
  // TODO : Figure out why func is set twice?
  // Probably because of garbage collection, but note this down
  // when implementing garbage collection
  compiler->func = make_obj_func(0, NULL);
  current_compiler = compiler;

  if (func_type != TYPE_SCRIPT) {
    ObjString* obj_string = make_obj_string_from_token(name);
    print_obj_string(obj_string);
    current_compiler->func->name = obj_string;
    // current_compiler->func->name = make_obj_string_sl("PLACEHOLDER TEST");
  }

  // Compiler implicitly claims stack slot 0
  // for vm's internal usage
  Local* local = &current_compiler->local_array
                      .locals[current_compiler->local_array.count++];
  local->depth = 0;
  if (func_type != TYPE_FUNCTION) {
    local->name.start = "this";
    local->name.length = 4;
  } else {
    local->name.start = "";
    local->name.length = 0;
  }
}

static ObjFunc* end_compiler() {
  // This is for the case when the function does not have a return at all, then
  // it will "return" out of the function from here.
  // If in the case the function has a return, this will still push, but be
  // i.e. return;
  // OP_NIL | OP_RETURN | OP_NIL | OP_RETURN
  // which will just pop off the function on the first return
  emit_byte(OP_NIL);
  emit_byte(OP_RETURN);

  ObjFunc* func = current_compiler->func;

#ifdef DEBUGGING
  for (int i = 0; i < current_compiler->func->chunk.code.count; i++) {
    // printf("OP %d : %d\n", i, main_func->chunk.code.ops[i]);
    switch (current_compiler->func->chunk.code.ops[i]) {
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
      case OP_CONSTANT: {
        i++;

        Value value =
            current_compiler->func->chunk.constants
                .values[current_compiler->func->chunk.code.ops[i] - 1];

        if (IS_NUMBER(value)) {
          printf("[%d-%d] [%-20s] at %d: %f\n", i - 1, i, "OP_CONSTANT", i,
                 AS_NUMBER(value));
        } else if (IS_STRING(value)) {
          ObjString* string = AS_OBJ_STRING(value);
          printf("[%d-%d] [%-20s] at %d: %s\n", i - 1, i, "OP_CONSTANT", i,
                 string->chars);
        }

        break;
      }
      case OP_RETURN:
        printf("[%d] [%-20s]\n", i, "OP_RETURN");
        break;
      case OP_PRINT:
        printf("[%d] [%-20s]\n", i, "OP_PRINT");
        break;
      case OP_SET_GLOBAL:
        printf("[%d-%d] [%-20s]\n", i, i + 1, "OP_SET_GLOBAL");
        i++;  // name index
        break;
      case OP_SET_LOCAL:
        printf("[%d-%d] [%-20s]\n", i, i + 1, "OP_SET_LOCAL");
        i++;
        break;
      case OP_GET_GLOBAL:
        i++;
        Value value = current_compiler->func->chunk.constants
                          .values[current_compiler->func->chunk.code.ops[i]];

        if (IS_NUMBER(value)) {
          printf("[%d-%d] [%-20s] at %d: %f\n", i - 1, i, "OP_GET_GLOBAL", i,
                 AS_NUMBER(value));
        } else if (IS_STRING(value)) {
          ObjString* string = AS_OBJ_STRING(value);
          printf("[%d-%d] [%-20s] at %d: %s\n", i - 1, i, "OP_GET_GLOBAL", i,
                 string->chars);
        }

        // printf("[%d-%d] [%-20s] at constants_array: \n", i - 1, i,
        //        "OP_GET_GLOBAL");
        break;
      case OP_GET_LOCAL:
        i++;  //
        printf("[%d-%d] [%-20s] at constants_array: %d\n", i - 1, i,
               "OP_GET_LOCAL", current_compiler->func->chunk.code.ops[i]);
        // op_arr->ops[i]);
        break;
      case OP_DEFINE_GLOBAL:
        printf("[%d] [%-20s]\n", i, "OP_DEFINE_GLOBAL");
        break;
      case OP_JUMP:
        printf("[%d] [%-20s]\n", i, "OP_JUMP");
        break;
      case OP_JUMP_IF_FALSE: {
        // i+=2; // get the index of the jump, if false
        // uint16_t number =
        //     (uint16_t)((op_arr->ops[i + 1] << 8) | (op_arr->ops[i + 2]));
        // i += 2;
        // printf("[%d-%d] [%-20s] jump if false to : %d\n", i - 2, i,
        //        "OP_JUMP_IF_FALSE", number);
        printf("OP_JUMP_IF_FALSE\n");
        break;
      }
      case OP_CALL:
        printf("[%d] [%-20s]\n", i, "OP_CALL");
        break;
      case OP_NIL:
        printf("[%d] [%-20s]\n", i, "OP_NIL");
        break;
      case OP_LOOP:
        printf("OP_LOOP\n");
        break;
    }
  }
#endif

  current_compiler = (Compiler*)current_compiler->enclosing;
  return func;
}

static void free_compiler(Compiler* compiler) {
  free_local_array(&compiler->local_array);
}

static void begin_scope(Compiler* compiler) {
  compiler->scope_depth++;
}

static void close_scope(Compiler* compiler) {
  compiler->scope_depth--;
}

static int emit_jump(uint8_t instruction) {
  emit_byte(instruction);
  emit_byte(0xff);
  emit_byte(0xff);
  return current_chunk()->count - 2;
}

static void emit_loop(int loop_start) {
  emit_byte(OP_LOOP);

  int offset = current_chunk()->count - loop_start + 2;
  if (offset > UINT16_MAX)
    printf("loop body too large\n");

  emit_byte((offset >> 8) & 0xff);
  emit_byte(offset & 0xff);
}

static void patch_jump(int start) {
  int jump = current_chunk()->count - start - 2;
  if (jump > UINT16_MAX) {
    printf("Too much code to jump over\n");
  }

  current_chunk()->code.ops[start] = (jump >> 8) & 0xff;
  current_chunk()->code.ops[start + 1] = jump & 0xff;
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

      int then_jump = emit_jump(OP_JUMP_IF_FALSE);
      emit_byte(OP_POP);

      gen(if_stmt->then_stmt);

      int else_jump = emit_jump(OP_JUMP);
      patch_jump(then_jump);
      emit_byte(OP_POP);

      if (if_stmt->else_stmt != AST_NONE)
        gen(if_stmt->else_stmt);
      patch_jump(else_jump);
      break;
    }
    case AST_WHILE: {
      WhileStmt* while_stmt = (WhileStmt*)ast->as;

      int loop_start = current_chunk()->count;

      // Generate the condition expr
      gen(while_stmt->condition_expr);

      int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
      emit_byte(OP_POP);

      gen(while_stmt->block_stmt);
      emit_loop(loop_start);

      patch_jump(exit_jump);
      emit_byte(OP_POP);

      break;
    }
    case AST_FOR: {
      ForStmt* for_stmt = (ForStmt*)ast->as;

      // Generate the assignment, assuming it is a completely
      // new initialization
      gen(for_stmt->assignment_stmt);

      int loop_start = current_chunk()->count;

      // Generate the condition expression
      gen(for_stmt->condition_expr);

      int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
      emit_byte(OP_POP);

      int body_jump = emit_jump(OP_JUMP);
      int increment_start = current_chunk()->count;

      // Generate the then expression
      gen(for_stmt->then_expr);
      emit_byte(OP_POP);

      emit_loop(loop_start);
      loop_start = increment_start;
      patch_jump(body_jump);

      // Generate main function body
      gen(for_stmt->block_stmt);
      emit_loop(loop_start);

      patch_jump(exit_jump);
      emit_byte(OP_POP);

      break;
    }
    case AST_BLOCK: {
      BlockStmt* block_stmt = (BlockStmt*)ast->as;
      begin_scope(current_compiler);
      // For every statement inside the block, do the codegen
      for (int i = 0; i < block_stmt->ast_array.count; i++) {
        gen(block_stmt->ast_array.ast[i]);
      }
      close_scope(current_compiler);
      while (current_compiler->local_array.count > 0 &&
             current_compiler->local_array
                     .locals[current_compiler->local_array.count - 1]
                     .depth > current_compiler->scope_depth) {
        emit_byte(OP_POP);
        current_compiler->local_array.count--;
      }
      break;
    }
    case AST_FUNC: {
      FuncStmt* func_stmt = (FuncStmt*)ast->as;

      // Initialize another compiler instance
      Compiler compiler;
      init_compiler(&compiler, TYPE_FUNCTION, func_stmt->name);

      // This begin_scope has no close_scope, as it will close with
      // end_compiler
      begin_scope(current_compiler);

      // Generate parameters as local variables here
      bool exists = false;
      for (int i = 0; i < func_stmt->parameters->count; i++) {
        for (int j = current_compiler->local_array.count - 1; j >= 0; j--) {
          Local* local = &current_compiler->local_array.locals[i];
          if (local->depth != -1 &&
              local->depth < current_compiler->scope_depth) {
            break;
          }

          if (identifier_equal(&local->name,
                               &func_stmt->parameters->tokens[i])) {
            printf(
                "There is already a variable with this name in this "
                "scope.\n");
            exists = true;
          }
        }

        if (!exists) {
          Local* local = &current_compiler->local_array
                              .locals[current_compiler->local_array.count++];
          local->name = func_stmt->parameters->tokens[i];
          local->depth = current_compiler->scope_depth;
        }
      }

      // Emit byte-code for the block statement
      gen(func_stmt->stmt);
      ObjFunc* func = end_compiler();

      // emit the function as a constant
      func->arity = func_stmt->arity;
      Value func_value = OBJ_VAL(func);
      emit_constant(func_value);

      emit_byte(OP_DEFINE_GLOBAL);

      // emit function name
      ObjString* name = func->name;
      Value string_value = OBJ_VAL(name);
      emit_constant(string_value);
      break;
    }
    case AST_VARIABLE_STMT: {
      VariableStmt* variable_stmt = (VariableStmt*)ast->as;
      Token name = variable_stmt->name;

      // Note that scope_depth 0 is the global scope
      // If this variable is a local variable
      if (current_compiler->scope_depth != 0) {
        // Check whether there is a variable of the same name
        // in the same local scope

        for (int i = current_compiler->local_array.count - 1; i >= 0; i--) {
          Local* local = &current_compiler->local_array.locals[i];
          if (local->depth != -1 &&
              local->depth < current_compiler->scope_depth) {
            break;
          }

          if (identifier_equal(&name, &local->name)) {
            PRINT_TOKEN_STRING(local->name);
            PRINT_TOKEN_STRING(name);
            printf(
                "There already exists a variable of this name in this "
                "scope\n");
            return;
          }
        }

        if (current_compiler->local_array.count == UINT8_MAX + 1) {
          printf("Tried to add more than 256 locals while codegen\n");
          return;
        }

        // Using the token, add to the local array
        printf("local_array count: %d\n", current_compiler->local_array.count);
        Local* local = &current_compiler->local_array
                            .locals[current_compiler->local_array.count++];
        local->name = name;
        local->depth = current_compiler->scope_depth;
      }

      if (variable_stmt->initializer_expr->type != AST_NONE)
        gen(variable_stmt->initializer_expr);

      int variable_scope = resolve_local(&name);
      // Not a local variable
      if (variable_scope == -1) {
        emit_byte(OP_SET_GLOBAL);
      }

      if (variable_stmt->initializer_expr->type != AST_NONE &&
          variable_stmt->initialized == false) {
        variable_stmt->initialized = true;
      }

      ObjString* variable_name = make_obj_string(variable_stmt->name.start,
                                                 variable_stmt->name.length);

      // Only make constant for when its in the global scope
      if (current_compiler->scope_depth == 0) {
        Value variable_name_value = OBJ_VAL(variable_name);
        make_constant(variable_name_value);
      }
      break;
    }
    case AST_NUMBER: {  // emit a constant
      NumberExpr* number_expr = (NumberExpr*)ast->as;
      emit_constant(NUMBER_VAL(number_expr->value));
      break;
    }
    case AST_BINARY: {
      BinaryExpr* binary_expr = (BinaryExpr*)ast->as;
      gen(binary_expr->left_expr);
      gen(binary_expr->right_expr);
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
      int variable_scope = resolve_local(&name);
      if (variable_scope == -1)
        emit_byte(OP_GET_GLOBAL);
      else
        emit_byte(OP_GET_LOCAL);

      // Find the constant
      // For global scope
      if (variable_scope == -1) {
        bool found_variable = false;
        for (int i = 0; i < current_chunk()->constants.count; i++) {
          Value value = current_chunk()->constants.values[i];
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
        for (int i = current_compiler->local_array.count - 1; i >= 0; i--) {
          Local* local = &current_compiler->local_array.locals[i];
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

      int variable_scope = resolve_local(&assignment_expr->name);
      if (variable_scope == -1) {
        emit_byte(OP_SET_GLOBAL);

        ObjString* variable_name = make_obj_string(
            assignment_expr->name.start, assignment_expr->name.length);
        Value variable_name_value = OBJ_VAL(variable_name);
        make_constant(variable_name_value);
      } else {
        emit_byte(OP_SET_LOCAL);
        emit_byte(variable_scope);
      }
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
    case AST_CALL: {
      CallExpr* call_expr = (CallExpr*)ast->as;

      VariableExpr* variable_expr = (VariableExpr*)call_expr->callee->as;
      ObjString* token_string = make_obj_string_from_token(variable_expr->name);
      Value token_string_value = OBJ_VAL(token_string);

      // Get the function here
      emit_byte(OP_GET_GLOBAL);
      make_constant(token_string_value);

      // printf("AST_CALL argument_count: %d\n", call_expr->arguments->count);
      for (int i = 0; i < call_expr->arguments->count; i++) {
        gen(call_expr->arguments->ast[i]);
      }

      emit_byte(OP_CALL);
      emit_constant(token_string_value);

      // emit argument count
      emit_byte(call_expr->arguments->count);

      break;
    }
    case AST_RETURN: {
      ReturnStmt* return_stmt = (ReturnStmt*)ast->as;

      // Check for when the user tries to return from top level function body
      if (current_compiler->func_type == TYPE_SCRIPT) {
        printf("Cannot return from top level function body\n");
        return;
      }

      // If user writes return; in the function body
      // it will return nil by default
      if (return_stmt->value_expr->type == AST_NONE) {
        emit_byte(OP_NIL);
      } else {
        gen(return_stmt->value_expr);
      }

      emit_byte(OP_RETURN);
      break;
    }
  }
}

ObjFunc* codegen(AstArray* ast_arr) {
  // Create the compiler instance that tracks scope and depth
  Compiler compiler;
  Token null_token;
  null_token.type = TOKEN_NIL;
  null_token.start = "Top-level";
  null_token.length = strlen("Top-Level");
  null_token.line = 0;
  init_compiler(&compiler, TYPE_SCRIPT, null_token);

  // Track which compiler is being used
  current_compiler = &compiler;

  for (int i = 0; i < ast_arr->count; i++) {
    gen(ast_arr->ast[i]);
  }

  ObjFunc* main_func = end_compiler();
  return main_func;
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
        i++;  // name index
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
      case OP_LOOP:
        break;
      case OP_CALL:
        printf("[%d] [%-20s]\n", i, "OP_CALL");
        break;
      case OP_NIL:
        printf("[%d] [%-20s]\n", i, "OP_NIL");
        break;
    }
  }
}
