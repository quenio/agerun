#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "ar_head_instruction_parser.h"
#include "ar_function_call_parser.h"
#include "ar_heap.h"
#include "ar_list.h"

struct ar_head_instruction_parser_s {
    ar_log_t *ref_log;
};

static void _log_error(ar_head_instruction_parser_t *mut_parser, const char *ref_error, size_t position) {
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

ar_head_instruction_parser_t* ar_head_instruction_parser__create(ar_log_t *ref_log) {
    ar_head_instruction_parser_t *own_parser = AR__HEAP__MALLOC(
        sizeof(ar_head_instruction_parser_t),
        "head_instruction_parser"
    );
    if (!own_parser) {
        return NULL;
    }

    own_parser->ref_log = ref_log;
    return own_parser;
}

void ar_head_instruction_parser__destroy(ar_head_instruction_parser_t *own_parser) {
    if (!own_parser) {
        return;
    }

    AR__HEAP__FREE(own_parser);
}

ar_instruction_ast_t* ar_head_instruction_parser__parse(
    ar_head_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
) {
    if (!mut_parser) {
        return NULL;
    }

    if (!ref_instruction) {
        _log_error(mut_parser, "NULL instruction provided to head parser", 0);
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

    if (strncmp(ref_instruction + pos, "head", 4) != 0) {
        _log_error(mut_parser, "Expected 'head' function", pos);
        return NULL;
    }
    pos += 4;

    pos = _skip_whitespace(ref_instruction, pos);
    if (ref_instruction[pos] != '(') {
        _log_error(mut_parser, "Expected '(' after 'head'", pos);
        return NULL;
    }
    pos++;

    char *own_arg = ar_function_call_parser__extract_argument(
        mut_parser->ref_log, ref_instruction, &pos, ')'
    );
    if (!own_arg) {
        return NULL;
    }

    pos++;
    pos = _skip_whitespace(ref_instruction, pos);
    if (ref_instruction[pos] != '\0') {
        ar_function_call_parser__destroy_arg(own_arg);
        _log_error(mut_parser, "Unexpected characters after head call", pos);
        return NULL;
    }

    const char *const_args[] = {own_arg};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__HEAD,
        "head",
        const_args,
        1,
        ref_result_path
    );

    if (!own_ast) {
        ar_function_call_parser__destroy_arg(own_arg);
        _log_error(mut_parser, "Failed to create AST node", 0);
        return NULL;
    }

    char *args[] = {own_arg};
    ar_list_t *own_arg_asts = ar_function_call_parser__parse_arg_asts(
        mut_parser->ref_log, args, 1, pos
    );
    if (!own_arg_asts) {
        ar_function_call_parser__destroy_arg(own_arg);
        ar_instruction_ast__destroy(own_ast);
        return NULL;
    }

    if (!ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts)) {
        ar_function_call_parser__destroy_arg(own_arg);
        ar_function_call_parser__destroy_arg_asts(own_arg_asts);
        ar_instruction_ast__destroy(own_ast);
        _log_error(mut_parser, "Failed to set argument ASTs", 0);
        return NULL;
    }

    ar_function_call_parser__destroy_arg(own_arg);
    return own_ast;
}

const char* ar_head_instruction_parser__get_error(
    const ar_head_instruction_parser_t *ref_parser
) {
    (void)ref_parser;
    return NULL;
}

size_t ar_head_instruction_parser__get_error_position(
    const ar_head_instruction_parser_t *ref_parser
) {
    (void)ref_parser;
    return 0;
}
