#pragma once

#include "array.h"
#include "ast.h"
#include "compiler.h"

void parse_tokens(Compiler* compiler,
                  TokenArray* token_arr,
                  AstArray* ast_arr,
                  ErrorArray* error_arr);
