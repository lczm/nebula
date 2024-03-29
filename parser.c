#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "ast.h"
#include "error.h"
#include "macros.h"
#include "token.h"

// Parser state
static int parser_index = 0;
static int token_array_len = 0;
static TokenArray* token_array;
static AstArray* ast_array;
static ErrorArray* error_array;

// TODO
// Error-reporting on the specific lines are a little wrong
// i.e.
// 1) print 1 + 2;
// 2) print 1 + 2;
// 3) let a =
// 4) print 1 + 2;
// Will report that the error is on line 4.
// Slightly small ish detail, so it's not that important right now
// but this needs to be fixed in the future.

static bool move() {
  // If it can still continue to increment
  if (parser_index != token_array->count) {
    parser_index++;
    return true;
  }

  // Otherwise, incrementing it will result it in going out of range
  return false;
}

static bool parser_is_at_end() {
  if (parser_index >= token_array->count) {
    return true;
  }
  return false;
}

static bool match(TokenType type) {
  if (parser_is_at_end())
    return false;
  if (token_array->tokens[parser_index].type == type)
    return true;
  return false;
}

static bool match_either(TokenType type1, TokenType type2) {
  if (token_array->tokens[parser_index].type == type1 ||
      token_array->tokens[parser_index].type == type2) {
    return true;
  }
  return false;
}

static bool match_and_move(TokenType type) {
  if (token_array->tokens[parser_index].type == type) {
    parser_index++;
    return true;
  }
  return false;
}

static Token peek(int offset) {
  return token_array->tokens[parser_index + offset];
}

static bool peek_match(int offset, TokenType type) {
  Token token = token_array->tokens[parser_index + offset];
  if (token.type == type)
    return true;
  return false;
}

static Token get_current() {
  return token_array->tokens[parser_index];
}

static Token get_previous() {
  return token_array->tokens[parser_index--];
}

static bool eat(TokenType type) {
  if (get_current().type == type) {
    move();
    return true;
  }
  return false;
}

static void eat_or_error(TokenType type, const char* error_message) {
  if (!eat(type)) {
    // printf("Syntax error: %s\n", error_message);
    Error* error = create_error(get_current().line, 0, "main.neb",
                                error_message, SyntaxError);
    push_error_array(error_array, error);
  }
}

// Statements
static Ast* declaration();
static Ast* func_declaration();
static Ast* return_statement();
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

// Not part of the parsing precedence
// Technically just helper functions to help with
// scoping and locals
static Ast* block();

void parse_tokens(TokenArray* token_arr,
                  AstArray* ast_arr,
                  ErrorArray* error_arr) {
  // Initialize the static variables for convenience
  parser_index = 0;  // Reset to 0 just in case
  token_array = token_arr;
  token_array_len = token_arr->count;
  ast_array = ast_arr;
  error_array = error_arr;

  while (parser_index != token_arr->count) {
    push_ast_array(ast_array, declaration());
  }
  // return declaration();
}

static Ast* declaration() {
  if (match(TOKEN_FUNC)) {
    move();
    return func_declaration();
  } else if (match(TOKEN_RETURN)) {
    move();
    return return_statement();
  } else if (match(TOKEN_LET)) {
    move();
    return var_declaration();
  }

  return statement();
}

