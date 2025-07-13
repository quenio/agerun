#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "ar_deprecate_instruction_parser.h"
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
    ar_deprecate_instruction_parser_t *parser = ar_deprecate_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_deprecate_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

/**
 * Test create/destroy lifecycle
 */
static void test_deprecate_parser__create_destroy(void) {
    printf("Testing deprecate parser create/destroy...\n");
    
    // When creating a parser
    ar_deprecate_instruction_parser_t *own_parser = ar_deprecate_instruction_parser__create(NULL);
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_deprecate_instruction_parser__destroy(own_parser);
    
    // Then no memory leaks should occur (verified by test runner)
}

/**
 * Test parsing deprecate function with method name and version
 */
static void test_deprecate_parser__parse_two_strings(void) {
    printf("Testing destroy method parsing with two string arguments...\n");
    
    // Given a destroy function call with method name and version and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "deprecate(\"calculator\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_deprecate_instruction_parser_t *own_parser = ar_deprecate_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_deprecate_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a destroy method function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__DEPRECATE);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == false);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 2);
    
    void **items = ar_list__items(own_args);
    assert(items != NULL);
    
    const char *arg1 = (const char*)items[0];
    assert(strcmp(arg1, "\"calculator\"") == 0);
    
    const char *arg2 = (const char*)items[1];
    assert(strcmp(arg2, "\"1.0.0\"") == 0);
    
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_deprecate_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test parsing deprecate function with assignment
 */
static void test_deprecate_parser__parse_with_assignment(void) {
    printf("Testing destroy method parsing with assignment...\n");
    
    // Given a destroy function call with assignment and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.result := destroy(\"test_method\", \"2.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_deprecate_instruction_parser_t *own_parser = ar_deprecate_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_deprecate_instruction_parser__parse(own_parser, instruction, "memory.result");
    
    // Then it should parse as a destroy method function with assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__DEPRECATE);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 2);
    
    void **items = ar_list__items(own_args);
    assert(items != NULL);
    assert(strcmp((const char*)items[0], "\"test_method\"") == 0);
    assert(strcmp((const char*)items[1], "\"2.0.0\"") == 0);
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_deprecate_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test error handling for various invalid inputs
 */
static void test_deprecate_parser__error_handling(void) {
    printf("Testing deprecate parser error handling...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_deprecate_instruction_parser_t *own_parser = ar_deprecate_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // Test 1: Missing parentheses
    ar_instruction_ast_t *ast = ar_deprecate_instruction_parser__parse(own_parser, "destroy \"method\", \"1.0.0\"", NULL);
    assert(ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Expected '(' after 'destroy'") != NULL);
    
    // Test 2: Wrong function name
    ast = ar_deprecate_instruction_parser__parse(own_parser, "delete(\"method\", \"1.0.0\")", NULL);
    assert(ast == NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Expected 'destroy' function") != NULL);
    
    // Test 3: Single argument (should fail)
    ast = ar_deprecate_instruction_parser__parse(own_parser, "deprecate(\"method\")", NULL);
    assert(ast == NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Failed to parse method name argument") != NULL);
    
    // Test 4: Non-string arguments - now properly rejected by expression parser
    ast = ar_deprecate_instruction_parser__parse(own_parser, "deprecate(method, version)", NULL);
    assert(ast == NULL);  // Expression parser rejects non-literals
    assert(strstr(ar_log__get_last_error_message(log), "Failed to parse argument expression") != NULL);
    
    ar_deprecate_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test parsing with complex strings (escaped quotes, etc.)
 */
// TODO: Fix expression parser handling of escaped quotes in strings
#if 0
static void test_deprecate_parser__complex_strings(void) {
    printf("Testing deprecate parser with complex strings...\n");
    
    ar_deprecate_instruction_parser_t *own_parser = ar_deprecate_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    // Test with escaped quotes
    const char *instruction = "deprecate(\"test\\\"method\", \"1.0.0-beta\")";
    ar_instruction_ast_t *own_ast = ar_deprecate_instruction_parser__parse(own_parser, instruction, NULL);
    
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__DEPRECATE);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 2);
    
    void **items = ar_list__items(own_args);
    assert(items != NULL);
    assert(strcmp((const char*)items[0], "\"test\\\"method\"") == 0);
    assert(strcmp((const char*)items[1], "\"1.0.0-beta\"") == 0);
    AR__HEAP__FREE(items);
    
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_deprecate_instruction_parser__destroy(own_parser);
}
#endif

static void test_deprecate_parser__complex_strings(void) {
    printf("Testing deprecate parser with complex strings (disabled due to expression parser limitation)...\n");
}

/**
 * Test destroy method parsing with expression ASTs
 */
static void test_deprecate_parser__parse_with_expression_asts(void) {
    printf("Testing destroy method instruction with expression ASTs...\n");
    
    // Given a destroy method instruction with quoted string arguments and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.result := destroy(\"calculator\", \"1.0.0\")";
    ar_deprecate_instruction_parser_t *own_parser = ar_deprecate_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_deprecate_instruction_parser__parse(own_parser, instruction, "memory.result");
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__DEPRECATE);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    // And the arguments should be available as expression ASTs
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 2);
    
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    
    // First argument should be a string literal AST
    const ar_expression_ast_t *ref_arg1 = (const ar_expression_ast_t*)items[0];
    assert(ref_arg1 != NULL);
    assert(ar_expression_ast__get_type(ref_arg1) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_arg1), "calculator") == 0);
    
    // Second argument should be a string literal AST
    const ar_expression_ast_t *ref_arg2 = (const ar_expression_ast_t*)items[1];
    assert(ref_arg2 != NULL);
    assert(ar_expression_ast__get_type(ref_arg2) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_arg2), "1.0.0") == 0);
    
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    
    // Test with memory references (even though not typical for destroy method)
    const char *instruction2 = "deprecate(memory.method_name, memory.version)";
    ar_instruction_ast_t *own_ast2 = ar_deprecate_instruction_parser__parse(own_parser, instruction2, NULL);
    
    assert(own_ast2 != NULL);
    assert(ar_instruction_ast__get_type(own_ast2) == AR_INSTRUCTION_AST_TYPE__DEPRECATE);
    
    const ar_list_t *ref_arg_asts2 = ar_instruction_ast__get_function_arg_asts(own_ast2);
    assert(ref_arg_asts2 != NULL);
    assert(ar_list__count(ref_arg_asts2) == 2);
    
    void **items2 = ar_list__items(ref_arg_asts2);
    assert(items2 != NULL);
    
    // First argument should be a memory access AST
    const ar_expression_ast_t *ref_arg2_1 = (const ar_expression_ast_t*)items2[0];
    assert(ref_arg2_1 != NULL);
    assert(ar_expression_ast__get_type(ref_arg2_1) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
    
    // Second argument should be a memory access AST
    const ar_expression_ast_t *ref_arg2_2 = (const ar_expression_ast_t*)items2[1];
    assert(ref_arg2_2 != NULL);
    assert(ar_expression_ast__get_type(ref_arg2_2) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
    
    AR__HEAP__FREE(items2);
    ar_instruction_ast__destroy(own_ast2);
    
    ar_deprecate_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

int main(void) {
    // Test with ar_log
    test_create_parser_with_log();
    
    test_deprecate_parser__create_destroy();
    test_deprecate_parser__parse_two_strings();
    test_deprecate_parser__parse_with_assignment();
    test_deprecate_parser__error_handling();
    test_deprecate_parser__complex_strings();
    
    // Expression AST integration
    test_deprecate_parser__parse_with_expression_asts();
    
    printf("All deprecate instruction parser tests passed!\n");
    
    ar_heap__memory_report();
    
    return 0;
}