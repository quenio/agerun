#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "agerun_assignment_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_heap.h"

/**
 * Opaque parser structure for assignment instructions.
 */
struct ar_assignment_instruction_parser_s {
    char *own_error;         /* Error message if parsing fails */
    size_t error_position;   /* Position where error occurred */
};

/**
 * Internal: Set error message and position.
 */
static void _set_error(ar_assignment_instruction_parser_t *mut_parser, const char *error, size_t position) {
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
static void _clear_error(ar_assignment_instruction_parser_t *mut_parser) {
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
 * Create a new assignment instruction parser instance.
 */
ar_assignment_instruction_parser_t* ar_assignment_instruction_parser__create(void) {
    ar_assignment_instruction_parser_t *own_parser = AR__HEAP__MALLOC(
        sizeof(ar_assignment_instruction_parser_t), 
        "assignment_instruction_parser"
    );
    if (!own_parser) {
        return NULL;
    }
    
    own_parser->own_error = NULL;
    own_parser->error_position = 0;
    
    return own_parser;
}

/**
 * Destroy an assignment instruction parser instance.
 */
void ar_assignment_instruction_parser__destroy(ar_assignment_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    AR__HEAP__FREE(own_parser->own_error);
    AR__HEAP__FREE(own_parser);
}

/**
 * Parse an assignment instruction.
 */
instruction_ast_t* ar_assignment_instruction_parser__parse(
    ar_assignment_instruction_parser_t *mut_parser,
    const char *ref_instruction
) {
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
 * Get the last error message from the parser.
 */
const char* ar_assignment_instruction_parser__get_error(
    const ar_assignment_instruction_parser_t *ref_parser
) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error;
}

/**
 * Get the error position from the last parse attempt.
 */
size_t ar_assignment_instruction_parser__get_error_position(
    const ar_assignment_instruction_parser_t *ref_parser
) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->error_position;
}