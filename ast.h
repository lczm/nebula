#pragma once

#include <stdbool.h>
#include "token.h"
#include "value.h"

typedef enum {
    AST_NONE,
    AST_PRINT,
    AST_VARIABLE_STMT,
    AST_NUMBER,
    AST_BINARY,
    AST_UNARY,
    AST_BOOL,
    AST_VARIABLE_EXPR,
    AST_GROUP,
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

// The difference between a VariableStmt and a VariableExpr
// is that a VariableStmt refers to `let x = 10;` for example
// and a VariableExpr refers to the `a` in `print a;`
// i.e. this is meant for the codegen backend to produce a 
// OP_GET_{} opcode
typedef struct {
    Token name;
} VariableExpr;

typedef struct {
    Ast* expr;
} GroupExpr;

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
VariableExpr* make_variable_expr(Token name);
GroupExpr* make_group_expr(Ast* expr);

// Convert expressions into values
Value ast_to_value(Ast* ast);

// Useful function
Ast* wrap_ast(void* raw_ast, AstType raw_type);
