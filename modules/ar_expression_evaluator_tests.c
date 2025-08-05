/**
 * @file ar_expression_evaluator_tests.c
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
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"

/**
 * Test creating and destroying an evaluator with ar_log
 */
static void test_create_destroy_with_log(void) {
    printf("Testing expression evaluator create/destroy with ar_log...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_create_destroy_with_log");
    assert(own_fixture != NULL);
    
    // When getting the expression evaluator from the fixture
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    // Then the evaluator should exist
    assert(ref_evaluator != NULL);
    
    // Clean up (fixture handles all cleanup)
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Create and destroy evaluator with ar_log\n");
}

/**
 * Test creating evaluator with context
 */
static void test_create_with_log_only(void) {
    printf("Testing expression evaluator create with log only...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_create_with_log_only");
    assert(own_fixture != NULL);
    
    // When getting the expression evaluator from the fixture
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    // Then the evaluator should exist
    assert(ref_evaluator != NULL);
    
    // Clean up (fixture handles all cleanup)
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Create evaluator with log only\n");
}


/**
 * Test evaluating integer literal
 */
static void test_evaluate_literal_int(void) {
    printf("Testing expression evaluator literal int...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_literal_int");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given an integer literal AST node
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_int(42);
    assert(own_ast != NULL);
    
    // When evaluating the integer literal using evaluate
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the integer value
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_result) == 42);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__take_ownership(own_result, ref_evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__drop_ownership(own_result, ref_evaluator);
    assert(transferred == true);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate integer literal\n");
}

/**
 * Test evaluate function handles string literal correctly
 */
static void test_evaluate_handles_string_literal(void) {
    printf("Testing expression evaluator with string when expecting int...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_handles_string_literal");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a string literal AST node (not an integer)
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_string("hello");
    assert(own_ast != NULL);
    
    // When evaluating with the general evaluate function
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return a string value (not NULL)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_result), "hello") == 0);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate returns correct type for any expression\n");
}

/**
 * Test evaluating double literal
 */
static void test_evaluate_literal_double(void) {
    printf("Testing expression evaluator literal double...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_literal_double");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a double literal AST node
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_double(3.14);
    assert(own_ast != NULL);
    
    // When evaluating the double literal with frame
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the double value
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(own_result) == 3.14);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate double literal\n");
}

/**
 * Test evaluate function handles integer literal when expecting double
 */
static void test_evaluate_handles_int_as_double(void) {
    printf("Testing expression evaluator with integer when expecting double...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_handles_int_as_double");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given an integer literal AST node (not a double)
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_int(42);
    assert(own_ast != NULL);
    
    // When evaluating with the general evaluate function
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return an integer value (not NULL)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_result) == 42);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate returns correct type for any expression\n");
}

/**
 * Test evaluating string literal
 */
static void test_evaluate_literal_string(void) {
    printf("Testing expression evaluator literal string...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_literal_string");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a string literal AST node
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_string("hello world");
    assert(own_ast != NULL);
    
    // When evaluating the string literal using evaluate
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the string value
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_result), "hello world") == 0);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__take_ownership(own_result, ref_evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__drop_ownership(own_result, ref_evaluator);
    assert(transferred == true);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate string literal\n");
}

/**
 * Test evaluate function handles integer literal when expecting string
 */
static void test_evaluate_handles_int_as_string(void) {
    printf("Testing expression evaluator with integer when expecting string...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_handles_int_as_string");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given an integer literal AST node (not a string)
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_int(42);
    assert(own_ast != NULL);
    
    // When evaluating with the general evaluate function
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return an integer value (not NULL)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_result) == 42);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate returns correct type for any expression\n");
}

/**
 * Test evaluating empty string literal
 */
static void test_evaluate_literal_string_empty(void) {
    printf("Testing expression ref_evaluator literal empty string...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_literal_string_empty");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given an empty string literal AST node
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_string("");
    assert(own_ast != NULL);
    
    // When evaluating the empty string literal using evaluate
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return an empty string value
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_result), "") == 0);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate empty string literal\n");
}

