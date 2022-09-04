#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALLOCATE(type, count) (type*)malloc(sizeof(type) * count)

#define PRINT_AST_STRING(str)                    \
  char s[str->name.length + 1];                  \
  strncpy(s, str->name.start, str->name.length); \
  s[str->name.length] = '\0';                    \
  printf("%s\n", s);

#define PRINT_TOKEN_STRING(token)          \
  do {                                     \
    char s[token.length + 1];              \
    strncpy(s, token.start, token.length); \
    s[token.length] = '\0';                \
    printf("%s\n", s);                     \
  } while (0)
