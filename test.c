#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "array.h"
#include "codegen.h"
#include "debugging.h"
#include "hashmap.h"
#include "lexer.h"
#include "macros.h"
#include "object.h"
#include "parser.h"
#include "value.h"
#include "vm.h"

#define TEST_DEBUGGING
// Comment out the undef if need to debug tests
#undef TEST_DEBUGGING

static int pass_count = 0;
static int fail_count = 0;

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

Vm* run_source_return_vm(const char* source) {
  TokenArray token_array;
  init_token_array(&token_array);
  lex_source(&token_array, source);
#ifdef TEST_DEBUGGING
  disassemble_token_array(&token_array);
#endif

  AstArray ast_array;
  init_ast_array(&ast_array);
  parse_tokens(&token_array, &ast_array);

#ifdef TEST_DEBUGGING
  disassemble_ast(&ast_array);
#endif

  OpArray op_array;
  ValueArray ast_constants_array;
  init_op_array(&op_array);
  init_value_array(&ast_constants_array);
  codegen(&op_array, &ast_constants_array, &ast_array);

  // Temporary, to get out of the VM loop
  push_op_array(&op_array, OP_RETURN);
#ifdef TEST_DEBUGGING
  disassemble_opcode_values(&op_array, &ast_constants_array);
#endif

  // Vm vm;
  Vm* vm = ALLOCATE(Vm, 1);
  init_vm(vm, false);
  run(vm, &op_array, &ast_constants_array);

  // free_vm(&vm);
  // free_op_array(&op_array);
  // free_value_array(&ast_constants_array);
  // free_token_array(&token_array);
  // free(source);

  return vm;
}

static void pass() {
  pass_count++;
}

static void fail() {
  fail_count++;
}

static void reset_count() {
  pass_count = 0;
  fail_count = 0;
}

#define PASS() \
  do {         \
    pass();    \
    return;    \
  } while (0);

#define FAIL()                                \
  do {                                        \
    printf("FAILED at line :%d\n", __LINE__); \
    fail();                                   \
    return;                                   \
  } while (0);

static void test_ast_wrap() {
  printf("test_ast_wrap()\n");

  Ast* number_ast = wrap_ast(make_number_expr(5), AST_NUMBER);
  if (number_ast->type != AST_NUMBER)
    FAIL();
  if (((NumberExpr*)number_ast->as)->value != 5)
    FAIL();

  NumberExpr* number_1 = make_number_expr(6);
  NumberExpr* number_2 = make_number_expr(10);
  Ast* binary_ast = wrap_ast(
      make_binary_expr(wrap_ast(number_1, AST_NUMBER),
                       wrap_ast(number_2, AST_NUMBER), make_token(TOKEN_PLUS)),
      AST_BINARY);
  if (binary_ast->type != AST_BINARY)
    FAIL();
  BinaryExpr* binary_expr = (BinaryExpr*)binary_ast->as;
  if (binary_expr->left_expr->type != AST_NUMBER)
    FAIL();
  if (binary_expr->right_expr->type != AST_NUMBER)
    FAIL();
  if ((((NumberExpr*)binary_expr->left_expr->as)->value) != 6)
    FAIL();
  if ((((NumberExpr*)binary_expr->right_expr->as)->value) != 10)
    FAIL();

  PASS();
}

static void test_token_array() {
  printf("test_token_array()\n");

  TokenArray token_array;
  init_token_array(&token_array);

  // Check the init variables
  if (token_array.capacity != 1 || token_array.count != 0) {
    FAIL();
  }

  // Push variables in
  Token token_and = {
      .type = TOKEN_AND,
      .start = NULL,
      .length = 0,
      .line = 1,
  };
  push_token_array(&token_array, token_and);
  if (token_array.capacity != 1 || token_array.count != 1) {
    FAIL();
  }

  // Loop push variables in
  int count = token_array.count;
  int capacity = token_array.capacity;
  int loop_count = 1000;
  for (int i = 0; i < loop_count; i++) {
    Token token_temp = {
        .type = TOKEN_LET,
        .start = NULL,
        .length = 0,
        .line = 1,
    };
    push_token_array(&token_array, token_temp);
    // Implement the logic of how the arrays should expand
    if (count + i > capacity) {
      capacity *= 2;
    }
  }

  // assert logic is correct
  if (capacity != token_array.capacity) {
    FAIL();
  }
  if (count + loop_count != token_array.count) {
    FAIL();
  }

  // assert the token types are correct
  for (int i = 0; i < token_array.count; i++) {
    if (i == 0) {
      if (token_array.tokens[i].type != TOKEN_AND) {
        FAIL();
      }
    } else {
      if (token_array.tokens[i].type != TOKEN_LET) {
        FAIL();
      }
    }
  }

  free_token_array(&token_array);
  PASS();
}

