#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_parser.h"
#include "ar_method_ast.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"
#include "ar_log.h"


static void test_create_parser_with_log(void) {
    printf("Testing parser creation with ar_log...\n");
    
    // Given an ar_log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with ar_log
    ar_method_parser_t *parser = ar_method_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_method_parser__destroy(parser);
    ar_log__destroy(log);
}

// Test create and destroy
static void test_method_parser__create_destroy(void) {
    printf("Testing method parser create and destroy...\n");
    
    // Given the need to create a method parser
    
    // When creating a new method parser
    ar_method_parser_t *own_parser = ar_method_parser__create(NULL);
    
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
    
    // Given a log instance, a parser and an empty method source
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
    assert(own_parser != NULL);
    const char *ref_source = "";
    
    // When parsing the empty source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then an AST should be created with no instructions
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 0);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__parse_empty_method passed\n");
}

// Test parse single instruction
static void test_method_parser__parse_single_instruction(void) {
    printf("Testing method parser parse single instruction...\n");
    
    // Given a log instance, a parser and a method with one instruction
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
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
    assert(ar_instruction_ast__get_type(ref_instruction) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__parse_single_instruction passed\n");
}

// Test parse multiple instructions
static void test_method_parser__parse_multiple_instructions(void) {
    printf("Testing method parser parse multiple instructions...\n");
    
    // Given a log instance, a parser and a method with multiple instructions
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
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
    assert(ar_instruction_ast__get_type(ref_instruction1) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    const ar_instruction_ast_t *ref_instruction2 = ar_method_ast__get_instruction(own_ast, 2);
    assert(ref_instruction2 != NULL);
    assert(ar_instruction_ast__get_type(ref_instruction2) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    const ar_instruction_ast_t *ref_instruction3 = ar_method_ast__get_instruction(own_ast, 3);
    assert(ref_instruction3 != NULL);
    assert(ar_instruction_ast__get_type(ref_instruction3) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__parse_multiple_instructions passed\n");
}

// Test parse with empty lines
static void test_method_parser__parse_with_empty_lines(void) {
    printf("Testing method parser parse with empty lines...\n");
    
    // Given a log instance, a parser and a method with instructions and empty lines
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
    assert(own_parser != NULL);
    const char *ref_source = "memory.x := 10\n\nmemory.y := 20\n\n\nmemory.z := 30\n";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then an AST should be created with only the non-empty instructions
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 3);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__parse_with_empty_lines passed\n");
}

// Test parse with comments
static void test_method_parser__parse_with_comments(void) {
    printf("Testing method parser parse with comments...\n");
    
    // Given a log instance, a parser and a method with comments
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
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
    assert(ar_instruction_ast__get_type(ref_instruction1) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__parse_with_comments passed\n");
}

// Test parse with hash in string
static void test_method_parser__parse_hash_in_string(void) {
    printf("Testing method parser parse with hash in string...\n");
    
    // Given a log instance, a parser and a method with # inside a string
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
    assert(own_parser != NULL);
    const char *ref_source = "memory.msg := \"Item #1\"";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then the AST should be created successfully
    assert(own_ast != NULL);
    assert(ar_method_ast__get_instruction_count(own_ast) == 1);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    // Clean up
    ar_method_ast__destroy(own_ast);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__parse_hash_in_string passed\n");
}

// Test parse with invalid instruction
static void test_method_parser__parse_invalid_instruction(void) {
    printf("Testing method parser parse with invalid instruction...\n");
    
    // Given a log instance and parser
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
    assert(own_parser != NULL);
    const char *ref_source = "memory.x := 10\ninvalid syntax here\nmemory.z := 30";
    
    // When parsing the source
    ar_method_ast_t *own_ast = ar_method_parser__parse(own_parser, ref_source);
    
    // Then parsing should fail
    assert(own_ast == NULL);
    
    // And an error message should be available in the log
    const char *ref_error = ar_log__get_last_error_message(log);
    assert(ref_error != NULL);
    assert(strlen(ref_error) > 0);
    
    // And the error position should be correct (line 2)
    int error_pos = ar_log__get_last_error_position(log);
    assert(error_pos == 2);
    
    // Clean up
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__parse_invalid_instruction passed\n");
}

// Test successful parse after failed parse
static void test_method_parser__successful_parse_after_failure(void) {
    printf("Testing method parser successful parse after failure...\n");
    
    // Given a log instance and parser
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_method_parser_t *own_parser = ar_method_parser__create(log);
    assert(own_parser != NULL);
    
    // First, cause an error
    const char *ref_bad_source = "invalid syntax";
    ar_method_ast_t *own_bad_ast = ar_method_parser__parse(own_parser, ref_bad_source);
    assert(own_bad_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(ar_log__get_last_error_position(log) == 1);
    
    // When parsing valid source
    const char *ref_good_source = "memory.x := 42";
    ar_method_ast_t *own_good_ast = ar_method_parser__parse(own_parser, ref_good_source);
    
    // Then parsing should succeed
    assert(own_good_ast != NULL);
    
    // Note: ar_log persists errors, so we can't test for cleared errors
    // The important thing is that parsing succeeded
    
    // Clean up
    ar_method_ast__destroy(own_good_ast);
    ar_method_parser__destroy(own_parser);
    ar_log__destroy(log);
    
    printf("✓ test_method_parser__successful_parse_after_failure passed\n");
}

int main(void) {
    printf("Running method parser tests...\n\n");
    
    // Test with ar_log
    test_create_parser_with_log();
    
    test_method_parser__create_destroy();
    test_method_parser__destroy_null();
    test_method_parser__parse_empty_method();
    test_method_parser__parse_single_instruction();
    test_method_parser__parse_multiple_instructions();
    test_method_parser__parse_with_empty_lines();
    test_method_parser__parse_with_comments();
    test_method_parser__parse_hash_in_string();
    test_method_parser__parse_invalid_instruction();
    test_method_parser__successful_parse_after_failure();
    
    printf("\nAll method parser tests passed!\n");
    return 0;
}