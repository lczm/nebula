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

static Token peek(int offset) {
    return token_array->tokens[index + offset];
}

static bool peek_match(int offset, TokenType type) {
    Token token = token_array->tokens[index + offset];
    if (token.type == type)
        return true;
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

    // This will return a binary expression as well
    if (match_either(TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL)) {
        Token operator = get_current();
        move();
        Ast* right = comparison();
        BinaryExpr* binary_expr = make_binary_expr(ast, right, operator);
        Ast* binary_ast = make_ast();
        binary_ast->type = AST_BINARY;
        binary_ast->as = binary_expr;
        return binary_ast;
    }

    return ast;
}
 
static Ast* comparison() {
    Ast* ast = addition();
    return ast;
}
 
static Ast* addition() {
    Ast* ast = multiplication(); // number_expr

    // Add or subtract
    if (match_either(TOKEN_PLUS, TOKEN_MINUS)) {
        Token operator = get_current();
        move();
        Ast* right = multiplication(); // number_expr
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

    // Multiply or divide
    if (match_either(TOKEN_STAR, TOKEN_SLASH)) {
        Token operator = get_current();
        move();
        Ast* right = unary();
        BinaryExpr* binary_expr = make_binary_expr(ast, right, operator);
        // Create ast wrapper
        Ast* binary_ast = make_ast();
        binary_ast->type = AST_BINARY;
        binary_ast->as = binary_expr;
        return binary_ast;
    }

    return ast;
}
 
static Ast* unary() {
    if (match_either(TOKEN_BANG, TOKEN_MINUS)) {
        Token operator = get_current();
        move();
        UnaryExpr* unary_expr = make_unary_expr(unary(), operator);
        // Create ast wrapper
        Ast* unary_ast = make_ast();
        unary_ast->type = AST_UNARY;
        unary_ast->as = unary_expr;
        return unary_ast;
    }

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
        // printf("Parsing number: %f\n", value);
        // Print out debug information before moving
        move();
        NumberExpr* number_expr = make_number_expr(value);
        ast->as = number_expr;
        ast->type = AST_NUMBER;
    } else if (match(TOKEN_TRUE)) {
        move();
        BoolExpr* bool_expr = make_bool_expr(true);
        ast->as = bool_expr;
        ast->type = AST_BOOL;
    } else if (match(TOKEN_FALSE)) {
        move();
        BoolExpr* bool_expr = make_bool_expr(false);
        ast->as = bool_expr;
        ast->type = AST_BOOL;
    }

    return ast;
}
