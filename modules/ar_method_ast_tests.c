#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_ast.h"
#include "ar_heap.h"

// Test create and destroy
static void test_method_ast__create_destroy(void) {
    printf("Testing method AST create and destroy...\n");
    
    // Given the need to create a method AST
    
    // When creating a new method AST
    ar_method_ast_t *own_ast = ar__method_ast__create();
    
    // Then the AST should be created successfully
    assert(own_ast != NULL);
    
    // And it should be destroyable without issues
    ar__method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__create_destroy passed\n");
}

// Test destroy with NULL
static void test_method_ast__destroy_null(void) {
    printf("Testing method AST destroy with NULL...\n");
    
    // Given a NULL pointer
    
    // When destroying NULL
    ar__method_ast__destroy(NULL);
    
    // Then no crash should occur
    
    printf("✓ test_method_ast__destroy_null passed\n");
}

int main(void) {
    printf("Running method AST tests...\n\n");
    
    test_method_ast__create_destroy();
    test_method_ast__destroy_null();
    
    printf("\nAll method AST tests passed!\n");
    return 0;
}