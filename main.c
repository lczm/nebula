#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "ast.h"
#include "codegen.h"
#include "debugging.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"

static void start_repl() {
  printf("Nebula REPL\n");
}

static char* read_file(const char* path) {
  // Open the file, the file is readable, checked
  // by file_exists before this
  FILE* file = fopen(path, "rb");

  // Seek to the end of the file
  fseek(file, 0L, SEEK_END);
  // Get the size of the file
  size_t fileSize = ftell(file);
  // Rewind to the beginning, since the main purpose of the above
  // is to get the fileSize
  rewind(file);

  // ALlocate a string of the filesize, + 1 for \0 at the end
  char* buffer = (char*)malloc(fileSize + 1);
  // System does not have enough memory to allocate the buffer
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }

  // Read the file into the buffer that is just allocated
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  // Reading the file itself might fail
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  // at the \0 right at the end.
  buffer[bytesRead] = '\0';

  // Close to file that that is opened above (fopen)
  fclose(file);

  // Return a pointer to the buffer, ownership of the pointer
  // belongs to whoever calls this function
  return buffer;
}

static void run_source(bool arguments[const], const char* source) {
  TokenArray token_array;
  init_token_array(&token_array);
  lex_source(&token_array, source);

  // All the errors will get pushed here
  ErrorArray error_array;
  init_error_array(&error_array);

  if (arguments[DUMP_TOKEN])
    disassemble_token_array(&token_array);

  AstArray ast_array;
  init_ast_array(&ast_array);

  parse_tokens(&token_array, &ast_array, &error_array);

  // Print out all the errors
  if (error_array.count > 0) {
    for (int i = 0; i < error_array.count; i++) {
      print_error(error_array.errors[i]);
    }
    // End the program
    return;
  }

  if (arguments[DUMP_AST])
    disassemble_ast(&ast_array);

  OpArray op_array;
  ValueArray value_array;
  LocalArray local_array;

  init_op_array(&op_array);
  init_value_array(&value_array);
  init_local_array(&local_array);
  reserve_local_array(&local_array, UINT8_MAX + 1);  // 256

  // Outputs OP code to op_array, stores locals in local_array
  // Codegen then outputs the "main" function
  // TODO : This main function isn't checked for whether it's "main"
  ObjFunc* main_func =
      codegen(&op_array, &value_array, &ast_array, &local_array);

  // Push the main_func onto the value array
  push_value_array(&value_array, OBJ_VAL(main_func));

  // Temporary, to get out of the VM loop
  push_op_array(&op_array, OP_RETURN);

  if (arguments[DUMP_CODEGEN])
    disassemble_opcode_values(&op_array, &value_array);

  Vm vm;
  init_vm(&vm);
  run(arguments, &vm, &op_array, &value_array, main_func);

  free_vm(&vm);
  free_op_array(&op_array);
  free_value_array(&value_array);
  free_local_array(&local_array);
  free_token_array(&token_array);
  free_error_array(&error_array);
  // free(source);
}

static void run_file(bool arguments[const], const char* path) {
  char* source = read_file(path);
  run_source(arguments, source);
  free(source);
}

static bool file_exists(const char* path) {
  FILE* file = fopen(path, "rb");
  // Check if fopen failed to open the file
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    return false;
  }

  return true;
}

// Flags guide
// ./nebula { no option } => REPL
// ./nebula { no option } { file } => Run file
// ./nebula { flags } => Error
// ./nebula { flags } { file } => Run file with flags
int main(int argc, const char* argv[]) {
  // Create an arguments array for all the total flags,
  // Set all of them to 0 at the start
  bool arguments[TOTAL_FLAGS] = {0};

  int available_flags_count = 0;

  // Note that by default the -v / --vm flag is turned on
  arguments[VM_OUTPUT] = true;

  // TODO : Need a way to determine whether it is a flag or a neb file
  // This can be done by checking for the .neb extension at the end

  // Check which arguments match which statement
  for (int i = 1; i < argc; i++) {
    // printf("Argument:%d : %s\n", i, argv[i]);

    if (strncmp(argv[i], "-b", 2) == 0 || strncmp(argv[i], "--byte", 6) == 0) {
      arguments[DUMP_TOKEN] = true;
      available_flags_count++;
    } else if (strncmp(argv[i], "-a", 2) == 0 ||
               strncmp(argv[i], "--ast", 5) == 0) {
      arguments[DUMP_AST] = true;
      available_flags_count++;
    } else if (strncmp(argv[i], "-c", 2) == 0 ||
               strncmp(argv[i], "--codegen", 9) == 0) {
      arguments[DUMP_CODEGEN] = true;
      available_flags_count++;
    } else if (strncmp(argv[i], "-v", 2) == 0 ||
               strncmp(argv[i], "--vm", 4) == 0) {
      arguments[VM_OUTPUT] = true;
      available_flags_count++;
    } else if (strncmp(argv[i], "-h", 2) == 0 ||
               strncmp(argv[i], "--help", 6) == 0) {
      arguments[HELP] = true;
      available_flags_count++;
    }
  }

  // If the user passed in help at all, don't run anything
  if (arguments[HELP]) {
    printf("Nebula flags:\n");
    printf("-a/--ast: Dump AST\n");
    printf("-c/--codegen: Dump Bytecode\n");
    printf("-v/--vm: Show VM output\n");
    printf("Nebula usage: ./nebula {flags} {file.neb}\n");
    return 0;
  }

  // printf("Flag count: %d\n", available_flags_count);
  // Just start the REPL
  if (argc - available_flags_count == 1) {
    start_repl();
  }
  // There are only two flags, check that the file exists first
  // ./nebula { no option } { file }
  // Pass it available_flags_count + 1, as {nebula [0]}, {flags[n]}, {file[n+1]}
  else if (argc - available_flags_count == 2) {
    if (!file_exists(argv[available_flags_count + 1])) {
      fprintf(stderr, "File %s does not exist\n",
              argv[available_flags_count + 1]);
      exit(74);
    }

    // File exists, just continue
    run_file(arguments, argv[available_flags_count + 1]);

    // Exit the program as there is no issue
    exit(0);
  } else {
    // Only can be an error, if it reaches this point
    fprintf(stderr, "Usage: nebula [path]\n");
    exit(64);
  }

  return 0;
}
