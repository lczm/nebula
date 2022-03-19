#pragma once

#include <stdlib.h>
#include "ast.h"

#define ALLOCATE(type, count) \
    (type*)malloc(sizeof(type) * count)

