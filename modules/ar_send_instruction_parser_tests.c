#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_send_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_event.h"

static void test_create_parser_with_log(void) {
    printf("Testing parser creation with ar_log...\n");
    
    // Given an ar_log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with ar_log
    ar_send_instruction_parser_t *parser = ar_send_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_send_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_send_instruction_parser__create_destroy(void) {
    printf("Testing send instruction parser create/destroy...\n");
    
    // Given the need for a send parser
    // When creating a parser
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(NULL);
    
    // Then it should create successfully
    assert(own_parser != NULL);
    
    // And it should destroy without issues
    ar_send_instruction_parser__destroy(own_parser);
}

static void test_send_instruction_parser__parse_simple_send(void) {
    printf("Testing simple send parsing...\n");
    
    // Given a send function call and a parser with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "send(0, \"Hello\")";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a send function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__SEND);
    assert(strcmp(ar_instruction_ast__get_function_name(own_ast), "send") == 0);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == false);
    
    // Verify arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "0") == 0);
    assert(strcmp((const char*)own_items[1], "\"Hello\"") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_send_instruction_parser__parse_send_with_assignment(void) {
    printf("Testing send with assignment parsing...\n");
    
    // Given a send with result assignment and a parser with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "send(1, \"Test\")";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing with a result path
    ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, "memory.result");
    
    // Then it should parse with result assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__SEND);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.result") == 0);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_send_instruction_parser__parse_send_with_expression_args(void) {
    printf("Testing send with expression arguments...\n");
    
    // Given a send with complex arguments and a parser with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "send(memory.agent_id, \"Count: \" + memory.count)";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse the expressions as arguments
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__SEND);
    
    // Verify arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "memory.agent_id") == 0);
    assert(strcmp((const char*)own_items[1], "\"Count: \" + memory.count") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_send_instruction_parser__parse_error_missing_args(void) {
    printf("Testing error handling - missing arguments...\n");
    
    // Given a send with missing arguments and a parser with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "send()";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    // And error information should be available
    assert(ar_log__get_last_error_message(log) != NULL);
    
    ar_send_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_send_instruction_parser__parse_error_invalid_syntax(void) {
    printf("Testing error handling - invalid syntax...\n");
    
    // Given invalid send syntax and a parser with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "send(1,)";  // Missing second argument
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    // And error information should be available
    assert(ar_log__get_last_error_message(log) != NULL);
    
    ar_send_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

// TODO: Function calls in expressions not supported yet
/*
static void test_send_instruction_parser__parse_nested_parentheses(void) {
    printf("Testing send with nested parentheses...\n");
    
    // Given a send with nested function calls in arguments and a parser
    const char *instruction = "send(0, build(\"Hello {0}\", memory.name))";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should handle nested parentheses correctly
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__SEND);
    
    // Verify arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "0") == 0);
    assert(strcmp((const char*)own_items[1], "build(\"Hello {0}\", memory.name)") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
}
*/

static void test_send_instruction_parser__reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a parser instance with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing multiple instructions with the same parser
    const char *instruction1 = "send(0, \"First\")";
    const char *instruction2 = "send(1, \"Second\")";
    
    ar_instruction_ast_t *own_ast1 = ar_send_instruction_parser__parse(own_parser, instruction1, NULL);
    ar_instruction_ast_t *own_ast2 = ar_send_instruction_parser__parse(own_parser, instruction2, NULL);
    
    // Then both should parse successfully
    assert(own_ast1 != NULL);
    assert(own_ast2 != NULL);
    
    // Verify first instruction
    ar_list_t *own_args1 = ar_instruction_ast__get_function_args(own_ast1);
    assert(own_args1 != NULL);
    void **own_items1 = ar_list__items(own_args1);
    assert(strcmp((const char*)own_items1[0], "0") == 0);
    assert(strcmp((const char*)own_items1[1], "\"First\"") == 0);
    AR__HEAP__FREE(own_items1);
    ar_list__destroy(own_args1);
    
    // Verify second instruction
    ar_list_t *own_args2 = ar_instruction_ast__get_function_args(own_ast2);
    assert(own_args2 != NULL);
    void **own_items2 = ar_list__items(own_args2);
    assert(strcmp((const char*)own_items2[0], "1") == 0);
    assert(strcmp((const char*)own_items2[1], "\"Second\"") == 0);
    AR__HEAP__FREE(own_items2);
    ar_list__destroy(own_args2);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast1);
    ar_instruction_ast__destroy(own_ast2);
    ar_send_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_send_instruction_parser__parse_with_expression_asts(void) {
    printf("Testing send parsing with expression ASTs...\n");
    
    // Given a send function call with integer and string arguments and log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "send(42, \"Hello World\")";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__SEND);
    
    // And the arguments should be available as expression ASTs
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 2);
    
    // First argument should be an integer literal AST
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    const ar_expression_ast_t *ref_first_arg = (const ar_expression_ast_t*)items[0];
    assert(ref_first_arg != NULL);
    assert(ar_expression_ast__get_type(ref_first_arg) == AR_EXPR__LITERAL_INT);
    assert(ar_expression_ast__get_int_value(ref_first_arg) == 42);
    
    // Second argument should be a string literal AST
    const ar_expression_ast_t *ref_second_arg = (const ar_expression_ast_t*)items[1];
    assert(ref_second_arg != NULL);
    assert(ar_expression_ast__get_type(ref_second_arg) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_second_arg), "Hello World") == 0);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    AR__HEAP__FREE(items);
    ar_instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

int main(void) {
    printf("Running send instruction parser tests...\n\n");
    
    // Test with ar_log
    test_create_parser_with_log();
    
    // Basic functionality
    test_send_instruction_parser__create_destroy();
    test_send_instruction_parser__parse_simple_send();
    test_send_instruction_parser__parse_send_with_assignment();
    test_send_instruction_parser__parse_send_with_expression_args();
    
    // Edge cases and error handling
    test_send_instruction_parser__parse_error_missing_args();
    test_send_instruction_parser__parse_error_invalid_syntax();
    // test_send_instruction_parser__parse_nested_parentheses(); // TODO: Function calls in expressions not supported
    test_send_instruction_parser__reusability();
    
    // Expression AST integration
    test_send_instruction_parser__parse_with_expression_asts();
    
    printf("\nAll send_instruction_parser tests passed!\n");
    return 0;
}