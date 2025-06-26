#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_parser.h"
#include "ar_method_ast.h"
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

int main(void) {
    printf("Running method parser tests...\n\n");
    
    test_method_parser__create_destroy();
    test_method_parser__destroy_null();
    test_method_parser__parse_empty_method();
    
    printf("\nAll method parser tests passed!\n");
    return 0;
}