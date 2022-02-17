#include <stdio.h>
#include "codegen.h"

static OpArray* op_array;
static ValueArray* constants_array;

static void emit_byte(OpCode op) {
    push_op_array(op_array, op);
}

static void emit_constant(Value value) {
    emit_byte(OP_CONSTANT);
    // Add to value_array
    push_value_array(constants_array, value);
    // Minus 1 of the current count as it is 0-indexed
    emit_byte(constants_array->count - 1);
}

static void gen(Ast* ast) {
    switch (ast->type) {
        case AST_NONE: 
            break;
        case AST_PRINT: {
            PrintStmt* print_stmt = (PrintStmt*)ast->as;
            // Emit nested statement, then emit print
            gen(print_stmt->expr);
            emit_byte(OP_PRINT);
            break;
        }
        case AST_NUMBER: { // emit a constant
            NumberExpr* number_expr = (NumberExpr*)ast->as;
            emit_constant(NUMBER_VAL(number_expr->value));
            break;
        }
        case AST_BINARY: {
            BinaryExpr* binary_expr = (BinaryExpr*)ast->as;
            gen(binary_expr->left_expr);
            gen(binary_expr->right_expr);
            //TODO : Handle the rest of the switch cases
            switch (binary_expr->op.type) {
                case TOKEN_PLUS:
                    emit_byte(OP_ADD);
                    break;
                case TOKEN_MINUS:
                    emit_byte(OP_SUBTRACT);
                    break;
                case TOKEN_STAR:
                    emit_byte(OP_MULTIPLY);
                    break;
                case TOKEN_SLASH:
                    emit_byte(OP_DIVIDE);
                    break;
                default:
                    break;
            }
            break;
        }
        case AST_UNARY: {
            UnaryExpr* unary_expr = (UnaryExpr*)ast->as;
            gen(unary_expr->right_expr);
            switch (unary_expr->op.type) {
                case TOKEN_BANG:
                    emit_byte(OP_NOT);
                    break;
                case TOKEN_MINUS:
                    emit_byte(OP_NEGATE);
                    break;
                default:
                    break;
            }
        }
        case AST_BOOL: {
            BoolExpr* bool_expr = (BoolExpr*)ast->as;
            if (bool_expr->value) {
                emit_byte(OP_TRUE);
                break;
            } else {
                emit_byte(OP_FALSE);
                break;
            }
        }
    }
}

void codegen(OpArray* op_arr, ValueArray* constants_arr, Ast* ast) {
    op_array = op_arr;
    constants_array = constants_arr;
    gen(ast);
}

void disassemble_opcode_values(OpArray* op_arr, ValueArray* value_arr) {
    printf("-----%s-----\n", "Codegen Disassembly");
    for (int i = 0; i < op_arr->count; i++) {
        switch (op_arr->ops[i]) {
            case OP_ADD:
                printf("[%-20s]\n", "OP_ADD"); break;
            case OP_TRUE:
                printf("[%-20s]\n", "OP_TRUE"); break;
            case OP_FALSE:
                printf("[%-20s]\n", "OP_FALSE"); break;
            case OP_SUBTRACT:
                printf("[%-20s]\n", "OP_SUBTRACT"); break;
            case OP_MULTIPLY:
                printf("[%-20s]\n", "OP_MULTIPLY"); break;
            case OP_DIVIDE:
                printf("[%-20s]\n", "OP_DIVIDE"); break;
            case OP_NEGATE:
                printf("[%-20s]\n", "OP_NEGATE"); break;
                break;
            case OP_NOT:
                printf("[%-20s]\n", "OP_NOT"); break;
                break;
            case OP_EQUAL:
                printf("[%-20s]\n", "OP_EQUAL"); break;
                break;
            case OP_CONSTANT:
                i++;
                printf("[%-20s] at %d: %f\n", "OP_CONSTANT", i,
                        AS_NUMBER(value_arr->values[op_arr->ops[i]])); break;
            case OP_RETURN:
                printf("[%-20s]\n", "OP_RETURN"); break;
            case OP_PRINT:
                printf("[%-20s]\n", "OP_PRINT"); break;
        }
    }
}
