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

int main(void) {
    printf("\n=== Expression Evaluator Tests ===\n\n");
    
    test_create_destroy();
    test_create_with_context();
    test_create_null_memory();
    
    printf("\nAll expression_evaluator tests passed!\n");
    return 0;
}
