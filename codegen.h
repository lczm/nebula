#pragma once

#include "array.h"
#include "ast.h"

void codegen(OpArray* op_arr, ValueArray* constants_arr, AstArray* ast_arr);
void disassemble_opcode_values(OpArray* op_arr, ValueArray* value_arr);