static void test_op_array() {
  printf("test_op_array()\n");

  OpArray op_array;
  init_op_array(&op_array);

  // Check the init variables
  if (op_array.capacity != 1 || op_array.count != 0) {
    FAIL();
  }

  // Push variables in
  OpCode op_code = OP_ADD;
  push_op_array(&op_array, OP_ADD);
  if (op_array.capacity != 1 || op_array.count != 1) {
    FAIL();
  }

  // Loop push variables in
  int count = op_array.count;
  int capacity = op_array.capacity;
  int loop_count = 1000;
  for (int i = 0; i < loop_count; i++) {
    OpCode op_code = OP_CONSTANT;
    push_op_array(&op_array, op_code);
    // Implement the logic of how arrays should expand
    if (count + i > capacity) {
      capacity *= 2;
    }
  }

  // assert logic is correct
  if (capacity != op_array.capacity) {
    FAIL();
  }
  if (count + loop_count != op_array.count) {
    FAIL();
  }

  // assert the OpCode are correct
  for (int i = 0; i < op_array.count; i++) {
    if (i == 0) {
      if (op_array.ops[i] != OP_ADD) {
        FAIL();
      }
    } else {
      if (op_array.ops[i] != OP_CONSTANT) {
        FAIL();
      }
    }
  }

  free_op_array(&op_array);
  PASS();
}

static void test_value_array() {
  printf("test_value_array()\n");

  ValueArray value_array;
  init_value_array(&value_array);

  // Check the init variables
  if (value_array.capacity != 1 || value_array.count != 0) {
    FAIL();
  }

  // Push a variable in
  Value value_one = NUMBER_VAL(0.0);
  push_value_array(&value_array, value_one);
  if (value_array.capacity != 1 || value_array.count != 1) {
    FAIL();
  }

  // Loop push variables in
  int count = value_array.count;
  int capacity = value_array.capacity;
  int offset = 1;
  int loop_count = 1000;
  for (int i = offset; i < loop_count; i++) {
    Value value_number = NUMBER_VAL((double)i);
    push_value_array(&value_array, value_number);
    if (count + i > capacity) {
      capacity *= 2;
    }
  }

  // assret the logic is correct
  if (capacity != value_array.capacity) {
    FAIL();
  }
  if (count + loop_count - offset != value_array.count) {
    FAIL();
  }

  // assert the values in value_array are correct
  for (int i = 0; i < value_array.count; i++) {
    Value value = value_array.values[i];
    if (!IS_NUMBER(value)) {
      FAIL();
    }
    if (AS_NUMBER(value) != (double)i) {
      FAIL();
    }
  }

  free_value_array(&value_array);
  PASS();
}

static void test_ast_array() {
  printf("test_ast_array()\n");

  AstArray ast_array;
  init_ast_array(&ast_array);

  NumberExpr* number_expr = make_number_expr(5);
  Ast* ast_number = wrap_ast(make_number_expr(5), AST_NUMBER);

  PASS();
}

static void test_hashmap() {
  printf("test_hashmap()\n");

  HashMap hashmap;
  init_hashmap(&hashmap);

  if (hashmap.count != 0)
    FAIL();
  if (hashmap.capacity != 0)
    FAIL();

  Value set_number_value = NUMBER_VAL(10);
  ObjString* number_key = make_obj_string("test_number", strlen("test_number"));
  push_hashmap(&hashmap, number_key, set_number_value);

  if (hashmap.count != 1)
    FAIL();
  if (hashmap.capacity != 4)
    FAIL();

  Value get_number_value = get_hashmap(&hashmap, number_key);
  if (!IS_NUMBER(get_number_value))
    FAIL();
  if (AS_NUMBER(get_number_value) != 10)
    FAIL();

  Value set_boolean_value = BOOLEAN_VAL(true);
  ObjString* boolean_key =
      make_obj_string("test_boolean", strlen("test_boolean"));
  push_hashmap(&hashmap, boolean_key, set_boolean_value);

  if (hashmap.count != 2)
    FAIL();
  if (hashmap.capacity != 4)
    FAIL();

  Value get_boolean_value = get_hashmap(&hashmap, boolean_key);
  if (!IS_BOOLEAN(get_boolean_value))
    FAIL();
  if (AS_BOOLEAN(get_boolean_value) != true)
    FAIL();

  free_hashmap(&hashmap);
  PASS();
}

