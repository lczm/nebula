#pragma once

#include <stdbool.h>
#include "token.h"

typedef enum {
    AST_NONE,
    AST_NUMBER,
    AST_BINARY,
    AST_UNARY,
} AstType;

typedef struct {
    AstType type;
    void*   as;
} Ast;

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

bool is_stmt(Ast* ast);
bool is_expr(Ast* ast);

Ast* make_ast();
NumberExpr* make_number_expr(double value);
BinaryExpr* make_binary_expr(Ast* left_expr, Ast* right_expr, Token op);
UnaryExpr* make_unary_expr(Ast* right_expr, Token op);

// Debugging
void disassemble_ast(Ast* ast);
