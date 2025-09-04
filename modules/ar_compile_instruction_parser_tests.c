#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_compile_instruction_parser.h"
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
    ar_compile_instruction_parser_t *parser = ar_compile_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_compile_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

// TODO: Fix expression parser handling of quoted strings
#if 0
/**
 * Test simple compile function parsing (adapted from instruction_parser_tests.c line 130-148)
 */
static void test_compile_instruction_parser__simple_parsing(void) {
    printf("Testing simple compile function parsing...\n");
    
    // Given a compile function call
    const char *instruction = "compile(\"greet\", \"memory.msg := \\\"Hello\\\"\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a compile function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPILE);
    
    ar_instruction_ast__destroy(own_ast);
    ar_compile_instruction_parser__destroy(own_parser);
}
#endif

/**
 * Test method parsing with assignment
 */
static void test_compile_instruction_parser__with_assignment(void) {
    printf("Testing compile parsing with assignment...\n");
    
    // Given a compile call with assignment
    const char *instruction = "memory.method_ref := compile(\"calculate\", \"memory.result := 42\", \"2.0.0\")";
    
    // When parsing with result path
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, "memory.method_ref");
    
    // Then it should parse correctly with assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPILE);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    const char *result_path = ar_instruction_ast__get_function_result_path(own_ast);
    assert(result_path != NULL);
    assert(strcmp(result_path, "memory.method_ref") == 0);
    
    // And should have 3 arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 3);
    
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"calculate\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"memory.result := 42\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"2.0.0\"") == 0);
    
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_compile_instruction_parser__destroy(own_parser);
}

// TODO: Fix expression parser handling of quoted strings with nested quotes
#if 0
/**
 * Test method parsing with complex code containing nested quotes
 */
static void test_compile_instruction_parser__complex_code(void) {
    printf("Testing compile parsing with complex code...\n");
    
    // Given a method with complex code
    const char *instruction = "compile(\"process\", \"memory.output := build(\\\"Result: {value}\\\", memory.data)\", \"1.0.0\")";
    
    // When parsing
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse correctly
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPILE);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 3);
    
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"process\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"memory.output := build(\\\"Result: {value}\\\", memory.data)\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"1.0.0\"") == 0);
    
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_compile_instruction_parser__destroy(own_parser);
}
#endif

/**
 * Test method parsing with whitespace variations
 */
static void test_compile_instruction_parser__whitespace_handling(void) {
    printf("Testing compile parsing with whitespace...\n");
    
    // Given a method call with extra whitespace and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "  compile  ( \"test\" , \"memory.x := 1\" , \"1.0.0\" )  ";
    
    // When parsing
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse correctly ignoring whitespace
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPILE);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test error when wrong function name
 */
static void test_compile_instruction_parser__wrong_function_name(void) {
    printf("Testing compile parser with wrong function name...\n");
    
    // Given a non-compile function call and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "build(\"template\", memory.data)";
    
    // When parsing
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "compile") != NULL);
    
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test error when wrong number of arguments
 */
static void test_compile_instruction_parser__wrong_arg_count(void) {
    printf("Testing compile parser with wrong argument count...\n");
    
    // Given a method call with only 2 arguments and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "compile(\"test\", \"code\")";
    
    // When parsing
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "argument") != NULL);
    
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test error with malformed syntax
 */
static void test_compile_instruction_parser__malformed_syntax(void) {
    printf("Testing compile parser with malformed syntax...\n");
    
    // Given a method call missing opening parenthesis and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "compile\"test\", \"code\", \"1.0.0\")";
    
    // When parsing
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "(") != NULL);
    
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test parser reusability
 */
