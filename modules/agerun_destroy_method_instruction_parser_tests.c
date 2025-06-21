#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "agerun_destroy_method_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_heap.h"
#include "agerun_list.h"

/**
 * Test create/destroy lifecycle
 */
static void test_destroy_method_parser__create_destroy(void) {
    printf("Testing destroy method parser create/destroy...\n");
    
    // When creating a parser
    ar_destroy_method_instruction_parser_t *own_parser = ar_destroy_method_instruction_parser__create();
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_destroy_method_instruction_parser__destroy(own_parser);
    
    // Then no memory leaks should occur (verified by test runner)
}

/**
 * Test parsing destroy function with method name and version
 */
static void test_destroy_method_parser__parse_two_strings(void) {
    printf("Testing destroy method parsing with two string arguments...\n");
    
    // Given a destroy function call with method name and version
    const char *instruction = "destroy(\"calculator\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_destroy_method_instruction_parser_t *own_parser = ar_destroy_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_destroy_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a destroy method function
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_DESTROY_METHOD);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == false);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(ar__list__count(own_args) == 2);
    
    void **items = ar__list__items(own_args);
    assert(items != NULL);
    
    const char *arg1 = (const char*)items[0];
    assert(strcmp(arg1, "\"calculator\"") == 0);
    
    const char *arg2 = (const char*)items[1];
    assert(strcmp(arg2, "\"1.0.0\"") == 0);
    
    AR__HEAP__FREE(items);
    
    ar__list__destroy(own_args);
    ar__instruction_ast__destroy(own_ast);
    ar_destroy_method_instruction_parser__destroy(own_parser);
}

/**
 * Test parsing destroy function with assignment
 */
static void test_destroy_method_parser__parse_with_assignment(void) {
    printf("Testing destroy method parsing with assignment...\n");
    
    // Given a destroy function call with assignment
    const char *instruction = "memory.result := destroy(\"test_method\", \"2.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_destroy_method_instruction_parser_t *own_parser = ar_destroy_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_destroy_method_instruction_parser__parse(own_parser, instruction, "memory.result");
    
    // Then it should parse as a destroy method function with assignment
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_DESTROY_METHOD);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == true);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(ar__list__count(own_args) == 2);
    
    void **items = ar__list__items(own_args);
    assert(items != NULL);
    assert(strcmp((const char*)items[0], "\"test_method\"") == 0);
    assert(strcmp((const char*)items[1], "\"2.0.0\"") == 0);
    AR__HEAP__FREE(items);
    
    ar__list__destroy(own_args);
    ar__instruction_ast__destroy(own_ast);
    ar_destroy_method_instruction_parser__destroy(own_parser);
}

/**
 * Test error handling for various invalid inputs
 */
static void test_destroy_method_parser__error_handling(void) {
    printf("Testing destroy method parser error handling...\n");
    
    ar_destroy_method_instruction_parser_t *own_parser = ar_destroy_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    // Test 1: Missing parentheses
    instruction_ast_t *ast = ar_destroy_method_instruction_parser__parse(own_parser, "destroy \"method\", \"1.0.0\"", NULL);
    assert(ast == NULL);
    assert(ar_destroy_method_instruction_parser__get_error(own_parser) != NULL);
    assert(strstr(ar_destroy_method_instruction_parser__get_error(own_parser), "Expected '(' after 'destroy'") != NULL);
    
    // Test 2: Wrong function name
    ast = ar_destroy_method_instruction_parser__parse(own_parser, "delete(\"method\", \"1.0.0\")", NULL);
    assert(ast == NULL);
    assert(strstr(ar_destroy_method_instruction_parser__get_error(own_parser), "Expected 'destroy' function") != NULL);
    
    // Test 3: Single argument (should fail)
    ast = ar_destroy_method_instruction_parser__parse(own_parser, "destroy(\"method\")", NULL);
    assert(ast == NULL);
    assert(strstr(ar_destroy_method_instruction_parser__get_error(own_parser), "Failed to parse method name argument") != NULL);
    
    // Test 4: Non-string arguments
    ast = ar_destroy_method_instruction_parser__parse(own_parser, "destroy(method, version)", NULL);
    assert(ast != NULL);  // Parser accepts them, evaluator will validate
    ar__instruction_ast__destroy(ast);
    
    ar_destroy_method_instruction_parser__destroy(own_parser);
}

/**
 * Test parsing with complex strings (escaped quotes, etc.)
 */
static void test_destroy_method_parser__complex_strings(void) {
    printf("Testing destroy method parser with complex strings...\n");
    
    ar_destroy_method_instruction_parser_t *own_parser = ar_destroy_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    // Test with escaped quotes
    const char *instruction = "destroy(\"test\\\"method\", \"1.0.0-beta\")";
    instruction_ast_t *own_ast = ar_destroy_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_DESTROY_METHOD);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(ar__list__count(own_args) == 2);
    
    void **items = ar__list__items(own_args);
    assert(items != NULL);
    assert(strcmp((const char*)items[0], "\"test\\\"method\"") == 0);
    assert(strcmp((const char*)items[1], "\"1.0.0-beta\"") == 0);
    AR__HEAP__FREE(items);
    
    ar__list__destroy(own_args);
    ar__instruction_ast__destroy(own_ast);
    ar_destroy_method_instruction_parser__destroy(own_parser);
}

int main(void) {
    test_destroy_method_parser__create_destroy();
    test_destroy_method_parser__parse_two_strings();
    test_destroy_method_parser__parse_with_assignment();
    test_destroy_method_parser__error_handling();
    test_destroy_method_parser__complex_strings();
    
    printf("All destroy method instruction parser tests passed!\n");
    return 0;
}