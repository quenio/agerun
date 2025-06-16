#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "agerun_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_heap.h"
#include "agerun_string.h"

/**
 * Opaque parser structure.
 */
struct instruction_parser_s {
    char *own_error;         /* Error message if parsing fails */
    size_t error_position;   /* Position where error occurred */
};

/**
 * Create a new instruction parser instance.
 */
instruction_parser_t* ar__instruction_parser__create(void) {
    instruction_parser_t *own_parser = AR__HEAP__MALLOC(sizeof(instruction_parser_t), "instruction_parser");
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->own_error = NULL;
    own_parser->error_position = 0;
    
    return own_parser;
}

/**
 * Destroy an instruction parser instance.
 */
void ar__instruction_parser__destroy(instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    AR__HEAP__FREE(own_parser->own_error);
    AR__HEAP__FREE(own_parser);
}

/**
 * Get the last error message from the parser.
 */
const char* ar__instruction_parser__get_error(const instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error;
}

/**
 * Get the error position from the last parse attempt.
 */
size_t ar__instruction_parser__get_error_position(const instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->error_position;
}

/**
 * Internal: Set error message and position.
 */
static void _set_error(instruction_parser_t *mut_parser, const char *error, size_t position) {
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
static void _clear_error(instruction_parser_t *mut_parser) {
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
 * Internal: Find the end of a memory path.
 */
static size_t _find_path_end(const char *str, size_t pos) {
    while (str[pos] && (isalnum((unsigned char)str[pos]) || str[pos] == '.' || str[pos] == '_')) {
        pos++;
    }
    return pos;
}

/**
 * Internal: Find the end of an expression (everything after :=).
 */
static size_t _find_expression_end(const char *str, size_t pos) {
    size_t len = strlen(str);
    
    /* Find the end, trimming trailing whitespace */
    size_t end = len;
    while (end > pos && isspace((unsigned char)str[end - 1])) {
        end--;
    }
    
    return end;
}

/**
 * Parse an assignment instruction.
 */
instruction_ast_t* ar__instruction_parser__parse_assignment(instruction_parser_t *mut_parser, const char *ref_instruction) {
    if (!mut_parser || !ref_instruction) {
        return NULL;
    }
    
    _clear_error(mut_parser);
    
    size_t pos = 0;
    size_t len = strlen(ref_instruction);
    
    /* Skip leading whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Check for empty instruction */
    if (pos >= len) {
        _set_error(mut_parser, "Empty instruction", pos);
        return NULL;
    }
    
    /* Find memory path */
    size_t path_start = pos;
    pos = _find_path_end(ref_instruction, pos);
    size_t path_end = pos;
    
    if (path_start == path_end) {
        _set_error(mut_parser, "Expected memory path", pos);
        return NULL;
    }
    
    /* Check that path starts with "memory" */
    if (path_end - path_start < 6 || strncmp(ref_instruction + path_start, "memory", 6) != 0) {
        _set_error(mut_parser, "Path must start with 'memory'", path_start);
        return NULL;
    }
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Check for assignment operator */
    if (pos + 1 >= len || ref_instruction[pos] != ':' || ref_instruction[pos + 1] != '=') {
        _set_error(mut_parser, "Expected ':=' operator", pos);
        return NULL;
    }
    pos += 2;
    
    /* Skip whitespace after := */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Find expression */
    size_t expr_start = pos;
    size_t expr_end = _find_expression_end(ref_instruction, pos);
    
    if (expr_start == expr_end) {
        _set_error(mut_parser, "Expected expression after ':='", pos);
        return NULL;
    }
    
    /* Extract path and expression */
    char *own_path = AR__HEAP__MALLOC(path_end - path_start + 1, "assignment path");
    if (!own_path) {
        _set_error(mut_parser, "Memory allocation failed", 0);
        return NULL;
    }
    memcpy(own_path, ref_instruction + path_start, path_end - path_start);
    own_path[path_end - path_start] = '\0';
    
    char *own_expr = AR__HEAP__MALLOC(expr_end - expr_start + 1, "assignment expression");
    if (!own_expr) {
        AR__HEAP__FREE(own_path);
        _set_error(mut_parser, "Memory allocation failed", 0);
        return NULL;
    }
    memcpy(own_expr, ref_instruction + expr_start, expr_end - expr_start);
    own_expr[expr_end - expr_start] = '\0';
    
    /* Create AST node */
    instruction_ast_t *own_ast = ar__instruction_ast__create_assignment(own_path, own_expr);
    
    AR__HEAP__FREE(own_path);
    AR__HEAP__FREE(own_expr);
    
    if (!own_ast) {
        _set_error(mut_parser, "Failed to create AST node", 0);
    }
    
    return own_ast;
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
 * Parse a send function.
 */
instruction_ast_t* ar__instruction_parser__parse_send(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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
    
    /* Check for "send" */
    if (strncmp(ref_instruction + pos, "send", 4) != 0) {
        _set_error(mut_parser, "Expected 'send' function", pos);
        return NULL;
    }
    pos += 4;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _set_error(mut_parser, "Expected '(' after 'send'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 2)) {
        _set_error(mut_parser, "Failed to parse send arguments", pos);
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
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_SEND, "send", const_args, arg_count, ref_result_path
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
 * Parse an if function.
 */
instruction_ast_t* ar__instruction_parser__parse_if(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", const_args, arg_count, ref_result_path
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
 * Parse a method function.
 */
instruction_ast_t* ar__instruction_parser__parse_method(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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
    
    /* Check for "method" */
    if (strncmp(ref_instruction + pos, "method", 6) != 0) {
        _set_error(mut_parser, "Expected 'method' function", pos);
        return NULL;
    }
    pos += 6;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _set_error(mut_parser, "Expected '(' after 'method'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 3)) {
        _set_error(mut_parser, "Failed to parse method arguments", pos);
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
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", const_args, arg_count, ref_result_path
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
 * Parse an agent function.
 */
instruction_ast_t* ar__instruction_parser__parse_agent(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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
    if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 3)) {
        _set_error(mut_parser, "Failed to parse agent arguments", pos);
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
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_AGENT, "agent", const_args, arg_count, ref_result_path
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
 * Parse a destroy function.
 */
instruction_ast_t* ar__instruction_parser__parse_destroy(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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
    
    /* Destroy can have 1 or 2 arguments - try parsing 2 first */
    char **args = NULL;
    size_t arg_count = 0;
    
    /* Save position in case we need to backtrack */
    size_t save_pos = pos;
    
    /* Try parsing 2 arguments */
    if (_parse_arguments(ref_instruction, &pos, &args, &arg_count, 2)) {
        /* Success with 2 args */
    } else {
        /* Try 1 argument */
        pos = save_pos;
        if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 1)) {
            _set_error(mut_parser, "Failed to parse destroy arguments", pos);
            return NULL;
        }
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
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", const_args, arg_count, ref_result_path
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
 * Parse a parse function.
 */
instruction_ast_t* ar__instruction_parser__parse_parse(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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
    
    /* Check for "parse" */
    if (strncmp(ref_instruction + pos, "parse", 5) != 0) {
        _set_error(mut_parser, "Expected 'parse' function", pos);
        return NULL;
    }
    pos += 5;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _set_error(mut_parser, "Expected '(' after 'parse'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 2)) {
        _set_error(mut_parser, "Failed to parse parse arguments", pos);
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
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", const_args, arg_count, ref_result_path
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
 * Parse a build function.
 */
instruction_ast_t* ar__instruction_parser__parse_build(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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
    
    /* Check for "build" */
    if (strncmp(ref_instruction + pos, "build", 5) != 0) {
        _set_error(mut_parser, "Expected 'build' function", pos);
        return NULL;
    }
    pos += 5;
    
    /* Skip whitespace */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Expect opening parenthesis */
    if (ref_instruction[pos] != '(') {
        _set_error(mut_parser, "Expected '(' after 'build'", pos);
        return NULL;
    }
    pos++;
    
    /* Parse arguments */
    char **args = NULL;
    size_t arg_count = 0;
    if (!_parse_arguments(ref_instruction, &pos, &args, &arg_count, 2)) {
        _set_error(mut_parser, "Failed to parse build arguments", pos);
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
    
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", const_args, arg_count, ref_result_path
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
