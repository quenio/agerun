#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ar_spawn_instruction_parser.h"
#include "ar_list.h"
#include "ar_data.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_log.h"

static void test_spawn_parser_with_log(void) {
    printf("Testing parser creation with ar_log...\n");
    
    // Given an ar_log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with ar_log
    ar_spawn_instruction_parser_t *parser = ar_spawn_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_spawn_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

/**
 * Test create and destroy functions
 */
static void test_spawn_parser__create_destroy(void) {
    printf("Testing create parser create/destroy...\n");
    
    // Given nothing
    
    // When creating a parser
    ar_spawn_instruction_parser_t *own_parser = ar_spawn_instruction_parser__create(NULL);
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_spawn_instruction_parser__destroy(own_parser);
    
    // Then no memory leaks should occur (verified by test runner)
}

/**
 * Test parsing create function with 3 parameters (method, version, context)
 */
static void test_spawn_parser__parse_with_context(void) {
    printf("Testing create function parsing with context...\n");
    
    // Given a create function call with assignment and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.agent_id := spawn(\"echo\", \"1.0.0\", memory.context)";
    
    // When creating a parser and parsing the instruction
    ar_spawn_instruction_parser_t *own_parser = ar_spawn_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_spawn_instruction_parser__parse(own_parser, instruction, "memory.agent_id");
    
    // Then it should parse as a create function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__SPAWN);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 3);
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_spawn_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test parsing create function with 2 parameters (method, version)
 */
static void test_spawn_parser__parse_without_context(void) {
    printf("Testing create function parsing without context...\n");
    
    // Given a create function call without context and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "spawn(\"echo\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_spawn_instruction_parser_t *own_parser = ar_spawn_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_spawn_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a create function with 2 arguments
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__SPAWN);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == false);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 3); // Parser adds "null" context for 2-arg calls
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_spawn_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test error handling for invalid inputs
 */
static void test_spawn_parser__error_handling(void) {
    printf("Testing create parser error handling...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_spawn_instruction_parser_t *own_parser = ar_spawn_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // Test missing parentheses
    ar_instruction_ast_t *own_ast = ar_spawn_instruction_parser__parse(own_parser, "spawn", NULL);
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    
    // Test wrong function name
    own_ast = ar_spawn_instruction_parser__parse(own_parser, "compile(\"test\", \"1.0.0\")", NULL);
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    
    // Test no arguments
    own_ast = ar_spawn_instruction_parser__parse(own_parser, "spawn()", NULL);
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    
    // Test one argument only
    own_ast = ar_spawn_instruction_parser__parse(own_parser, "spawn(\"echo\")", NULL);
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    
    ar_spawn_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test parsing with NULL instruction
 */
static void test_spawn_parser__null_instruction(void) {
    printf("Testing spawn parser with NULL instruction...\n");
    
    // Given a parser with a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_spawn_instruction_parser_t *own_parser = ar_spawn_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing a NULL instruction
    ar_instruction_ast_t *own_ast = ar_spawn_instruction_parser__parse(own_parser, NULL, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    // And an error should be logged
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "NULL instruction") != NULL);
    
    ar_spawn_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test create parsing with expression ASTs
 */
static void test_spawn_parser__parse_with_expression_asts(void) {
    printf("Testing create instruction with expression ASTs...\n");
    
    // Given a create instruction with string literal method/version and memory access context, and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.worker := spawn(\"process\", \"2.1.0\", memory.config)";
    ar_spawn_instruction_parser_t *own_parser = ar_spawn_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_spawn_instruction_parser__parse(own_parser, instruction, "memory.worker");
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__SPAWN);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    // And the arguments should be available as expression ASTs
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 3);
    
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    
    // First argument - method name
    const ar_expression_ast_t *ref_method = (const ar_expression_ast_t*)items[0];
    assert(ref_method != NULL);
    assert(ar_expression_ast__get_type(ref_method) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_method), "process") == 0);
    
    // Second argument - version
    const ar_expression_ast_t *ref_version = (const ar_expression_ast_t*)items[1];
    assert(ref_version != NULL);
    assert(ar_expression_ast__get_type(ref_version) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_version), "2.1.0") == 0);
    
    // Third argument - context (memory access)
    const ar_expression_ast_t *ref_context = (const ar_expression_ast_t*)items[2];
    assert(ref_context != NULL);
    assert(ar_expression_ast__get_type(ref_context) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
    size_t path_count = 0;
    char **path_components = ar_expression_ast__get_memory_path(ref_context, &path_count);
    assert(path_components != NULL);
    assert(path_count == 1);
    assert(strcmp(path_components[0], "config") == 0);
    AR__HEAP__FREE(path_components);
    
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_spawn_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

int main(void) {
    printf("=== Create Instruction Parser Tests ===\n");
    
    // Test with ar_log
    test_spawn_parser_with_log();
    
    test_spawn_parser__create_destroy();
    test_spawn_parser__parse_with_context();
    test_spawn_parser__parse_without_context();
    test_spawn_parser__error_handling();
    test_spawn_parser__null_instruction();
    
    // Expression AST integration
    test_spawn_parser__parse_with_expression_asts();
    
    printf("All create instruction parser tests passed!\n");
    
    ar_heap__memory_report();
    
    return 0;
}