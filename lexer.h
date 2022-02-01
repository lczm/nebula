#pragma once

#include "array.h"

void lex_source(TokenArray* token_array, const char* source);
void disassemble_token_array(TokenArray* token_array);
