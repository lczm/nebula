#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "value.h"

bool is_stmt(Ast* ast) {
  switch (ast->type) {
    case AST_NONE:
    case AST_NUMBER:
    case AST_BINARY:
    case AST_UNARY:
    case AST_BOOL:
    case AST_VARIABLE_EXPR:
    case AST_GROUP:
    case AST_ASSIGNMENT_EXPR:
    case AST_STRING:
    case AST_CALL:
      return false;
    case AST_PRINT:
    case AST_VARIABLE_STMT:
    case AST_IF:
    case AST_WHILE:
    case AST_FOR:
    case AST_BLOCK:
    case AST_FUNC:
      return true;
  }
  return false;
}

bool is_expr(Ast* ast) {
  switch (ast->type) {
    case AST_NONE:
    case AST_PRINT:
    case AST_VARIABLE_STMT:
    case AST_IF:
    case AST_WHILE:
    case AST_FOR:
    case AST_BLOCK:
    case AST_FUNC:
      return false;
    case AST_NUMBER:
    case AST_BINARY:
    case AST_UNARY:
    case AST_BOOL:
    case AST_VARIABLE_EXPR:
    case AST_GROUP:
    case AST_ASSIGNMENT_EXPR:
    case AST_STRING:
    case AST_CALL:
      return true;
  }
  return false;
}

Ast* make_ast() {
  // Creating only one ast
  Ast* ast = (Ast*)malloc(sizeof(Ast) * 1);
  // Init the members
  ast->type = AST_NONE;
  ast->as = NULL;
  return ast;
}

PrintStmt* make_print_stmt(Ast* expr) {
  PrintStmt* print_stmt = (PrintStmt*)malloc(sizeof(PrintStmt) * 1);
  print_stmt->expr = expr;
  return print_stmt;
}

VariableStmt* make_variable_stmt(Token name, Ast* initializer_expr) {
  VariableStmt* variable_stmt = (VariableStmt*)malloc(sizeof(VariableStmt) * 1);
  variable_stmt->initialized = false;
  variable_stmt->name = name;
  variable_stmt->initializer_expr = initializer_expr;
  return variable_stmt;
}

IfStmt* make_if_stmt(Ast* condition_expr, Ast* then_stmt, Ast* else_stmt) {
  IfStmt* if_stmt = (IfStmt*)malloc(sizeof(IfStmt) * 1);
  if_stmt->condition_expr = condition_expr;
  if_stmt->then_stmt = then_stmt;
  if_stmt->else_stmt = else_stmt;
  return if_stmt;
}

WhileStmt* make_while_stmt(Ast* condition_expr, Ast* block_stmt) {
  WhileStmt* while_stmt = (WhileStmt*)malloc(sizeof(WhileStmt) * 1);
  while_stmt->condition_expr = condition_expr;
  while_stmt->block_stmt = block_stmt;
  return while_stmt;
}

ForStmt* make_for_stmt(Ast* assignment_stmt,
                       Ast* condition_expr,
                       Ast* then_expr,
                       Ast* block_stmt) {
  ForStmt* for_stmt = (ForStmt*)malloc(sizeof(ForStmt) * 1);
  for_stmt->assignment_stmt = assignment_stmt;
  for_stmt->condition_expr = condition_expr;
  for_stmt->then_expr = then_expr;
  for_stmt->block_stmt = block_stmt;
  return for_stmt;
}

BlockStmt* make_block_stmt() {
  BlockStmt* block_stmt = (BlockStmt*)malloc(sizeof(BlockStmt) * 1);
  init_ast_array(&block_stmt->ast_array);
  return block_stmt;
}

FuncStmt* make_func_stmt(Token name,
                         Ast* stmt,
                         TokenArray* parameters,
                         int arity) {
  FuncStmt* func_stmt = (FuncStmt*)malloc(sizeof(FuncStmt) * 1);
  func_stmt->name = name;
  func_stmt->stmt = stmt;
  func_stmt->parameters = parameters;
  func_stmt->arity = arity;
  return func_stmt;
}

NumberExpr* make_number_expr(double value) {
  NumberExpr* number_expr = (NumberExpr*)malloc(sizeof(NumberExpr) * 1);
  number_expr->value = value;
  return number_expr;
}

BinaryExpr* make_binary_expr(Ast* left_expr, Ast* right_expr, Token op) {
  BinaryExpr* binary_expr = (BinaryExpr*)malloc(sizeof(BinaryExpr) * 1);
  binary_expr->left_expr = left_expr;
  binary_expr->right_expr = right_expr;
  binary_expr->op = op;
  return binary_expr;
}

UnaryExpr* make_unary_expr(Ast* right_expr, Token op) {
  UnaryExpr* unary_expr = (UnaryExpr*)malloc(sizeof(UnaryExpr) * 1);
  unary_expr->right_expr = right_expr;
  unary_expr->op = op;
  return unary_expr;
}

BoolExpr* make_bool_expr(bool value) {
  BoolExpr* bool_expr = (BoolExpr*)malloc(sizeof(BoolExpr) * 1);
  bool_expr->value = value;
  return bool_expr;
}

VariableExpr* make_variable_expr(Token name) {
  VariableExpr* variable_expr = (VariableExpr*)malloc(sizeof(VariableExpr) * 1);
  variable_expr->name = name;
  return variable_expr;
}

GroupExpr* make_group_expr(Ast* expr) {
  GroupExpr* group_expr = (GroupExpr*)malloc(sizeof(GroupExpr) * 1);
  group_expr->expr = expr;
  return group_expr;
}

AssignmentExpr* make_assignment_expr(Token name, Ast* expr) {
  AssignmentExpr* assignment_expr =
      (AssignmentExpr*)malloc(sizeof(AssignmentExpr) * 1);
  assignment_expr->name = name;
  assignment_expr->expr = expr;
  return assignment_expr;
}

StringExpr* make_string_expr(const char* start, int length) {
  StringExpr* string_expr = (StringExpr*)malloc(sizeof(StringExpr) * 1);
  string_expr->start = start;
  string_expr->length = length;
  return string_expr;
}

CallExpr* make_call_expr(Ast* callee, TokenArray* arguments) {
  CallExpr* call_expr = (CallExpr*)malloc(sizeof(CallExpr) * 1);
  call_expr->callee = callee;
  call_expr->arguments = arguments;
  return call_expr;
}

Value ast_to_value(Ast* ast) {
  switch (ast->type) {
    case AST_NUMBER: {
      NumberExpr* number_expr = (NumberExpr*)ast->as;
      return NUMBER_VAL(number_expr->value);
    }
    case AST_BOOL: {
      BoolExpr* bool_expr = (BoolExpr*)ast->as;
      return BOOLEAN_VAL(bool_expr->value);
    }
    case AST_BINARY: {
      BinaryExpr* binary_expr = (BinaryExpr*)ast->as;
      Value left_value = ast_to_value(binary_expr->left_expr);
      Value right_value = ast_to_value(binary_expr->right_expr);
      if (IS_NUMBER(left_value) && IS_NUMBER(right_value)) {
        return NUMBER_VAL(AS_NUMBER(left_value) + AS_NUMBER(right_value));
      }
      break;
    }
    default: {
      return NIL_VAL;
    }
  }
}

Ast* wrap_ast(void* raw_ast, AstType raw_type) {
  Ast* ast = make_ast();
  ast->type = raw_type;
  ast->as = raw_ast;
  return ast;
}
