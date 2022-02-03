#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

bool is_stmt(Ast* ast) {
    switch (ast->type) {
        case AST_NONE:
        case AST_NUMBER:
        case AST_BINARY:
            return false;
    }
    return false;
}

bool is_expr(Ast* ast) {
    switch (ast->type) {
        case AST_NONE:
            return false;
        case AST_NUMBER:
        case AST_BINARY:
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

void disassemble_individual_ast(Ast* ast) {
    switch (ast->type) {
        case AST_NONE:
            return;
        case AST_NUMBER: {
            NumberExpr* number_expr = (NumberExpr*)ast->as;
            printf("[%-20s]: %f\n", "AST_NUMBER", number_expr->value);
            break;
        }
        case AST_BINARY: {
            BinaryExpr* binary_expr = (BinaryExpr*)ast->as;
            printf("[%-20s]\n", "BINARY_EXPR");
            printf("[%-10s]: ", "Left:");
            disassemble_individual_ast(binary_expr->left_expr);
            printf("[%-10s]: ", "Right:");
            disassemble_individual_ast(binary_expr->right_expr);
            // Delimit it with c_str end char
            char s[binary_expr->op.length + 1]; 
            strncpy(s, binary_expr->op.start, binary_expr->op.length);
            s[binary_expr->op.length] = '\0';
            printf("[%-10s]: %s\n", "Token: ", s);
            break;
        }
    }
}

void disassemble_ast(Ast* ast) {
    printf("-----%s-----\n", "Ast Disassembly");
    disassemble_individual_ast(ast);
}
