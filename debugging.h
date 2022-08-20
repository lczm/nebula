#pragma once

#include "array.h"
#include "token.h"

#define TOTAL_FLAGS 4

static const int DUMP_TOKEN = 0;
static const int DUMP_AST = 1;
static const int DUMP_CODEGEN = 2;
static const int VM_OUTPUT = 3;

// Debugging
void disassemble_individual_ast(Ast* ast);
void disassemble_ast(AstArray* ast_array);

// Helper functions
char* get_string_from_token(Token token);