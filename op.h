#pragma once

#include <stdint.h>

#include "value.h"

typedef enum {
  OP_CONSTANT,  // 0
  OP_POP,       // 1
  OP_TRUE,      // 2
  OP_FALSE,     // 3
  OP_ADD,       // 4
  OP_SUBTRACT,  // 5
  OP_MULTIPLY,  // 6
  OP_DIVIDE,    // 7
  OP_NEGATE,    // 8
  OP_GREATER,   // 9
  OP_LESS,      // 10
  OP_NOT,       // 11
  OP_EQUAL,     // 12
  OP_RETURN,    // 13
  OP_PRINT,     // 14
  // Variable setters and getters
  OP_SET_GLOBAL,  // 15
  OP_GET_GLOBAL,  // 16
  // Local variable setters and getters
  OP_SET_LOCAL,  // 17
  OP_GET_LOCAL,  // 18

  // To define function when declared
  OP_DEFINE_GLOBAL,  // 19

  // Jumps
  OP_JUMP,           // 20
  OP_JUMP_IF_FALSE,  // 21
  OP_LOOP,           // 22

  // Functions
  OP_CALL,  // 23

  // NIL
  OP_NIL,  // 24
} OpCode;