static void test_single_character_lexer() {
  printf("test_single_character_lexer()\n");

  TokenArray token_array;
  init_token_array(&token_array);
  // space out the ones that can have double character tokens
  char source[] = "(){},.-+;/* ! = < >";
  int types[] = {
      TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
      TOKEN_COMMA,      TOKEN_DOT,         TOKEN_MINUS,      TOKEN_PLUS,
      TOKEN_SEMICOLON,  TOKEN_SLASH,       TOKEN_STAR,       TOKEN_BANG,
      TOKEN_EQUAL,      TOKEN_LESS,        TOKEN_GREATER,
  };
  lex_source(&token_array, source);

  // assertions
  for (int i = 0; i < token_array.count; i++) {
    if (token_array.tokens[i].length != 1)
      FAIL();
    if (token_array.tokens[i].type != types[i])
      FAIL();
  }

  free_token_array(&token_array);
  PASS();
}

static void test_double_character_lexer() {
  printf("test_double_character_lexer()\n");

  TokenArray token_array_length_two;
  init_token_array(&token_array_length_two);

  char source[] = "!= == >= <=";
  int length_two_types[] = {
      TOKEN_BANG_EQUAL,
      TOKEN_EQUAL_EQUAL,
      TOKEN_GREATER_EQUAL,
      TOKEN_LESS_EQUAL,
  };
  lex_source(&token_array_length_two, source);

  for (int i = 0; i < token_array_length_two.count; i++) {
    if (token_array_length_two.tokens[i].length != 2)
      FAIL();
    if (token_array_length_two.tokens[i].type != length_two_types[i])
      FAIL();
  }

  free_token_array(&token_array_length_two);
  PASS();
}

static void test_keyword_character_lexer() {
  printf("test_keyword_character_lexer()\n");

  TokenArray token_array_keywords;
  init_token_array(&token_array_keywords);

  char source[] = "else for func if else nil return let while true false";
  int keyword_types[] = {TOKEN_ELSE,  TOKEN_FOR,  TOKEN_FUNC,   TOKEN_IF,
                         TOKEN_ELSE,  TOKEN_NIL,  TOKEN_RETURN, TOKEN_LET,
                         TOKEN_WHILE, TOKEN_TRUE, TOKEN_FALSE};
  lex_source(&token_array_keywords, source);

  for (int i = 0; i < token_array_keywords.count; i++) {
    if (token_array_keywords.tokens[i].type != keyword_types[i])
      FAIL();
  }

  free_token_array(&token_array_keywords);
  PASS();
}

static void test_parse_binary_expressions() {
  printf("test_parse_binary_expressions()\n");

  char test_string[] =
      "let a = 10 + 1;"
      "let b = 10 - 1;"
      "let c = (10 + 1) * 3;"
      "let d = (10 + 2) / 3;"
      "let e = ((10 + 2) / 3) * 2;";

  Vm* vm = run_source_return_vm(test_string);
  HashMap* variables = &vm->variables;

  ObjString* obj_string_a = make_obj_string_sl("a");
  ObjString* obj_string_b = make_obj_string_sl("b");
  ObjString* obj_string_c = make_obj_string_sl("c");
  ObjString* obj_string_d = make_obj_string_sl("d");
  ObjString* obj_string_e = make_obj_string_sl("e");
  Value value_a = get_hashmap(variables, obj_string_a);
  Value value_b = get_hashmap(variables, obj_string_b);
  Value value_c = get_hashmap(variables, obj_string_c);
  Value value_d = get_hashmap(variables, obj_string_d);
  Value value_e = get_hashmap(variables, obj_string_e);

  if (!IS_NUMBER(value_a) || !IS_NUMBER(value_b) || !IS_NUMBER(value_c) ||
      !IS_NUMBER(value_d) || !IS_NUMBER(value_e))
    FAIL();

  if (AS_NUMBER(value_a) != 11.0)
    FAIL();
  if (AS_NUMBER(value_b) != 9.0)
    FAIL();
  if (AS_NUMBER(value_c) != 33.0)
    FAIL();
  if (AS_NUMBER(value_d) != 4.0)
    FAIL();
  if (AS_NUMBER(value_e) != 8.0)
    FAIL();

  PASS();
}

