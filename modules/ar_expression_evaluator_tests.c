/**
 * @file agerun_expression_evaluator_tests.c
 * @brief Test suite for expression evaluator module
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ar_expression_evaluator.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_event.h"

/**
 * Test creating and destroying an evaluator with ar_log
 */
static void test_create_destroy_with_log(void) {
    printf("Testing expression evaluator create/destroy with ar_log...\n");
    
    // Given an ar_log instance and a memory map
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    // When creating an evaluator with ar_log, memory, and no context
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    
    // Then the evaluator should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_expression_evaluator__destroy(evaluator);
    
    // Clean up
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    printf("  ✓ Create and destroy evaluator with ar_log\n");
}

/**
 * Test creating evaluator with context
 */
static void test_create_with_context(void) {
    printf("Testing expression evaluator create with context...\n");
    
    // Given a log, memory map and context map
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    data_t *memory = ar_data__create_map();
    data_t *context = ar_data__create_map();
    assert(memory != NULL);
    assert(context != NULL);
    
    // Add some data to context
    ar_data__set_map_string(context, "user", "test_user");
    
    // When creating an evaluator with log, memory and context
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, context);
    
    // Then the evaluator should be created successfully
    assert(evaluator != NULL);
    
    // Clean up
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    ar_data__destroy(context);
    ar_log__destroy(log);
    
    printf("  ✓ Create evaluator with context\n");
}

/**
 * Test NULL memory handling
 */
static void test_create_null_memory(void) {
    printf("Testing expression evaluator with NULL memory...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating an evaluator with NULL memory
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, NULL, NULL);
    
    // Then creation should fail
    assert(evaluator == NULL);
    
    // And an error should be logged
    ar_event_t *error_event = ar_log__get_last_error(log);
    assert(error_event != NULL);
    const char *error_msg = ar_event__get_message(error_event);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "NULL memory") != NULL);
    
    // Clean up
    ar_log__destroy(log);
    
    printf("  ✓ Handle NULL memory correctly\n");
}

/**
 * Test evaluating integer literal
 */