/**
 * Test evaluating memory access
 */
static void test_evaluate_memory_access(void) {
    printf("Testing expression evaluator memory access...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_memory_access");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    
    // Add some values to memory
    ar_data__set_map_integer(mut_memory, "x", 42);
    ar_data__set_map_string(mut_memory, "name", "Alice");
    
    // Given a memory access AST node for "memory.x"
    const char *path[] = {"x"};
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("memory", path, 1);
    assert(own_ast != NULL);
    
    // When evaluating the memory access using evaluate
    ar_data_t *ref_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the value from memory (a reference, not owned)
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_result) == 42);
    
    // Verify ownership: result should be owned by the memory map
    // Try to hold ownership with a different owner - should fail
    bool held = ar_data__take_ownership(ref_result, ref_evaluator);
    assert(held == false);  // Should fail because memory map owns it
    
    // Clean up (do NOT destroy result - it's a reference)
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate memory access\n");
}

/**
 * Test evaluate function handles integer literal when expecting memory access
 */
static void test_evaluate_handles_int_as_memory_access(void) {
    printf("Testing expression evaluator with integer when expecting memory access...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_handles_int_as_memory_access");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given an integer literal AST node (not a memory access)
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_int(42);
    assert(own_ast != NULL);
    
    // When evaluating with the general evaluate function
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return an integer value (not NULL)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_result) == 42);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate returns correct type for any expression\n");
}

/**
 * Test evaluating nested memory access
 */
static void test_evaluate_memory_access_nested(void) {
    printf("Testing expression evaluator nested memory access...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_memory_access_nested");
    assert(own_fixture != NULL);
    
    // Get memory from fixture
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *user = ar_data__create_map();
    ar_data__set_map_string(user, "name", "Bob");
    ar_data__set_map_integer(user, "age", 30);
    ar_data__set_map_data(mut_memory, "user", user);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a memory access AST node for "memory.user.name"
    const char *path[] = {"user", "name"};
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("memory", path, 2);
    assert(own_ast != NULL);
    
    // When evaluating the nested memory access
    ar_data_t *ref_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the nested value (a reference, not owned)
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_result), "Bob") == 0);
    
    // Clean up (do NOT destroy result - it's a reference)
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate nested memory access\n");
}

/**
 * Test evaluating memory access for missing key
 */
static void test_evaluate_memory_access_missing(void) {
    printf("Testing expression ref_evaluator mut_memory access for missing key...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_memory_access_missing");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a memory access AST node for "memory.missing"
    const char *path[] = {"missing"};
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("memory", path, 1);
    assert(own_ast != NULL);
    
    // When evaluating the memory access for a missing key
    ar_data_t *ref_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return NULL
    assert(ref_result == NULL);
    
    // Clean up
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Return NULL for missing memory key\n");
}

/**
 * Test evaluating binary addition of integers
 */
static void test_evaluate_binary_op_add_integers(void) {
    printf("Testing expression evaluator binary op add integers...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_binary_op_add_integers");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a binary addition AST node for "5 + 3"
    ar_expression_ast_t *own_left = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *own_right = ar_expression_ast__create_literal_int(3);
    ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__ADD, own_left, own_right);
    assert(own_ast != NULL);
    
    // When evaluating the binary operation using evaluate
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the sum (a new owned value)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_result) == 8);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__take_ownership(own_result, ref_evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__drop_ownership(own_result, ref_evaluator);
    assert(transferred == true);
    
    // Clean up (MUST destroy result - it's owned)
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate binary addition of integers\n");
}

/**
 * Test evaluating binary multiplication of doubles
 */
static void test_evaluate_binary_op_multiply_doubles(void) {
    printf("Testing expression ref_evaluator binary op multiply doubles...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_binary_op_multiply_doubles");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a binary multiplication AST node for "2.5 * 4.0"
    ar_expression_ast_t *left = ar_expression_ast__create_literal_double(2.5);
    ar_expression_ast_t *right = ar_expression_ast__create_literal_double(4.0);
    ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__MULTIPLY, left, right);
    assert(own_ast != NULL);
    
    // When evaluating the binary operation
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the product (a new owned value)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(own_result) == 10.0);
    
    // Clean up (MUST destroy own_result - it's owned)
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate binary multiplication of doubles\n");
}

