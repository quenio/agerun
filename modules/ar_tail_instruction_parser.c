#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "ar_tail_instruction_parser.h"
#include "ar_expression_ast.h"
#include "ar_expression_parser.h"
#include "ar_heap.h"
#include "ar_list.h"

struct ar_tail_instruction_parser_s {
    ar_log_t *ref_log;
};

static void _log_error(ar_tail_instruction_parser_t *mut_parser, const char *ref_error, size_t position) {
    if (!mut_parser || !mut_parser->ref_log) {
        return;
    }

    ar_log__error_at(mut_parser->ref_log, ref_error, (int)position);
}

static size_t _skip_whitespace(const char *ref_str, size_t pos) {
    while (ref_str[pos] && isspace((unsigned char)ref_str[pos])) {
        pos++;
    }
    return pos;
}

static char* _extract_argument(
    ar_tail_instruction_parser_t *mut_parser,
    const char *ref_str,
    size_t *mut_pos
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

    if (ref_str[*mut_pos] == ')') {
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
                } else if (bracket_depth == 0 && brace_depth == 0) {
                    break;
                }
            }
        }
        (*mut_pos)++;
    }

    if (ref_str[*mut_pos] != ')') {
        _log_error(mut_parser, "Expected ')' after argument", *mut_pos);
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

static ar_list_t* _parse_argument_to_ast(
    ar_tail_instruction_parser_t *mut_parser,
    const char *ref_arg,
    size_t error_offset
) {
    ar_list_t *own_arg_asts = ar_list__create();
    if (!own_arg_asts) {
        _log_error(mut_parser, "Failed to create argument AST list", error_offset);
        return NULL;
    }

    ar_expression_parser_t *own_expr_parser = ar_expression_parser__create(mut_parser->ref_log, ref_arg);
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
    return own_arg_asts;
}

ar_tail_instruction_parser_t* ar_tail_instruction_parser__create(ar_log_t *ref_log) {
    ar_tail_instruction_parser_t *own_parser = AR__HEAP__MALLOC(
        sizeof(ar_tail_instruction_parser_t),
        "tail_instruction_parser"
    );
    if (!own_parser) {
        return NULL;
    }

    own_parser->ref_log = ref_log;
    return own_parser;
}

void ar_tail_instruction_parser__destroy(ar_tail_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }

    AR__HEAP__FREE(own_parser);
}

ar_instruction_ast_t* ar_tail_instruction_parser__parse(
    ar_tail_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    if (!mut_parser) {
        return NULL;
    }

    if (!ref_instruction) {
        _log_error(mut_parser, "NULL instruction provided to tail parser", 0);
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

    if (strncmp(ref_instruction + pos, "tail", 4) != 0) {
        _log_error(mut_parser, "Expected 'tail' function", pos);
        return NULL;
    }
    pos += 4;

    pos = _skip_whitespace(ref_instruction, pos);
    if (ref_instruction[pos] != '(') {
        _log_error(mut_parser, "Expected '(' after 'tail'", pos);
        return NULL;
    }
    pos++;

    char *own_arg = _extract_argument(mut_parser, ref_instruction, &pos);
    if (!own_arg) {
        return NULL;
    }

    pos++;
    pos = _skip_whitespace(ref_instruction, pos);
    if (ref_instruction[pos] != '\0') {
        AR__HEAP__FREE(own_arg);
        _log_error(mut_parser, "Unexpected characters after tail call", pos);
        return NULL;
    }

    const char *args[] = {own_arg};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__TAIL,
        "tail",
        args,
        1,
        ref_result_path
    );

    if (!own_ast) {
        AR__HEAP__FREE(own_arg);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }

    ar_list_t *own_arg_asts = _parse_argument_to_ast(mut_parser, own_arg, pos);
    if (!own_arg_asts) {
        AR__HEAP__FREE(own_arg);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }

    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        AR__HEAP__FREE(own_arg);
        _cleanup_arg_asts(own_arg_asts);
        ar_instruction_ast__destroy(own_ast);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }

    AR__HEAP__FREE(own_arg);
    return own_ast;
}

const char* ar_tail_instruction_parser__get_error(
    const ar_tail_instruction_parser_t *ref_parser
) {
    (void)ref_parser;
    return NULL;
}

size_t ar_tail_instruction_parser__get_error_position(
    const ar_tail_instruction_parser_t *ref_parser
) {
    (void)ref_parser;
    return 0;
}
