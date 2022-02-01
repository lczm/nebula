#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "lexer.h"

static void start_repl() {
    printf("Nebula\n");
}

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    // Check if fopen failed to open the file
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

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

static void run_file(const char* path) {
    char* source = read_file(path);
    TokenArray token_array;
    init_token_array(&token_array);

    lex_source(&token_array, source);
    disassemble_token_array(&token_array);

    free(source);
    free_token_array(&token_array);
}

int main(int argc, const char* argv[]) {
    if (argc == 1) { // start the repl
        start_repl();
    } else if (argc == 2) {
        // TODO: will need to support multiple files in the future
        run_file(argv[1]);
    } else {
        fprintf(stderr, "Usage: nebula[path]\n");
        exit(64);
    }

    return 0;
}
