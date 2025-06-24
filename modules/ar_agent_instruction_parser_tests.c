#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ar_agent_instruction_parser.h"
#include "ar_list.h"
#include "ar_data.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"

/**
 * Test create and destroy functions
 */
static void test_agent_parser__create_destroy(void) {
    printf("Testing agent parser create/destroy...\n");
    
    // Given nothing
    
    // When creating a parser
    ar_agent_instruction_parser_t *own_parser = ar_agent_instruction_parser__create();
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_agent_instruction_parser__destroy(own_parser);
    
    // Then no memory leaks should occur (verified by test runner)
}

/**
 * Test parsing agent function with 3 parameters (method, version, context)
 */
static void test_agent_parser__parse_with_context(void) {
    printf("Testing agent function parsing with context...\n");
    
    // Given an agent function call with assignment
    const char *instruction = "memory.agent_id := agent(\"echo\", \"1.0.0\", memory.context)";
    
    // When creating a parser and parsing the instruction
    ar_agent_instruction_parser_t *own_parser = ar_agent_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(own_parser, instruction, "memory.agent_id");
    
    // Then it should parse as an agent function
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__AGENT);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == true);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(ar__list__count(own_args) == 3);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_agent_instruction_parser__destroy(own_parser);
}

/**
 * Test parsing agent function with 2 parameters (method, version)
 */
static void test_agent_parser__parse_without_context(void) {
    printf("Testing agent function parsing without context...\n");
    
    // Given an agent function call without context
    const char *instruction = "agent(\"echo\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_agent_instruction_parser_t *own_parser = ar_agent_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as an agent function with 2 arguments
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__AGENT);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == false);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(ar__list__count(own_args) == 3); // Parser adds "null" context for 2-arg calls
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_agent_instruction_parser__destroy(own_parser);
}

/**
 * Test error handling for invalid inputs
 */
static void test_agent_parser__error_handling(void) {
    printf("Testing agent parser error handling...\n");
    
    ar_agent_instruction_parser_t *own_parser = ar_agent_instruction_parser__create();
    assert(own_parser != NULL);
    
    // Test missing parentheses
    ar_instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(own_parser, "agent", NULL);
    assert(own_ast == NULL);
    assert(ar_agent_instruction_parser__get_error(own_parser) != NULL);
    
    // Test wrong function name
    own_ast = ar_agent_instruction_parser__parse(own_parser, "method(\"test\", \"1.0.0\")", NULL);
    assert(own_ast == NULL);
    assert(ar_agent_instruction_parser__get_error(own_parser) != NULL);
    
    // Test no arguments
    own_ast = ar_agent_instruction_parser__parse(own_parser, "agent()", NULL);
    assert(own_ast == NULL);
    assert(ar_agent_instruction_parser__get_error(own_parser) != NULL);
    
    // Test one argument only
    own_ast = ar_agent_instruction_parser__parse(own_parser, "agent(\"echo\")", NULL);
    assert(own_ast == NULL);
    assert(ar_agent_instruction_parser__get_error(own_parser) != NULL);
    
    ar_agent_instruction_parser__destroy(own_parser);
}

/**
 * Test agent parsing with expression ASTs
 */
static void test_agent_parser__parse_with_expression_asts(void) {
    printf("Testing agent instruction with expression ASTs...\n");
    
    // Given an agent instruction with string literal method/version and memory access context
    const char *instruction = "memory.worker := agent(\"process\", \"2.1.0\", memory.config)";
    ar_agent_instruction_parser_t *own_parser = ar_agent_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(own_parser, instruction, "memory.worker");
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__AGENT);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == true);
    
    // And the arguments should be available as expression ASTs
    const list_t *ref_arg_asts = ar__instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar__list__count(ref_arg_asts) == 3);
    
    void **items = ar__list__items(ref_arg_asts);
    assert(items != NULL);
    
    // First argument - method name
    const ar_expression_ast_t *ref_method = (const ar_expression_ast_t*)items[0];
    assert(ref_method != NULL);
    assert(ar__expression_ast__get_type(ref_method) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(ref_method), "process") == 0);
    
    // Second argument - version
    const ar_expression_ast_t *ref_version = (const ar_expression_ast_t*)items[1];
    assert(ref_version != NULL);
    assert(ar__expression_ast__get_type(ref_version) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(ref_version), "2.1.0") == 0);
    
    // Third argument - context (memory access)
    const ar_expression_ast_t *ref_context = (const ar_expression_ast_t*)items[2];
    assert(ref_context != NULL);
    assert(ar__expression_ast__get_type(ref_context) == AR_EXPR__MEMORY_ACCESS);
    size_t path_count = 0;
    char **path_components = ar__expression_ast__get_memory_path(ref_context, &path_count);
    assert(path_components != NULL);
    assert(path_count == 1);
    assert(strcmp(path_components[0], "config") == 0);
    AR__HEAP__FREE(path_components);
    
    AR__HEAP__FREE(items);
    ar__instruction_ast__destroy(own_ast);
    ar_agent_instruction_parser__destroy(own_parser);
}

int main(void) {
    printf("=== Agent Instruction Parser Tests ===\n");
    
    test_agent_parser__create_destroy();
    test_agent_parser__parse_with_context();
    test_agent_parser__parse_without_context();
    test_agent_parser__error_handling();
    
    // Expression AST integration
    test_agent_parser__parse_with_expression_asts();
    
    printf("All agent instruction parser tests passed!\n");
    
    ar__heap__memory_report();
    
    return 0;
}