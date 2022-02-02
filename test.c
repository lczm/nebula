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

static void test_single_character_lexer() {
    printf("test-single_character_lexer()\n");

    TokenArray token_array;
    init_token_array(&token_array);

    char source[] = "(){},.-+;/*";
    lex_source(&token_array, source);
    int types[] = {
        TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
        TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
        TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS,
        TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH,
        TOKEN_STAR 
    };

    // assertions
    for (int i = 0; i < token_array.count; i++) {
        if (token_array.tokens[i].length != 1) FAIL();
        if (token_array.tokens[i].type != types[i]) FAIL();
    }

    PASS();
}

int main(int argc, const char* argv[]) {
    printf("[---Starting tests---]\n");

    test_single_character_lexer();

    printf("[---Tests results---]\n");
    printf("Pass : %d\n", pass_count);
    printf("Fail : %d\n", fail_count);
    return 0;
}
