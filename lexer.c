#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "token.h"

static int line = 1;
static int start = 0;
static int current = 0;
static const char* s = 0;

static Token make_token(TokenType type) {
    Token token;
    token.type = type;
    token.start = s + start;
    token.length = (int)(current - start);
    token.line = line;
    return token;
}

static void reset_local_varables() {
    line = 1;
    start = 0;
    current = 0;
}

static bool is_whitespace() {
    if (s[current] == ' '  ||
        s[current] == '\t' ||
        s[current] == '\r') {
        return true;
    }
    if (s[current] == '\n') {
        line++;
        return true;
    }
    return false;
}

static bool is_end() {
    if (s[current] == '\0') {
        return true;
    }
    return false;
}

static bool is_digit() {
    if (s[current] >= '0' && s[current] <= '9') {
        return true;
    }
    return false;
}

static bool is_alpha() {
    if ((s[current] >= 'a' && s[current] <= 'z') ||
        (s[current] >= 'A' && s[current] <= 'Z') ||
        (s[current] == '_')) {
        return true;
    }
    return false;
}

void lex_source(TokenArray* token_array, const char* source) {
    reset_local_varables();
    s = source;

    // While it is not the end
    while (!is_end()) {
        if (is_whitespace()) { // Will also handle incrementing new lines
            start++;
            current++;
        }

        if (is_digit()) {
            current = start;
            while (is_digit()) {
                current++;
            }
            Token token_digit = make_token(TOKEN_NUMBER);
            push_token_array(token_array, token_digit);
            start = current;
        } else if (is_alpha()) {
            current = start;
            while (is_alpha()) {
                current++;
            }
            Token token_identifier = make_token(TOKEN_IDENTIFIER);
            push_token_array(token_array, token_identifier);
            start = current;
        }

        // Lex all the single character tokens first
        if (s[current] == '(') {
            current++;
            Token token_left_paren = make_token(TOKEN_LEFT_PAREN);
            push_token_array(token_array, token_left_paren);
            start = current;
        } else if (s[current] == ')') {
            current++;
            Token token_right_paren = make_token(TOKEN_RIGHT_PAREN);
            push_token_array(token_array, token_right_paren);
            start = current;
        } else if (s[current] == '{') {
            current++;
            Token token_left_paren = make_token(TOKEN_LEFT_BRACE);
            push_token_array(token_array, token_left_paren);
            start = current;
        } else if (s[current] == '}') {
            current++;
            Token token_right_paren = make_token(TOKEN_RIGHT_BRACE);
            push_token_array(token_array, token_right_paren);
            start = current;
        } else if (s[current] == ',') {
            current++;
            Token token_comma = make_token(TOKEN_COMMA);
            push_token_array(token_array, token_comma);
            start = current;
        } else if (s[current] == '.') {
            current++;
            Token token_dot = make_token(TOKEN_DOT);
            push_token_array(token_array, token_dot);
            start = current;
        } else if (s[current] == '-') {
            current++;
            Token token_minus = make_token(TOKEN_MINUS);
            push_token_array(token_array, token_minus);
            start = current;
        } else if (s[current] == '+') {
            current++;
            Token token_plus = make_token(TOKEN_PLUS);
            push_token_array(token_array, token_plus);
            start = current;
        } else if (s[current] == ';') {
            current++;
            Token token_semicolon = make_token(TOKEN_SEMICOLON);
            push_token_array(token_array, token_semicolon);
            start = current;
        } else if (s[current] == '/') {
            current++;
            Token token_slash = make_token(TOKEN_SLASH);
            push_token_array(token_array, token_slash);
            start = current;
        } else if (s[current] == '*') {
            current++;
            Token token_star = make_token(TOKEN_STAR);
            push_token_array(token_array, token_star);
            start = current;
        }
    }
}

