#include <stdio.h>
#include <string.h>

#include "ar_assert.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_instruction_ast.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_tail_instruction_parser.h"

static void test_tail_instruction_parser__create_destroy(void) {
    printf("Testing tail instruction parser create/destroy...\n");

    ar_tail_instruction_parser_t *own_parser = ar_tail_instruction_parser__create(NULL);

    AR_ASSERT(own_parser != NULL, "Tail parser creation should succeed");

    ar_tail_instruction_parser__destroy(own_parser);
}

static void test_tail_instruction_parser__parse_simple_tail(void) {
    printf("Testing simple tail parsing...\n");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");
    ar_tail_instruction_parser_t *own_parser = ar_tail_instruction_parser__create(own_log);
    AR_ASSERT(own_parser != NULL, "Tail parser creation should succeed");

    ar_instruction_ast_t *own_ast = ar_tail_instruction_parser__parse(
        own_parser,
        "tail(memory.targets)",
        NULL
    );

    AR_ASSERT(own_ast != NULL, "Tail parser should return an AST");
    AR_ASSERT(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__TAIL, "AST type should be TAIL");
    AR_ASSERT(strcmp(ar_instruction_ast__get_function_name(own_ast), "tail") == 0, "Function name should be tail");
    AR_ASSERT(ar_instruction_ast__has_result_assignment(own_ast) == false, "Tail should allow no assignment");

    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    AR_ASSERT(own_args != NULL, "Tail AST should expose argument strings");
    AR_ASSERT(ar_list__count(own_args) == 1, "Tail should have one argument");
    void **own_items = ar_list__items(own_args);
    AR_ASSERT(own_items != NULL, "Argument list should expose items");
    AR_ASSERT(strcmp((const char*)own_items[0], "memory.targets") == 0, "Tail argument should be preserved");
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);

    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    AR_ASSERT(ref_arg_asts != NULL, "Tail AST should include parsed argument ASTs");
    AR_ASSERT(ar_list__count(ref_arg_asts) == 1, "Tail should have one argument AST");

    own_items = ar_list__items(ref_arg_asts);
    AR_ASSERT(own_items != NULL, "Argument AST list should expose items");
    const ar_expression_ast_t *ref_list_ast = (const ar_expression_ast_t*)own_items[0];
    AR_ASSERT(
        ar_expression_ast__get_type(ref_list_ast) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS,
        "Tail argument should parse as memory access"
    );
    AR_ASSERT(strcmp(ar_expression_ast__get_memory_base(ref_list_ast), "memory") == 0, "Base should be memory");
    AR_ASSERT(ar_expression_ast__get_memory_path_count(ref_list_ast) == 1, "Path should have one component");
    AR_ASSERT(
        strcmp(ar_expression_ast__get_memory_path_component(ref_list_ast, 0), "targets") == 0,
        "Path component should be targets"
    );
    AR__HEAP__FREE(own_items);

    AR_ASSERT(ar_log__get_last_error_message(own_log) == NULL, "Successful parse should not log errors");

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_tail_instruction_parser__parse_tail_with_assignment(void) {
    printf("Testing tail with assignment parsing...\n");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");
    ar_tail_instruction_parser_t *own_parser = ar_tail_instruction_parser__create(own_log);
    AR_ASSERT(own_parser != NULL, "Tail parser creation should succeed");

    ar_instruction_ast_t *own_ast = ar_tail_instruction_parser__parse(
        own_parser,
        "memory.remaining := tail(message.targets)",
        "memory.remaining"
    );

    AR_ASSERT(own_ast != NULL, "Assigned tail parser should return an AST");
    AR_ASSERT(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__TAIL, "AST type should be TAIL");
    AR_ASSERT(ar_instruction_ast__has_result_assignment(own_ast) == true, "Tail should capture assignment");
    AR_ASSERT(
        strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.remaining") == 0,
        "Result path should be preserved"
    );
    AR_ASSERT(ar_log__get_last_error_message(own_log) == NULL, "Successful parse should not log errors");

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_tail_instruction_parser__rejects_missing_argument(void) {
    printf("Testing tail rejects missing argument...\n");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");
    ar_tail_instruction_parser_t *own_parser = ar_tail_instruction_parser__create(own_log);
    AR_ASSERT(own_parser != NULL, "Tail parser creation should succeed");

    ar_instruction_ast_t *own_ast = ar_tail_instruction_parser__parse(own_parser, "tail()", NULL);

    AR_ASSERT(own_ast == NULL, "Tail should reject a missing argument");
    AR_ASSERT(ar_log__get_last_error_message(own_log) != NULL, "Invalid tail should log an error");

    ar_tail_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_tail_instruction_parser__rejects_extra_argument(void) {
    printf("Testing tail rejects extra argument...\n");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");
    ar_tail_instruction_parser_t *own_parser = ar_tail_instruction_parser__create(own_log);
    AR_ASSERT(own_parser != NULL, "Tail parser creation should succeed");

    ar_instruction_ast_t *own_ast = ar_tail_instruction_parser__parse(
        own_parser,
        "tail(memory.targets, 1)",
        NULL
    );

    AR_ASSERT(own_ast == NULL, "Tail should reject an extra argument");
    AR_ASSERT(ar_log__get_last_error_message(own_log) != NULL, "Invalid tail should log an error");

    ar_tail_instruction_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

int main(void) {
    printf("Running tail instruction parser tests...\n\n");

    test_tail_instruction_parser__create_destroy();
    test_tail_instruction_parser__parse_simple_tail();
    test_tail_instruction_parser__parse_tail_with_assignment();
    test_tail_instruction_parser__rejects_missing_argument();
    test_tail_instruction_parser__rejects_extra_argument();

    printf("\nAll tail_instruction_parser tests passed!\n");
    return 0;
}
