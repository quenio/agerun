/**
 * @file agerun_expression_evaluator_tests.c
 * @brief Test suite for expression evaluator module
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "agerun_expression_evaluator.h"
#include "agerun_expression_ast.h"
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

/**
 * Test evaluating double literal
 */
static void test_evaluate_literal_double(void) {
    printf("Testing expression evaluator literal double...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given a double literal AST node
    expression_ast_t *ast = ar__expression_ast__create_literal_double(3.14);
    assert(ast != NULL);
    
    // When evaluating the double literal
    data_t *result = ar__expression_evaluator__evaluate_literal_double(evaluator, ast);
    
    // Then it should return the double value
    assert(result != NULL);
    assert(ar__data__get_type(result) == DATA_DOUBLE);
    assert(ar__data__get_double(result) == 3.14);
    
    // Clean up
    ar__data__destroy(result);
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Evaluate double literal\n");
}

/**
 * Test evaluating non-double node returns NULL
 */
static void test_evaluate_literal_double_wrong_type(void) {
    printf("Testing expression evaluator literal double with wrong type...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node (wrong type)
    expression_ast_t *ast = ar__expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating with double evaluator
    data_t *result = ar__expression_evaluator__evaluate_literal_double(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Return NULL for non-double node\n");
}

/**
 * Test evaluating string literal
 */
static void test_evaluate_literal_string(void) {
    printf("Testing expression evaluator literal string...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given a string literal AST node
    expression_ast_t *ast = ar__expression_ast__create_literal_string("hello world");
    assert(ast != NULL);
    
    // When evaluating the string literal
    data_t *result = ar__expression_evaluator__evaluate_literal_string(evaluator, ast);
    
    // Then it should return the string value
    assert(result != NULL);
    assert(ar__data__get_type(result) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result), "hello world") == 0);
    
    // Clean up
    ar__data__destroy(result);
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Evaluate string literal\n");
}

/**
 * Test evaluating non-string node returns NULL
 */
static void test_evaluate_literal_string_wrong_type(void) {
    printf("Testing expression evaluator literal string with wrong type...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node (wrong type)
    expression_ast_t *ast = ar__expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating with string evaluator
    data_t *result = ar__expression_evaluator__evaluate_literal_string(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Return NULL for non-string node\n");
}

/**
 * Test evaluating empty string literal
 */
static void test_evaluate_literal_string_empty(void) {
    printf("Testing expression evaluator literal empty string...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given an empty string literal AST node
    expression_ast_t *ast = ar__expression_ast__create_literal_string("");
    assert(ast != NULL);
    
    // When evaluating the empty string literal
    data_t *result = ar__expression_evaluator__evaluate_literal_string(evaluator, ast);
    
    // Then it should return an empty string value
    assert(result != NULL);
    assert(ar__data__get_type(result) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result), "") == 0);
    
    // Clean up
    ar__data__destroy(result);
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Evaluate empty string literal\n");
}

/**
 * Test evaluating memory access
 */
static void test_evaluate_memory_access(void) {
    printf("Testing expression evaluator memory access...\n");
    
    // Given a memory map with some values and an evaluator
    data_t *memory = ar__data__create_map();
    ar__data__set_map_integer(memory, "x", 42);
    ar__data__set_map_string(memory, "name", "Alice");
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given a memory access AST node for "memory.x"
    const char *path[] = {"x"};
    expression_ast_t *ast = ar__expression_ast__create_memory_access("memory", path, 1);
    assert(ast != NULL);
    
    // When evaluating the memory access
    data_t *result = ar__expression_evaluator__evaluate_memory_access(evaluator, ast);
    
    // Then it should return the value from memory (a reference, not owned)
    assert(result != NULL);
    assert(ar__data__get_type(result) == DATA_INTEGER);
    assert(ar__data__get_integer(result) == 42);
    
    // Clean up (do NOT destroy result - it's a reference)
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Evaluate memory access\n");
}

/**
 * Test evaluating non-memory-access node returns NULL
 */
static void test_evaluate_memory_access_wrong_type(void) {
    printf("Testing expression evaluator memory access with wrong type...\n");
    
    // Given a memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node (wrong type)
    expression_ast_t *ast = ar__expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating with memory access evaluator
    data_t *result = ar__expression_evaluator__evaluate_memory_access(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Return NULL for non-memory-access node\n");
}

/**
 * Test evaluating nested memory access
 */
static void test_evaluate_memory_access_nested(void) {
    printf("Testing expression evaluator nested memory access...\n");
    
    // Given a memory map with nested structure
    data_t *memory = ar__data__create_map();
    data_t *user = ar__data__create_map();
    ar__data__set_map_string(user, "name", "Bob");
    ar__data__set_map_integer(user, "age", 30);
    ar__data__set_map_data(memory, "user", user);
    
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given a memory access AST node for "memory.user.name"
    const char *path[] = {"user", "name"};
    expression_ast_t *ast = ar__expression_ast__create_memory_access("memory", path, 2);
    assert(ast != NULL);
    
    // When evaluating the nested memory access
    data_t *result = ar__expression_evaluator__evaluate_memory_access(evaluator, ast);
    
    // Then it should return the nested value (a reference, not owned)
    assert(result != NULL);
    assert(ar__data__get_type(result) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result), "Bob") == 0);
    
    // Clean up (do NOT destroy result - it's a reference)
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Evaluate nested memory access\n");
}

/**
 * Test evaluating memory access for missing key
 */
static void test_evaluate_memory_access_missing(void) {
    printf("Testing expression evaluator memory access for missing key...\n");
    
    // Given an empty memory map and evaluator
    data_t *memory = ar__data__create_map();
    expression_evaluator_t *evaluator = ar__expression_evaluator__create(memory, NULL);
    assert(evaluator != NULL);
    
    // Given a memory access AST node for "memory.missing"
    const char *path[] = {"missing"};
    expression_ast_t *ast = ar__expression_ast__create_memory_access("memory", path, 1);
    assert(ast != NULL);
    
    // When evaluating the memory access for a missing key
    data_t *result = ar__expression_evaluator__evaluate_memory_access(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar__expression_ast__destroy(ast);
    ar__expression_evaluator__destroy(evaluator);
    ar__data__destroy(memory);
    
    printf("  ✓ Return NULL for missing memory key\n");
}

int main(void) {
    printf("\n=== Expression Evaluator Tests ===\n\n");
    
    test_create_destroy();
    test_create_with_context();
    test_create_null_memory();
    test_evaluate_literal_int();
    test_evaluate_literal_int_wrong_type();
    test_evaluate_literal_double();
    test_evaluate_literal_double_wrong_type();
    test_evaluate_literal_string();
    test_evaluate_literal_string_wrong_type();
    test_evaluate_literal_string_empty();
    test_evaluate_memory_access();
    test_evaluate_memory_access_wrong_type();
    test_evaluate_memory_access_nested();
    test_evaluate_memory_access_missing();
    
    printf("\nAll expression_evaluator tests passed!\n");
    return 0;
}