static void test_parse_unary_expressions() {
  printf("test_parse_unary_expressions()\n");

  char test_string[] =
      "let a = -3;"
      "let b = !(1 == 1);"
      "let c = !true;"
      "let d = !false;";

  Vm* vm = run_source_return_vm(test_string);
  HashMap* variables = &vm->variables;

  ObjString* obj_string_a = make_obj_string_sl("a");
  ObjString* obj_string_b = make_obj_string_sl("b");
  ObjString* obj_string_c = make_obj_string_sl("c");
  ObjString* obj_string_d = make_obj_string_sl("d");
  Value value_a = get_hashmap(variables, obj_string_a);
  Value value_b = get_hashmap(variables, obj_string_b);
  Value value_c = get_hashmap(variables, obj_string_c);
  Value value_d = get_hashmap(variables, obj_string_d);

  if (!IS_NUMBER(value_a))
    FAIL();
  if (!IS_BOOLEAN(value_b))
    FAIL();
  if (!IS_BOOLEAN(value_c))
    FAIL();
  if (!IS_BOOLEAN(value_d))
    FAIL();

  if (AS_NUMBER(value_a) != -3.0)
    FAIL();
  if (AS_BOOLEAN(value_b) != false)
    FAIL();
  if (AS_BOOLEAN(value_c) != false)
    FAIL();
  if (AS_BOOLEAN(value_d) != true)
    FAIL();

  PASS();
}

static void test_codegen_numbers() {
  printf("test_codegen_numbers()\n");
  PASS();
}

static void test_codegen_binary_numbers() {
  printf("test_codegen_binary_numbers()\n");
  PASS();
}

static void test_obj_string() {
  printf("test_obj_string()\n");

  char test_string[] = "test string";
  ObjString* obj_string = make_obj_string(test_string, strlen(test_string));

  // Can only check length and hash
  if (obj_string->length != 11)
    FAIL();
  // The hash value for "test string"
  if (obj_string->hash != 2533107786)
    FAIL();

  PASS();
}

static void test_vm_global_environment() {
  printf("test_vm_global_environment()\n");

  char test_string[] = "let test1 = 10;";
  ObjString* obj_string = make_obj_string(test_string, strlen(test_string));
  // Test obj_string length
  if (obj_string->length != 15)
    FAIL();

  Vm* vm = run_source_return_vm(test_string);

  HashMap* variables = &vm->variables;
  if (variables->count != 1)
    FAIL();

  char variable_string_test1[] = "test1";
  Value value = get_hashmap(
      variables,
      make_obj_string(variable_string_test1, strlen(variable_string_test1)));

  // Check that the value of the variable is 10
  if (!IS_NUMBER(value))
    FAIL();

  if (AS_NUMBER(value) != 10.0) {
    printf("%f\n", AS_NUMBER(value));
    FAIL();
  }

  PASS();
}

static void test_vm_string_concatenation() {
  printf("test_vm_string_concatenation()\n");

  char test_string1[] =
      "let a = \"hello\";"
      "let b = \"there\";"
      "let c = a + b;";

  Vm* vm = run_source_return_vm(test_string1);
  HashMap* variables = &vm->variables;
  if (variables->count != 3)
    FAIL();

  ObjString* obj_string_a = make_obj_string_sl("a");
  ObjString* obj_string_b = make_obj_string_sl("b");
  ObjString* obj_string_c = make_obj_string_sl("c");
  Value value_a = get_hashmap(variables, obj_string_a);
  Value value_b = get_hashmap(variables, obj_string_b);
  Value value_c = get_hashmap(variables, obj_string_c);

  if (!IS_OBJ(value_a) && OBJ_TYPE(value_a) != OBJ_STRING)
    FAIL();
  if (!IS_OBJ(value_b) && OBJ_TYPE(value_b) != OBJ_STRING)
    FAIL();
  if (!IS_OBJ(value_c) && OBJ_TYPE(value_c) != OBJ_STRING)
    FAIL();

  ObjString* obj_a = AS_OBJ_STRING(value_a);
  ObjString* obj_b = AS_OBJ_STRING(value_b);
  ObjString* obj_c = AS_OBJ_STRING(value_c);

  if (obj_a->length != 5)
    FAIL();
  if (obj_b->length != 5)
    FAIL();
  if (obj_c->length != obj_a->length + obj_b->length)
    FAIL();

  // Check the first half of the concatenation
  for (int i = 0; i < obj_a->length; i++) {
    if (obj_c->chars[i] != obj_a->chars[i])
      FAIL();
  }
  // Check the second half of the concatenation
  for (int i = obj_a->length; i < obj_a->length + obj_b->length; i++) {
    if (obj_c->chars[i] != obj_b->chars[i - obj_b->length])
      FAIL();
  }

  PASS();
}