static Ast* func_declaration() {
  if (!match(TOKEN_IDENTIFIER)) {
    Error* error = create_error(
        get_current().line, 0, "main.neb",
        "func_declaration could not find an identifier after 'func'",
        SyntaxError);
    push_error_array(error_array, error);
  }

  // Match the function name
  Token function_name = get_current();
  move();

  if (!match(TOKEN_LEFT_PAREN)) {
    Error* error = create_error(
        get_current().line, 0, "main.neb",
        "func_declaration could not find a ( after the function identifier",
        SyntaxError);
    push_error_array(error_array, error);
  }
  // Move past the '('
  move();

  int arity = 0;

  // Function parameters, stores the identifiers as tokens
  TokenArray* parameters = (TokenArray*)malloc(sizeof(TokenArray) * 1);
  init_token_array(parameters);

  while (!match(TOKEN_RIGHT_PAREN)) {
    if (!match(TOKEN_IDENTIFIER)) {
      Error* error = create_error(
          get_current().line, 0, "main.neb",
          "func_declaration could not find identifiers in the parameter",
          SyntaxError);
      push_error_array(error_array, error);
    }

    Token parameter_identifier = get_current();
    push_token_array(parameters, parameter_identifier);

    move();

    // If there are multiple parameters
    if (match(TOKEN_COMMA))
      move();

    arity++;
  }

  // Move past right_paren
  move();

  // printf("arity count : %d | parameter count : %d\n", arity,
  // parameters->count);

  // sanity check
  if (arity != parameters->count) {
    Error* error = create_error(
        get_current().line, 0, "main.neb",
        "func_declaration arity and parameter_count does not tally",
        SyntaxError);
    push_error_array(error_array, error);

    printf("Something seriously wrong with parsing here\n");
  }

  if (!match(TOKEN_LEFT_BRACE)) {
    Error* error =
        create_error(get_current().line, 0, "main.neb",
                     "func_declaration does not have a block statement after )",
                     SyntaxError);
    push_error_array(error_array, error);
  }
  move();

  Ast* block_stmt = block();

  FuncStmt* func_stmt =
      make_func_stmt(function_name, block_stmt, parameters, arity);

  Ast* ast = make_ast();
  ast->type = AST_FUNC;
  ast->as = func_stmt;

  return ast;
}

static Ast* return_statement() {
  Ast* value_expression;
  if (match(TOKEN_SEMICOLON)) {
    value_expression = make_ast();
  } else {
    value_expression = expression();
    match_and_move(TOKEN_SEMICOLON);
  }

  ReturnStmt* return_stmt = make_return_stmt(value_expression);
  Ast* ast = make_ast();
  ast->type = AST_RETURN;
  ast->as = return_stmt;

  return ast;
}

static Ast* var_declaration() {
  // Make sure that there is an identifier for it to assign to
  if (!match(TOKEN_IDENTIFIER)) {
    Error* error =
        create_error(get_current().line, 0, "main.neb",
                     "var_declaration could not find an identifier after 'let'",
                     SyntaxError);
    push_error_array(error_array, error);
  }

  // Already previously matched the identifier token
  Token identifier_name = get_current();
  move();

  // initialization assignment
  // example: let a =
  Ast* ast = make_ast();
  if (match(TOKEN_EQUAL)) {
    move();

    Ast* initializer_expr = expression();
    if (!match_and_move(TOKEN_SEMICOLON)) {
      Error* error = create_error(
          get_current().line, 0, "main.neb",
          "Did not have a semicolon after variable declaration", SyntaxError);
      push_error_array(error_array, error);
      return NULL;
    }
    VariableStmt* variable_stmt =
        make_variable_stmt(identifier_name, initializer_expr);
    ast->type = AST_VARIABLE_STMT;
    ast->as = variable_stmt;
    return ast;
  } else {  // variable declaration without initialization, let a;
    Ast* ast_none = make_ast();
    VariableStmt* variable_stmt = make_variable_stmt(identifier_name, ast_none);
    ast->type = AST_VARIABLE_STMT;
    ast->as = variable_stmt;
    return ast;
  }
}

