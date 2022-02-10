#pragma once

#include "array.h"

Token make_token(TokenType type);
void lex_source(TokenArray* token_array, const char* source);
void disassemble_token_array(TokenArray* token_array);
