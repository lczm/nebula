#include <stdio.h>
#include <string.h>

#include "debugging.h"
#include "ast.h"

void disassemble_individual_ast(Ast* ast) {
    if (ast == NULL) {
        printf("[%-20s]\n", "NULL");
        return;
    }
    switch (ast->type) {
        case AST_NONE:
            return;
        case AST_PRINT: {
            PrintStmt* print_stmt = (PrintStmt*)ast->as;
            printf("[%-20s]\n", "PRINT_STMT");
            disassemble_individual_ast(print_stmt->expr);
            break;
        }
        case AST_IF: {
            IfStmt* if_stmt = (IfStmt*)ast->as;
            printf("[%-20s]\n", "IF_STMT");
            printf("[%-20s]\n", " IF_STMT [CONDITION]");
            disassemble_individual_ast(if_stmt->condition_expr);
            printf("[%-20s]\n", " IF_STMT [THEN]");
            disassemble_individual_ast(if_stmt->then_stmt);
            printf("[%-20s]\n", " IF_STMT [ELSE]");
            disassemble_individual_ast(if_stmt->else_stmt);
            // condition, then & else
            break;
        }
        case AST_BLOCK: {
            BlockStmt* block_stmt = (BlockStmt*)ast->as;
            printf("[%-20s]\n", "BLOCK_STMT");
            for (int i = 0; i < block_stmt->ast_array.count; i++) {
                disassemble_individual_ast(block_stmt->ast_array.ast[i]);
            }
            break;
        }
        case AST_VARIABLE_STMT: {
            VariableStmt* variable_stmt = (VariableStmt*)ast->as;
            printf("[%-20s]\n", "VARIABLE_STMT");
            char s[variable_stmt->name.length + 1];
            strncpy(s, variable_stmt->name.start, variable_stmt->name.length);
            s[variable_stmt->name.length] = '\0';
            printf("[%-10s]: %s\n", "IDENTIFIER", s);
            if (variable_stmt->initializer_expr->type != AST_NONE) {
                printf("[%-20s]", "INITIALIZER_EXPR");
                disassemble_individual_ast(variable_stmt->initializer_expr);
            }
            break;
        }
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
        case AST_UNARY: {
            UnaryExpr* unary_expr = (UnaryExpr*)ast->as;
            printf("[%-20s]\n", "UNARY_EXPR");
            // Delimit it with c_str end char
            char s[unary_expr->op.length + 1];
            strncpy(s, unary_expr->op.start, unary_expr->op.length);
            s[unary_expr->op.length] = '\0';
            printf("[%-10s]: %s\n", "Token: ", s);
            printf("[%-10s]: ", "Right:");
            disassemble_individual_ast(unary_expr->right_expr);
            break;
        }
        case AST_BOOL: {
            BoolExpr* bool_expr = (BoolExpr*)ast->as;
            if (bool_expr->value) { // Check explicitly
                printf("[%-20s]: %s\n", "AST_BOOL", "true");
            } else {
                printf("[%-20s]: %s\n", "AST_BOOL", "false");
            }
            break;
        }
        case AST_VARIABLE_EXPR: {
            VariableExpr* variable_expr = (VariableExpr*)ast->as;
            char s[variable_expr->name.length + 1];
            strncpy(s, variable_expr->name.start, variable_expr->name.length);
            s[variable_expr->name.length] = '\0';
            printf("[%-20s]: %s\n", "AST_VARIABLE_EXPR", s);
            break;
        }
        case AST_GROUP: {
            GroupExpr* group_expr = (GroupExpr*)ast->as;
            printf("[%-20s]\n", "GROUP_EXPR");
            disassemble_individual_ast(group_expr->expr);
            break;
        }
    }
}

void disassemble_ast(AstArray* ast_array) {
    for (int i = 0; i < ast_array->count; i++) {
        printf("-----%s-----\n", "Ast Disassembly");
        disassemble_individual_ast(ast_array->ast[i]);
    }
}
