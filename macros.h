#pragma once

#include <stdlib.h>

#define ALLOCATE(type, count) \
    (type*)malloc(sizeof(type) * count)