void disassemble_token_array(TokenArray* token_array) {
    printf("---%s---\n", "Token Disassembly");
    for (int i = 0; i < token_array->count; i++) {
        switch (token_array->tokens[i].type) {
            case TOKEN_LEFT_PAREN: 
                printf("[%-20s]: %s\n", "TOKEN_LEFT_PAREN", ")"); break;
            case TOKEN_RIGHT_PAREN: 
                printf("[%-20s]: %s\n", "TOKEN_RIGHT_PAREN", ")"); break;
            case TOKEN_LEFT_BRACE:
                printf("[%-20s]: %s\n", "TOKEN_LEFT_BRACE", "{"); break;
            case TOKEN_RIGHT_BRACE:
                printf("[%-20s]: %s\n", "TOKEN_RIGHT_BRACE", "}"); break;
            case TOKEN_COMMA:
                printf("[%-20s]: %s\n", "TOKEN_COMMA", ","); break;
            case TOKEN_DOT:
                printf("[%-20s]: %s\n", "TOKEN_DOT", "."); break;
            case TOKEN_MINUS:
                printf("[%-20s]: %s\n", "TOKEN_MINUS", "-"); break;
            case TOKEN_PLUS:
                printf("[%-20s]: %s\n", "TOKEN_PLUS", "+"); break;
            case TOKEN_SEMICOLON:
                printf("[%-20s]: %s\n", "TOKEN_SEMICOLON", ";"); break;
            case TOKEN_SLASH:
                printf("[%-20s]: %s\n", "TOKEN_SLASH", "/"); break;
            case TOKEN_STAR:
                printf("[%-20s]: %s\n", "TOKEN_STAR", "*"); break;
            case TOKEN_BANG:
                printf("[%-20s]: %s\n", "TOKEN_BANG", "!"); break;
            case TOKEN_BANG_EQUAL:
                printf("[%-20s]: %s\n", "TOKEN_BANG_EQUAL", "!="); break;
            case TOKEN_EQUAL_EQUAL:
                printf("[%-20s]: %s\n", "TOKEN_EQUAL_EQUAL", "=="); break;
            case TOKEN_GREATER:
                printf("[%-20s]: %s\n", "TOKEN_GREATER", ">"); break;
            case TOKEN_GREATER_EQUAL:
                printf("[%-20s]: %s\n", "TOKEN_GREATER_EQUAL", ">="); break;
            case TOKEN_LESS:
                printf("[%-20s]: %s\n", "TOKEN_LESS", "<"); break;
            case TOKEN_LESS_EQUAL:
                printf("[%-20s]: %s\n", "TOKEN_LESS_EQUAL", "<="); break;
            case TOKEN_IDENTIFIER: {
                char s[token_array->tokens[i].length + 1];
                strncpy(s, token_array->tokens[i].start, token_array->tokens[i].length);
                // Delimit it with c_str end char
                s[token_array->tokens[i].length] = '\0';
                printf("[%-20s]: %s\n", "TOKEN_IDENTIFIER", s); break;
            }
            case TOKEN_STRING:
                printf("[%-20s]: %s\n", "TOKEN_STRING", "STRING-PLACEHOLDER"); break;
            case TOKEN_AND:
                printf("[%-20s]: %s\n", "TOKEN_AND", "AND"); break;
            case TOKEN_ELSE:
                printf("[%-20s]: %s\n", "TOKEN_ELSE", "ELSE"); break;
            case TOKEN_FOR:
                printf("[%-20s]: %s\n", "TOKEN_FOR", "FOR"); break;
            case TOKEN_FUNC:
                printf("[%-20s]: %s\n", "TOKEN_FUNC", "FUNC"); break;
            case TOKEN_IF:
                printf("[%-20s]: %s\n", "TOKEN_IF", "IF"); break;
            case TOKEN_NIL:
                printf("[%-20s]: %s\n", "TOKEN_NIL", "NIL"); break;
            case TOKEN_OR:
                printf("[%-20s]: %s\n", "TOKEN_OR", "OR"); break;
            case TOKEN_PRINT:
                printf("[%-20s]: %s\n", "TOKEN_PRINT", "PRINT"); break;
            case TOKEN_RETURN:
                printf("[%-20s]: %s\n", "TOKEN_RETURN", "RETURN"); break;
            case TOKEN_VAR:
                printf("[%-20s]: %s\n", "TOKEN_VAR", "VAR"); break;
            case TOKEN_WHILE:
                printf("[%-20s]: %s\n", "TOKEN_WHILE", "WHILE"); break;
            case TOKEN_TRUE:
                printf("[%-20s]: %s\n", "TOKEN_TRUE", "TRUE"); break;
            case TOKEN_FALSE:
                printf("[%-20s]: %s\n", "TOKEN_FALSE", "FALSE"); break;
            case TOKEN_ERROR:
                printf("[%-20s]: %s\n", "TOKEN_ERROR", "ERROR"); break;
            case TOKEN_EOF:
                printf("[%-20s]: %s\n", "TOKEN_EOF", "EOF"); break;
            case TOKEN_NUMBER: {
                // TODO : use strtod to conver to double
                // printf("[%-20s]: %f\n", "TOKEN_NUMBER", strtod();
                const char* start = token_array->tokens[i].start;
                // Cast to char*, otherwise it will be regarded as a const char*
                // strtod expects a char* for it's second parameter.
                char* end = (char*)token_array->tokens[i].start + token_array->tokens[i].length;
                double number = strtod(start, &end);
                printf("[%-20s]: %f\n", "TOKEN_NUMBER", number); break;
            }
            case TOKEN_EQUAL:
                printf("[%-20s]: %s\n", "TOKEN_EQUAL", "="); break;
        }
    }
}
