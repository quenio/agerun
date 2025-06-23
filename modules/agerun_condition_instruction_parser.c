#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "agerun_condition_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_heap.h"
#include "agerun_string.h"
#include "agerun_list.h"

/**
 * Opaque parser structure.
 */
struct ar_condition_instruction_parser_s {
    char *own_error;         /* Error message if parsing fails */
    size_t error_position;   /* Position where error occurred */
};

/**
 * Create a new condition instruction parser instance.
 */
ar_condition_instruction_parser_t* ar_condition_instruction_parser__create(void) {
    ar_condition_instruction_parser_t *own_parser = AR__HEAP__MALLOC(
        sizeof(ar_condition_instruction_parser_t), 
        "condition_instruction_parser"
    );
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->own_error = NULL;
    own_parser->error_position = 0;
    
    return own_parser;
}

/**
 * Destroy a condition instruction parser instance.
 */
void ar_condition_instruction_parser__destroy(ar_condition_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    AR__HEAP__FREE(own_parser->own_error);
    AR__HEAP__FREE(own_parser);
}

/**
 * Get the last error message from the parser.
 */
const char* ar_condition_instruction_parser__get_error(const ar_condition_instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error;
}

/**
 * Get the error position from the last parse attempt.
 */
size_t ar_condition_instruction_parser__get_error_position(const ar_condition_instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->error_position;
}

/**
 * Internal: Set error message and position.
 */
static void _set_error(ar_condition_instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error);
    mut_parser->own_error = AR__HEAP__STRDUP(error, "parser error message");
    mut_parser->error_position = position;
}

/**
 * Internal: Clear any previous error.
 */
static void _clear_error(ar_condition_instruction_parser_t *mut_parser) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error);
    mut_parser->own_error = NULL;
    mut_parser->error_position = 0;
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
 * Internal: Extract a single argument from function call.
 * Handles nested parentheses and quoted strings.
 */
static char* _extract_argument(const char *str, size_t *pos, char delimiter) {
    size_t start = *pos;
    int paren_depth = 0;
    bool in_quotes = false;
    
    /* Skip leading whitespace */
    while (str[*pos] && isspace((unsigned char)str[*pos])) {
        (*pos)++;
        start++;
    }
    
    /* Find delimiter or end */
    while (str[*pos]) {
        char c = str[*pos];
        
        if (c == '"' && (*pos == 0 || str[*pos - 1] != '\\')) {
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
    
    if (str[*pos] != delimiter) {
        return NULL;
    }
    
    /* Trim trailing whitespace */
    size_t end = *pos;
    while (end > start && isspace((unsigned char)str[end - 1])) {
        end--;
    }
    
    /* Extract argument */
    size_t len = end - start;
    char *arg = AR__HEAP__MALLOC(len + 1, "function argument");
    if (!arg) {
        return NULL;
    }
    memcpy(arg, str + start, len);
    arg[len] = '\0';
    
    return arg;
}

/**
 * Internal: Parse function arguments into an array.
 */
static bool _parse_arguments(const char *str, size_t *pos, char ***out_args, size_t *out_count, size_t expected_count) {
    *out_args = AR__HEAP__MALLOC(expected_count * sizeof(char*), "function arguments array");
    if (!*out_args) {
        return false;
    }
    
    *out_count = 0;
    
    for (size_t i = 0; i < expected_count; i++) {
        char delimiter = (i < expected_count - 1) ? ',' : ')';
        char *arg = _extract_argument(str, pos, delimiter);
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
            while (str[*pos] && isspace((unsigned char)str[*pos])) {
                (*pos)++;
            }
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
static void _cleanup_arg_asts(list_t *arg_asts) {
    if (arg_asts) {
        void **items = ar__list__items(arg_asts);
        if (items) {
            size_t list_count = ar__list__count(arg_asts);
            for (size_t j = 0; j < list_count; j++) {
                ar__expression_ast__destroy((ar_expression_ast_t*)items[j]);
            }
            AR__HEAP__FREE(items);
        }
        ar__list__destroy(arg_asts);
    }
}

/**
 * Internal: Parse argument strings into expression ASTs and return as a list.
 */
static list_t* _parse_arguments_to_asts(ar_condition_instruction_parser_t *mut_parser, 
                                        char **ref_args, 
                                        size_t arg_count,
                                        size_t error_offset) {
    list_t *own_arg_asts = ar__list__create();
    if (!own_arg_asts) {
        _set_error(mut_parser, "Failed to create argument AST list", error_offset);
        return NULL;
    }
    
    for (size_t i = 0; i < arg_count; i++) {
        ar_expression_parser_t *own_expr_parser = ar__expression_parser__create(ref_args[i]);
        if (!own_expr_parser) {
            _cleanup_arg_asts(own_arg_asts);
            _set_error(mut_parser, "Failed to create expression parser", error_offset);
            return NULL;
        }
        
        ar_expression_ast_t *own_expr_ast = ar__expression_parser__parse_expression(own_expr_parser);
        if (!own_expr_ast) {
            const char *expr_error = ar__expression_parser__get_error(own_expr_parser);
            char *own_error_copy = expr_error ? AR__HEAP__STRDUP(expr_error, "error message copy") : NULL;
            _cleanup_arg_asts(own_arg_asts);
            ar__expression_parser__destroy(own_expr_parser);
            _set_error(mut_parser, own_error_copy ? own_error_copy : "Failed to parse argument expression", error_offset);
            AR__HEAP__FREE(own_error_copy);
            return NULL;
        }
        
        if (!ar__list__add_last(own_arg_asts, own_expr_ast)) {
            _cleanup_arg_asts(own_arg_asts);
            ar__expression_ast__destroy(own_expr_ast);
            ar__expression_parser__destroy(own_expr_parser);
            _set_error(mut_parser, "Failed to add argument AST to list", error_offset);
            return NULL;
        }
        
        ar__expression_parser__destroy(own_expr_parser);
    }
    
    return own_arg_asts;
}

/**
 * Parse a condition (if) instruction.
 */
ar_instruction_ast_t* ar_condition_instruction_parser__parse(
    ar_condition_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    if (!mut_parser || !ref_instruction) {
        return NULL;
    }
    
    _clear_error(mut_parser);
    
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
    
    /* Check for "if" */
    if (strncmp(ref_instruction + pos, "if", 2) != 0) {
        _set_error(mut_parser, "Expected 'if' function", pos);
        return NULL;
    }
    pos += 2;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _set_error(mut_parser, "Expected '(' after 'if'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 3)) {
        _set_error(mut_parser, "Failed to parse if arguments", pos);
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
        _set_error(mut_parser, "Memory allocation failed", 0);
        return NULL;
    }
    for (size_t i = 0; i < arg_count; i++) {
        const_args[i] = args[i];
    }
    
    ar_instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        AR_INST__IF, "if", const_args, arg_count, ref_result_path
    );
    
    AR__HEAP__FREE(const_args);
    
    if (!own_ast) {
        _cleanup_args(args, arg_count);
        _set_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }
    
    /* Parse arguments into expression ASTs and set them in the instruction AST */
    list_t *own_arg_asts = _parse_arguments_to_asts(mut_parser, args, arg_count, pos);
    if (!own_arg_asts) {
        _cleanup_args(args, arg_count);
        ar__instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    if (!ar__instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        _cleanup_args(args, arg_count);
        ar__instruction_ast__destroy(own_ast);
        _cleanup_arg_asts(own_arg_asts);
        _set_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }
    
    _cleanup_args(args, arg_count);
    
    return own_ast;
}