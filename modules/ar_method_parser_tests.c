#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_parser.h"
#include "ar_method_ast.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"

// Test create and destroy
static void test_method_parser__create_destroy(void) {
    printf("Testing method parser create and destroy...\n");
    
    // Given the need to create a method parser
    
    // When creating a new method parser
    ar_method_parser_t *own_parser = ar_method_parser__create();
    
    // Then the parser should be created successfully
    assert(own_parser != NULL);
    
    // And it should be destroyable without issues
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__create_destroy passed\n");
}

// Test destroy with NULL
static void test_method_parser__destroy_null(void) {
    printf("Testing method parser destroy with NULL...\n");
    
    // Given a NULL pointer
    
    // When destroying NULL
    ar_method_parser__destroy(NULL);
    
    // Then no crash should occur
    
    printf("✓ test_method_parser__destroy_null passed\n");
}

// Test parse empty method
static void test_method_parser__parse_empty_method(void) {
    printf("Testing method parser parse empty method...\n");
    
    // Given a parser and an empty method source
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    const char *ref_source = "";
    
    // When parsing the empty source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then an AST should be created with no instructions
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 0);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__parse_empty_method passed\n");
}

// Test parse single instruction
static void test_method_parser__parse_single_instruction(void) {
    printf("Testing method parser parse single instruction...\n");
    
    // Given a parser and a method with one instruction
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    const char *ref_source = "memory.x := 42";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then an AST should be created with one instruction
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 1);
    
    // And the instruction should be the correct type
    const ar_instruction_ast_t *ref_instruction = ar_method_ast__get_instruction(own_ast, 1);
    assert(ref_instruction != NULL);
    assert(ar__instruction_ast__get_type(ref_instruction) == AR_INST__ASSIGNMENT);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__parse_single_instruction passed\n");
}

// Test parse multiple instructions
static void test_method_parser__parse_multiple_instructions(void) {
    printf("Testing method parser parse multiple instructions...\n");
    
    // Given a parser and a method with multiple instructions
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    const char *ref_source = "memory.x := 10\nmemory.y := 20\nmemory.z := 30";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then an AST should be created with three instructions
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 3);
    
    // And each instruction should be the correct type in order
    const ar_instruction_ast_t *ref_instruction1 = ar_method_ast__get_instruction(own_ast, 1);
    assert(ref_instruction1 != NULL);
    assert(ar__instruction_ast__get_type(ref_instruction1) == AR_INST__ASSIGNMENT);
    
    const ar_instruction_ast_t *ref_instruction2 = ar_method_ast__get_instruction(own_ast, 2);
    assert(ref_instruction2 != NULL);
    assert(ar__instruction_ast__get_type(ref_instruction2) == AR_INST__ASSIGNMENT);
    
    const ar_instruction_ast_t *ref_instruction3 = ar_method_ast__get_instruction(own_ast, 3);
    assert(ref_instruction3 != NULL);
    assert(ar__instruction_ast__get_type(ref_instruction3) == AR_INST__ASSIGNMENT);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__parse_multiple_instructions passed\n");
}

// Test parse with empty lines
static void test_method_parser__parse_with_empty_lines(void) {
    printf("Testing method parser parse with empty lines...\n");
    
    // Given a parser and a method with instructions and empty lines
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    const char *ref_source = "memory.x := 10\n\nmemory.y := 20\n\n\nmemory.z := 30\n";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then an AST should be created with only the non-empty instructions
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 3);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__parse_with_empty_lines passed\n");
}

// Test parse with comments
static void test_method_parser__parse_with_comments(void) {
    printf("Testing method parser parse with comments...\n");
    
    // Given a parser and a method with comments
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    const char *ref_source = "# This is a comment\nmemory.x := 10\n# Another comment\nmemory.y := 20\nmemory.z := 30 # Inline comment";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then an AST should be created with only the instructions
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 3);
    
    // And each instruction should be correctly parsed
    const ar_instruction_ast_t *ref_instruction1 = ar_method_ast__get_instruction(own_ast, 1);
    assert(ref_instruction1 != NULL);
    assert(ar__instruction_ast__get_type(ref_instruction1) == AR_INST__ASSIGNMENT);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__parse_with_comments passed\n");
}

// Test parse with hash in string
static void test_method_parser__parse_hash_in_string(void) {
    printf("Testing method parser parse with hash in string...\n");
    
    // Given a parser and a method with # inside a string
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    const char *ref_source = "memory.msg := \"Item #1\"";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then the AST should be created successfully
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 1);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__parse_hash_in_string passed\n");
}

// Test parse with invalid instruction
static void test_method_parser__parse_invalid_instruction(void) {
    printf("Testing method parser parse with invalid instruction...\n");
    
    // Given a parser and a method with invalid syntax
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    const char *ref_source = "memory.x := 10\ninvalid syntax here\nmemory.z := 30";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then parsing should fail
    assert(own_ast == NULL);
    
    // And an error message should be available
    const char *ref_error = ar_method_parser__get_error(own_parser);
    assert(ref_error != NULL);
    assert(strlen(ref_error) > 0);
    
    // And the error line should be correct
    int error_line = ar_method_parser__get_error_line(own_parser);
    assert(error_line == 2);
    
    // Clean up
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__parse_invalid_instruction passed\n");
}

// Test error cleared on successful parse
static void test_method_parser__error_cleared_on_success(void) {
    printf("Testing method parser error cleared on successful parse...\n");
    
    // Given a parser that had a previous error
    ar_method_parser_t *own_parser = ar_method_parser__create();
    assert(own_parser != NULL);
    
    // First, cause an error
    const char *ref_bad_source = "invalid syntax";
    ar_method_ast_t *own_bad_ast = ar_method_parser__parse(own_parser, ref_bad_source);
    assert(own_bad_ast == NULL);
    assert(ar_method_parser__get_error(own_parser) != NULL);
    assert(ar_method_parser__get_error_line(own_parser) == 1);
    
    // When parsing valid source
    const char *ref_good_source = "memory.x := 42";
    ar_method_ast_t *own_good_ast = ar_method_parser__parse(own_parser, ref_good_source);
    
    // Then parsing should succeed
    assert(own_good_ast != NULL);
    
    // And error should be cleared
    assert(ar_method_parser__get_error(own_parser) == NULL);
    assert(ar_method_parser__get_error_line(own_parser) == 0);
    
    // Clean up
    ar_method_ast__destroy(own_good_ast);
    ar_method_parser__destroy(own_parser);
    
    printf("✓ test_method_parser__error_cleared_on_success passed\n");
}

int main(void) {
    printf("Running method parser tests...\n\n");
    
    test_method_parser__create_destroy();
    test_method_parser__destroy_null();
    test_method_parser__parse_empty_method();
    test_method_parser__parse_single_instruction();
    test_method_parser__parse_multiple_instructions();
    test_method_parser__parse_with_empty_lines();
    test_method_parser__parse_with_comments();
    test_method_parser__parse_hash_in_string();
    test_method_parser__parse_invalid_instruction();
    test_method_parser__error_cleared_on_success();
    
    printf("\nAll method parser tests passed!\n");
    return 0;
}