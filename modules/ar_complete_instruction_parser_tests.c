#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ar_complete_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_log.h"

static void test_complete_instruction_parser__create_destroy(void) {
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);

    ar_complete_instruction_parser_t *own_parser = ar_complete_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_complete_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_complete_instruction_parser__parse_one_argument(void) {
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_complete_instruction_parser_t *own_parser = ar_complete_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_complete_instruction_parser__parse(
        own_parser,
        "complete(\"The largest country in South America is {country}.\")",
        NULL
    );

    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPLETE);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_complete_instruction_parser__parse_two_argument(void) {
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_complete_instruction_parser_t *own_parser = ar_complete_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_complete_instruction_parser__parse(
        own_parser,
        "complete(\"The capital is {city}.\", memory.location)",
        "memory.ok"
    );

    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPLETE);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.ok") == 0);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_complete_instruction_parser__accepts_map_expression_second_argument(void) {
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_complete_instruction_parser_t *own_parser = ar_complete_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_complete_instruction_parser__parse(
        own_parser,
        "complete(\"The capital is {city}.\", context.location)",
        NULL
    );

    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPLETE);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_complete_instruction_parser__accepts_template_without_placeholders(void) {
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_complete_instruction_parser_t *own_parser = ar_complete_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_complete_instruction_parser__parse(
        own_parser,
        "complete(\"No placeholders here.\")",
        "memory.result"
    );

    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPLETE);

    ar_instruction_ast__destroy(own_ast);
    ar_complete_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_complete_instruction_parser__rejects_invalid_placeholder_name(void) {
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_complete_instruction_parser_t *own_parser = ar_complete_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_complete_instruction_parser__parse(
        own_parser,
        "complete(\"The capital is {city-name}.\")",
        NULL
    );

    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(own_log) != NULL);

    ar_complete_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

int main(void) {
    printf("Running complete instruction parser tests...\n");
    test_complete_instruction_parser__create_destroy();
    test_complete_instruction_parser__parse_one_argument();
    test_complete_instruction_parser__parse_two_argument();
    test_complete_instruction_parser__accepts_map_expression_second_argument();
    test_complete_instruction_parser__accepts_template_without_placeholders();
    test_complete_instruction_parser__rejects_invalid_placeholder_name();
    printf("All complete instruction parser tests passed!\n");
    return 0;
}
