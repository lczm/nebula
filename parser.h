#pragma once

#include "array.h"
#include "ast.h"

void parse_tokens(TokenArray* token_arr,
                  AstArray* ast_arr,
                  ErrorArray* error_arr);
