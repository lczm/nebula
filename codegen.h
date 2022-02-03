#pragma once

#include "ast.h"
#include "array.h"

void codegen(OpArray* op_arr, ValueArray* value_arr, Ast* ast);
void disassemble_opcode_values(OpArray* op_arr, ValueArray* value_arr);