static Ast* statement() {
  if (match_and_move(TOKEN_PRINT)) {
    Ast* ast = expression();
    PrintStmt* print_stmt = make_print_stmt(ast);
    Ast* ast_stmt = wrap_ast(print_stmt, AST_PRINT);

    eat_or_error(TOKEN_SEMICOLON, "Must have ';' after statement");
    return ast_stmt;
  } else if (match_and_move(TOKEN_WHILE)) {
    match_and_move(TOKEN_LEFT_PAREN);
    // This is the condition that the while loop uses to evaluate
    // whether it will go next or not
    Ast* condition_expr = expression();
    // The closing parameter on the condition
    match_and_move(TOKEN_RIGHT_PAREN);

    // Check that it has a left brace, as the block condition
    if (!match(TOKEN_LEFT_BRACE)) {
      // printf(
      //     "Syntax error: After a while statement needs to have a left
      //     brace\n");
      Error* error =
          create_error(get_current().line, 0, "main.neb",
                       "After a while statement, there should be a left brace.",
                       SyntaxError);
      push_error_array(error_array, error);
    }

    // Parse the block statement here
    // TODO : In the semantic analysis portion of the block statement here
    // the semanti analyzer can check whether the block statement includes
    // an increment or a return to break out of the loop, in the case
    // the user includes an infinite loop that does not go anywhere
    Ast* block_stmt = statement();

    WhileStmt* while_stmt = make_while_stmt(condition_expr, block_stmt);
    Ast* ast_stmt = wrap_ast(while_stmt, AST_WHILE);
    return ast_stmt;
  } else if (match_and_move(TOKEN_FOR)) {
    // Deal with the variable assignment here first
    // The for (let x =...

    match_and_move(TOKEN_LEFT_PAREN);

    Ast* assignment_stmt = NULL;
    if (match(TOKEN_LET)) {
      assignment_stmt = declaration();
    }

    Ast* condition_expr = expression();
    match_and_move(TOKEN_SEMICOLON);

    // Note that there is no need for a semicolon at the end here
    Ast* then_expr = expression();

    match_and_move(TOKEN_RIGHT_PAREN);

    if (!match(TOKEN_LEFT_BRACE)) {
      // printf("After a for statement needs to have a left brace\n");
      Error* error =
          create_error(get_current().line, 0, "main.neb",
                       "After a while statement, there should be a left brace.",
                       SyntaxError);
      push_error_array(error_array, error);
    }
    Ast* block_stmt = statement();

    ForStmt* for_stmt =
        make_for_stmt(assignment_stmt, condition_expr, then_expr, block_stmt);
    Ast* ast_stmt = wrap_ast(for_stmt, AST_FOR);
    return ast_stmt;
  } else if (match_and_move(TOKEN_IF)) {
    match_and_move(TOKEN_LEFT_PAREN);
    Ast* condition_expr = expression();
    match_and_move(TOKEN_RIGHT_PAREN);

    // Check that there is a left brace
    if (!match(TOKEN_LEFT_BRACE)) {
      // printf("After if needs to have a left brace\n");
      Error* error = create_error(
          get_current().line, 0, "main.neb",
          "After an if statement, there should be a left brace.", SyntaxError);
      push_error_array(error_array, error);
    }

    // Block statement
    // Note that it will not eat a token right_brace here
    // as the then_stmt here will be a block_statement, which
    // will handle the eating of the right_brace in itself
    Ast* then_stmt = statement();

    Ast* else_stmt = NULL;
    // If there is an else statement
    if (match_and_move(TOKEN_ELSE)) {
      if (!match(TOKEN_LEFT_BRACE)) {
        // printf("After else needs to have a left brace\n");
        Error* error = create_error(
            get_current().line, 0, "main.neb",
            "After an else statement, there should be a left brace.",
            SyntaxError);
        push_error_array(error_array, error);
      }
      // Same as above, the block statement will cover eating of the
      // token_left and token_right brace
      else_stmt = statement();
    }

    IfStmt* if_stmt = make_if_stmt(condition_expr, then_stmt, else_stmt);
    Ast* ast_stmt = wrap_ast(if_stmt, AST_IF);

    return ast_stmt;
  } else if (match_and_move(TOKEN_LEFT_BRACE)) {
    Ast* block_ast_stmt = block();
    return block_ast_stmt;
  } else {
    Ast* ast = expression_statement();
    return ast;
  }
}

static Ast* expression_statement() {
  Ast* ast = expression();

  if (!eat(TOKEN_SEMICOLON)) {
    // printf("After an expression_statement, there needs to be a semicolon\n");
    Error* error = create_error(
        get_current().line, 0, "main.neb",
        "After an expression statement, there should be a semicolon.",
        SyntaxError);
    push_error_array(error_array, error);

    // exit(0);
    return NULL;
  }

  return ast;
}

