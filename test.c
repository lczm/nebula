#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "array.h"
#include "lexer.h"

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

#define FAIL() do { \
    fail();         \
    return;         \
} while (0);        \

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
            .type = TOKEN_VAR,
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
            if (token_array.tokens[i].type != TOKEN_VAR) {
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

static void test_single_character_lexer() {
    printf("test_single_character_lexer()\n");

    TokenArray token_array;
    init_token_array(&token_array);
    char source[] = "(){},.-+;/*";
    int types[] = {
        TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
        TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
        TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS,
        TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH,
        TOKEN_STAR 
    };
    lex_source(&token_array, source);

    // assertions
    for (int i = 0; i < token_array.count; i++) {
        if (token_array.tokens[i].length != 1) FAIL();
        if (token_array.tokens[i].type != types[i]) FAIL();
    }

    free_token_array(&token_array);
    PASS();
}

int main(int argc, const char* argv[]) {
    // Time start
    clock_t start = clock();

    printf("[-----Starting tests-----]\n");

    // Test functions should go from the basics
    // to the bigger parts of the program that composes
    // the basic "parts" of the program

    // arrays
    test_token_array();
    test_op_array();
    test_value_array();
    // lexer
    test_single_character_lexer();

    printf("[-----Tests results-----]\n");
    printf("Pass : %d\n", pass_count);
    printf("Fail : %d\n", fail_count);

    clock_t end = clock();
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Time taken : %f seconds\n", time_taken);

    return 0;
}
