#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ar_append_instruction_parser.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_instruction_ast.h"
#include "ar_list.h"
#include "ar_log.h"

static void test_append_instruction_parser__create_destroy(void) {
    printf("Testing append instruction parser create/destroy...\n");

    ar_append_instruction_parser_t *own_parser = ar_append_instruction_parser__create(NULL);

    assert(own_parser != NULL);

    ar_append_instruction_parser__destroy(own_parser);
}

static void test_append_instruction_parser__parse_simple_append(void) {
    printf("Testing simple append parsing...\n");

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_append_instruction_parser_t *own_parser = ar_append_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_append_instruction_parser__parse(
        own_parser,
        "append(memory.results, message.value)",
        NULL
    );

    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__APPEND);
    assert(strcmp(ar_instruction_ast__get_function_name(own_ast), "append") == 0);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == false);

    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "memory.results") == 0);
    assert(strcmp((const char*)own_items[1], "message.value") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);

    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 2);

    own_items = ar_list__items(ref_arg_asts);
    assert(own_items != NULL);
    const ar_expression_ast_t *ref_target_ast = (const ar_expression_ast_t*)own_items[0];
    const ar_expression_ast_t *ref_value_ast = (const ar_expression_ast_t*)own_items[1];
    assert(ar_expression_ast__get_type(ref_target_ast) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
    assert(strcmp(ar_expression_ast__get_memory_base(ref_target_ast), "memory") == 0);
    assert(ar_expression_ast__get_memory_path_count(ref_target_ast) == 1);
    assert(strcmp(ar_expression_ast__get_memory_path_component(ref_target_ast, 0), "results") == 0);
    assert(ar_expression_ast__get_type(ref_value_ast) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
    assert(strcmp(ar_expression_ast__get_memory_base(ref_value_ast), "message") == 0);
    AR__HEAP__FREE(own_items);

    assert(ar_log__get_last_error_message(own_log) == NULL);

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_append_instruction_parser__parse_append_with_assignment(void) {
    printf("Testing append with assignment parsing...\n");

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_append_instruction_parser_t *own_parser = ar_append_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_append_instruction_parser__parse(
        own_parser,
        "memory.append_ok := append(memory.results, 42)",
        "memory.append_ok"
    );

    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__APPEND);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.append_ok") == 0);
    assert(ar_log__get_last_error_message(own_log) == NULL);

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_append_instruction_parser__rejects_message_target(void) {
    printf("Testing append rejects message target...\n");

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_append_instruction_parser_t *own_parser = ar_append_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_append_instruction_parser__parse(
        own_parser,
        "append(message.results, 42)",
        NULL
    );

    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(own_log) != NULL);

    ar_append_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_append_instruction_parser__rejects_context_target(void) {
    printf("Testing append rejects context target...\n");

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_append_instruction_parser_t *own_parser = ar_append_instruction_parser__create(own_log);
    assert(own_parser != NULL);

    ar_instruction_ast_t *own_ast = ar_append_instruction_parser__parse(
        own_parser,
        "append(context.results, 42)",
        NULL
    );

    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(own_log) != NULL);

    ar_append_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

int main(void) {
    printf("Running append instruction parser tests...\n\n");

    test_append_instruction_parser__create_destroy();
    test_append_instruction_parser__parse_simple_append();
    test_append_instruction_parser__parse_append_with_assignment();
    test_append_instruction_parser__rejects_message_target();
    test_append_instruction_parser__rejects_context_target();

    printf("\nAll append_instruction_parser tests passed!\n");
    return 0;
}