static Ast* expression() {
  Ast* ast = assignment();
  return ast;
}

static Ast* assignment() {
  Ast* ast = and_();

  // a = 10;
  if (match_and_move(TOKEN_EQUAL)) {
    // check that the Ast* ast above is a VariableExpr
    if (ast->type != AST_VARIABLE_EXPR) {
      // printf("assignment parsing tried to assign a non variable\n");
      Error* error =
          create_error(get_current().line, 0, "main.neb",
                       "Tried to assign data to a non variable.", SyntaxError);
      push_error_array(error_array, error);
      return NULL;
    }

    Ast* value = assignment();
    AssignmentExpr* assignment_expr =
        make_assignment_expr(((VariableExpr*)ast->as)->name, value);
    Ast* assignment_ast = wrap_ast(assignment_expr, AST_ASSIGNMENT_EXPR);
    return assignment_ast;
  } else if (match(TOKEN_PLUS_EQUAL) || match(TOKEN_MINUS_EQUAL) ||
             match(TOKEN_STAR_EQUAL) || match(TOKEN_SLASH_EQUAL)) {
    Token token_augmented = get_current();
    move();

    // check that the Ast* ast above is a VariableExpr
    if (ast->type != AST_VARIABLE_EXPR) {
      // printf("assignment parsing tried to assign a non variable\n");
      Error* error =
          create_error(get_current().line, 0, "main.neb",
                       "Tried to assign data to a non variable.", SyntaxError);
      push_error_array(error_array, error);
      return NULL;
    }

    // This will come out to be some form of a expression
    // in the simplest cases this will just be a NumberExpr
    Ast* value_expr = expression();
    // Here, we can use the token_plus_equal as the token to the
    // binary expression creation, as in codegen, it will treat
    // token_plus_equal and token_plus equlaly the same.
    BinaryExpr* binary_expr =
        make_binary_expr(ast, value_expr, token_augmented);
    Ast* binary_ast = wrap_ast(binary_expr, AST_BINARY);

    AssignmentExpr* assignment_expr =
        make_assignment_expr(((VariableExpr*)ast->as)->name, binary_ast);
    Ast* assignment_ast = wrap_ast(assignment_expr, AST_ASSIGNMENT_EXPR);
    return assignment_ast;
  }

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
    Token token_operator = get_current();
    move();
    Ast* right = comparison();
    BinaryExpr* binary_expr = make_binary_expr(ast, right, token_operator);
    Ast* binary_ast = make_ast();
    binary_ast->type = AST_BINARY;
    binary_ast->as = binary_expr;
    return binary_ast;
  }

  return ast;
}

static Ast* comparison() {
  Ast* ast = addition();

  if (match(TOKEN_LESS) || match(TOKEN_LESS_EQUAL) || match(TOKEN_GREATER) ||
      match(TOKEN_GREATER_EQUAL)) {
    Token token_operator = get_current();
    move();
    Ast* right = addition();

    BinaryExpr* binary_expr = make_binary_expr(ast, right, token_operator);
    Ast* binary_ast = make_ast();
    binary_ast->type = AST_BINARY;
    binary_ast->as = binary_expr;
    return binary_ast;
  }

  return ast;
}

static Ast* addition() {
  Ast* ast = multiplication();  // number_expr

  // Add or subtract
  while (match_either(TOKEN_PLUS, TOKEN_MINUS)) {
    Token token_operator = get_current();
    move();
    Ast* right = multiplication();  // number_expr
    BinaryExpr* binary_expr = make_binary_expr(ast, right, token_operator);
    ast = make_ast();
    ast->type = AST_BINARY;
    ast->as = binary_expr;
    // Create ast wrapper
    // Ast* binary_ast = make_ast();
    // binary_ast->type = AST_BINARY;
    // binary_ast->as = binary_expr;
    // return binary_ast;
  }

  return ast;
}

