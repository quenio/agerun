/**
 * @file agerun_expression_evaluator_tests.c
 * @brief Test suite for expression evaluator module
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "agerun_expression_evaluator.h"
#include "agerun_data.h"
#include "agerun_heap.h"

/**
 * Test creating and destroying an evaluator
 */
static void test_create_destroy(void) {
    printf("Testing expression evaluator create/destroy...\n");
    
    // Given a memory map
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    // When creating an evaluator with just memory (no context)
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    
    // Then the evaluator should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar__expression_evaluator__destroy(evaluator);
    
    // Clean up
    ar__data__destroy(memory);
    
    printf("  ✓ Create and destroy evaluator\n");
}

/**
 * Test creating evaluator with context
 */
static void test_create_with_context(void) {
    printf("Testing expression evaluator create with context...\n");
    
    // Given a memory map and context map
    data_t *memory = ar__data__create_map();
    data_t *context = ar__data__create_map();
    assert(memory != NULL);
    assert(context != NULL);
    
    // Add some data to context
    ar__data__set_map_string(context, "user", "test_user");
    
    // When creating an evaluator with memory and context
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, context);
    
    // Then the evaluator should be created successfully
    assert(evaluator != NULL);
    
    // Clean up
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    
    printf("  ✓ Create evaluator with context\n");
}

/**
 * Test NULL memory handling
 */
static void test_create_null_memory(void) {
    printf("Testing expression evaluator with NULL memory...\n");
    
    // When creating an evaluator with NULL memory
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(NULL, NULL);
    
    // Then creation should fail
    assert(evaluator == NULL);
    
    printf("  ✓ Handle NULL memory correctly\n");
}

/**
 * Test evaluating integer literal
 */
static void test_evaluate_literal_int(void) {
    printf("Testing expression evaluator literal int...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node
    expression_ast_t *ast = ar__expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating the integer literal
    data_t *result = ar__expression_evaluator__evaluate_literal_int(evaluator, ast);
    
    // Then it should return the integer value
    assert(result != NULL);
    assert(ar__data__get_type(result) == DATA_INTEGER);
    assert(ar__data__get_integer(result) == 42);
    
    // Clean up
    ar__data__destroy(result);
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Evaluate integer literal\n");
}

/**
 * Test evaluating non-integer node returns NULL
 */
static void test_evaluate_literal_int_wrong_type(void) {
    printf("Testing expression evaluator literal int with wrong type...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given a string literal AST node (wrong type)
    expression_ast_t *ast = ar__expression_ast__create_literal_string("hello");
    assert(ast != NULL);
    
    // When evaluating with integer evaluator
    data_t *result = ar__expression_evaluator__evaluate_literal_int(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Return NULL for non-integer node\n");
}

int main(void) {
    printf("\n=== Expression Evaluator Tests ===\n\n");
    
    test_create_destroy();
    test_create_with_context();
    test_create_null_memory();
    test_evaluate_literal_int();
    test_evaluate_literal_int_wrong_type();
    
    printf("\nAll expression_evaluator tests passed!\n");
    return 0;
}
