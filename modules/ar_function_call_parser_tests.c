#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ar_function_call_parser.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_log.h"

static void test_function_call_parser__parse_exact_respects_expression_nesting(void) {
    printf("Testing function call parser respects expression nesting...\n");

    // Given an instruction call with nested expression syntax
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    const char *ref_instruction = "send(42, {items: [1, 2], text: \"a,b\"})";
    size_t pos = strlen("send(");
    char **own_args = NULL;
    size_t arg_count = 0;

    // When parsing the instruction argument boundaries
    bool parsed = ar_function_call_parser__parse_exact(
        own_log,
        ref_instruction,
        &pos,
        &own_args,
        &arg_count,
        2
    );

    // Then nested delimiters should stay inside the second argument
    assert(parsed == true);
    assert(arg_count == 2);
    assert(strcmp(own_args[0], "42") == 0);
    assert(strcmp(own_args[1], "{items: [1, 2], text: \"a,b\"}") == 0);
    assert(ref_instruction[pos] == ')');
    assert(ar_log__get_last_error_message(own_log) == NULL);

    // Cleanup
    ar_function_call_parser__destroy_args(own_args, arg_count);
    ar_log__destroy(own_log);
}

static void test_function_call_parser__parse_arg_asts_uses_expression_parser(void) {
    printf("Testing function call parser parses argument ASTs...\n");

    // Given an instruction call with expression arguments
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    const char *ref_instruction = "build(\"Items: {items}\", {items: [1, 2]})";
    size_t pos = strlen("build(");
    char **own_args = NULL;
    size_t arg_count = 0;

    // When parsing the instruction argument boundaries
    bool parsed = ar_function_call_parser__parse_exact(
        own_log,
        ref_instruction,
        &pos,
        &own_args,
        &arg_count,
        2
    );
    assert(parsed == true);

    // When converting arguments into expression ASTs
    ar_list_t *own_arg_asts = ar_function_call_parser__parse_arg_asts(
        NULL,
        own_args,
        arg_count,
        pos
    );

    // Then argument ASTs should use the expression parser
    assert(own_arg_asts != NULL);
    assert(ar_list__count(own_arg_asts) == 2);
    void **own_items = ar_list__items(own_arg_asts);
    assert(own_items != NULL);
    assert(
        ar_expression_ast__get_type((const ar_expression_ast_t*)own_items[0]) ==
        AR_EXPRESSION_AST_TYPE__LITERAL_STRING
    );
    assert(
        ar_expression_ast__get_type((const ar_expression_ast_t*)own_items[1]) ==
        AR_EXPRESSION_AST_TYPE__LITERAL_MAP
    );
    assert(ar_log__get_last_error_message(own_log) == NULL);

    // Cleanup
    AR__HEAP__FREE(own_items);
    ar_function_call_parser__destroy_arg_asts(own_arg_asts);
    ar_function_call_parser__destroy_args(own_args, arg_count);
    ar_log__destroy(own_log);
}

static void test_function_call_parser__parse_if_call_argument_ast(void) {
    printf("Testing function call parser parses if() argument AST...\n");

    // Given an instruction call with if() in an expression argument
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    const char *ref_instruction = "send(42, if(message.enabled, \"yes\", \"no\"))";
    size_t pos = strlen("send(");
    char **own_args = NULL;
    size_t arg_count = 0;

    // When parsing the instruction argument boundaries
    bool parsed = ar_function_call_parser__parse_exact(
        own_log,
        ref_instruction,
        &pos,
        &own_args,
        &arg_count,
        2
    );
    assert(parsed == true);

    // When converting arguments into expression ASTs
    ar_list_t *own_arg_asts = ar_function_call_parser__parse_arg_asts(
        own_log,
        own_args,
        arg_count,
        pos
    );

    // Then the if() argument should be a pure call expression AST
    assert(own_arg_asts != NULL);
    assert(ar_list__count(own_arg_asts) == 2);
    void **own_items = ar_list__items(own_arg_asts);
    assert(own_items != NULL);
    const ar_expression_ast_t *ref_if_ast = (const ar_expression_ast_t*)own_items[1];
    assert(ar_expression_ast__get_type(ref_if_ast) == AR_EXPRESSION_AST_TYPE__CALL);
    assert(strcmp(ar_expression_ast__get_function_name(ref_if_ast), "if") == 0);
    assert(ar_expression_ast__get_function_arg_count(ref_if_ast) == 3);
    assert(ar_log__get_last_error_message(own_log) == NULL);

    // Cleanup
    AR__HEAP__FREE(own_items);
    ar_function_call_parser__destroy_arg_asts(own_arg_asts);
    ar_function_call_parser__destroy_args(own_args, arg_count);
    ar_log__destroy(own_log);
}

