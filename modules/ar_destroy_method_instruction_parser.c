#include "ar_destroy_method_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"
#include "ar_expression_parser.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Internal parser structure.
 */
struct ar_destroy_method_instruction_parser_s {
    ar_log_t *ref_log;         /* Log instance for error reporting (borrowed) */
    char *own_error_message;
    size_t error_position;
};

/**
 * Creates a new destroy method instruction parser.
 */
ar_destroy_method_instruction_parser_t* ar_destroy_method_instruction_parser__create(ar_log_t *ref_log) {
    ar_destroy_method_instruction_parser_t *own_parser = AR__HEAP__MALLOC(
        sizeof(ar_destroy_method_instruction_parser_t),
        "destroy method instruction parser"
    );
    
    if (own_parser == NULL) {
        return NULL;
    }
    
    own_parser->ref_log = ref_log;
    own_parser->own_error_message = NULL;
    own_parser->error_position = 0;
    
    return own_parser;
}

/**
 * Destroys a destroy method instruction parser.
 */
void ar_destroy_method_instruction_parser__destroy(ar_destroy_method_instruction_parser_t *own_parser) {
    if (own_parser == NULL) {
        return;
    }
    
    if (own_parser->own_error_message != NULL) {
        AR__HEAP__FREE(own_parser->own_error_message);
    }
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Internal: Set error message and position.
 */
static void _set_error(ar_destroy_method_instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error_message);
    mut_parser->own_error_message = AR__HEAP__STRDUP(error, "parser error message");
    mut_parser->error_position = position;
    
    /* Also log the error with position */
    if (mut_parser->ref_log) {
        ar_log__error_at(mut_parser->ref_log, error, (int)position);
    }
}

/**
 * Internal: Clear any previous error.
 */
static void _clear_error(ar_destroy_method_instruction_parser_t *mut_parser) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error_message);
    mut_parser->own_error_message = NULL;
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
    
    /* Check for empty argument */
    if (start == end) {
        return NULL;
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
 * Internal: Cleanup argument AST list and all contained ASTs.
 */
static void _cleanup_arg_asts(list_t *arg_asts) {
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
static list_t* _parse_arguments_to_asts(ar_destroy_method_instruction_parser_t *mut_parser, 
                                        char **ref_args, 
                                        size_t arg_count,
                                        size_t error_offset) {
    list_t *own_arg_asts = ar_list__create();
    if (!own_arg_asts) {
        _set_error(mut_parser, "Failed to create argument AST list", error_offset);
        return NULL;
    }
    
    for (size_t i = 0; i < arg_count; i++) {
        ar_expression_parser_t *own_expr_parser = ar_expression_parser__create(mut_parser->ref_log, ref_args[i]);
        if (!own_expr_parser) {
            _cleanup_arg_asts(own_arg_asts);
            _set_error(mut_parser, "Failed to create expression parser", error_offset);
            return NULL;
        }
        
        ar_expression_ast_t *own_expr_ast = ar_expression_parser__parse_expression(own_expr_parser);
        if (!own_expr_ast) {
            const char *expr_error = ar_expression_parser__get_error(own_expr_parser);
            // Copy error message before destroying parser
            char *own_error_copy = NULL;
            if (expr_error) {
                own_error_copy = AR__HEAP__STRDUP(expr_error, "error message copy");
            }
            _cleanup_arg_asts(own_arg_asts);
            ar_expression_parser__destroy(own_expr_parser);
            _set_error(mut_parser, own_error_copy ? own_error_copy : "Failed to parse argument expression", error_offset);
            AR__HEAP__FREE(own_error_copy);
            return NULL;
        }
        
        if (!ar_list__add_last(own_arg_asts, own_expr_ast)) {
            _cleanup_arg_asts(own_arg_asts);
            ar_expression_ast__destroy(own_expr_ast);
            ar_expression_parser__destroy(own_expr_parser);
            _set_error(mut_parser, "Failed to add argument AST to list", error_offset);
            return NULL;
        }
        
        ar_expression_parser__destroy(own_expr_parser);
    }
    
    return own_arg_asts;
}

/**
 * Parses a destroy method instruction.
 */
ar_instruction_ast_t* ar_destroy_method_instruction_parser__parse(
    ar_destroy_method_instruction_parser_t *mut_parser,
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
    
    /* Check for "destroy" */
    if (strncmp(ref_instruction + pos, "destroy", 7) != 0) {
        _set_error(mut_parser, "Expected 'destroy' function", pos);
        return NULL;
    }
    pos += 7;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _set_error(mut_parser, "Expected '(' after 'destroy'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse first argument (method name) */
    char *arg1 = _extract_argument(ref_instruction, &pos, ',');
    if (!arg1) {
        _set_error(mut_parser, "Failed to parse method name argument", pos);
        return NULL;
    }
    
    /* Skip comma */
    pos++;
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Parse second argument (version) */
    char *arg2 = _extract_argument(ref_instruction, &pos, ')');
    if (!arg2) {
        AR__HEAP__FREE(arg1);
        _set_error(mut_parser, "Failed to parse version argument", pos);
        return NULL;
    }
    
    /* Skip closing parenthesis */
    pos++;
    
    /* Create AST node */
    const char *const_args[] = { arg1, arg2 };
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INST__DESTROY_METHOD, "destroy", const_args, 2, ref_result_path
    );
    
    if (!own_ast) {
        AR__HEAP__FREE(arg1);
        AR__HEAP__FREE(arg2);
        _set_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }
    
    /* Parse arguments into expression ASTs and set them in the instruction AST */
    char *args_array[] = { arg1, arg2 };
    list_t *own_arg_asts = _parse_arguments_to_asts(mut_parser, args_array, 2, pos);
    
    /* Clean up argument strings - no longer needed after parsing */
    AR__HEAP__FREE(arg1);
    AR__HEAP__FREE(arg2);
    
    if (!own_arg_asts) {
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        ar_instruction_ast__destroy(own_ast);
        _cleanup_arg_asts(own_arg_asts);
        _set_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }
    
    return own_ast;
}

/**
 * Gets the last error message from the parser.
 */
const char* ar_destroy_method_instruction_parser__get_error(
    const ar_destroy_method_instruction_parser_t *ref_parser
) {
    if (ref_parser == NULL) {
        return NULL;
    }
    return ref_parser->own_error_message;
}

/**
 * Gets the position of the last error.
 */
size_t ar_destroy_method_instruction_parser__get_error_position(
    const ar_destroy_method_instruction_parser_t *ref_parser
) {
    if (ref_parser == NULL) {
        return 0;
    }
    return ref_parser->error_position;
}