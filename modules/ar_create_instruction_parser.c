#include "ar_create_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"
#include "ar_expression_parser.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_log.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @file ar_create_instruction_parser.c
 * @brief Implementation of the create instruction parser module
 */

/* Parser state structure */
struct ar_create_instruction_parser_s {
    ar_log_t *ref_log;          /* Log instance for error reporting (borrowed) */
};

/* Helper functions */
static void _log_error(ar_create_instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser || !error) {
        return;
    }
    
    /* Log the error with position */
    if (mut_parser->ref_log) {
        ar_log__error_at(mut_parser->ref_log, error, (int)position);
    }
}


static size_t _skip_whitespace(const char *str, size_t pos) {
    while (str[pos] && isspace((unsigned char)str[pos])) {
        pos++;
    }
    return pos;
}

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

static void _cleanup_string_array(char **args, size_t count) {
    if (args) {
        for (size_t i = 0; i < count; i++) {
            AR__HEAP__FREE(args[i]);
        }
        AR__HEAP__FREE(args);
    }
}

static void _cleanup_parsed_args(char ***args, size_t count) {
    if (*args) {
        _cleanup_string_array(*args, count);
        *args = NULL;
    }
}

static bool _parse_create_arguments(const char *str, size_t *pos, char ***out_args, size_t *out_count) {
    // For create(), we support 2 or 3 arguments
    const size_t max_args = 3;
    *out_args = AR__HEAP__MALLOC(max_args * sizeof(char*), "function arguments array");
    if (!*out_args) {
        return false;
    }
    
    *out_count = 0;
    
    // Parse first argument (method name)
    char *arg = _extract_argument(str, pos, ',');
    if (!arg) {
        AR__HEAP__FREE(*out_args);
        *out_args = NULL;
        return false;
    }
    (*out_args)[0] = arg;
    (*out_count)++;
    (*pos)++; /* Skip comma */
    
    *pos = _skip_whitespace(str, *pos);
    
    // Parse second argument (version)
    // First, check ahead to see if there's a comma or closing paren after this argument
    size_t look_ahead = *pos;
    arg = _extract_argument(str, &look_ahead, ',');
    if (arg && str[look_ahead] == ',') {
        // Found a comma, this is a 3-argument call
        (*out_args)[1] = arg;
        (*out_count)++;
        *pos = look_ahead + 1; /* Skip comma */
    } else {
        // No comma found, try parsing until closing paren (2-argument call)
        AR__HEAP__FREE(arg); // Free the lookahead result
        arg = _extract_argument(str, pos, ')');
        if (!arg) {
            _cleanup_parsed_args(out_args, *out_count);
            return false;
        }
        (*out_args)[1] = arg;
        (*out_count)++;
        return true; // 2-argument form complete
    }
    
    *pos = _skip_whitespace(str, *pos);
    
    // Parse third argument (context)
    arg = _extract_argument(str, pos, ')');
    if (!arg) {
        _cleanup_parsed_args(out_args, *out_count);
        return false;
    }
    (*out_args)[2] = arg;
    (*out_count)++;
    
    return true;
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
static ar_list_t* _parse_arguments_to_asts(ar_create_instruction_parser_t *mut_parser, 
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
            const char *expr_error = ar_expression_parser__get_error(own_expr_parser);
            char *own_error_copy = expr_error ? AR__HEAP__STRDUP(expr_error, "error message copy") : NULL;
            _cleanup_arg_asts(own_arg_asts);
            ar_expression_parser__destroy(own_expr_parser);
            _log_error(mut_parser, own_error_copy ? own_error_copy : "Failed to parse argument expression", error_offset);
            AR__HEAP__FREE(own_error_copy);
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

/**
 * Create a new create instruction parser instance
 */
ar_create_instruction_parser_t* ar_create_instruction_parser__create(ar_log_t *ref_log) {
    ar_create_instruction_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_create_instruction_parser_t), "create parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->ref_log = ref_log;
    
    return own_parser; // Ownership transferred to caller
}

/**
 * Destroy a create instruction parser instance
 */
void ar_create_instruction_parser__destroy(ar_create_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Parse a create instruction
 */
ar_instruction_ast_t* ar_create_instruction_parser__parse(
    ar_create_instruction_parser_t *mut_parser,
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
    
    /* Check for "create" */
    if (strncmp(ref_instruction + pos, "create", 6) != 0) {
        _log_error(mut_parser, "Expected 'create' function", pos);
        return NULL;
    }
    pos += 6;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _log_error(mut_parser, "Expected '(' after 'create'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_create_arguments(ref_instruction, &pos, &args, &arg_count)) {
        _log_error(mut_parser, "Failed to parse create arguments", pos);
        return NULL;
    }
    
    /* Skip closing parenthesis */
    pos++;
    
    /* For create(), if only 2 args provided, add NULL context to make it 3 args for evaluator */
    size_t final_arg_count = (arg_count == 2) ? 3 : arg_count;
    
    /* Create AST node - need to copy args to const array to avoid cast-qual warning */
    const char **const_args = AR__HEAP__MALLOC(final_arg_count * sizeof(const char*), "const args");
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
    
    /* If only 2 args, add "null" as third argument */
    if (arg_count == 2) {
        const_args[2] = "null";
    }
    
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__CREATE, "create", const_args, final_arg_count, ref_result_path
    );
    
    AR__HEAP__FREE(const_args);
    
    if (!own_ast) {
        /* Clean up arguments before returning */
        _cleanup_string_array(args, arg_count);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }
    
    /* Parse arguments into expression ASTs and set them in the instruction AST */
    ar_list_t *own_arg_asts = _parse_arguments_to_asts(mut_parser, args, arg_count, pos);
    if (!own_arg_asts) {
        /* Clean up arguments and AST before returning */
        _cleanup_string_array(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    /* If we added a "null" context, add it to the AST list */
    if (arg_count == 2) {
        /* Create a null literal AST for the third argument */
        ar_log_t *own_log_for_null = ar_log__create();
        if (!own_log_for_null) {
            _cleanup_string_array(args, arg_count);
            ar_list__destroy(own_arg_asts);
            ar_instruction_ast__destroy(own_ast);
            _log_error(mut_parser, "Failed to create ar_log", pos);
            return NULL;
        }
        ar_expression_parser_t *own_null_parser = ar_expression_parser__create(own_log_for_null, "null");
        if (own_null_parser) {
            ar_expression_ast_t *own_null_ast = ar_expression_parser__parse_expression(own_null_parser);
            if (own_null_ast) {
                ar_list__add_last(own_arg_asts, own_null_ast);
            }
            ar_expression_parser__destroy(own_null_parser);
        }
        ar_log__destroy(own_log_for_null);
    }
    
    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        /* Clean up everything on failure */
        _cleanup_string_array(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        _cleanup_arg_asts(own_arg_asts);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }
    
    /* Clean up arguments */
    _cleanup_string_array(args, arg_count);
    
    return own_ast;
}

/**
 * Get the last error message from the parser
 * DEPRECATED: This function always returns NULL. Use ar_log for error reporting.
 */
const char* ar_create_instruction_parser__get_error(const ar_create_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return NULL;
}

/**
 * Get the position where the last error occurred
 * DEPRECATED: This function always returns 0. Use ar_log for error reporting.
 */
size_t ar_create_instruction_parser__get_error_position(const ar_create_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return 0;
}