static void test_function_call_parser__keeps_effectful_nested_calls_whole(void) {
    printf("Testing effectful nested calls stay whole but are not expressions...\n");

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    const char *ref_instruction = "send(0, send(1, memory.data))";
    size_t pos = strlen("send(");
    char **own_args = NULL;
    size_t arg_count = 0;

    bool parsed = ar_function_call_parser__parse_exact(
        own_log,
        ref_instruction,
        &pos,
        &own_args,
        &arg_count,
        2
    );

    assert(parsed == true);
    assert(arg_count == 2);
    assert(strcmp(own_args[1], "send(1, memory.data)") == 0);

    ar_list_t *own_arg_asts = ar_function_call_parser__parse_arg_asts(
        own_log,
        own_args,
        arg_count,
        pos
    );

    assert(own_arg_asts == NULL);

    ar_function_call_parser__destroy_args(own_args, arg_count);
    ar_log__destroy(own_log);
}

static void test_function_call_parser__rejects_trailing_argument_comma(void) {
    printf("Testing trailing argument comma is rejected...\n");

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    const char *ref_instruction = "send(42, memory.value,)";
    size_t pos = strlen("send(");
    char **own_args = NULL;
    size_t arg_count = 0;

    bool parsed = ar_function_call_parser__parse_exact(
        own_log,
        ref_instruction,
        &pos,
        &own_args,
        &arg_count,
        2
    );

    assert(parsed == false);
    assert(own_args == NULL);
    assert(arg_count == 0 || arg_count == 1);
    assert(ar_log__get_last_error_message(own_log) != NULL);

    ar_log__destroy(own_log);
}

static void test_function_call_parser__quote_after_even_backslashes_closes_string(void) {
    printf("Testing quote after even backslashes closes string...\n");

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    const char *ref_instruction = "build(\"C:\\\\\", memory.data)";
    size_t pos = strlen("build(");
    char **own_args = NULL;
    size_t arg_count = 0;

    bool parsed = ar_function_call_parser__parse_exact(
        own_log,
        ref_instruction,
        &pos,
        &own_args,
        &arg_count,
        2
    );

    assert(parsed == true);
    assert(arg_count == 2);
    assert(strcmp(own_args[0], "\"C:\\\\\"") == 0);
    assert(strcmp(own_args[1], "memory.data") == 0);
    assert(ref_instruction[pos] == ')');
    assert(ar_log__get_last_error_message(own_log) == NULL);

    ar_function_call_parser__destroy_args(own_args, arg_count);
    ar_log__destroy(own_log);
}

int main(void) {
    printf("Running function call parser tests...\n\n");

    test_function_call_parser__parse_exact_respects_expression_nesting();
    test_function_call_parser__parse_arg_asts_uses_expression_parser();
    test_function_call_parser__parse_if_call_argument_ast();
    test_function_call_parser__keeps_effectful_nested_calls_whole();
    test_function_call_parser__rejects_trailing_argument_comma();
    test_function_call_parser__quote_after_even_backslashes_closes_string();

    printf("\nAll function call parser tests passed!\n");
    return 0;
}
