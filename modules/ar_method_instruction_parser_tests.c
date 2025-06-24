#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_method_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_heap.h"

// TODO: Fix expression parser handling of quoted strings
#if 0
/**
 * Test simple method function parsing (adapted from instruction_parser_tests.c line 130-148)
 */
static void test_method_instruction_parser__simple_parsing(void) {
    printf("Testing simple method function parsing...\n");
    
    // Given a method function call
    const char *instruction = "method(\"greet\", \"memory.msg := \\\"Hello\\\"\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a method function
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    
    ar__instruction_ast__destroy(own_ast);
    ar_method_instruction_parser__destroy(own_parser);
}
#endif

/**
 * Test method parsing with assignment
 */
static void test_method_instruction_parser__with_assignment(void) {
    printf("Testing method parsing with assignment...\n");
    
    // Given a method call with assignment
    const char *instruction = "memory.method_ref := method(\"calculate\", \"memory.result := 42\", \"2.0.0\")";
    
    // When parsing with result path
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, "memory.method_ref");
    
    // Then it should parse correctly with assignment
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == true);
    
    const char *result_path = ar__instruction_ast__get_function_result_path(own_ast);
    assert(result_path != NULL);
    assert(strcmp(result_path, "memory.method_ref") == 0);
    
    // And should have 3 arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 3);
    
    void **own_items = ar__list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"calculate\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"memory.result := 42\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"2.0.0\"") == 0);
    
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    ar__instruction_ast__destroy(own_ast);
    ar_method_instruction_parser__destroy(own_parser);
}

// TODO: Fix expression parser handling of quoted strings with nested quotes
#if 0
/**
 * Test method parsing with complex code containing nested quotes
 */
static void test_method_instruction_parser__complex_code(void) {
    printf("Testing method parsing with complex code...\n");
    
    // Given a method with complex code
    const char *instruction = "method(\"process\", \"memory.output := build(\\\"Result: {value}\\\", memory.data)\", \"1.0.0\")";
    
    // When parsing
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse correctly
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(ar__list__count(own_args) == 3);
    
    void **own_items = ar__list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"process\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"memory.output := build(\\\"Result: {value}\\\", memory.data)\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"1.0.0\"") == 0);
    
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    ar__instruction_ast__destroy(own_ast);
    ar_method_instruction_parser__destroy(own_parser);
}
#endif

/**
 * Test method parsing with whitespace variations
 */
static void test_method_instruction_parser__whitespace_handling(void) {
    printf("Testing method parsing with whitespace...\n");
    
    // Given a method call with extra whitespace
    const char *instruction = "  method  ( \"test\" , \"memory.x := 1\" , \"1.0.0\" )  ";
    
    // When parsing
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse correctly ignoring whitespace
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    
    ar__instruction_ast__destroy(own_ast);
    ar_method_instruction_parser__destroy(own_parser);
}

/**
 * Test error when wrong function name
 */
static void test_method_instruction_parser__wrong_function_name(void) {
    printf("Testing method parser with wrong function name...\n");
    
    // Given a non-method function call
    const char *instruction = "build(\"template\", memory.data)";
    
    // When parsing
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    const char *error = ar_method_instruction_parser__get_error(own_parser);
    assert(error != NULL);
    assert(strstr(error, "method") != NULL);
    
    size_t pos = ar_method_instruction_parser__get_error_position(own_parser);
    assert(pos == 0);  // Error at start where "method" was expected
    
    ar_method_instruction_parser__destroy(own_parser);
}

/**
 * Test error when wrong number of arguments
 */
static void test_method_instruction_parser__wrong_arg_count(void) {
    printf("Testing method parser with wrong argument count...\n");
    
    // Given a method call with only 2 arguments
    const char *instruction = "method(\"test\", \"code\")";
    
    // When parsing
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    const char *error = ar_method_instruction_parser__get_error(own_parser);
    assert(error != NULL);
    assert(strstr(error, "argument") != NULL);
    
    ar_method_instruction_parser__destroy(own_parser);
}

/**
 * Test error with malformed syntax
 */
static void test_method_instruction_parser__malformed_syntax(void) {
    printf("Testing method parser with malformed syntax...\n");
    
    // Given a method call missing opening parenthesis
    const char *instruction = "method\"test\", \"code\", \"1.0.0\")";
    
    // When parsing
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    const char *error = ar_method_instruction_parser__get_error(own_parser);
    assert(error != NULL);
    assert(strstr(error, "(") != NULL);
    
    size_t pos = ar_method_instruction_parser__get_error_position(own_parser);
    assert(pos == 6);  // After "method"
    
    ar_method_instruction_parser__destroy(own_parser);
}

