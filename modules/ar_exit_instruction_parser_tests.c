#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "ar_exit_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_expression_ast.h"
#include "ar_log.h"

static void test_create_parser_with_log(void) {
    printf("Testing parser creation with ar_log...\n");
    
    // Given an ar_log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with ar_log
    ar_exit_instruction_parser_t *parser = ar_exit_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_exit_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

/**
 * Test create/destroy lifecycle
 */
static void test_exit_agent_parser__create_destroy(void) {
    printf("Testing exit agent parser create/destroy...\n");
    
    // When creating a parser
    ar_exit_instruction_parser_t *own_parser = ar_exit_instruction_parser__create(NULL);
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_exit_instruction_parser__destroy(own_parser);
    
    // Then no memory leaks should occur (verified by test runner)
}

/**
 * Test parsing destroy function with integer agent ID
 */
static void test_exit_agent_parser__parse_integer_id(void) {
    printf("Testing exit agent parsing with integer ID...\n");
    
    // Given a destroy function call with integer agent ID and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "exit(123)";
    
    // When creating a parser and parsing the instruction
    ar_exit_instruction_parser_t *own_parser = ar_exit_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_exit_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a exit agent function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__EXIT);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == false);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 1);
    
    void **items = ar_list__items(own_args);
    assert(items != NULL);
    const char *arg = (const char*)items[0];
    assert(strcmp(arg, "123") == 0);
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_exit_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test parsing destroy function with memory reference
 */
static void test_exit_agent_parser__parse_memory_reference(void) {
    printf("Testing exit agent parsing with memory reference...\n");
    
    // Given a destroy function call with memory reference and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "exit(memory.agent_id)";
    
    // When creating a parser and parsing the instruction
    ar_exit_instruction_parser_t *own_parser = ar_exit_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_exit_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a exit agent function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__EXIT);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 1);
    
    void **items = ar_list__items(own_args);
    assert(items != NULL);
    const char *arg = (const char*)items[0];
    assert(strcmp(arg, "memory.agent_id") == 0);
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_exit_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test parsing destroy function with assignment
 */
static void test_exit_agent_parser__parse_with_assignment(void) {
    printf("Testing exit agent parsing with assignment...\n");
    
    // Given a destroy function call with assignment and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.result := destroy(memory.agent_id)";
    
    // When creating a parser and parsing the instruction
    ar_exit_instruction_parser_t *own_parser = ar_exit_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_exit_instruction_parser__parse(own_parser, instruction, "memory.result");
    
    // Then it should parse as a exit agent function with assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__EXIT);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 1);
    
    void **items = ar_list__items(own_args);
    assert(items != NULL);
    const char *arg = (const char*)items[0];
    assert(strcmp(arg, "memory.agent_id") == 0);
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_exit_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test error handling for various invalid inputs
 */
static void test_exit_agent_parser__error_handling(void) {
    printf("Testing exit agent parser error handling...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_exit_instruction_parser_t *own_parser = ar_exit_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // Test 1: Missing parentheses
    ar_instruction_ast_t *ast = ar_exit_instruction_parser__parse(own_parser, "destroy 123", NULL);
    assert(ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Expected '(' after 'exit'") != NULL);
    
    // Test 2: Wrong function name
    ast = ar_exit_instruction_parser__parse(own_parser, "delete(123)", NULL);
    assert(ast == NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Expected 'exit' function") != NULL);
    
    // Test 3: Empty arguments
    ast = ar_exit_instruction_parser__parse(own_parser, "exit()", NULL);
    assert(ast == NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Failed to parse destroy argument") != NULL);
    
    // Test 4: Multiple arguments - should fail because exit() only accepts one argument
    ast = ar_exit_instruction_parser__parse(own_parser, "exit(123, 456)", NULL);
    assert(ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "exit() expects exactly one argument") != NULL);
    
    // Test 5: Multiple arguments with spaces
    ast = ar_exit_instruction_parser__parse(own_parser, "exit(  123  ,  456  )", NULL);
    assert(ast == NULL);
    assert(strstr(ar_log__get_last_error_message(log), "exit() expects exactly one argument") != NULL);
    
    ar_exit_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test exit agent parsing with expression ASTs
 */
static void test_exit_agent_parser__parse_with_expression_asts(void) {
    printf("Testing exit agent instruction with expression ASTs...\n");
    
    // Given a destroy instruction with integer literal and memory access arguments, and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.destroyed := destroy(42)";
    ar_exit_instruction_parser_t *own_parser = ar_exit_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_exit_instruction_parser__parse(own_parser, instruction, "memory.destroyed");
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__EXIT);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    // And the argument should be available as an expression AST
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 1);
    
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    
    // The argument should be an integer literal AST
    const ar_expression_ast_t *ref_arg = (const ar_expression_ast_t*)items[0];
    assert(ref_arg != NULL);
    assert(ar_expression_ast__get_type(ref_arg) == AR_EXPRESSION_AST_TYPE__LITERAL_INT);
    assert(ar_expression_ast__get_int_value(ref_arg) == 42);
    
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    
    // Test with memory reference
    const char *instruction2 = "exit(memory.agent_id)";
    ar_instruction_ast_t *own_ast2 = ar_exit_instruction_parser__parse(own_parser, instruction2, NULL);
    
    assert(own_ast2 != NULL);
    assert(ar_instruction_ast__get_type(own_ast2) == AR_INSTRUCTION_AST_TYPE__EXIT);
    
    const ar_list_t *ref_arg_asts2 = ar_instruction_ast__get_function_arg_asts(own_ast2);
    assert(ref_arg_asts2 != NULL);
    assert(ar_list__count(ref_arg_asts2) == 1);
    
    void **items2 = ar_list__items(ref_arg_asts2);
    assert(items2 != NULL);
    
    const ar_expression_ast_t *ref_arg2 = (const ar_expression_ast_t*)items2[0];
    assert(ref_arg2 != NULL);
    assert(ar_expression_ast__get_type(ref_arg2) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
    
    size_t path_count = 0;
    char **path_components = ar_expression_ast__get_memory_path(ref_arg2, &path_count);
    assert(path_components != NULL);
    assert(path_count == 1);
    assert(strcmp(path_components[0], "agent_id") == 0);
    AR__HEAP__FREE(path_components);
    
    AR__HEAP__FREE(items2);
    ar_instruction_ast__destroy(own_ast2);
    
    ar_exit_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

int main(void) {
    // Test with ar_log
    test_create_parser_with_log();
    
    test_exit_agent_parser__create_destroy();
    test_exit_agent_parser__parse_integer_id();
    test_exit_agent_parser__parse_memory_reference();
    test_exit_agent_parser__parse_with_assignment();
    test_exit_agent_parser__error_handling();
    
    // Expression AST integration
    test_exit_agent_parser__parse_with_expression_asts();
    
    printf("All exit agent instruction parser tests passed!\n");
    
    // ar_heap__memory_report(); // Memory report not needed in exit parser tests
    
    return 0;
}