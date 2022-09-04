#pragma once

#include "array.h"
#include "ast.h"
#include "compiler.h"

void codegen(OpArray* op_arr,
             ValueArray* constants_arr,
             AstArray* ast_arr,
             LocalArray* local_arr,
             Compiler* compiler);
void disassemble_opcode_values(OpArray* op_arr, ValueArray* value_arr);
