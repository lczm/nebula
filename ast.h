#pragma once

#include <stdbool.h>
#include "token.h"

typedef enum {
    AST_NONE,
    AST_PRINT,
    AST_VARIABLE,
    AST_NUMBER,
    AST_BINARY,
    AST_UNARY,
    AST_BOOL,
} AstType;

typedef struct {
    AstType type;
    void*   as;
} Ast;

// This should only be used for debugging
// until built-in functions work
typedef struct {
    Ast* expr;
} PrintStmt;

typedef struct {
    Token name;
    Ast* initializer_expr;
} VariableStmt;

typedef struct {
    double value;
} NumberExpr;

typedef struct {
    Ast* left_expr;
    Ast* right_expr;
    Token op;
} BinaryExpr;

typedef struct {
    Ast* right_expr;
    Token op;
} UnaryExpr;

typedef struct {
    bool value;
} BoolExpr;

bool is_stmt(Ast* ast);
bool is_expr(Ast* ast);

Ast* make_ast();

// Statements
PrintStmt* make_print_stmt(Ast* expr);
VariableStmt* make_variable_stmt(Token name, Ast* initializer_expr);

// Expressions
NumberExpr* make_number_expr(double value);
BinaryExpr* make_binary_expr(Ast* left_expr, Ast* right_expr, Token op);
UnaryExpr* make_unary_expr(Ast* right_expr, Token op);
BoolExpr* make_bool_expr(bool value);

// Debugging
void disassemble_ast(Ast* ast);
