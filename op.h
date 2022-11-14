#pragma once

#include <stdint.h>

#include "array.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_POP,
  OP_TRUE,
  OP_FALSE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_GREATER,
  OP_LESS,
  OP_NOT,
  OP_EQUAL,
  OP_RETURN,
  OP_PRINT,
  // Variable setters and getters
  OP_SET_GLOBAL,
  OP_GET_GLOBAL,
  // Local variable setters and getters
  OP_SET_LOCAL,
  OP_GET_LOCAL,

  // Jumps
  OP_JUMP,
  OP_JUMP_IF_FALSE,

  // Functions
  OP_CALL,

  // NIL
  OP_NIL,
} OpCode;
