#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ar_append_instruction_parser.h"
#include "ar_expression_ast.h"
#include "ar_expression_parser.h"
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

/**
 * Internal: Extract a single argument from function call.
 * Handles nested parentheses, list/map literals, and quoted strings.
 */
static char* _extract_argument(
    ar_append_instruction_parser_t *mut_parser,
    const char *ref_str,
    size_t *mut_pos,
    char delimiter
) {
    size_t start = *mut_pos;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    bool in_quotes = false;

    while (ref_str[*mut_pos] && isspace((unsigned char)ref_str[*mut_pos])) {
        (*mut_pos)++;
        start++;
    }

    if (ref_str[*mut_pos] == delimiter) {
        _log_error(mut_parser, "Empty argument", *mut_pos);
        return NULL;
    }

    while (ref_str[*mut_pos]) {
        char c = ref_str[*mut_pos];

        if (c == '"' && (*mut_pos == 0 || ref_str[*mut_pos - 1] != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes) {
            if (c == '(') {
                paren_depth++;
            } else if (c == '[') {
                bracket_depth++;
            } else if (c == '{') {
                brace_depth++;
            } else if (c == ']') {
                if (bracket_depth > 0) {
                    bracket_depth--;
                }
            } else if (c == '}') {
                if (brace_depth > 0) {
                    brace_depth--;
                }
            } else if (c == ')') {
                if (paren_depth > 0) {
                    paren_depth--;
                } else if (delimiter == ')' && bracket_depth == 0 && brace_depth == 0) {
                    break;
                }
            } else if (
                c == delimiter &&
                paren_depth == 0 &&
                bracket_depth == 0 &&
                brace_depth == 0
            ) {
                break;
            }
        }
        (*mut_pos)++;
    }

    if (ref_str[*mut_pos] != delimiter) {
        _log_error(mut_parser, "Expected delimiter not found", *mut_pos);
        return NULL;
    }

    size_t end = *mut_pos;
    while (end > start && isspace((unsigned char)ref_str[end - 1])) {
        end--;
    }

    size_t len = end - start;
    char *own_arg = AR__HEAP__MALLOC(len + 1, "function argument");
    if (!own_arg) {
        _log_error(mut_parser, "Memory allocation failed", start);
        return NULL;
    }
    memcpy(own_arg, ref_str + start, len);
    own_arg[len] = '\0';

    return own_arg;
}

/**
 * Internal: Parse function arguments into an array.
 */
static bool _parse_arguments(
    ar_append_instruction_parser_t *mut_parser,
    const char *ref_str,
    size_t *mut_pos,
    char ***out_args,
    size_t *out_count,
    size_t expected_count
) {
    *out_args = AR__HEAP__MALLOC(expected_count * sizeof(char*), "function arguments array");
    if (!*out_args) {
        _log_error(mut_parser, "Memory allocation failed", *mut_pos);
        return false;
    }

    *out_count = 0;

    for (size_t i = 0; i < expected_count; i++) {
        char delimiter = (i < expected_count - 1) ? ',' : ')';
        char *own_arg = _extract_argument(mut_parser, ref_str, mut_pos, delimiter);
        if (!own_arg) {
            for (size_t j = 0; j < *out_count; j++) {
                AR__HEAP__FREE((*out_args)[j]);
            }
            AR__HEAP__FREE(*out_args);
            *out_args = NULL;
            return false;
        }
        (*out_args)[i] = own_arg;
        (*out_count)++;

        if (i < expected_count - 1) {
            (*mut_pos)++;
            while (ref_str[*mut_pos] && isspace((unsigned char)ref_str[*mut_pos])) {
                (*mut_pos)++;
            }
        }
    }

    return true;
}

/**
 * Internal: Cleanup argument strings array.
 */
static void _cleanup_args(char **own_args, size_t arg_count) {
    if (!own_args) {
        return;
    }

    for (size_t i = 0; i < arg_count; i++) {
        AR__HEAP__FREE(own_args[i]);
    }
    AR__HEAP__FREE(own_args);
}

/**
 * Internal: Cleanup argument AST list and all contained ASTs.
 */
static void _cleanup_arg_asts(ar_list_t *own_arg_asts) {
    if (!own_arg_asts) {
        return;
    }

    void **own_items = ar_list__items(own_arg_asts);
    if (own_items) {
        size_t list_count = ar_list__count(own_arg_asts);
        for (size_t i = 0; i < list_count; i++) {
            ar_expression_ast__destroy((ar_expression_ast_t*)own_items[i]);
        }
        AR__HEAP__FREE(own_items);
    }
    ar_list__destroy(own_arg_asts);
}

/**
 * Internal: Parse argument strings into expression ASTs and return as a list.
 */
static ar_list_t* _parse_arguments_to_asts(
    ar_append_instruction_parser_t *mut_parser,
    char **ref_args,
    size_t arg_count,
    size_t error_offset
) {
    ar_list_t *own_arg_asts = ar_list__create();
    if (!own_arg_asts) {
        _log_error(mut_parser, "Failed to create argument AST list", error_offset);
        return NULL;
    }

    for (size_t i = 0; i < arg_count; i++) {
        ar_expression_parser_t *own_expr_parser = ar_expression_parser__create(
            mut_parser->ref_log,
            ref_args[i]
        );
        if (!own_expr_parser) {
            _cleanup_arg_asts(own_arg_asts);
            _log_error(mut_parser, "Failed to create expression parser", error_offset);
            return NULL;
        }

        ar_expression_ast_t *own_expr_ast = ar_expression_parser__parse_expression(own_expr_parser);
        if (!own_expr_ast) {
            _cleanup_arg_asts(own_arg_asts);
            ar_expression_parser__destroy(own_expr_parser);
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

/**
 * Internal: Validate the append target is a non-root memory path.
 */
static bool _validate_target_ast(
    ar_append_instruction_parser_t *mut_parser,
    const ar_expression_ast_t *ref_target_ast
) {
    if (!ref_target_ast || ar_expression_ast__get_type(ref_target_ast) != AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS) {
        _log_error(mut_parser, "Append target must be a memory path", 0);
        return false;
    }

    const char *ref_base = ar_expression_ast__get_memory_base(ref_target_ast);
    if (!ref_base || strcmp(ref_base, "memory") != 0) {
        _log_error(mut_parser, "Append target must start with 'memory.'", 0);
        return false;
    }

    if (ar_expression_ast__get_memory_path_count(ref_target_ast) == 0) {
        _log_error(mut_parser, "Append target must include a memory field", 0);
        return false;
    }

    return true;
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
    if (!_parse_arguments(mut_parser, ref_instruction, &pos, &own_args, &arg_count, 2)) {
        return NULL;
    }

    pos++;

    const char **own_const_args = AR__HEAP__MALLOC(arg_count * sizeof(const char*), "const args");
    if (!own_const_args) {
        _cleanup_args(own_args, arg_count);
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
        _cleanup_args(own_args, arg_count);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }

    ar_list_t *own_arg_asts = _parse_arguments_to_asts(mut_parser, own_args, arg_count, pos);
    if (!own_arg_asts) {
        _cleanup_args(own_args, arg_count);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }

    ar_expression_ast_t *ref_target_ast = (ar_expression_ast_t*)ar_list__first(own_arg_asts);
    if (!_validate_target_ast(mut_parser, ref_target_ast)) {
        _cleanup_args(own_args, arg_count);
        _cleanup_arg_asts(own_arg_asts);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }

    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        _cleanup_args(own_args, arg_count);
        _cleanup_arg_asts(own_arg_asts);
        ar_instruction_ast__destroy(own_ast);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }

    _cleanup_args(own_args, arg_count);
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