static Ast* multiplication() {
  Ast* ast = unary();

  // Multiply or divide
  while (match_either(TOKEN_STAR, TOKEN_SLASH)) {
    Token token_operator = get_current();
    move();
    Ast* right = unary();
    BinaryExpr* binary_expr = make_binary_expr(ast, right, token_operator);
    ast = make_ast();
    ast->type = AST_BINARY;
    ast->as = binary_expr;
    // Create ast wrapper
    // Ast* binary_ast = make_ast();
    // binary_ast->type = AST_BINARY;
    // binary_ast->as = binary_expr;
    // return binary_ast;
  }

  return ast;
}

static Ast* unary() {
  if (match_either(TOKEN_BANG, TOKEN_MINUS)) {
    Token token_operator = get_current();
    move();
    UnaryExpr* unary_expr = make_unary_expr(unary(), token_operator);
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

  if (match(TOKEN_LEFT_PAREN)) {
    move();
    // printf("Reached call() in parser\n");

    AstArray* arguments = (AstArray*)malloc(sizeof(AstArray) * 1);
    init_ast_array(arguments);
    // Sanity check
    int argument_count = 0;

    // Parse the arguments
    CallExpr* call_expr = make_call_expr(ast, arguments);

    while (!match(TOKEN_RIGHT_PAREN)) {
      Ast* expr = expression();
      push_ast_array(arguments, expr);
      argument_count++;

      if (match(TOKEN_COMMA))
        move();
    }

    // Move past right_paren
    move();

    if (argument_count != arguments->count) {
      printf(
          "Something went wrong with parsing number of arguments in "
          "call()\n");
    }

    Ast* call_expr_ast = make_ast();
    call_expr_ast->as = call_expr;
    call_expr_ast->type = AST_CALL;
    return call_expr_ast;
  }

  return ast;
}

static Ast* primary() {
  // Sanity check at the end before trying to malloc an ast node
  // endlessly for bad use cases
  if (parser_is_at_end()) {
    return NULL;
  }

  Ast* ast = make_ast();

  if (match(TOKEN_NUMBER)) {
    const char* start = token_array->tokens[parser_index].start;
    char* end = (char*)token_array->tokens[parser_index].start +
                token_array->tokens[parser_index].length;
    double value = strtod(start, &end);
    // printf("Parsing number: %f\n", value);
    // Print out debug information before moving
    move();
    NumberExpr* number_expr = make_number_expr(value);
    ast->as = number_expr;
    ast->type = AST_NUMBER;
  } else if (match(TOKEN_STRING)) {
    Token token_string = get_current();
    move();
    StringExpr* string_expr =
        make_string_expr(token_string.start, token_string.length);
    ast->as = string_expr;
    ast->type = AST_STRING;
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
  } else if (match(TOKEN_IDENTIFIER)) {
    Token identifier_name = get_current();
    move();
    VariableExpr* variable_expr = make_variable_expr(identifier_name);
    ast->as = variable_expr;
    ast->type = AST_VARIABLE_EXPR;
  } else if (match(TOKEN_LEFT_PAREN)) {  // let a = (10 + 2)
    move();
    Ast* expr = expression();
    if (!match_and_move(TOKEN_RIGHT_PAREN)) {
      Error* error = create_error(get_current().line, 0, "main.neb",
                                  "After a '(', followed by an expression, "
                                  "should have a closing ')'.",
                                  SyntaxError);
      push_error_array(error_array, error);
      return NULL;
    }
    GroupExpr* group_expr = make_group_expr(expr);
    ast->as = group_expr;
    ast->type = AST_GROUP;
  }

  return ast;
}

static Ast* block() {
  BlockStmt* block_stmt = make_block_stmt();

  while (get_current().type != TOKEN_RIGHT_BRACE) {
    push_ast_array(&block_stmt->ast_array, declaration());
  }
  // Move past the right brace
  move();

  Ast* ast_stmt = wrap_ast(block_stmt, AST_BLOCK);
  return ast_stmt;
}
