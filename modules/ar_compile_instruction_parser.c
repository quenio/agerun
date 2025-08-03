#include "ar_compile_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_parser.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_string.h"
#include "ar_list.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct ar_compile_instruction_parser_s {
    ar_log_t *ref_log;       /* Log instance for error reporting (borrowed) */
};

/**
 * Internal: Log error message with position.
 */
static void _log_error(ar_compile_instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    /* Log the error with position */
    if (mut_parser->ref_log) {
        ar_log__error_at(mut_parser->ref_log, error, (int)position);
    }
}

/**
 * Skip whitespace and return new position.
 */
static size_t _skip_whitespace(const char *ref_str, size_t pos) {
    while (ref_str[pos] && ar_string__isspace(ref_str[pos])) {
        pos++;
    }
    return pos;
}

/**
 * Extract a single argument from function call.
 * Handles nested parentheses and quoted strings.
 */
static char* _extract_argument(const char *ref_str, size_t *pos, char delimiter) {
    int paren_depth = 0;
    bool in_quotes = false;
    
    /* Skip leading whitespace */
    *pos = _skip_whitespace(ref_str, *pos);
    size_t start = *pos;
    
    /* Check for empty argument */
    if (ref_str[*pos] == delimiter) {
        return NULL;
    }
    
    /* Find delimiter or end */
    while (ref_str[*pos]) {
        char c = ref_str[*pos];
        
        if (c == '"' && (*pos == 0 || ref_str[*pos - 1] != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes) {
            if (c == '(') paren_depth++;
            else if (c == ')') {
                if (paren_depth > 0) paren_depth--;
                else if (delimiter == ')') break;
            }
            else if (c == delimiter && paren_depth == 0) break;
        }
        (*pos)++;
    }
    
    if (ref_str[*pos] != delimiter) {
        return NULL;
    }
    
    /* Trim trailing whitespace */
    size_t end = *pos;
    while (end > start && ar_string__isspace(ref_str[end - 1])) {
        end--;
    }
    
    /* Extract argument */
    size_t len = end - start;
    char *arg = AR__HEAP__MALLOC(len + 1, "function argument");
    if (!arg) {
        return NULL;
    }
    memcpy(arg, ref_str + start, len);
    arg[len] = '\0';
    
    return arg;
}

/**
 * Parse function arguments.
 */
static bool _parse_arguments(const char *ref_str, size_t *pos, char ***out_args, size_t *out_count, size_t expected_count) {
    *out_args = AR__HEAP__MALLOC(expected_count * sizeof(char*), "function arguments array");
    if (!*out_args) {
        return false;
    }
    
    *out_count = 0;
    
    for (size_t i = 0; i < expected_count; i++) {
        char delimiter = (i < expected_count - 1) ? ',' : ')';
        char *arg = _extract_argument(ref_str, pos, delimiter);
        if (!arg) {
            /* Clean up on failure */
            for (size_t j = 0; j < *out_count; j++) {
                AR__HEAP__FREE((*out_args)[j]);
            }
            AR__HEAP__FREE(*out_args);
            *out_args = NULL;
            return false;
        }
        (*out_args)[i] = arg;
        (*out_count)++;
        
        if (i < expected_count - 1) {
            (*pos)++; /* Skip comma */
            /* Skip whitespace after comma */
            *pos = _skip_whitespace(ref_str, *pos);
        }
    }
    
    return true;
}

/**
 * Internal: Cleanup argument strings array.
 */
static void _cleanup_args(char **args, size_t arg_count) {
    if (args) {
        for (size_t i = 0; i < arg_count; i++) {
            AR__HEAP__FREE(args[i]);
        }
        AR__HEAP__FREE(args);
    }
}

/**
 * Internal: Cleanup argument AST list and all contained ASTs.
 */
static void _cleanup_arg_asts(ar_list_t *arg_asts) {
    if (arg_asts) {
        void **items = ar_list__items(arg_asts);
        if (items) {
            size_t list_count = ar_list__count(arg_asts);
            for (size_t j = 0; j < list_count; j++) {
                ar_expression_ast__destroy((ar_expression_ast_t*)items[j]);
            }
            AR__HEAP__FREE(items);
        }
        ar_list__destroy(arg_asts);
    }
}

/**
 * Internal: Parse argument strings into expression ASTs and return as a list.
 */
static ar_list_t* _parse_arguments_to_asts(ar_compile_instruction_parser_t *mut_parser, 
                                        char **ref_args, 
                                        size_t arg_count,
                                        size_t error_offset) {
    ar_list_t *own_arg_asts = ar_list__create();
    if (!own_arg_asts) {
        _log_error(mut_parser, "Failed to create argument AST list", error_offset);
        return NULL;
    }
    
    for (size_t i = 0; i < arg_count; i++) {
        ar_expression_parser_t *own_expr_parser = ar_expression_parser__create(mut_parser->ref_log, ref_args[i]);
        if (!own_expr_parser) {
            _cleanup_arg_asts(own_arg_asts);
            _log_error(mut_parser, "Failed to create expression parser", error_offset);
            return NULL;
        }
        
        ar_expression_ast_t *own_expr_ast = ar_expression_parser__parse_expression(own_expr_parser);
        if (!own_expr_ast) {
            /* Error already logged by expression parser to shared log */
            _cleanup_arg_asts(own_arg_asts);
            ar_expression_parser__destroy(own_expr_parser);
            /* Add context about which parser failed */
            _log_error(mut_parser, "Failed to parse argument expression", error_offset);
            return NULL;
        }
        
        if (!ar_list__add_last(own_arg_asts, own_expr_ast)) {
            _cleanup_arg_asts(own_arg_asts);
            ar_expression_ast__destroy(own_expr_ast);
            ar_expression_parser__destroy(own_expr_parser);
            _log_error(mut_parser, "Failed to add argument AST to list", error_offset);
            return NULL;
        }
        
        ar_expression_parser__destroy(own_expr_parser);
    }
    
    return own_arg_asts;
}

ar_compile_instruction_parser_t* ar_compile_instruction_parser__create(ar_log_t *ref_log) {
    ar_compile_instruction_parser_t *own_parser = AR__HEAP__CALLOC(1, sizeof(ar_compile_instruction_parser_t), "method parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->ref_log = ref_log;
    
    return own_parser;
}

void ar_compile_instruction_parser__destroy(ar_compile_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    AR__HEAP__FREE(own_parser);
}

ar_instruction_ast_t* ar_compile_instruction_parser__parse(
    ar_compile_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    if (!mut_parser || !ref_instruction) {
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
    
    /* Check for "compile" */
    if (strncmp(ref_instruction + pos, "compile", 7) != 0) {
        _log_error(mut_parser, "Expected 'compile' function", pos);
        return NULL;
    }
    pos += 7;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _log_error(mut_parser, "Expected '(' after 'compile'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 3)) {
        _log_error(mut_parser, "Failed to parse compile arguments", pos);
        return NULL;
    }
    
    /* Skip closing parenthesis */
    pos++;
    
    /* Create AST node - need to copy args to const array to avoid cast-qual warning */
    const char **const_args = AR__HEAP__MALLOC(arg_count * sizeof(const char*), "const args");
    if (!const_args) {
        for (size_t i = 0; i < arg_count; i++) {
            AR__HEAP__FREE(args[i]);
        }
        AR__HEAP__FREE(args);
        _log_error(mut_parser, "Memory allocation failed", 0);
        return NULL;
    }
    for (size_t i = 0; i < arg_count; i++) {
        const_args[i] = args[i];
    }
    
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "compile", const_args, arg_count, ref_result_path
    );
    
    AR__HEAP__FREE(const_args);
    
    if (!own_ast) {
        _cleanup_args(args, arg_count);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }
    
    /* Parse arguments into expression ASTs and set them in the instruction AST */
    ar_list_t *own_arg_asts = _parse_arguments_to_asts(mut_parser, args, arg_count, pos);
    if (!own_arg_asts) {
        _cleanup_args(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        _cleanup_args(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        _cleanup_arg_asts(own_arg_asts);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }
    
    _cleanup_args(args, arg_count);
    
    return own_ast;
}

/**
 * Gets the last error message from the parser.
 * DEPRECATED: This function always returns NULL. Use ar_log for error reporting.
 */
const char* ar_compile_instruction_parser__get_error(const ar_compile_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return NULL;
}

/**
 * Gets the position of the last error.
 * DEPRECATED: This function always returns 0. Use ar_log for error reporting.
 */
size_t ar_compile_instruction_parser__get_error_position(const ar_compile_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return 0;
}