static void test_vm_order_of_operations() {
  printf("test_vm_order_of_operations()\n");

  char test_string1[] =
      "let a1 = 10;"
      "let b1 = 20;"
      "let c1 = a1 + b1;";

  Vm* vm = run_source_return_vm(test_string1);
  HashMap* variables = &vm->variables;
  if (variables->count != 3)
    FAIL();

  ObjString* obj_string_a1 = make_obj_string("a1", strlen("a1"));
  ObjString* obj_string_b1 = make_obj_string("b1", strlen("b1"));
  ObjString* obj_string_c1 = make_obj_string("c1", strlen("c1"));
  Value value_a1 = get_hashmap(variables, obj_string_a1);
  Value value_b1 = get_hashmap(variables, obj_string_b1);
  Value value_c1 = get_hashmap(variables, obj_string_c1);

  if (!IS_NUMBER(value_a1) || !IS_NUMBER(value_b1) || !IS_NUMBER(value_c1))
    FAIL();

  if (AS_NUMBER(value_c1) != AS_NUMBER(value_a1) + AS_NUMBER(value_b1))
    FAIL();

  if (AS_NUMBER(value_c1) != 30.0)
    FAIL();

  char test_string2[] =
      "let a2 = 10 / 2 + 3;"
      "let b2 = (10 + 5) / 3;"
      "let c2 = 3 - (10 * 5);"
      "let d2 = 3 + (10 * 5);";
  vm = run_source_return_vm(test_string2);
  variables = &vm->variables;
  if (variables->count != 4)
    FAIL();

  ObjString* obj_string_a2 = make_obj_string("a2", strlen("a2"));
  ObjString* obj_string_b2 = make_obj_string("b2", strlen("b2"));
  ObjString* obj_string_c2 = make_obj_string("c2", strlen("c2"));
  ObjString* obj_string_d2 = make_obj_string("d2", strlen("d2"));
  Value value_a2 = get_hashmap(variables, obj_string_a2);
  Value value_b2 = get_hashmap(variables, obj_string_b2);
  Value value_c2 = get_hashmap(variables, obj_string_c2);
  Value value_d2 = get_hashmap(variables, obj_string_d2);

  if (!IS_NUMBER(value_a2) || !IS_NUMBER(value_b2) || !IS_NUMBER(value_c2) ||
      !IS_NUMBER(value_d2))
    FAIL();

  if (AS_NUMBER(value_a2) != 8.0)
    FAIL();

  if (AS_NUMBER(value_b2) != 5.0)
    FAIL();

  if (AS_NUMBER(value_c2) != -47.0)
    FAIL();

  if (AS_NUMBER(value_d2) != 53)
    FAIL();

  PASS();
}

static void test_vm_augmented_assignments() {
  printf("test_vm_augmented_assignments\n");

  char test_string1[] =
      "let a = 0;"
      "a += 10;"
      "a -= 5;"
      "a *= 4;"
      "a /= 2;";

  Vm* vm = run_source_return_vm(test_string1);
  HashMap* variables = &vm->variables;

  ObjString* obj_string_a = make_obj_string_sl("a");
  Value value_a = get_hashmap(variables, obj_string_a);

  if (!IS_NUMBER(value_a))
    FAIL();

  if (AS_NUMBER(value_a) != 10.0)
    FAIL();

  PASS();
}

