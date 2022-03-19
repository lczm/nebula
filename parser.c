#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "parser.h"

// Parser state
static int index = 0;
static int token_array_len = 0;
static TokenArray* token_array;
static AstArray* ast_array;

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

static bool eat(TokenType type) {
    if (get_current().type == type) {
        move();
        return true;
    }
    return false;
}

static void eat_or_error(TokenType type, const char* error) {
    if (!eat(type)) {
        printf("%s\n", error);
    }
}

// Statements
static Ast* declaration();
static Ast* var_declaration();
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

void parse_tokens(TokenArray* token_arr, AstArray* ast_arr) {
    // Initialize the static variables for convenience
    index = 0; // Reset to 0 just in case
    token_array = token_arr;
    token_array_len = token_arr->count;
    ast_array = ast_arr;

    while (index != token_arr->count) {
        push_ast_array(ast_array, declaration());
    }
    // return declaration();
}

static Ast* declaration() {
    if (match(TOKEN_LET)) {
        return var_declaration();
    }

    return statement();
}

static Ast* var_declaration() {
    move();
    if (!match(TOKEN_IDENTIFIER)) {
        printf("var_declaration could not find an identifier after 'let'\n");
    }

    Token identifier_name = get_current();
    move();

    // initialization assignment
    // example: let a =
    Ast* ast = make_ast();
    if (match(TOKEN_EQUAL)) {
        move();
        Ast* initializer_expr = expression();
        if (!match_and_move(TOKEN_SEMICOLON)) {
            printf("Did not have semicolon after variable declaration initialization\n");
        }
        VariableStmt* variable_stmt = make_variable_stmt(identifier_name, initializer_expr);
        ast->type = AST_VARIABLE;
        ast->as = variable_stmt;
        return ast;
    } else { // variable declaration without initialization, let a;
        Ast* ast_none = make_ast();
        VariableStmt* variable_stmt = make_variable_stmt(identifier_name, ast_none);
        ast->type = AST_VARIABLE;
        ast->as = variable_stmt;
        return ast;
    }
}

static Ast* statement() {
    Ast* ast = expression_statement();
    return ast;
}

static Ast* expression_statement() {
    if (match(TOKEN_PRINT)) {
        move();
        Ast* ast = expression();
        PrintStmt* print_stmt = make_print_stmt(ast);
        Ast* ast_stmt = make_ast();
        ast_stmt->as = print_stmt;
        ast_stmt->type = AST_PRINT;

        eat_or_error(TOKEN_SEMICOLON, "Must have ';' after statement");
        return ast_stmt;
    }

    return expression();

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