static void test_evaluate_literal_int(void) {
    printf("Testing expression evaluator literal int...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating the integer literal using the public evaluate method
    data_t *result = ar_expression_evaluator__evaluate(evaluator, ast);
    
    // Then it should return the integer value
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_INTEGER);
    assert(ar_data__get_integer(result) == 42);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__hold_ownership(result, evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__transfer_ownership(result, evaluator);
    assert(transferred == true);
    
    // Clean up
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate integer literal\n");
}

/**
 * Test evaluating non-integer node returns NULL
 */
static void test_evaluate_literal_int_wrong_type(void) {
    printf("Testing expression evaluator literal int with wrong type...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a string literal AST node (wrong type)
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_string("hello");
    assert(ast != NULL);
    
    // When evaluating with integer evaluator
    data_t *result = ar_expression_evaluator__evaluate_literal_int(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Return NULL for non-integer node\n");
}

/**
 * Test evaluating double literal
 */
static void test_evaluate_literal_double(void) {
    printf("Testing expression evaluator literal double...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a double literal AST node
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_double(3.14);
    assert(ast != NULL);
    
    // When evaluating the double literal
    data_t *result = ar_expression_evaluator__evaluate_literal_double(evaluator, ast);
    
    // Then it should return the double value
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_DOUBLE);
    assert(ar_data__get_double(result) == 3.14);
    
    // Clean up
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate double literal\n");
}

/**
 * Test evaluating non-double node returns NULL
 */
static void test_evaluate_literal_double_wrong_type(void) {
    printf("Testing expression evaluator literal double with wrong type...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node (wrong type)
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating with double evaluator
    data_t *result = ar_expression_evaluator__evaluate_literal_double(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Return NULL for non-double node\n");
}

/**
 * Test evaluating string literal
 */
static void test_evaluate_literal_string(void) {
    printf("Testing expression evaluator literal string...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a string literal AST node
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_string("hello world");
    assert(ast != NULL);
    
    // When evaluating the string literal using the public evaluate method
    data_t *result = ar_expression_evaluator__evaluate(evaluator, ast);
    
    // Then it should return the string value
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_STRING);
    assert(strcmp(ar_data__get_string(result), "hello world") == 0);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__hold_ownership(result, evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__transfer_ownership(result, evaluator);
    assert(transferred == true);
    
    // Clean up
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate string literal\n");
}

/**
 * Test evaluating non-string node returns NULL
 */
static void test_evaluate_literal_string_wrong_type(void) {
    printf("Testing expression evaluator literal string with wrong type...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node (wrong type)
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating with string evaluator
    data_t *result = ar_expression_evaluator__evaluate_literal_string(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Return NULL for non-string node\n");
}

/**
 * Test evaluating empty string literal
 */
static void test_evaluate_literal_string_empty(void) {
    printf("Testing expression evaluator literal empty string...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given an empty string literal AST node
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_string("");
    assert(ast != NULL);
    
    // When evaluating the empty string literal
    data_t *result = ar_expression_evaluator__evaluate_literal_string(evaluator, ast);
    
    // Then it should return an empty string value
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_STRING);
    assert(strcmp(ar_data__get_string(result), "") == 0);
    
    // Clean up
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate empty string literal\n");
}

/**
 * Test evaluating memory access
 */
static void test_evaluate_memory_access(void) {
    printf("Testing expression evaluator memory access...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map with some values and an evaluator
    data_t *memory = ar_data__create_map();
    ar_data__set_map_integer(memory, "x", 42);
    ar_data__set_map_string(memory, "name", "Alice");
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a memory access AST node for "memory.x"
    const char *path[] = {"x"};
    ar_expression_ast_t *ast = ar_expression_ast__create_memory_access("memory", path, 1);
    assert(ast != NULL);
    
    // When evaluating the memory access using the public evaluate method
    data_t *result = ar_expression_evaluator__evaluate(evaluator, ast);
    
    // Then it should return the value from memory (a reference, not owned)
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_INTEGER);
    assert(ar_data__get_integer(result) == 42);
    
    // Verify ownership: result should be owned by the memory map
    // Try to hold ownership with a different owner - should fail
    bool held = ar_data__hold_ownership(result, evaluator);
    assert(held == false);  // Should fail because memory map owns it
    
    // Clean up (do NOT destroy result - it's a reference)
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate memory access\n");
}

/**
 * Test evaluating non-memory-access node returns NULL
 */
static void test_evaluate_memory_access_wrong_type(void) {
    printf("Testing expression evaluator memory access with wrong type...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node (wrong type)
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating with memory access evaluator
    data_t *result = ar_expression_evaluator__evaluate_memory_access(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Return NULL for non-memory-access node\n");
}

/**
 * Test evaluating nested memory access
 */
static void test_evaluate_memory_access_nested(void) {
    printf("Testing expression evaluator nested memory access...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map with nested structure
    data_t *memory = ar_data__create_map();
    data_t *user = ar_data__create_map();
    ar_data__set_map_string(user, "name", "Bob");
    ar_data__set_map_integer(user, "age", 30);
    ar_data__set_map_data(memory, "user", user);
    
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a memory access AST node for "memory.user.name"
    const char *path[] = {"user", "name"};
    ar_expression_ast_t *ast = ar_expression_ast__create_memory_access("memory", path, 2);
    assert(ast != NULL);
    
    // When evaluating the nested memory access
    data_t *result = ar_expression_evaluator__evaluate_memory_access(evaluator, ast);
    
    // Then it should return the nested value (a reference, not owned)
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_STRING);
    assert(strcmp(ar_data__get_string(result), "Bob") == 0);
    
    // Clean up (do NOT destroy result - it's a reference)
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate nested memory access\n");
}

/**
 * Test evaluating memory access for missing key
 */
static void test_evaluate_memory_access_missing(void) {
    printf("Testing expression evaluator memory access for missing key...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an empty memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a memory access AST node for "memory.missing"
    const char *path[] = {"missing"};
    ar_expression_ast_t *ast = ar_expression_ast__create_memory_access("memory", path, 1);
    assert(ast != NULL);
    
    // When evaluating the memory access for a missing key
    data_t *result = ar_expression_evaluator__evaluate_memory_access(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Return NULL for missing memory key\n");
}

/**
 * Test evaluating binary addition of integers
 */
static void test_evaluate_binary_op_add_integers(void) {
    printf("Testing expression evaluator binary op add integers...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a binary addition AST node for "5 + 3"
    ar_expression_ast_t *left = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *right = ar_expression_ast__create_literal_int(3);
    ar_expression_ast_t *ast = ar_expression_ast__create_binary_op(AR_OP__ADD, left, right);
    assert(ast != NULL);
    
    // When evaluating the binary operation using the public evaluate method
    data_t *result = ar_expression_evaluator__evaluate(evaluator, ast);
    
    // Then it should return the sum (a new owned value)
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_INTEGER);
    assert(ar_data__get_integer(result) == 8);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__hold_ownership(result, evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__transfer_ownership(result, evaluator);
    assert(transferred == true);
    
    // Clean up (MUST destroy result - it's owned)
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate binary addition of integers\n");
}

/**
 * Test evaluating binary multiplication of doubles
 */
static void test_evaluate_binary_op_multiply_doubles(void) {
    printf("Testing expression evaluator binary op multiply doubles...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a binary multiplication AST node for "2.5 * 4.0"
    ar_expression_ast_t *left = ar_expression_ast__create_literal_double(2.5);
    ar_expression_ast_t *right = ar_expression_ast__create_literal_double(4.0);
    ar_expression_ast_t *ast = ar_expression_ast__create_binary_op(AR_OP__MULTIPLY, left, right);
    assert(ast != NULL);
    
    // When evaluating the binary operation
    data_t *result = ar_expression_evaluator__evaluate_binary_op(evaluator, ast);
    
    // Then it should return the product (a new owned value)
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_DOUBLE);
    assert(ar_data__get_double(result) == 10.0);
    
    // Clean up (MUST destroy result - it's owned)
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate binary multiplication of doubles\n");
}

/**
 * Test evaluating binary string concatenation
 */
static void test_evaluate_binary_op_concatenate_strings(void) {
    printf("Testing expression evaluator binary op concatenate strings...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a binary addition AST node for "Hello" + " World"
    ar_expression_ast_t *left = ar_expression_ast__create_literal_string("Hello");
    ar_expression_ast_t *right = ar_expression_ast__create_literal_string(" World");
    ar_expression_ast_t *ast = ar_expression_ast__create_binary_op(AR_OP__ADD, left, right);
    assert(ast != NULL);
    
    // When evaluating the binary operation
    data_t *result = ar_expression_evaluator__evaluate_binary_op(evaluator, ast);
    
    // Then it should return the concatenated string (a new owned value)
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_STRING);
    assert(strcmp(ar_data__get_string(result), "Hello World") == 0);
    
    // Clean up (MUST destroy result - it's owned)
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate binary string concatenation\n");
}

/**
 * Test evaluating non-binary-op node returns NULL
 */
static void test_evaluate_binary_op_wrong_type(void) {
    printf("Testing expression evaluator binary op with wrong type...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map and evaluator
    data_t *memory = ar_data__create_map();
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given an integer literal AST node (wrong type)
    ar_expression_ast_t *ast = ar_expression_ast__create_literal_int(42);
    assert(ast != NULL);
    
    // When evaluating with binary op evaluator
    data_t *result = ar_expression_evaluator__evaluate_binary_op(evaluator, ast);
    
    // Then it should return NULL
    assert(result == NULL);
    
    // Clean up
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Return NULL for non-binary-op node\n");
}

/**
 * Test evaluating nested binary operations
 */
static void test_evaluate_binary_op_nested(void) {
    printf("Testing expression evaluator nested binary operations...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a memory map with some values
    data_t *memory = ar_data__create_map();
    ar_data__set_map_integer(memory, "x", 10);
    ar_data__set_map_integer(memory, "y", 5);
    ar_expression_evaluator_t *evaluator = ar_expression_evaluator__create(log, memory, NULL);
    assert(evaluator != NULL);
    
    // Given a nested binary operation AST node for "(memory.x + 2) * memory.y"
    // First create memory.x
    const char *path_x[] = {"x"};
    ar_expression_ast_t *mem_x = ar_expression_ast__create_memory_access("memory", path_x, 1);
    
    // Create memory.x + 2
    ar_expression_ast_t *two = ar_expression_ast__create_literal_int(2);
    ar_expression_ast_t *add = ar_expression_ast__create_binary_op(AR_OP__ADD, mem_x, two);
    
    // Create memory.y
    const char *path_y[] = {"y"};
    ar_expression_ast_t *mem_y = ar_expression_ast__create_memory_access("memory", path_y, 1);
    
    // Create (memory.x + 2) * memory.y
    ar_expression_ast_t *ast = ar_expression_ast__create_binary_op(AR_OP__MULTIPLY, add, mem_y);
    assert(ast != NULL);
    
    // When evaluating the nested binary operation using the public evaluate method
    data_t *result = ar_expression_evaluator__evaluate(evaluator, ast);
    
    // Then it should return (10 + 2) * 5 = 60 (a new owned value)
    assert(result != NULL);
    assert(ar_data__get_type(result) == DATA_INTEGER);
    assert(ar_data__get_integer(result) == 60);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__hold_ownership(result, evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__transfer_ownership(result, evaluator);
    assert(transferred == true);
    
    // Clean up (MUST destroy result - it's owned)
    ar_data__destroy(result);
    ar_expression_ast__destroy(ast);
    ar_expression_evaluator__destroy(evaluator);
    ar_data__destroy(memory);
    
    ar_log__destroy(log);
    
    printf("  ✓ Evaluate nested binary operations\n");
}

int main(void) {
    printf("\n=== Expression Evaluator Tests ===\n\n");
    
    test_create_destroy_with_log();
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
    test_evaluate_binary_op_add_integers();
    test_evaluate_binary_op_multiply_doubles();
    test_evaluate_binary_op_concatenate_strings();
    test_evaluate_binary_op_wrong_type();
    test_evaluate_binary_op_nested();
    
    printf("\nAll expression_evaluator tests passed!\n");
    return 0;
}
