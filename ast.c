#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "value.h"

bool is_stmt(Ast* ast) {
    switch (ast->type) {
        case AST_NONE:
        case AST_NUMBER:
        case AST_BINARY:
        case AST_UNARY:
        case AST_BOOL:
        case AST_VARIABLE_EXPR:
            return false;
        case AST_PRINT:
        case AST_VARIABLE_STMT:
            return true;
    }
    return false;
}

bool is_expr(Ast* ast) {
    switch (ast->type) {
        case AST_NONE:
        case AST_PRINT:
        case AST_VARIABLE_STMT:
            return false;
        case AST_NUMBER:
        case AST_BINARY:
        case AST_UNARY:
        case AST_BOOL:
        case AST_VARIABLE_EXPR:
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
    PrintStmt* print_stmt = (
            PrintStmt*)malloc(sizeof(PrintStmt) * 1);
    print_stmt->expr = expr;
    return print_stmt;
}

VariableStmt* make_variable_stmt(Token name, Ast* initializer_expr) {
    VariableStmt* variable_stmt = (
        VariableStmt*)malloc(sizeof(VariableStmt) * 1);
    variable_stmt->name = name;
    variable_stmt->initializer_expr = initializer_expr;
    return variable_stmt;
}

NumberExpr* make_number_expr(double value) {
    NumberExpr* number_expr = (
            NumberExpr*)malloc(sizeof(NumberExpr) * 1);
    number_expr->value = value;
    return number_expr;
}

BinaryExpr* make_binary_expr(Ast* left_expr, Ast* right_expr, Token op) {
    BinaryExpr* binary_expr = (
            BinaryExpr*)malloc(sizeof(BinaryExpr) * 1);
    binary_expr->left_expr = left_expr;
    binary_expr->right_expr = right_expr;
    binary_expr->op = op;
    return binary_expr;
}

UnaryExpr* make_unary_expr(Ast* right_expr, Token op) {
    UnaryExpr* unary_expr = (
            UnaryExpr*)malloc(sizeof(UnaryExpr) * 1);
    unary_expr->right_expr = right_expr;
    unary_expr->op = op;
    return unary_expr;
}

BoolExpr* make_bool_expr(bool value) {
    BoolExpr* bool_expr = (
            BoolExpr*)malloc(sizeof(BoolExpr) * 1);
    bool_expr->value = value;
    return bool_expr;
}

VariableExpr* make_variable_expr(Token name) {
    VariableExpr* variable_expr = (
            VariableExpr*)malloc(sizeof(VariableExpr) * 1);
    variable_expr->name = name;
    return variable_expr;
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

