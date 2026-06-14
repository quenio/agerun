#include "ar_spawn_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"
#include "ar_function_call_parser.h"
#include "ar_expression_parser.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_log.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @file ar_spawn_instruction_parser.c
 * @brief Implementation of the spawn instruction parser module
 */

/* Parser state structure */
struct ar_spawn_instruction_parser_s {
    ar_log_t *ref_log;          /* Log instance for error reporting (borrowed) */
};

/* Helper functions */
static void _log_error(ar_spawn_instruction_parser_t *mut_parser, const char *error, size_t position) {
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

static void _cleanup_parsed_args(char ***args, size_t count) {
    if (*args) {
        ar_function_call_parser__destroy_args(*args, count);
        *args = NULL;
    }
}

static bool _parse_create_arguments(ar_spawn_instruction_parser_t *mut_parser, const char *str, size_t *pos, char ***out_args, size_t *out_count) {
    // For create(), we support 2 or 3 arguments
    const size_t max_args = 3;
    *out_args = AR__HEAP__MALLOC(max_args * sizeof(char*), "function arguments array");
    if (!*out_args) {
        _log_error(mut_parser, "Memory allocation failed", *pos);
        return false;
    }
    
    *out_count = 0;
    
    // Parse first argument (method name)
    char *arg = ar_function_call_parser__extract_argument(mut_parser->ref_log, str, pos, ',');
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
    arg = ar_function_call_parser__extract_argument(NULL, str, &look_ahead, ',');
    if (arg && str[look_ahead] == ',') {
        // Found a comma, this is a 3-argument call
        (*out_args)[1] = arg;
        (*out_count)++;
        *pos = look_ahead + 1; /* Skip comma */
    } else {
        // No comma found, try parsing until closing paren (2-argument call)
        ar_function_call_parser__destroy_arg(arg); // Free the lookahead result
        arg = ar_function_call_parser__extract_argument(mut_parser->ref_log, str, pos, ')');
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
    look_ahead = *pos;
    arg = ar_function_call_parser__extract_argument(NULL, str, &look_ahead, ',');
    if (arg && str[look_ahead] == ',') {
        ar_function_call_parser__destroy_arg(arg);
        _log_error(mut_parser, "spawn() expects two or three arguments", look_ahead);
        _cleanup_parsed_args(out_args, *out_count);
        return false;
    }
    ar_function_call_parser__destroy_arg(arg);

    arg = ar_function_call_parser__extract_argument(mut_parser->ref_log, str, pos, ')');
    if (!arg) {
        _cleanup_parsed_args(out_args, *out_count);
        return false;
    }
    (*out_args)[2] = arg;
    (*out_count)++;
    
    return true;
}

/**
 * Create a new spawn instruction parser instance
 */
ar_spawn_instruction_parser_t* ar_spawn_instruction_parser__create(ar_log_t *ref_log) {
    ar_spawn_instruction_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_spawn_instruction_parser_t), "spawn parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->ref_log = ref_log;
    
    return own_parser; // Ownership transferred to caller
}

/**
 * Destroy a spawn instruction parser instance
 */
void ar_spawn_instruction_parser__destroy(ar_spawn_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Parse a spawn instruction
 */
ar_instruction_ast_t* ar_spawn_instruction_parser__parse(
    ar_spawn_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    if (!mut_parser) {
        return NULL;
    }
    
    if (!ref_instruction) {
        _log_error(mut_parser, "NULL instruction provided to spawn parser", 0);
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
    
    /* Check for "spawn" */
    if (strncmp(ref_instruction + pos, "spawn", 5) != 0) {
        _log_error(mut_parser, "Expected 'spawn' function", pos);
        return NULL;
    }
    pos += 5;
    
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
    if (!_parse_create_arguments(mut_parser, ref_instruction, &pos, &args, &arg_count)) {
        /* Error already logged by _parse_create_arguments */
        return NULL;
    }
    
    /* Skip closing parenthesis */
    pos++;
    
    /* For create(), if only 2 args provided, add NULL context to make it 3 args for evaluator */
    size_t final_arg_count = (arg_count == 2) ? 3 : arg_count;
    
    /* Create AST node - need to copy args to const array to avoid cast-qual warning */
    const char **const_args = AR__HEAP__MALLOC(final_arg_count * sizeof(const char*), "const args");
    if (!const_args) {
        ar_function_call_parser__destroy_args(args, arg_count);
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
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", const_args, final_arg_count, ref_result_path
    );
    
    AR__HEAP__FREE(const_args);
    
    if (!own_ast) {
        /* Clean up arguments before returning */
        ar_function_call_parser__destroy_args(args, arg_count);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }
    
    /* Parse arguments into expression ASTs and set them in the instruction AST */
    ar_list_t *own_arg_asts = ar_function_call_parser__parse_arg_asts(
        mut_parser->ref_log, args, arg_count, pos
    );
    if (!own_arg_asts) {
        /* Clean up arguments and AST before returning */
        ar_function_call_parser__destroy_args(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }
    
    /* If we added a "null" context, add it to the AST list */
    if (arg_count == 2) {
        /* Create a null literal AST for the third argument */
        ar_log_t *own_log_for_null = ar_log__create();
        if (!own_log_for_null) {
            ar_function_call_parser__destroy_args(args, arg_count);
            ar_function_call_parser__destroy_arg_asts(own_arg_asts);
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
        ar_function_call_parser__destroy_args(args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        ar_function_call_parser__destroy_arg_asts(own_arg_asts);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }
    
    /* Clean up arguments */
    ar_function_call_parser__destroy_args(args, arg_count);
    
    return own_ast;
}

/**
 * Get the last error message from the parser
 * DEPRECATED: This function always returns NULL. Use ar_log for error reporting.
 */
const char* ar_spawn_instruction_parser__get_error(const ar_spawn_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return NULL;
}

/**
 * Get the position where the last error occurred
 * DEPRECATED: This function always returns 0. Use ar_log for error reporting.
 */
size_t ar_spawn_instruction_parser__get_error_position(const ar_spawn_instruction_parser_t *ref_parser) {
    (void)ref_parser; // Suppress unused parameter warning
    return 0;
}
