#pragma once

typedef enum {
	SyntaxError,
	RuntimeError,
} ErrorType;

typedef struct {
	int line;
	int column;
	const char* file_name;
	const char* error_message;
	ErrorType type;
} Error;

Error* create_error(int line, int column,
				   const char* file_name,
				   const char* error_message,
				   ErrorType type);
void free_error(Error* error);
void print_error(Error* error);