#pragma once

#include "array.h"
#include "ast.h"
#include "object.h"

// ObjFunc* codegen(OpArray* op_arr,
//                  ValueArray* constants_arr,
//                  AstArray* ast_arr,
//                  LocalArray* local_arr);
ObjFunc* codegen(AstArray* ast_arr);
void disassemble_opcode_values(OpArray* op_arr, ValueArray* value_arr);