/**
 * Test evaluating binary string concatenation
 */
static void test_evaluate_binary_op_concatenate_strings(void) {
    printf("Testing expression ref_evaluator binary op concatenate strings...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_binary_op_concatenate_strings");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given a binary addition AST node for "Hello" + " World"
    ar_expression_ast_t *left = ar_expression_ast__create_literal_string("Hello");
    ar_expression_ast_t *right = ar_expression_ast__create_literal_string(" World");
    ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__ADD, left, right);
    assert(own_ast != NULL);
    
    // When evaluating the binary operation
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return the concatenated string (a new owned value)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_result), "Hello World") == 0);
    
    // Clean up (MUST destroy own_result - it's owned)
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate binary string concatenation\n");
}

/**
 * Test evaluate function handles integer literal when expecting binary op
 */
static void test_evaluate_handles_int_as_binary_op(void) {
    printf("Testing expression evaluator with integer when expecting binary op...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_handles_int_as_binary_op");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Given an integer literal AST node (not a binary op)
    ar_expression_ast_t *own_ast = ar_expression_ast__create_literal_int(42);
    assert(own_ast != NULL);
    
    // When evaluating with the general evaluate function
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return an integer value (not NULL)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_result) == 42);
    
    // Clean up
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate returns correct type for any expression\n");
}

/**
 * Test evaluating nested binary operations
 */
static void test_evaluate_binary_op_nested(void) {
    printf("Testing expression evaluator nested binary operations...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_binary_op_nested");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    
    // Add some values to memory
    ar_data__set_map_integer(mut_memory, "x", 10);
    ar_data__set_map_integer(mut_memory, "y", 5);
    
    // Given a nested binary operation AST node for "(memory.x + 2) * memory.y"
    // First create memory.x
    const char *path_x[] = {"x"};
    ar_expression_ast_t *own_mem_x = ar_expression_ast__create_memory_access("memory", path_x, 1);
    
    // Create memory.x + 2
    ar_expression_ast_t *own_two = ar_expression_ast__create_literal_int(2);
    ar_expression_ast_t *own_add = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__ADD, own_mem_x, own_two);
    
    // Create memory.y
    const char *path_y[] = {"y"};
    ar_expression_ast_t *own_mem_y = ar_expression_ast__create_memory_access("memory", path_y, 1);
    
    // Create (memory.x + 2) * memory.y
    ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__MULTIPLY, own_add, own_mem_y);
    assert(own_ast != NULL);
    
    // When evaluating the nested binary operation using evaluate
    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    
    // Then it should return (10 + 2) * 5 = 60 (a new owned value)
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_result) == 60);
    
    // Verify ownership: result should be unowned (we can claim it)
    bool held = ar_data__take_ownership(own_result, ref_evaluator);
    assert(held == true);  // Should succeed because nobody owns it
    
    // Transfer ownership to NULL so we can destroy it
    bool transferred = ar_data__drop_ownership(own_result, ref_evaluator);
    assert(transferred == true);
    
    // Clean up (MUST destroy result - it's owned)
    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Evaluate nested binary operations\n");
}

/**
 * Test evaluating string comparison
 */
