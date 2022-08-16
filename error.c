#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Error* create_error(int line,
                    int column,
                    const char* file_name,
                    const char* error_message,
                    ErrorType type) {
  Error* error = (Error*)malloc(sizeof(Error));
  error->line = line;
  error->column = column;
  // Error shall allocate the space for the file_name and
  // error_message, and when it is clearing up, it will free both
  error->file_name = file_name;
  error->error_message = error_message;
  error->type = type;
  return error;
}

// Only if the error object owns the error_message
// this free will be called, but otherwise it's just for
// consistency.
void free_error(Error* error) {}

void print_error(Error* error) {
  if (error->type == SyntaxError) {
    printf("%s: (%d|%d) Syntax Error: %s\n", error->file_name, error->line,
           error->column, error->error_message);
  } else if (error->type == RuntimeError) {
    printf("\n");
  } else {
    printf("Error type is not defined.\n");
  }
}