static void test_vm_comparison_operators() {
  printf("test_vm_comparison_operators\n");

  char test_string1[] =
      "let e = 0;"
      "let a = 5;"
      "let f = 0;"
      "if (a > 10) {"
      "  e = 10;"
      "}"
      "if (a < 10) {"
      "  e = 50;"
      "}"
      "if (f == 0) {"
      "  f = 100;"
      "}";

  Vm* vm = run_source_return_vm(test_string1);
  HashMap* variables = &vm->variables;

  ObjString* obj_string_e = make_obj_string_sl("e");
  ObjString* obj_string_f = make_obj_string_sl("f");
  Value value_e = get_hashmap(variables, obj_string_e);
  Value value_f = get_hashmap(variables, obj_string_f);

  if (!IS_NUMBER(value_e))
    FAIL();
  if (AS_NUMBER(value_e) != 50.0)
    FAIL();

  if (!IS_NUMBER(value_f))
    FAIL();
  if (AS_NUMBER(value_f) != 100.0)
    FAIL();

  PASS();
}

static void test_vm_if_conditions() {
  printf("test_vm_if_conditions()\n");

  char test_string1[] =
      "let a = 10;"
      "let b = 0;"
      "if (a == 10) {"
      "    a = 50;"
      "}"
      "if (b == 10) {"
      "   b = 50;"
      "} else {"
      "   b = 100;"
      "}";

  Vm* vm = run_source_return_vm(test_string1);
  HashMap* variables = &vm->variables;
  if (variables->count != 2)
    FAIL();

  ObjString* obj_string_a = make_obj_string("a", strlen("a"));
  ObjString* obj_string_b = make_obj_string("b", strlen("b"));
  Value value_a = get_hashmap(variables, obj_string_a);
  Value value_b = get_hashmap(variables, obj_string_b);

  // Test then branch codegen & run
  if (!IS_NUMBER(value_a))
    FAIL();
  if (AS_NUMBER(value_a) != 50.0)
    FAIL();

  // Test else branch codegen & run
  if (!IS_NUMBER(value_b))
    FAIL();
  if (AS_NUMBER(value_b) != 100.0)
    FAIL();

  PASS();
}

static void test_vm_while_loops() {
  printf("test_vm_while_loops()\n");

  char test_string1[] =
      "let a = 0;"
      "while (a != 10) {"
      " a = a + 1;"
      "}";

  Vm* vm = run_source_return_vm(test_string1);
  HashMap* variables = &vm->variables;

  ObjString* obj_string_a = make_obj_string("a", strlen("a"));
  Value value_a = get_hashmap(variables, obj_string_a);

  if (!IS_NUMBER(value_a))
    FAIL();
  if (AS_NUMBER(value_a) != 10.0)
    FAIL();

  PASS();
}

static void test_vm_for_loops() {
  printf("test_vm_for_loops()\n");

  char test_string1[] =
      "let a = 0;"
      "for (let i = 0; i < 10; i += 1;) {"
      "  print i;"
      "  a += i;"
      "}";

  Vm* vm = run_source_return_vm(test_string1);
  HashMap* variables = &vm->variables;

  ObjString* obj_string_a = make_obj_string("a", strlen("a"));
  Value value_a = get_hashmap(variables, obj_string_a);

  if (!IS_NUMBER(value_a))
    FAIL();
  if (AS_NUMBER(value_a) != 55.0) {
    printf("value_a is :%f\n", AS_NUMBER(value_a));
    FAIL();
  }

  PASS();
}

int main(int argc, const char* argv[]) {
  clock_t start = clock();

  printf("[-----Starting tests-----]\n");

  // Test functions should go from the basics
  // to the bigger parts of the program that composes
  // the basic "parts" of the program
  // ast wrapping
  test_ast_wrap();
  // arrays
  test_token_array();
  test_op_array();
  test_value_array();
  test_ast_array();
  // hashmaps
  test_hashmap();
  // lexer
  test_single_character_lexer();
  test_double_character_lexer();
  test_keyword_character_lexer();
  // parser tests
  test_parse_binary_expressions();
  test_parse_unary_expressions();
  // codegen to ast tests
  test_codegen_numbers();
  test_codegen_binary_numbers();
  // obj tests
  test_obj_string();
  // vm tests
  test_vm_global_environment();
  test_vm_string_concatenation();
  test_vm_order_of_operations();
  test_vm_augmented_assignments();
  test_vm_comparison_operators();
  test_vm_if_conditions();
  test_vm_while_loops();
  test_vm_for_loops();

  printf("[-----Tests results-----]\n");
  printf("Pass : %d\n", pass_count);
  printf("Fail : %d\n", fail_count);

  // Time taken
  clock_t end = clock();
  double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
  printf("Time taken : %f seconds\n", time_taken);

  return 0;
}
