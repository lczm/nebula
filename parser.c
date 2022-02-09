#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

// Parser state
static int index = 0;
static TokenArray* token_array;

static void move() {
    index++;
}

static bool match(TokenType type) {
    if (token_array->tokens[index].type == type)
        return true;
    return false;
}

static bool match_either(TokenType type1, TokenType type2) {
    if (token_array->tokens[index].type == type1 ||
        token_array->tokens[index].type == type2) {
        return true;
    }
    return false;
}

static bool match_and_move(TokenType type) {
    if (token_array->tokens[index].type == type) {
        index++;
        return true;
    }
    return false;
}

static Token get_current() {
    return token_array->tokens[index];
}

static Token get_previous() {
    return token_array->tokens[index--];
}

// Statements
static Ast* declaration();
static Ast* statement();
static Ast* expression_statement();

// Expressions
static Ast* expression();
static Ast* assignment();
static Ast* and_();
static Ast* or_();
static Ast* equality();
static Ast* comparison();
static Ast* addition();
static Ast* multiplication();
static Ast* unary();
static Ast* call();
static Ast* primary();

Ast* parse_tokens(TokenArray* token_arr) {
    // Initialize the static variables for convenience
    index = 0; // Reset to 0 just in case
    token_array = token_arr;

    Ast* ast = declaration();
    return ast;
}

static Ast* declaration() {
    Ast* ast = statement();
    return ast;
}

static Ast* statement() {
    Ast* ast = expression_statement();
    return ast;
}

static Ast* expression_statement() {
    Ast* ast = expression();
    return ast;
}

static Ast* expression() {
    Ast* ast = assignment();
    return ast;
}

static Ast* assignment() {
    Ast* ast = and_();
    return ast;
}
 
static Ast* and_() {
    Ast* ast = or_();
    return ast;
}
 
static Ast* or_() {
    Ast* ast = equality();
    return ast;
}
 
static Ast* equality() {
    Ast* ast = comparison();
    return ast;
}
 
static Ast* comparison() {
    Ast* ast = addition();
    return ast;
}
 
static Ast* addition() {
    Ast* ast = multiplication(); // number_expr

    if (match(TOKEN_PLUS)) {
        Token operator = get_current(); // operator
        move();
        Ast* right = primary(); // number_expr
        BinaryExpr* binary_expr = make_binary_expr(ast, right, operator);
        // Create ast wrapper
        Ast* binary_ast = make_ast();
        binary_ast->type = AST_BINARY;
        binary_ast->as = binary_expr;
        return binary_ast;
    } else if (match(TOKEN_MINUS)) {
        Token operator = get_current();
        move();
        Ast* right = primary(); // number_expr
        BinaryExpr* binary_expr = make_binary_expr(ast, right, operator);
        // Create ast wrapper
        Ast* binary_ast = make_ast();
        binary_ast->type = AST_BINARY;
        binary_ast->as = binary_expr;
        return binary_ast;
    }

    return ast;
}
 
static Ast* multiplication() {
    Ast* ast = unary();
    return ast;
}
 
static Ast* unary() {
    Ast* ast = call();
    return ast;
}
 
static Ast* call() {
    Ast* ast = primary();
    return ast;
}
 
static Ast* primary() {
    Ast* ast = make_ast();

    if (match(TOKEN_NUMBER)) {
        const char* start = token_array->tokens[index].start;
        char* end = (char*)token_array->tokens[index].start +
            token_array->tokens[index].length;
        double value = strtod(start, &end);
        printf("Parsing number: %f\n", value);
        // Print out debug information before moving
        move();
        NumberExpr* number_expr = make_number_expr(value);
        ast->as = number_expr;
        ast->type = AST_NUMBER;
    }

    return ast;
}
