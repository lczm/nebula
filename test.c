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

static void test_1() {
    printf("Test-pass\n");
    pass();
}

int main(int argc, const char* argv[]) {
    printf("[---Starting tests---]\n");

    test_1();

    printf("[---Tests results---]\n");
    printf("Pass : %d\n", pass_count);
    printf("Fail : %d\n", fail_count);
    return 0;
}