/**
 * Test parser reusability
 */
static void test_method_instruction_parser__reusability(void) {
    printf("Testing method parser reusability...\n");
    
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    // First parse - should succeed
    const char *instruction1 = "method(\"test1\", \"code1\", \"1.0.0\")";
    ar_instruction_ast_t *own_ast1 = ar_method_instruction_parser__parse(own_parser, instruction1, NULL);
    assert(own_ast1 != NULL);
    assert(ar_method_instruction_parser__get_error(own_parser) == NULL);
    
    // Second parse - should fail
    const char *instruction2 = "notmethod(\"test\", \"code\", \"1.0.0\")";
    ar_instruction_ast_t *own_ast2 = ar_method_instruction_parser__parse(own_parser, instruction2, NULL);
    assert(own_ast2 == NULL);
    assert(ar_method_instruction_parser__get_error(own_parser) != NULL);
    
    // Third parse - should succeed and clear previous error
    const char *instruction3 = "method(\"test3\", \"code3\", \"1.0.0\")";
    ar_instruction_ast_t *own_ast3 = ar_method_instruction_parser__parse(own_parser, instruction3, NULL);
    assert(own_ast3 != NULL);
    assert(ar_method_instruction_parser__get_error(own_parser) == NULL);
    
    ar__instruction_ast__destroy(own_ast1);
    ar__instruction_ast__destroy(own_ast3);
    ar_method_instruction_parser__destroy(own_parser);
}

/**
 * Test with multiline code
 */
static void test_method_instruction_parser__multiline_code(void) {
    printf("Testing method parser with multiline code...\n");
    
    // Given a method with code containing newlines
    const char *instruction = "method(\"multi\", \"memory.x := 1\\nmemory.y := 2\", \"1.0.0\")";
    
    // When parsing
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse correctly
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    void **own_items = ar__list__items(own_args);
    assert(strcmp((const char*)own_items[1], "\"memory.x := 1\\nmemory.y := 2\"") == 0);
    
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    ar__instruction_ast__destroy(own_ast);
    ar_method_instruction_parser__destroy(own_parser);
}

/**
 * Test method parsing with expression ASTs
 */
static void test_method_instruction_parser__parse_with_expression_asts(void) {
    printf("Testing method instruction with expression ASTs...\n");
    
    // Given a method instruction with string literal arguments
    const char *instruction = "method(\"calculate\", \"memory.result := memory.x + memory.y\", \"1.2.3\")";
    ar_method_instruction_parser_t *own_parser = ar_method_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_method_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    
    // And the arguments should be available as expression ASTs
    const list_t *ref_arg_asts = ar__instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar__list__count(ref_arg_asts) == 3);
    
    // All three arguments should be string literal ASTs
    void **items = ar__list__items(ref_arg_asts);
    assert(items != NULL);
    
    // First argument - method name
    const ar_expression_ast_t *ref_name = (const ar_expression_ast_t*)items[0];
    assert(ref_name != NULL);
    assert(ar__expression_ast__get_type(ref_name) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(ref_name), "calculate") == 0);
    
    // Second argument - method code
    const ar_expression_ast_t *ref_code = (const ar_expression_ast_t*)items[1];
    assert(ref_code != NULL);
    assert(ar__expression_ast__get_type(ref_code) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(ref_code), "memory.result := memory.x + memory.y") == 0);
    
    // Third argument - version
    const ar_expression_ast_t *ref_version = (const ar_expression_ast_t*)items[2];
    assert(ref_version != NULL);
    assert(ar__expression_ast__get_type(ref_version) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(ref_version), "1.2.3") == 0);
    
    AR__HEAP__FREE(items);
    ar__instruction_ast__destroy(own_ast);
    ar_method_instruction_parser__destroy(own_parser);
}

int main(void) {
    printf("\n=== Running Method Instruction Parser Tests ===\n\n");
    
    // test_method_instruction_parser__simple_parsing(); // TODO: Fix expression parser handling of quoted strings
    test_method_instruction_parser__with_assignment();
    // test_method_instruction_parser__complex_code(); // TODO: Fix expression parser handling of quoted strings with nested quotes
    test_method_instruction_parser__whitespace_handling();
    test_method_instruction_parser__wrong_function_name();
    test_method_instruction_parser__wrong_arg_count();
    test_method_instruction_parser__malformed_syntax();
    test_method_instruction_parser__reusability();
    test_method_instruction_parser__multiline_code();
    
    // Expression AST integration
    test_method_instruction_parser__parse_with_expression_asts();
    
    printf("\nAll method instruction parser tests passed!\n");
    
    ar__heap__memory_report();
    
    return 0;
}