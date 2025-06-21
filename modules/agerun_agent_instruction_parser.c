#include "agerun_agent_instruction_parser.h"
#include "agerun_heap.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @file agerun_agent_instruction_parser.c
 * @brief Implementation of the agent instruction parser module
 */

/* Parser state structure */
struct ar_agent_instruction_parser_s {
    char *own_error_message;    /* Error message from last parse */
    size_t error_position;      /* Position where error occurred */
};

/* Helper functions */
static void _set_error(ar_agent_instruction_parser_t *mut_parser, const char *error, size_t position) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error_message);
    mut_parser->own_error_message = AR__HEAP__STRDUP(error, "parser error message");
    mut_parser->error_position = position;
}

static void _clear_error(ar_agent_instruction_parser_t *mut_parser) {
    if (!mut_parser) {
        return;
    }
    
    AR__HEAP__FREE(mut_parser->own_error_message);
    mut_parser->own_error_message = NULL;
    mut_parser->error_position = 0;
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

static bool _parse_agent_arguments(const char *str, size_t *pos, char ***out_args, size_t *out_count) {
    // For agent(), we support 2 or 3 arguments
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
    
    /* Skip whitespace after comma */
    while (str[*pos] && isspace((unsigned char)str[*pos])) {
        (*pos)++;
    }
    
    // Parse second argument (version) - check if it's the last one
    arg = _extract_argument(str, pos, ')');
    if (arg) {
        // Found closing parenthesis, this is a 2-argument call
        (*out_args)[1] = arg;
        (*out_count)++;
        return true;
    }
    
    // Not a closing parenthesis, must be a comma, so try 3-argument form
    arg = _extract_argument(str, pos, ',');
    if (!arg) {
        /* Clean up on failure */
        for (size_t j = 0; j < *out_count; j++) {
            AR__HEAP__FREE((*out_args)[j]);
        }
        AR__HEAP__FREE(*out_args);
        *out_args = NULL;
        return false;
    }
    (*out_args)[1] = arg;
    (*out_count)++;
    (*pos)++; /* Skip comma */
    
    /* Skip whitespace after comma */
    while (str[*pos] && isspace((unsigned char)str[*pos])) {
        (*pos)++;
    }
    
    // Parse third argument (context)
    arg = _extract_argument(str, pos, ')');
    if (!arg) {
        /* Clean up on failure */
        for (size_t j = 0; j < *out_count; j++) {
            AR__HEAP__FREE((*out_args)[j]);
        }
        AR__HEAP__FREE(*out_args);
        *out_args = NULL;
        return false;
    }
    (*out_args)[2] = arg;
    (*out_count)++;
    
    return true;
}

/**
 * Create a new agent instruction parser instance
 */
ar_agent_instruction_parser_t* ar_agent_instruction_parser__create(void) {
    ar_agent_instruction_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(ar_agent_instruction_parser_t), "agent parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->own_error_message = NULL;
    own_parser->error_position = 0;
    
    return own_parser; // Ownership transferred to caller
}

/**
 * Destroy an agent instruction parser instance
 */
void ar_agent_instruction_parser__destroy(ar_agent_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    if (own_parser->own_error_message) {
        AR__HEAP__FREE(own_parser->own_error_message);
    }
    
    AR__HEAP__FREE(own_parser);
}

/**
 * Parse an agent instruction
 */
instruction_ast_t* ar_agent_instruction_parser__parse(
    ar_agent_instruction_parser_t *mut_parser,
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
    
    /* Check for "agent" */
    if (strncmp(ref_instruction + pos, "agent", 5) != 0) {
        _set_error(mut_parser, "Expected 'agent' function", pos);
        return NULL;
    }
    pos += 5;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _set_error(mut_parser, "Expected '(' after 'agent'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_agent_arguments(ref_instruction, &pos, &args, &arg_count)) {
        _set_error(mut_parser, "Failed to parse agent arguments", pos);
        return NULL;
    }
    
    /* Skip closing parenthesis */
    pos++;
    
    /* For agent(), if only 2 args provided, add NULL context to make it 3 args for evaluator */
    size_t final_arg_count = (arg_count == 2) ? 3 : arg_count;
    
    /* Create AST node - need to copy args to const array to avoid cast-qual warning */
    const char **const_args = AR__HEAP__MALLOC(final_arg_count * sizeof(const char*), "const args");
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
    
    /* If only 2 args, add "null" as third argument */
    if (arg_count == 2) {
        const_args[2] = "null";
    }
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_AGENT, "agent", const_args, final_arg_count, ref_result_path
    );
    
    AR__HEAP__FREE(const_args);
    
    /* Clean up arguments */
    for (size_t i = 0; i < arg_count; i++) {
        AR__HEAP__FREE(args[i]);
    }
    AR__HEAP__FREE(args);
    
    if (!own_ast) {
        _set_error(mut_parser, "Failed to create AST node", 0);
    }
    
    return own_ast;
}

/**
 * Get the last error message from the parser
 */
const char* ar_agent_instruction_parser__get_error(const ar_agent_instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error_message;
}

/**
 * Get the position where the last error occurred
 */
size_t ar_agent_instruction_parser__get_error_position(const ar_agent_instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->error_position;
}