static void test_evaluate_string_comparison(void) {
    printf("Testing expression evaluator string comparison...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_string_comparison");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    
    // Test 1: Direct string equality that should return 1
    {
        // Create AST for "hello" = "hello"
        ar_expression_ast_t *own_left = ar_expression_ast__create_literal_string("hello");
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_string("hello");
        ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
        assert(own_ast != NULL);
        
        // When evaluating the comparison
        ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
        
        // Then it should return 1 (true)
        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 1);
        
        // Clean up
        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }
    
    // Test 2: String inequality that should return 0
    {
        // Create AST for "hello" = "world"
        ar_expression_ast_t *own_left = ar_expression_ast__create_literal_string("hello");
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_string("world");
        ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
        assert(own_ast != NULL);
        
        // When evaluating the comparison
        ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
        
        // Then it should return 0 (false)
        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 0);
        
        // Clean up
        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }
    
    // Test 3: String comparison with memory access (like memory.operation = "add")
    {
        // Add operation = "add" to memory
        ar_data__set_map_string(mut_memory, "operation", "add");
        
        // Create AST for memory.operation = "add"
        const char *path[] = {"operation"};
        ar_expression_ast_t *own_left = ar_expression_ast__create_memory_access("memory", path, 1);
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_string("add");
        ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
        assert(own_ast != NULL);
        
        // When evaluating the comparison
        ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
        
        // Then it should return 1 (true)
        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 1);
        
        // Clean up
        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }
    
    // Test 4: String comparison returning 0
    {
        // memory.operation still = "add"
        
        // Create AST for memory.operation = "multiply"
        const char *path[] = {"operation"};
        ar_expression_ast_t *own_left = ar_expression_ast__create_memory_access("memory", path, 1);
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_string("multiply");
        ar_expression_ast_t *own_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
        assert(own_ast != NULL);
        
        // When evaluating the comparison
        ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
        
        // Then it should return 0 (false)
        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 0);
        
        // Clean up
        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }
    
    // Clean up
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ String comparison returns correct integer result\n");
}

/**
 * Test that accessing a field on a non-map value produces a detailed error message
 */
static void test_evaluate_type_mismatch_error_message(void) {
    printf("Testing expression evaluator type mismatch error message...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_type_mismatch_error");
    assert(own_fixture != NULL);
    
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    
    // Create a custom frame with a string message instead of a map
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_string("__wake__");
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    assert(own_frame != NULL);
    
    // Create an expression that tries to access a field on the string message: message.type_mismatch_test_field
    const char *path[] = {"type_mismatch_test_field"};
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("message", path, 1);
    assert(own_ast != NULL);
    
    // Clear any previous error messages
    ar_log__error(ref_log, NULL);
    
    // When evaluating the expression using our custom frame
    ar_data_t *result = ar_expression_evaluator__evaluate(ref_evaluator, own_frame, own_ast);
    
    // Then it should return NULL and log a detailed error message
    assert(result == NULL);
    
    // Check that a detailed error message was logged
    const char *error_msg = ar_log__get_last_error_message(ref_log);
    assert(error_msg != NULL);
    
    // Print the actual error message to see what we got
    printf("    Actual error message: '%s'\n", error_msg);
    
    // The error message should contain type information and field name
    assert(strstr(error_msg, "Cannot access field 'type_mismatch_test_field'") != NULL);
    assert(strstr(error_msg, "STRING") != NULL);
    assert(strstr(error_msg, "__wake__") != NULL);
    
    // Clean up
    ar_expression_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_context);
    ar_data__destroy(own_message);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Type mismatch produces detailed error message\n");
    printf("    (The error above 'Cannot access field 'type_mismatch_test_field' on STRING value \"__wake__\"' was expected)\n");
}

int main(void) {
    printf("\n=== Expression Evaluator Tests ===\n\n");
    
    test_create_destroy_with_log();
    test_create_with_log_only();
    test_evaluate_literal_int();
    test_evaluate_handles_string_literal();
    test_evaluate_literal_double();
    test_evaluate_handles_int_as_double();
    test_evaluate_literal_string();
    test_evaluate_handles_int_as_string();
    test_evaluate_literal_string_empty();
    test_evaluate_memory_access();
    test_evaluate_handles_int_as_memory_access();
    test_evaluate_memory_access_nested();
    test_evaluate_memory_access_missing();
    test_evaluate_binary_op_add_integers();
    test_evaluate_binary_op_multiply_doubles();
    test_evaluate_binary_op_concatenate_strings();
    test_evaluate_handles_int_as_binary_op();
    test_evaluate_binary_op_nested();
    test_evaluate_string_comparison();
    test_evaluate_type_mismatch_error_message();
    
    printf("\nAll expression_evaluator tests passed!\n");
    return 0;
}
