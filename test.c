#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "array.h"
#include "lexer.h"
#include "parser.h"
#include "object.h"
#include "hashmap.h"

static int pass_count = 0;
static int fail_count = 0;

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

#define PASS() do { \
    pass();         \
    return;         \
} while (0);        \

#define FAIL() do {     \
    printf("FAILED\n"); \
    fail();             \
    return;             \
} while (0);            \

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

static void test_hashmap() {
    printf("test_hashmap\n");

    HashMap hashmap;
    init_hashmap(&hashmap);

    if (hashmap.count != 0)
        FAIL();
    if (hashmap.capacity != 0)
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
        TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
        TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
        TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS,
        TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH,
        TOKEN_STAR, TOKEN_BANG, TOKEN_EQUAL,
        TOKEN_LESS, TOKEN_GREATER,
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
        TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL,
        TOKEN_GREATER_EQUAL, TOKEN_LESS_EQUAL,
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

    char source[] = "else for func if nil return let while true false";
    int keyword_types[] = { TOKEN_ELSE, TOKEN_FOR, TOKEN_FUNC, TOKEN_IF,
        TOKEN_NIL, TOKEN_RETURN, TOKEN_LET, TOKEN_WHILE, TOKEN_TRUE,
        TOKEN_FALSE };
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

    TokenArray token_array;
    init_token_array(&token_array);
    Token token_number = make_token(TOKEN_NUMBER);
    token_number.start = "15";
    token_number.length = 2;
    push_token_array(&token_array, token_number);
    Token token_plus = make_token(TOKEN_PLUS);
    token_plus.start = "+";
    token_plus.length = 1;
    push_token_array(&token_array, token_plus);
    Token token_number2 = make_token(TOKEN_NUMBER);
    token_number2.start = "2";
    token_number2.length = 2;
    push_token_array(&token_array, token_number2);

    // For test debugging purposes
    Ast* ast = parse_tokens(&token_array);
    // Make sure its a binary
    if (ast->type != AST_BINARY)
        FAIL();
    BinaryExpr* binary_expr = (BinaryExpr*)ast->as;
    if (binary_expr->left_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->right_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->op.type != TOKEN_PLUS) 
        FAIL();
    if (((NumberExpr*)binary_expr->left_expr->as)->value != 15.0)
        FAIL();
    if (((NumberExpr*)binary_expr->right_expr->as)->value != 2.0)
        FAIL();

    // Change from plus to minus
    token_array.tokens[1].type = TOKEN_MINUS;
    token_array.tokens[1].start = "-";
    ast = parse_tokens(&token_array);
    // Make sure its a binary
    if (ast->type != AST_BINARY)
        FAIL();
    binary_expr = (BinaryExpr*)ast->as;
    if (binary_expr->left_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->right_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->op.type != TOKEN_MINUS) 
        FAIL();
    if (((NumberExpr*)binary_expr->left_expr->as)->value != 15.0)
        FAIL();
    if (((NumberExpr*)binary_expr->right_expr->as)->value != 2.0)
        FAIL();

    // Change from minus to star
    token_array.tokens[1].type = TOKEN_STAR;
    token_array.tokens[1].start = "*";
    ast = parse_tokens(&token_array);
    // Make sure its a binary
    if (ast->type != AST_BINARY)
        FAIL();
    binary_expr = (BinaryExpr*)ast->as;
    if (binary_expr->left_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->right_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->op.type != TOKEN_STAR) 
        FAIL();
    if (((NumberExpr*)binary_expr->left_expr->as)->value != 15.0)
        FAIL();
    if (((NumberExpr*)binary_expr->right_expr->as)->value != 2.0)
        FAIL();

    // Change from star to slash
    token_array.tokens[1].type = TOKEN_SLASH;
    token_array.tokens[1].start = "/";
    ast = parse_tokens(&token_array);
    // Make sure its a binary
    if (ast->type != AST_BINARY)
        FAIL();
    binary_expr = (BinaryExpr*)ast->as;
    if (binary_expr->left_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->right_expr->type != AST_NUMBER) 
        FAIL();
    if (binary_expr->op.type != TOKEN_SLASH) 
        FAIL();
    if (((NumberExpr*)binary_expr->left_expr->as)->value != 15.0)
        FAIL();
    if (((NumberExpr*)binary_expr->right_expr->as)->value != 2.0)
        FAIL();

    free_token_array(&token_array);
    PASS();
}

static void test_parse_unary_expressions() {
    printf("test_parse_unary_expressions()\n");

    TokenArray token_array;
    init_token_array(&token_array);
    Token token_minus = make_token(TOKEN_MINUS);
    token_minus.start = "-";
    token_minus.length = 1;
    push_token_array(&token_array, token_minus);
    Token token_number = make_token(TOKEN_NUMBER);
    token_number.start = "15";
    token_number.length = 2;
    push_token_array(&token_array, token_number);

    // disassemble_token_array(&token_array);
    Ast* ast = parse_tokens(&token_array);
    // disassemble_ast(ast);
    if (ast->type != AST_UNARY)
        FAIL();
    UnaryExpr* unary_expr = (UnaryExpr*)ast->as;
    if (unary_expr->op.type != TOKEN_MINUS)
        FAIL();
    if (unary_expr->right_expr->type != AST_NUMBER)
        FAIL();
    if (((NumberExpr*)unary_expr->right_expr->as)->value != 15.0)
        FAIL();

    // change from minus to bang
    token_array.tokens[0].type = TOKEN_BANG;
    token_array.tokens[0].start = "!";

    ast = parse_tokens(&token_array);
    if (ast->type != AST_UNARY)
        FAIL();
    unary_expr = (UnaryExpr*)ast->as;
    if (unary_expr->op.type != TOKEN_BANG)
        FAIL();
    if (unary_expr->right_expr->type != AST_NUMBER)
        FAIL();
    if (((NumberExpr*)unary_expr->right_expr->as)->value != 15.0)
        FAIL();

    free_token_array(&token_array);
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
    ObjString* obj_string = make_obj_string(&test_string, strlen(test_string));

    // Can only check length and hash
    if (obj_string->length != 11)
        FAIL();
    // The hash value for "test string"
    if (obj_string->hash != 2533107786)
        FAIL();

    PASS();
}

static void test_vm_stack() {
    printf("test_vm_stack()\n");
    PASS();
}

static void test_vm_eval_binary() {
    printf("test_vm_eval_binary()\n");
    PASS();
}

int main(int argc, const char* argv[]) {
    clock_t start = clock();

    printf("[-----Starting tests-----]\n");

    // Test functions should go from the basics
    // to the bigger parts of the program that composes
    // the basic "parts" of the program

    // arrays
    test_token_array();
    test_op_array();
    test_value_array();
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
    test_vm_stack();
    test_vm_eval_binary();

    printf("[-----Tests results-----]\n");
    printf("Pass : %d\n", pass_count);
    printf("Fail : %d\n", fail_count);

    // Time taken
    clock_t end = clock();
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Time taken : %f seconds\n", time_taken);

    return 0;
}
