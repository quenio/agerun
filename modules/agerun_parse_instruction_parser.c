#include "agerun_parse_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_heap.h"
#include "agerun_string.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/**
 * Parse instruction parser structure
 */
struct ar_parse_instruction_parser_s {
    char *own_error;           /* Owned error message */
    size_t error_position;     /* Position of last error */
};

/**
 * Helper function to clear error state
 */
static void _clear_error(ar_parse_instruction_parser_t *mut_parser) {
    if (mut_parser->own_error) {
        AR__HEAP__FREE(mut_parser->own_error);
        mut_parser->own_error = NULL;
    }
    mut_parser->error_position = 0;
}

/**
 * Helper function to set error message
 */
static void _set_error(ar_parse_instruction_parser_t *mut_parser, const char *ref_message, size_t position) {
    _clear_error(mut_parser);
    mut_parser->own_error = AR__HEAP__STRDUP(ref_message, "error message");
    mut_parser->error_position = position;
}

/**
 * Skip whitespace and return new position
 */
static size_t _skip_whitespace(const char *ref_str, size_t pos) {
    while (ref_str[pos] && ar__string__isspace(ref_str[pos])) {
        pos++;
    }
    return pos;
}

/**
 * Parse a quoted string argument
 */
static char* _parse_string_argument(const char *ref_instruction, size_t *mut_pos) {
    size_t pos = *mut_pos;
    
    if (ref_instruction[pos] != '"') {
        return NULL;
    }
    
    size_t start = pos;
    pos++; /* Skip opening quote */
    
    /* Find closing quote, handling escapes */
    while (ref_instruction[pos] && ref_instruction[pos] != '"') {
        if (ref_instruction[pos] == '\\' && ref_instruction[pos + 1]) {
            pos += 2; /* Skip escape sequence */
        } else {
            pos++;
        }
    }
    
    if (ref_instruction[pos] != '"') {
        return NULL; /* Unterminated string */
    }
    
    pos++; /* Include closing quote */
    
    /* Extract the string including quotes */
    size_t len = pos - start;
    char *own_arg = AR__HEAP__MALLOC(len + 1, "string argument");
    if (!own_arg) {
        return NULL;
    }
    
    strncpy(own_arg, ref_instruction + start, len);
    own_arg[len] = '\0';
    
    *mut_pos = pos;
    return own_arg;
}

/**
 * Parse arguments for the parse function (expects exactly 2)
 */
static bool _parse_arguments(const char *ref_instruction, size_t *mut_pos, char ***mut_args, size_t *mut_arg_count, size_t expected_count) {
    size_t pos = *mut_pos;
    *mut_arg_count = 0;
    *mut_args = AR__HEAP__MALLOC(expected_count * sizeof(char*), "args array");
    if (!*mut_args) {
        return false;
    }
    
    for (size_t i = 0; i < expected_count; i++) {
        /* Skip whitespace */
        pos = _skip_whitespace(ref_instruction, pos);
        
        /* Parse string argument */
        char *own_arg = _parse_string_argument(ref_instruction, &pos);
        if (!own_arg) {
            /* Clean up on failure */
            for (size_t j = 0; j < i; j++) {
                AR__HEAP__FREE((*mut_args)[j]);
            }
            AR__HEAP__FREE(*mut_args);
            *mut_args = NULL;
            return false;
        }
        
        (*mut_args)[i] = own_arg;
        (*mut_arg_count)++;
        
        /* Skip whitespace */
        pos = _skip_whitespace(ref_instruction, pos);
        
        /* Expect comma between arguments (except after last) */
        if (i < expected_count - 1) {
            if (ref_instruction[pos] != ',') {
                /* Clean up on failure */
                for (size_t j = 0; j <= i; j++) {
                    AR__HEAP__FREE((*mut_args)[j]);
                }
                AR__HEAP__FREE(*mut_args);
                *mut_args = NULL;
                return false;
            }
            pos++; /* Skip comma */
        }
    }
    
    /* Skip whitespace after last argument */
    pos = _skip_whitespace(ref_instruction, pos);
    
    /* Check for closing parenthesis */
    if (ref_instruction[pos] != ')') {
        /* Clean up on failure */
        for (size_t j = 0; j < expected_count; j++) {
            AR__HEAP__FREE((*mut_args)[j]);
        }
        AR__HEAP__FREE(*mut_args);
        *mut_args = NULL;
        return false;
    }
    
    *mut_pos = pos;
    return true;
}

ar_parse_instruction_parser_t* ar_parse_instruction_parser__create(void) {
    ar_parse_instruction_parser_t *own_parser = AR__HEAP__CALLOC(1, sizeof(ar_parse_instruction_parser_t), "parse instruction parser");
    if (!own_parser) {
        return NULL;
    }
    
    return own_parser;
}

void ar_parse_instruction_parser__destroy(ar_parse_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }
    
    _clear_error(own_parser);
    AR__HEAP__FREE(own_parser);
}

instruction_ast_t* ar_parse_instruction_parser__parse(ar_parse_instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path) {
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

const char* ar_parse_instruction_parser__get_error(const ar_parse_instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return NULL;
    }
    return ref_parser->own_error;
}

size_t ar_parse_instruction_parser__get_error_position(const ar_parse_instruction_parser_t *ref_parser) {
    if (!ref_parser) {
        return 0;
    }
    return ref_parser->error_position;
}