static void test_compile_instruction_parser__reusability(void) {
    printf("Testing compile parser reusability...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // First parse - should succeed
    const char *instruction1 = "compile(\"test1\", \"code1\", \"1.0.0\")";
    ar_instruction_ast_t *own_ast1 = ar_compile_instruction_parser__parse(own_parser, instruction1, NULL);
    assert(own_ast1 != NULL);
    assert(ar_log__get_last_error_message(log) == NULL);
    
    // Second parse - should fail
    const char *instruction2 = "notcompile(\"test\", \"code\", \"1.0.0\")";
    ar_instruction_ast_t *own_ast2 = ar_compile_instruction_parser__parse(own_parser, instruction2, NULL);
    assert(own_ast2 == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    
    // NOTE: With shared log, errors persist across parse attempts
    // The third parse will succeed but the error from parse 2 remains in the log
    const char *instruction3 = "compile(\"test3\", \"code3\", \"1.0.0\")";
    ar_instruction_ast_t *own_ast3 = ar_compile_instruction_parser__parse(own_parser, instruction3, NULL);
    assert(own_ast3 != NULL);
    // Error from previous parse still exists in log
    
    ar_instruction_ast__destroy(own_ast1);
    ar_instruction_ast__destroy(own_ast3);
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test with multiline code
 */
static void test_compile_instruction_parser__multiline_code(void) {
    printf("Testing compile parser with multiline code...\n");
    
    // Given a method with code containing newlines and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "compile(\"multi\", \"memory.x := 1\\nmemory.y := 2\", \"1.0.0\")";
    
    // When parsing
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse correctly
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPILE);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[1], "\"memory.x := 1\\nmemory.y := 2\"") == 0);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    ar_instruction_ast__destroy(own_ast);
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test error when NULL instruction is passed
 */
static void test_compile_instruction_parser__null_instruction(void) {
    printf("Testing compile parser with NULL instruction...\n");
    
    // Given a parser with a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing a NULL instruction
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, NULL, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    // And an error should be logged
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "NULL instruction") != NULL);
    
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

/**
 * Test safe handling when NULL parser is passed
 */
static void test_compile_instruction_parser__null_parser(void) {
    printf("Testing compile parser with NULL parser...\n");
    
    // Given a valid instruction
    const char *instruction = "compile(\"test\", \"code\", \"1.0.0\")";
    
    // When parsing with a NULL parser
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(NULL, instruction, NULL);
    
    // Then it should fail safely
    assert(own_ast == NULL);
    
    // Note: Cannot verify error logging since we don't have a log instance
}

/**
 * Test method parsing with expression ASTs
 */
static void test_compile_instruction_parser__parse_with_expression_asts(void) {
    printf("Testing compile instruction with expression ASTs...\n");
    
    // Given a method instruction with string literal arguments and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "compile(\"calculate\", \"memory.result := memory.x + memory.y\", \"1.2.3\")";
    ar_compile_instruction_parser_t *own_parser = ar_compile_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_compile_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__COMPILE);
    
    // And the arguments should be available as expression ASTs
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 3);
    
    // All three arguments should be string literal ASTs
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    
    // First argument - method name
    const ar_expression_ast_t *ref_name = (const ar_expression_ast_t*)items[0];
    assert(ref_name != NULL);
    assert(ar_expression_ast__get_type(ref_name) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_name), "calculate") == 0);
    
    // Second argument - method code
    const ar_expression_ast_t *ref_code = (const ar_expression_ast_t*)items[1];
    assert(ref_code != NULL);
    assert(ar_expression_ast__get_type(ref_code) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_code), "memory.result := memory.x + memory.y") == 0);
    
    // Third argument - version
    const ar_expression_ast_t *ref_version = (const ar_expression_ast_t*)items[2];
    assert(ref_version != NULL);
    assert(ar_expression_ast__get_type(ref_version) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_version), "1.2.3") == 0);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    AR__HEAP__FREE(items);
    ar_instruction_ast__destroy(own_ast);
    ar_compile_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

int main(void) {
    printf("\n=== Running Method Instruction Parser Tests ===\n\n");
    
    // Test with ar_log
    test_create_parser_with_log();
    
    // test_compile_instruction_parser__simple_parsing(); // TODO: Fix expression parser handling of quoted strings
    test_compile_instruction_parser__with_assignment();
    // test_compile_instruction_parser__complex_code(); // TODO: Fix expression parser handling of quoted strings with nested quotes
    test_compile_instruction_parser__whitespace_handling();
    test_compile_instruction_parser__wrong_function_name();
    test_compile_instruction_parser__wrong_arg_count();
    test_compile_instruction_parser__malformed_syntax();
    test_compile_instruction_parser__reusability();
    test_compile_instruction_parser__multiline_code();
    test_compile_instruction_parser__null_instruction();
    test_compile_instruction_parser__null_parser();
    
    // Expression AST integration
    test_compile_instruction_parser__parse_with_expression_asts();
    
    printf("\nAll compile instruction parser tests passed!\n");
    
    ar_heap__memory_report();
    
    return 0;
}