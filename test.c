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

    char source[] = ".;";
    lex_source(&token_array, source);

    // assertions
    for (int i = 0; i < token_array.count; i++)
        if (token_array.tokens[i].length != 1) FAIL();

    if (token_array.tokens[0].type != TOKEN_DOT) FAIL();
    if (token_array.tokens[1].type != TOKEN_SEMICOLON) FAIL();

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
