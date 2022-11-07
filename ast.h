#pragma once

#include <stdbool.h>

#include "array.h"
#include "token.h"
#include "value.h"

typedef enum {
  AST_NONE,
  AST_PRINT,
  AST_IF,
  AST_WHILE,
  AST_FOR,
  AST_BLOCK,
  AST_VARIABLE_STMT,
  AST_NUMBER,
  AST_BINARY,
  AST_UNARY,
  AST_BOOL,
  AST_VARIABLE_EXPR,
  AST_GROUP,
  AST_ASSIGNMENT_EXPR,
  AST_STRING,
  AST_FUNC,
  AST_CALL,
} AstType;

// Since in array.h it already declares typdef struct Ast to Ast
struct Ast {
  AstType type;
  void* as;
};

// This should only be used for debugging
// until built-in functions work
typedef struct {
  Ast* expr;
} PrintStmt;

typedef struct {
  Ast* condition_expr;
  Ast* then_stmt;
  Ast* else_stmt;
} IfStmt;

typedef struct {
  Ast* condition_expr;
  Ast* block_stmt;
} WhileStmt;

typedef struct {
  Ast* assignment_stmt;
  Ast* condition_expr;
  Ast* then_expr;
  Ast* block_stmt;
} ForStmt;

typedef struct {
  AstArray ast_array;
} BlockStmt;

typedef struct {
  Token name;
  Ast* stmt;
  TokenArray* parameters;
  int arity;
} FuncStmt;

typedef struct {
  Token name;
  bool initialized;
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

// a = 10; where a is the name, and NumberExpr{10} is the expr
typedef struct {
  Token name;
  Ast* expr;
} AssignmentExpr;

typedef struct {
  const char* start;
  int length;
} StringExpr;

typedef struct {
  Ast* callee;
  TokenArray* arguments;
} CallExpr;

bool is_stmt(Ast* ast);
bool is_expr(Ast* ast);

Ast* make_ast();

// Statements
PrintStmt* make_print_stmt(Ast* expr);
VariableStmt* make_variable_stmt(Token name, Ast* initializer_expr);
IfStmt* make_if_stmt(Ast* condition_expr, Ast* then_stmt, Ast* else_stmt);
WhileStmt* make_while_stmt(Ast* condition_expr, Ast* block_stmt);
ForStmt* make_for_stmt(Ast* assignment_stmt,
                       Ast* condition_expr,
                       Ast* then_expr,
                       Ast* block_stmt);
BlockStmt* make_block_stmt();
FuncStmt* make_func_stmt(Token name,
                         Ast* stmt,
                         TokenArray* parameters,
                         int arity);

// Expressions
NumberExpr* make_number_expr(double value);
BinaryExpr* make_binary_expr(Ast* left_expr, Ast* right_expr, Token op);
UnaryExpr* make_unary_expr(Ast* right_expr, Token op);
BoolExpr* make_bool_expr(bool value);
VariableExpr* make_variable_expr(Token name);
GroupExpr* make_group_expr(Ast* expr);
AssignmentExpr* make_assignment_expr(Token name, Ast* expr);
StringExpr* make_string_expr(const char* start, int length);
CallExpr* make_call_expr(Ast* callee, TokenArray* arguments);

// Convert expressions into values
Value ast_to_value(Ast* ast);

// Useful function
Ast* wrap_ast(void* raw_ast, AstType raw_type);
