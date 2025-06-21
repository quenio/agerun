#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "agerun_agent_instruction_parser.h"
#include "agerun_list.h"
#include "agerun_data.h"

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
    
    instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(own_parser, instruction, "memory.agent_id");
    
    // Then it should parse as an agent function
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_AGENT);
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
    
    instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as an agent function with 2 arguments
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_AGENT);
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
    instruction_ast_t *own_ast = ar_agent_instruction_parser__parse(own_parser, "agent", NULL);
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

int main(void) {
    printf("=== Agent Instruction Parser Tests ===\n");
    
    test_agent_parser__create_destroy();
    test_agent_parser__parse_with_context();
    test_agent_parser__parse_without_context();
    test_agent_parser__error_handling();
    
    printf("All agent instruction parser tests passed!\n");
    return 0;
}