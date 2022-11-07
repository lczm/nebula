#include "debugging.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "macros.h"

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
    case AST_WHILE: {
      WhileStmt* while_stmt = (WhileStmt*)ast->as;
      printf("[%-20s]\n", "WHILE_STMT");
      printf("[%-20s]\n", " WHILE_STMT [CONDITION]");
      disassemble_individual_ast(while_stmt->condition_expr);
      printf("[%-20s]\n", " WHILE_STMT [BLOCK]");
      disassemble_individual_ast(while_stmt->block_stmt);
      break;
    }
    case AST_FOR: {
      ForStmt* for_stmt = (ForStmt*)ast->as;
      printf("[%-20s]\n", "FOR_STMT");
      printf("[%-20s]\n", " FOR_STMT [ASSIGNMENT]");
      disassemble_individual_ast(for_stmt->assignment_stmt);
      printf("[%-20s]\n", " FOR_STMT [CONDITION]");
      disassemble_individual_ast(for_stmt->condition_expr);
      printf("[%-20s]\n", " FOR_STMT [THEN_EXPR]");
      disassemble_individual_ast(for_stmt->then_expr);
      printf("[%-20s]\n", " FOR_STMT [BLOCK]");
      disassemble_individual_ast(for_stmt->block_stmt);
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
    case AST_FUNC: {
      FuncStmt* func_stmt = (FuncStmt*)ast->as;
      printf("[%-20s]\n", "FUNC_STMT");
      printf("  [Parameter Count: %d]\n", func_stmt->arity);
      for (int i = 0; i < func_stmt->parameters->count; i++) {
        PRINT_TOKEN_STRING(func_stmt->parameters->tokens[i]);
      }
      printf("  ");
      disassemble_individual_ast(func_stmt->stmt);
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
      if (bool_expr->value) {  // Check explicitly
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
    case AST_ASSIGNMENT_EXPR: {
      AssignmentExpr* assignment_expr = (AssignmentExpr*)ast->as;
      char s[assignment_expr->name.length + 1];
      strncpy(s, assignment_expr->name.start, assignment_expr->name.length);
      s[assignment_expr->name.length] = '\0';
      printf("[%-20s]: %s", "ASSIGNMENT_EXPR", s);
      disassemble_individual_ast(assignment_expr->expr);
      break;
    }
    case AST_STRING: {
      StringExpr* string_expr = (StringExpr*)ast->as;
      char s[string_expr->length + 1];
      strncpy(s, string_expr->start, string_expr->length);
      s[string_expr->length] = '\0';
      printf("[%-20s]: %s\n", "STRING", s);
      break;
    }
    case AST_CALL: {
      CallExpr* call_expr = (CallExpr*)ast->as;
      printf("CallExpr callee: ");
      disassemble_individual_ast(call_expr->callee);
      printf("CallExpr arguments (Count: %d) :\n", call_expr->arguments->count);
      for (int i = 0; i < call_expr->arguments->count; i++) {
        PRINT_TOKEN_STRING(call_expr->arguments->tokens[i]);
      }
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

char* get_string_from_token(Token token) {
  // Allocate a string
  char* s = malloc((token.length + 1) * sizeof(char));
  strncpy(s, token.start, token.length);
  s[token.length] = '\0';
  return s;
}
