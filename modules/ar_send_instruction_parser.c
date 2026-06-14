#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "ar_send_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_function_call_parser.h"
#include "ar_heap.h"

/**
 * Opaque parser structure.
 */
struct ar_send_instruction_parser_s {
    ar_log_t *ref_log;       /* Log instance for error reporting (borrowed) */
};

/**
 * Create a new send instruction parser instance.
 */
ar_send_instruction_parser_t* ar_send_instruction_parser__create(ar_log_t *ref_log) {
    ar_send_instruction_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_send_instruction_parser_t), "send_instruction_parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->ref_log = ref_log;
    
    return own_parser;
}

/**
 * Destroy a send instruction parser instance.
 */
void ar_send_instruction_parser__destroy(ar_send_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Get the last error message from the parser.
 * DEPRECATED: This function always returns NULL. Use ar_log for error reporting.
 */
const char* ar_send_instruction_parser__get_error(const ar_send_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return NULL;
}

/**
 * Get the error position from the last parse attempt.
 * DEPRECATED: This function always returns 0. Use ar_log for error reporting.
 */
size_t ar_send_instruction_parser__get_error_position(const ar_send_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return 0;
}

/**
 * Internal: Log error message with position.
 */
static void _log_error(ar_send_instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    /* Log the error with position */
    if (mut_parser->ref_log) {
        ar_log__error_at(mut_parser->ref_log, error, (int)position);
    }
}


/**
 * Internal: Skip whitespace in string.
 */
static size_t _skip_whitespace(const char *str, size_t pos) {
    while (str[pos] && isspace((unsigned char)str[pos])) {
        pos++;
    }
    return pos;
}

/**
 * Parse a send instruction.
 */
ar_instruction_ast_t* ar_send_instruction_parser__parse(
    ar_send_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    if (!mut_parser) {
        return NULL;
    }
    
    if (!ref_instruction) {
        _log_error(mut_parser, "NULL instruction provided to send parser", 0);
        return NULL;
    }
    
    size_t pos = 0;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Handle optional assignment */
    if (ref_result_path) {
        /* Find where the function call starts after the assignment */
        const char *assign_pos = strstr(ref_instruction, ":=");
        if (assign_pos) {
            pos = (size_t)(assign_pos - ref_instruction) + 2;
            pos = _skip_whitespace(ref_instruction, pos);
        }
    }
    
    /* Check for "send" */
    if (strncmp(ref_instruction + pos, "send", 4) != 0) {
        _log_error(mut_parser, "Expected 'send' function", pos);
        return NULL;
    }
    pos += 4;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _log_error(mut_parser, "Expected '(' after 'send'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!ar_function_call_parser__parse_exact(mut_parser->ref_log, ref_instruction, &pos, &args, &arg_count, 2)) {
        /* Error already logged by the function-call parser. */
        return NULL;
    }
    
    /* Skip closing parenthesis */
    pos++;
    
    /* Create AST node - need to copy args to const array to avoid cast-qual warning */
    const char **const_args = AR__HEAP__MALLOC(arg_count * sizeof(const char*), "const args");
    if (!const_args) {
        ar_function_call_parser__destroy_args(args, arg_count);
        _log_error(mut_parser, "Memory allocation failed", 0);
        return NULL;
    }
    for (size_t i = 0; i < arg_count; i++) {
        const_args[i] = args[i];
    }
    
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", const_args, arg_count, ref_result_path
    );
    
    AR__HEAP__FREE(const_args);
    
    if (!own_ast) {
        ar_function_call_parser__destroy_args(args, arg_count);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }
    
    /* Parse arguments into expression ASTs and set them in the instruction AST */
    ar_list_t *own_arg_asts = ar_function_call_parser__parse_arg_asts(
        mut_parser->ref_log, args, arg_count, pos
    );
    if (!own_arg_asts) {
        ar_function_call_parser__destroy_args(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        ar_function_call_parser__destroy_args(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        ar_function_call_parser__destroy_arg_asts(own_arg_asts);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }
    
    ar_function_call_parser__destroy_args(args, arg_count);
    
    return own_ast;
}
