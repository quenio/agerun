#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ar_append_instruction_parser.h"
#include "ar_function_call_parser.h"
#include "ar_heap.h"
#include "ar_list.h"

/**
 * Opaque parser structure.
 */
struct ar_append_instruction_parser_s {
    ar_log_t *ref_log;       /* Log instance for error reporting (borrowed) */
};

/**
 * Internal: Log error message with position.
 */
static void _log_error(ar_append_instruction_parser_t *mut_parser, const char *ref_error, size_t position) {
    if (!mut_parser || !mut_parser->ref_log) {
        return;
    }

    ar_log__error_at(mut_parser->ref_log, ref_error, (int)position);
}

/**
 * Internal: Skip whitespace in string.
 */
static size_t _skip_whitespace(const char *ref_str, size_t pos) {
    while (ref_str[pos] && isspace((unsigned char)ref_str[pos])) {
        pos++;
    }
    return pos;
}

ar_append_instruction_parser_t* ar_append_instruction_parser__create(ar_log_t *ref_log) {
    ar_append_instruction_parser_t *own_parser = AR__HEAP__MALLOC(
        sizeof(ar_append_instruction_parser_t),
        "append_instruction_parser"
    );
    if (!own_parser) {
        return NULL;
    }

    own_parser->ref_log = ref_log;
    return own_parser;
}

void ar_append_instruction_parser__destroy(ar_append_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }

    AR__HEAP__FREE(own_parser);
}

ar_instruction_ast_t* ar_append_instruction_parser__parse(
    ar_append_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    if (!mut_parser) {
        return NULL;
    }

    if (!ref_instruction) {
        _log_error(mut_parser, "NULL instruction provided to append parser", 0);
        return NULL;
    }

    size_t pos = 0;
    pos = _skip_whitespace(ref_instruction, pos);

    if (ref_result_path) {
        const char *assign_pos = strstr(ref_instruction, ":=");
        if (assign_pos) {
            pos = (size_t)(assign_pos - ref_instruction) + 2;
            pos = _skip_whitespace(ref_instruction, pos);
        }
    }

    if (strncmp(ref_instruction + pos, "append", 6) != 0) {
        _log_error(mut_parser, "Expected 'append' function", pos);
        return NULL;
    }
    pos += 6;

    pos = _skip_whitespace(ref_instruction, pos);
    if (ref_instruction[pos] != '(') {
        _log_error(mut_parser, "Expected '(' after 'append'", pos);
        return NULL;
    }
    pos++;

    char **own_args = NULL;
    size_t arg_count = 0;
    if (!ar_function_call_parser__parse_exact(
        mut_parser->ref_log, ref_instruction, &pos, &own_args, &arg_count, 2
    )) {
        return NULL;
    }

    pos++;

    const char **own_const_args = AR__HEAP__MALLOC(arg_count * sizeof(const char*), "const args");
    if (!own_const_args) {
        ar_function_call_parser__destroy_args(own_args, arg_count);
        _log_error(mut_parser, "Memory allocation failed", 0);
        return NULL;
    }
    for (size_t i = 0; i < arg_count; i++) {
        own_const_args[i] = own_args[i];
    }

    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__APPEND,
        "append",
        own_const_args,
        arg_count,
        ref_result_path
    );
    AR__HEAP__FREE(own_const_args);

    if (!own_ast) {
        ar_function_call_parser__destroy_args(own_args, arg_count);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }

    ar_list_t *own_arg_asts = ar_function_call_parser__parse_arg_asts(
        mut_parser->ref_log, own_args, arg_count, pos
    );
    if (!own_arg_asts) {
        ar_function_call_parser__destroy_args(own_args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }

    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        ar_function_call_parser__destroy_args(own_args, arg_count);
        ar_function_call_parser__destroy_arg_asts(own_arg_asts);
        ar_instruction_ast__destroy(own_ast);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }

    ar_function_call_parser__destroy_args(own_args, arg_count);
    return own_ast;
}

const char* ar_append_instruction_parser__get_error(
    const ar_append_instruction_parser_t *ref_parser
) {
    (void)ref_parser;
    return NULL;
}

size_t ar_append_instruction_parser__get_error_position(
    const ar_append_instruction_parser_t *ref_parser
) {
    (void)ref_parser;
    return 0;
}
