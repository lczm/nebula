#pragma once

#include "array.h"
#include "object.h"

typedef struct {
  ObjFunc* func;
  OpCode* ip;
  // ValueArray* slots;
  Value* slots;
} CallFrame;
