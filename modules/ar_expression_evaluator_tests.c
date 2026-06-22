/**
 * @file ar_expression_evaluator_tests.c
 * @brief Test suite for expression evaluator module
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ar_expression_evaluator.h"
#include "ar_expression_ast.h"
#include "ar_expression_parser.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"
#include "ar_assert.h"

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

static void test_evaluate_empty_list_comparison(void) {
    printf("Testing expression evaluator empty list comparison...\n");

    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_empty_list_comparison");
    assert(own_fixture != NULL);

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_evaluator =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    assert(ref_frame != NULL);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_data_t *own_payloads = ar_data__create_list();
    assert(own_payloads != NULL);
    assert(ar_data__set_map_data(mut_memory, "payloads", own_payloads));
    own_payloads = NULL;

    {
        const char *path[] = {"payloads"};
        ar_expression_ast_t *own_left =
            ar_expression_ast__create_memory_access("memory", path, 1);
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_list(NULL, 0);
        ar_expression_ast_t *own_ast =
            ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
        assert(own_ast != NULL);

        ar_data_t *own_result =
            ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);

        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 1);

        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }

    ar_data_t *ref_payloads = ar_data__get_map_data(mut_memory, "payloads");
    assert(ref_payloads != NULL);
    assert(ar_data__list_add_last_string(ref_payloads, "opaque"));

    {
        const char *path[] = {"payloads"};
        ar_expression_ast_t *own_left =
            ar_expression_ast__create_memory_access("memory", path, 1);
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_list(NULL, 0);
        ar_expression_ast_t *own_ast =
            ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
        assert(own_ast != NULL);

        ar_data_t *own_result =
            ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);

        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 0);

        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }

    {
        const char *path[] = {"payloads"};
        ar_expression_ast_t *own_left =
            ar_expression_ast__create_memory_access("memory", path, 1);
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_list(NULL, 0);
        ar_expression_ast_t *own_ast =
            ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__NOT_EQUAL,
                                                own_left,
                                                own_right);
        assert(own_ast != NULL);

        ar_data_t *own_result =
            ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);

        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 1);

        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }

    assert(ar_data__set_map_integer(mut_memory, "empty_sentinel", 0));

    {
        const char *path[] = {"empty_sentinel"};
        ar_expression_ast_t *own_left =
            ar_expression_ast__create_memory_access("memory", path, 1);
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_list(NULL, 0);
        ar_expression_ast_t *own_ast =
            ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
        assert(own_ast != NULL);

        ar_data_t *own_result =
            ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);

        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 0);

        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }

    {
        const char *path[] = {"empty_sentinel"};
        ar_expression_ast_t *own_left =
            ar_expression_ast__create_memory_access("memory", path, 1);
        ar_expression_ast_t *own_right = ar_expression_ast__create_literal_list(NULL, 0);
        ar_expression_ast_t *own_ast =
            ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__NOT_EQUAL,
                                                own_left,
                                                own_right);
        assert(own_ast != NULL);

        ar_data_t *own_result =
            ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);

        assert(own_result != NULL);
        assert(ar_data__get_type(own_result) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(own_result) == 1);

        ar_data__destroy(own_result);
        ar_expression_ast__destroy(own_ast);
    }

    assert(ar_log__get_last_error_message(ref_log) == NULL);

    ar_evaluator_fixture__destroy(own_fixture);
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
    ar_data_t *own_message = ar_data__create_string("test_string_value");
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
    assert(strstr(error_msg, "test_string_value") != NULL);
    
    // Clean up
    ar_expression_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_context);
    ar_data__destroy(own_message);
    ar_evaluator_fixture__destroy(own_fixture);
    
    printf("  ✓ Type mismatch produces detailed error message\n");
    printf("    (The error above 'Cannot access field 'type_mismatch_test_field' on STRING value \"test_string_value\"' was expected)\n");
}

static void test_evaluate_list_literal(void) {
    printf("Testing expression evaluator list literal...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_list_literal");
    assert(own_fixture != NULL);

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    ar_expression_parser_t *own_parser = ar_expression_parser__create(ref_log, "[1, \"two\", {n: 3}]");
    assert(own_parser != NULL);
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    assert(own_ast != NULL);

    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);

    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(own_result) == 3);

    ar_data_t **own_items = ar_data__list_items(own_result);
    assert(own_items != NULL);
    assert(ar_data__get_integer(own_items[0]) == 1);
    assert(strcmp(ar_data__get_string(own_items[1]), "two") == 0);
    assert(ar_data__get_type(own_items[2]) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_integer(own_items[2], "n") == 3);
    AR__HEAP__FREE(own_items);

    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_map_literal(void) {
    printf("Testing expression evaluator map literal...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_map_literal");
    assert(own_fixture != NULL);

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_evaluator = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    ar_expression_parser_t *own_parser = ar_expression_parser__create(ref_log, "{name: \"Ada\", values: [1, 2]}");
    assert(own_parser != NULL);
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    assert(own_ast != NULL);

    ar_data_t *own_result = ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);

    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(own_result, "name"), "Ada") == 0);

    ar_data_t *ref_values = ar_data__get_map_data(own_result, "values");
    assert(ref_values != NULL);
    assert(ar_data__get_type(ref_values) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(ref_values) == 2);

    ar_data__destroy(own_result);
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_evaluator_fixture__destroy(own_fixture);
}

static size_t _map_key_count(const ar_data_t *ref_map) {
    ar_data_t *own_keys = ar_data__get_map_keys(ref_map);
    assert(own_keys != NULL);
    size_t count = ar_data__list_count(own_keys);
    ar_data__destroy(own_keys);
    return count;
}

static ar_data_t *_evaluate_expression_text(
    ar_evaluator_fixture_t *own_fixture,
    const char *ref_expression
) {
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_evaluator =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    ar_expression_parser_t *own_parser =
        ar_expression_parser__create(ref_log, ref_expression);
    assert(own_parser != NULL);
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    assert(own_ast != NULL);

    ar_data_t *own_result =
        ar_expression_evaluator__evaluate(ref_evaluator, ref_frame, own_ast);
    assert(own_result != NULL);

    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);

    // Ownership transferred to caller
    return own_result;
}

static void _assert_string_value(ar_data_t *ref_data, const char *ref_expected) {
    AR_ASSERT(ref_data != NULL, "Result should not be NULL");
    AR_ASSERT(ar_data__get_type(ref_data) == AR_DATA_TYPE__STRING,
              "Result should be a string");
    AR_ASSERT(strcmp(ar_data__get_string(ref_data), ref_expected) == 0,
              "String result should match expected value");
}

static void _assert_integer_value(ar_data_t *ref_data, int expected) {
    AR_ASSERT(ref_data != NULL, "Result should not be NULL");
    AR_ASSERT(ar_data__get_type(ref_data) == AR_DATA_TYPE__INTEGER,
              "Result should be an integer");
    AR_ASSERT(ar_data__get_integer(ref_data) == expected,
              "Integer result should match expected value");
}

static void _assert_empty_list(ar_data_t *ref_data) {
    AR_ASSERT(ref_data != NULL, "Result should not be NULL");
    AR_ASSERT(ar_data__get_type(ref_data) == AR_DATA_TYPE__LIST,
              "Result should be a list");
    AR_ASSERT(ar_data__list_count(ref_data) == 0,
              "List result should be empty");
}

static void test_evaluate_if_function_call_truthiness(void) {
    printf("Testing expression evaluator if function call truthiness...\n");

    // Given a fixture for evaluating pure if() calls
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_if_function_call_truthiness");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When the condition is a nonzero integer
    ar_data_t *own_true = _evaluate_expression_text(own_fixture, "if(7, \"yes\", \"no\")");

    // Then the true branch should be selected
    _assert_string_value(own_true, "yes");
    ar_data__destroy(own_true);

    // When the condition is integer zero
    ar_data_t *own_false = _evaluate_expression_text(own_fixture, "if(0, \"yes\", \"no\")");

    // Then the false branch should be selected
    _assert_string_value(own_false, "no");
    ar_data__destroy(own_false);

    // When the condition is not an integer
    ar_data_t *own_non_integer = _evaluate_expression_text(
        own_fixture,
        "if(\"truthy-looking\", \"yes\", \"no\")"
    );

    // Then the false branch should be selected
    _assert_string_value(own_non_integer, "no");
    ar_data__destroy(own_non_integer);

    // When the condition cannot produce a value
    ar_data_t *own_missing = _evaluate_expression_text(
        own_fixture,
        "if(memory.missing_condition, \"yes\", \"no\")"
    );

    // Then it should behave like a false condition
    _assert_string_value(own_missing, "no");
    ar_data__destroy(own_missing);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_if_function_call_composition(void) {
    printf("Testing expression evaluator if function call composition...\n");

    // Given a fixture for evaluating if() in composite expressions
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_if_function_call_composition");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating if() around nested pure calls
    ar_data_t *own_nested = _evaluate_expression_text(
        own_fixture,
        "if(1, head([4, 5]), head(tail([4, 5])))"
    );

    // Then nested pure calls should compose in selected branches
    _assert_integer_value(own_nested, 4);
    ar_data__destroy(own_nested);

    // When evaluating a list literal containing if()
    ar_data_t *own_list = _evaluate_expression_text(
        own_fixture,
        "[if(1, 10, 20), if(0, 30, 40)]"
    );

    // Then the literal should contain selected branch values
    AR_ASSERT(ar_data__get_type(own_list) == AR_DATA_TYPE__LIST,
              "Outer result should be a list");
    AR_ASSERT(ar_data__list_count(own_list) == 2,
              "Outer list should contain two items");
    ar_data_t **own_items = ar_data__list_items(own_list);
    AR_ASSERT(own_items != NULL, "List items should be available");
    _assert_integer_value(own_items[0], 10);
    _assert_integer_value(own_items[1], 40);
    AR__HEAP__FREE(own_items);
    ar_data__destroy(own_list);

    // When evaluating a map literal containing if()
    ar_data_t *own_map = _evaluate_expression_text(
        own_fixture,
        "{status: if(1, \"ready\", \"blocked\")}"
    );

    // Then the map value should be the selected branch
    AR_ASSERT(ar_data__get_type(own_map) == AR_DATA_TYPE__MAP,
              "Outer result should be a map");
    const char *ref_status = ar_data__get_map_string(own_map, "status");
    AR_ASSERT(ref_status != NULL, "Map should contain a status string");
    AR_ASSERT(strcmp(ref_status, "ready") == 0,
              "Map should store the selected branch value");
    ar_data__destroy(own_map);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_if_function_call_is_lazy(void) {
    printf("Testing expression evaluator if function call laziness...\n");

    // Given a fixture with a branch expression that would fail if evaluated
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_if_function_call_is_lazy");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_string(mut_memory, "poison", "not-a-map"),
              "Memory setup should store a non-map value");
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);

    // When the invalid false branch is not selected
    ar_data_t *own_true = _evaluate_expression_text(
        own_fixture,
        "if(1, \"selected\", memory.poison.field)"
    );

    // Then only the true branch should be evaluated
    _assert_string_value(own_true, "selected");
    AR_ASSERT(ar_log__get_last_error_message(ref_log) == NULL,
              "Unselected false branch should not log an evaluation error");
    ar_data__destroy(own_true);

    // When the invalid true branch is not selected
    ar_data_t *own_false = _evaluate_expression_text(
        own_fixture,
        "if(0, memory.poison.field, \"selected\")"
    );

    // Then only the false branch should be evaluated
    _assert_string_value(own_false, "selected");
    AR_ASSERT(ar_log__get_last_error_message(ref_log) == NULL,
              "Unselected true branch should not log an evaluation error");
    ar_data__destroy(own_false);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_if_selected_invalid_branch_returns_zero(void) {
    printf("Testing expression evaluator if selected invalid branch fallback...\n");

    // Given a fixture for evaluating fallback behavior
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_if_selected_invalid_branch_returns_zero");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When the selected branch cannot produce a value
    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "if(1, memory.missing_value, 9)"
    );

    // Then expression-level if() should still return a reasonable value
    _assert_integer_value(own_result, 0);
    ar_data__destroy(own_result);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_if_is_path_neutral_for_memory_self(void) {
    printf("Testing expression evaluator if path-neutral memory.self handling...\n");

    // Given memory.self contains an ordinary map value
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_if_is_path_neutral_for_memory_self");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_self_map = ar_data__create_map();
    AR_ASSERT(own_self_map != NULL, "Self map creation should succeed");
    AR_ASSERT(ar_data__set_map_integer(own_self_map, "enabled", 1),
              "Self map should store enabled flag");
    AR_ASSERT(ar_data__set_map_integer(own_self_map, "value", 42),
              "Self map should store branch value");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "self", own_self_map),
              "Memory setup should store self map");

    // When memory.self and nested memory.self paths are used as if() arguments
    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "if(memory.self.enabled, memory.self.value, 0)"
    );

    // Then they should be treated as ordinary argument values
    _assert_integer_value(own_result, 42);
    ar_data__destroy(own_result);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_head_function_call(void) {
    printf("Testing expression evaluator head function call...\n");

    // Given a fixture for evaluating pure expression calls
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_head_function_call");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating head() against a non-empty list
    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "head([1, 2, 3])"
    );

    // Then it should return the first item
    _assert_integer_value(own_result, 1);

    // Cleanup
    ar_data__destroy(own_result);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_tail_function_call(void) {
    printf("Testing expression evaluator tail function call...\n");

    // Given a fixture for evaluating pure expression calls
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_tail_function_call");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating tail() against a non-empty list
    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "tail([1, 2, 3])"
    );

    // Then it should return a list containing every item after the first
    AR_ASSERT(ar_data__get_type(own_result) == AR_DATA_TYPE__LIST,
              "Tail result should be a list");
    AR_ASSERT(ar_data__list_count(own_result) == 2,
              "Tail result should contain remaining items");
    ar_data_t **own_items = ar_data__list_items(own_result);
    AR_ASSERT(own_items != NULL, "Tail items should be available");
    _assert_integer_value(own_items[0], 2);
    _assert_integer_value(own_items[1], 3);
    AR__HEAP__FREE(own_items);

    // Cleanup
    ar_data__destroy(own_result);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_head_tail_sentinel_cases(void) {
    printf("Testing expression evaluator head/tail sentinel cases...\n");

    // Given a fixture for evaluating head() and tail() fallback cases
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_head_tail_sentinel_cases");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating head() against an empty list
    ar_data_t *own_empty_head = _evaluate_expression_text(own_fixture, "head([])");

    // Then it should return integer zero
    _assert_integer_value(own_empty_head, 0);
    ar_data__destroy(own_empty_head);

    // When evaluating tail() against an empty list
    ar_data_t *own_empty_tail = _evaluate_expression_text(own_fixture, "tail([])");

    // Then it should return a new empty list
    _assert_empty_list(own_empty_tail);
    ar_data__destroy(own_empty_tail);

    // When evaluating tail() against a singleton list
    ar_data_t *own_single_tail = _evaluate_expression_text(own_fixture, "tail([42])");

    // Then it should return a new empty list
    _assert_empty_list(own_single_tail);
    ar_data__destroy(own_single_tail);

    // When evaluating head() against a missing value
    ar_data_t *own_head_missing = _evaluate_expression_text(own_fixture, "head(memory.missing)");

    // Then it should return integer zero
    _assert_integer_value(own_head_missing, 0);
    ar_data__destroy(own_head_missing);

    // When evaluating tail() against a missing value
    ar_data_t *own_tail_missing = _evaluate_expression_text(own_fixture, "tail(memory.missing)");

    // Then it should return integer zero
    _assert_integer_value(own_tail_missing, 0);
    ar_data__destroy(own_tail_missing);

    // When evaluating head() against a non-list value
    ar_data_t *own_head_non_list = _evaluate_expression_text(own_fixture, "head(42)");

    // Then it should return integer zero
    _assert_integer_value(own_head_non_list, 0);
    ar_data__destroy(own_head_non_list);

    // When evaluating tail() against a non-list value
    ar_data_t *own_tail_non_list = _evaluate_expression_text(own_fixture, "tail(\"not-list\")");

    // Then it should return integer zero
    _assert_integer_value(own_tail_non_list, 0);
    ar_data__destroy(own_tail_non_list);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_head_tail_nested_composition(void) {
    printf("Testing expression evaluator head/tail nested composition...\n");

    // Given a fixture for evaluating nested pure expression calls
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_head_tail_nested_composition");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating head() over a tail() result
    ar_data_t *own_second = _evaluate_expression_text(
        own_fixture,
        "head(tail([1, 2, 3]))"
    );

    // Then it should return the second item from the original list
    _assert_integer_value(own_second, 2);
    ar_data__destroy(own_second);

    // When evaluating tail() over a tail() result
    ar_data_t *own_last_list = _evaluate_expression_text(
        own_fixture,
        "tail(tail([1, 2, 3]))"
    );

    // Then it should return a list containing only the final item
    AR_ASSERT(ar_data__get_type(own_last_list) == AR_DATA_TYPE__LIST,
              "Nested tail result should be a list");
    AR_ASSERT(ar_data__list_count(own_last_list) == 1,
              "Nested tail should keep only the last item");
    ar_data_t **own_items = ar_data__list_items(own_last_list);
    AR_ASSERT(own_items != NULL, "Nested tail item should be available");
    _assert_integer_value(own_items[0], 3);
    AR__HEAP__FREE(own_items);
    ar_data__destroy(own_last_list);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_head_tail_function_call_in_literals(void) {
    printf("Testing expression evaluator head/tail function call in literals...\n");

    // Given a fixture for evaluating pure calls inside literal expressions
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_head_tail_function_call_in_literals");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating a list literal containing head() and tail()
    ar_data_t *own_list = _evaluate_expression_text(
        own_fixture,
        "[head([1, 2]), tail([1, 2, 3])]"
    );

    // Then the literal should contain the evaluated call results
    AR_ASSERT(ar_data__get_type(own_list) == AR_DATA_TYPE__LIST,
              "Outer literal should evaluate to a list");
    AR_ASSERT(ar_data__list_count(own_list) == 2,
              "Outer literal should contain both call results");
    ar_data_t **own_list_items = ar_data__list_items(own_list);
    AR_ASSERT(own_list_items != NULL, "Outer list items should be available");
    _assert_integer_value(own_list_items[0], 1);
    AR_ASSERT(ar_data__get_type(own_list_items[1]) == AR_DATA_TYPE__LIST,
              "Tail literal item should be a list");
    AR_ASSERT(ar_data__list_count(own_list_items[1]) == 2,
              "Tail literal item should contain remaining items");
    AR__HEAP__FREE(own_list_items);
    ar_data__destroy(own_list);

    // When evaluating a map literal containing head() and tail()
    ar_data_t *own_map = _evaluate_expression_text(
        own_fixture,
        "{first: head([5, 6]), rest: tail([5, 6, 7])}"
    );

    // Then the map should contain the evaluated call results
    AR_ASSERT(ar_data__get_type(own_map) == AR_DATA_TYPE__MAP,
              "Outer literal should evaluate to a map");
    AR_ASSERT(ar_data__get_map_integer(own_map, "first") == 5,
              "Map should store head result");
    ar_data_t *ref_rest = ar_data__get_map_data(own_map, "rest");
    AR_ASSERT(ref_rest != NULL, "Map should store tail result");
    AR_ASSERT(ar_data__get_type(ref_rest) == AR_DATA_TYPE__LIST,
              "Tail map value should be a list");
    AR_ASSERT(ar_data__list_count(ref_rest) == 2,
              "Tail map value should contain remaining items");
    ar_data__destroy(own_map);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_head_tail_is_path_neutral_for_memory_self(void) {
    printf("Testing expression evaluator head/tail path-neutral memory.self handling...\n");

    // Given memory.self contains a list value
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_head_tail_is_path_neutral_for_memory_self");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // Given memory.self is populated with list values
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_self_list = ar_data__create_list();
    AR_ASSERT(own_self_list != NULL, "Self list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_self_list, 7),
              "Self list setup should store first value");
    AR_ASSERT(ar_data__list_add_last_integer(own_self_list, 8),
              "Self list setup should store second value");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "self", own_self_list),
              "Memory setup should store self list");

    // When evaluating head() with memory.self as the argument
    ar_data_t *own_head_result = _evaluate_expression_text(own_fixture, "head(memory.self)");

    // Then memory.self should be treated as an ordinary value path
    _assert_integer_value(own_head_result, 7);
    ar_data__destroy(own_head_result);

    // When evaluating tail() with memory.self as the argument
    ar_data_t *own_tail_result = _evaluate_expression_text(own_fixture, "tail(memory.self)");

    // Then memory.self should be treated as an ordinary value path
    AR_ASSERT(ar_data__get_type(own_tail_result) == AR_DATA_TYPE__LIST,
              "Tail result should be a list for memory.self list value");
    AR_ASSERT(ar_data__list_count(own_tail_result) == 1,
              "Tail result should contain one item");
    ar_data_t **own_tail_items = ar_data__list_items(own_tail_result);
    AR_ASSERT(own_tail_items != NULL, "Tail item should be available");
    _assert_integer_value(own_tail_items[0], 8);
    AR__HEAP__FREE(own_tail_items);
    ar_data__destroy(own_tail_result);

    // Given memory.self contains a nested list path
    ar_data_t *own_nested_items = ar_data__create_list();
    AR_ASSERT(own_nested_items != NULL, "Nested items list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_nested_items, 11),
              "Nested list setup should store first value");
    AR_ASSERT(ar_data__list_add_last_integer(own_nested_items, 12),
              "Nested list setup should store second value");
    ar_data_t *own_self_map = ar_data__create_map();
    AR_ASSERT(own_self_map != NULL, "Self map creation should succeed");
    AR_ASSERT(ar_data__set_map_data(own_self_map, "items", own_nested_items),
              "Self map setup should store nested list");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "self", own_self_map),
              "Memory setup should replace self with nested map");

    // When evaluating head() with a nested memory.self path argument
    ar_data_t *own_nested_head = _evaluate_expression_text(own_fixture, "head(memory.self.items)");

    // Then the nested path should also be treated as an ordinary value path
    _assert_integer_value(own_nested_head, 11);
    ar_data__destroy(own_nested_head);

    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_parse_function_call(void) {
    printf("Testing expression evaluator parse function call...\n");

    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_parse_function_call");
    assert(own_fixture != NULL);

    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "parse(\"name={name}\", \"name=Ada\")"
    );

    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__MAP);
    ar_data_t *ref_name = ar_data__get_map_data(own_result, "name");
    assert(ref_name != NULL);
    assert(ar_data__get_type(ref_name) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_name), "Ada") == 0);

    ar_data__destroy(own_result);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_parse_function_call_in_literals(void) {
    printf("Testing expression evaluator parse function call in literals...\n");

    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_parse_function_call_in_literals");
    assert(own_fixture != NULL);

    ar_data_t *own_list = _evaluate_expression_text(
        own_fixture,
        "[parse(\"name={name}\", \"name=Ada\")]"
    );
    assert(ar_data__get_type(own_list) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(own_list) == 1);
    ar_data_t **own_items = ar_data__list_items(own_list);
    assert(own_items != NULL);
    assert(ar_data__get_type(own_items[0]) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(own_items[0], "name"), "Ada") == 0);
    AR__HEAP__FREE(own_items);
    ar_data__destroy(own_list);

    ar_data_t *own_map = _evaluate_expression_text(
        own_fixture,
        "{payload: parse(\"id={id}\", \"id=42\")}"
    );
    assert(ar_data__get_type(own_map) == AR_DATA_TYPE__MAP);
    ar_data_t *ref_payload = ar_data__get_map_data(own_map, "payload");
    assert(ref_payload != NULL);
    assert(ar_data__get_type(ref_payload) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_integer(ref_payload, "id") == 42);
    ar_data__destroy(own_map);

    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_parse_returns_empty_map_for_bad_inputs(void) {
    printf("Testing expression evaluator parse returns empty map for bad inputs...\n");

    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_parse_returns_empty_map_for_bad_inputs");
    assert(own_fixture != NULL);

    ar_data_t *own_non_matching = _evaluate_expression_text(
        own_fixture,
        "parse(\"name={name}\", \"user=Ada\")"
    );
    assert(ar_data__get_type(own_non_matching) == AR_DATA_TYPE__MAP);
    assert(_map_key_count(own_non_matching) == 0);
    ar_data__destroy(own_non_matching);

    ar_data_t *own_malformed = _evaluate_expression_text(
        own_fixture,
        "parse(\"name={name\", \"name=Ada\")"
    );
    assert(ar_data__get_type(own_malformed) == AR_DATA_TYPE__MAP);
    assert(_map_key_count(own_malformed) == 0);
    ar_data__destroy(own_malformed);

    ar_data_t *own_bad_types = _evaluate_expression_text(
        own_fixture,
        "parse([1], {value: 2})"
    );
    assert(ar_data__get_type(own_bad_types) == AR_DATA_TYPE__MAP);
    assert(_map_key_count(own_bad_types) == 0);
    ar_data__destroy(own_bad_types);

    ar_data_t *own_missing = _evaluate_expression_text(
        own_fixture,
        "parse(\"name={name}\", memory.missing)"
    );
    assert(ar_data__get_type(own_missing) == AR_DATA_TYPE__MAP);
    assert(_map_key_count(own_missing) == 0);
    ar_data__destroy(own_missing);

    ar_data_t *own_ambiguous_parent = _evaluate_expression_text(
        own_fixture,
        "parse(\"{a.b}-{a}\", \"1-2\")"
    );
    assert(ar_data__get_type(own_ambiguous_parent) == AR_DATA_TYPE__MAP);
    assert(_map_key_count(own_ambiguous_parent) == 0);
    ar_data__destroy(own_ambiguous_parent);

    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_parse_is_path_neutral_for_memory_self(void) {
    printf("Testing expression evaluator parse path-neutral memory.self handling...\n");

    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_parse_is_path_neutral_for_memory_self");
    assert(own_fixture != NULL);

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_integer(mut_memory, "self", 7));

    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "parse(\"{self}\", memory.self)"
    );
    assert(ar_data__get_type(own_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_integer(own_result, "self") == 7);
    ar_data__destroy(own_result);

    ar_data_t *own_nested = _evaluate_expression_text(
        own_fixture,
        "parse(\"{self.anything}\", \"99\")"
    );
    assert(ar_data__get_type(own_nested) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_integer(own_nested, "self.anything") == 99);
    ar_data__destroy(own_nested);

    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_parse_preserves_borrowed_frame_maps(void) {
    printf("Testing expression evaluator parse preserves borrowed frame maps...\n");

    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_parse_preserves_borrowed_frame_maps");
    assert(own_fixture != NULL);

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_string(mut_memory, "sentinel", "alive"));

    ar_data_t *own_template_result = _evaluate_expression_text(
        own_fixture,
        "parse(memory, \"ignored\")"
    );
    assert(ar_data__get_type(own_template_result) == AR_DATA_TYPE__MAP);
    assert(_map_key_count(own_template_result) == 0);
    ar_data__destroy(own_template_result);
    assert(strcmp(ar_data__get_map_string(mut_memory, "sentinel"), "alive") == 0);

    ar_data_t *own_input_result = _evaluate_expression_text(
        own_fixture,
        "parse(\"{value}\", memory)"
    );
    assert(ar_data__get_type(own_input_result) == AR_DATA_TYPE__MAP);
    assert(_map_key_count(own_input_result) == 0);
    ar_data__destroy(own_input_result);
    assert(strcmp(ar_data__get_map_string(mut_memory, "sentinel"), "alive") == 0);

    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_build_function_call(void) {
    printf("Testing expression evaluator build function call...\n");

    // Given a fixture with an expression evaluator
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_build_function_call");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating a pure build() expression
    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "build(\"Hello {name}!\", {name: \"Ada\"})"
    );

    // Then it should return the built string
    _assert_string_value(own_result, "Hello Ada!");

    ar_data__destroy(own_result);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_build_function_call_in_literals(void) {
    printf("Testing expression evaluator build function call in literals...\n");

    // Given a fixture with an expression evaluator
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_build_function_call_in_literals");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When evaluating a list literal containing build()
    ar_data_t *own_list = _evaluate_expression_text(
        own_fixture,
        "[build(\"id={id}\", {id: 42})]"
    );

    // Then the nested build() result should be stored as a string value
    AR_ASSERT(ar_data__get_type(own_list) == AR_DATA_TYPE__LIST,
              "List literal result should be a list");
    AR_ASSERT(ar_data__list_count(own_list) == 1,
              "List literal should contain one item");
    ar_data_t **own_items = ar_data__list_items(own_list);
    AR_ASSERT(own_items != NULL, "List items should be available");
    _assert_string_value(own_items[0], "id=42");
    AR__HEAP__FREE(own_items);
    ar_data__destroy(own_list);

    // When evaluating a map literal containing build()
    ar_data_t *own_map = _evaluate_expression_text(
        own_fixture,
        "{payload: build(\"name={name}\", {name: \"Ada\"})}"
    );

    // Then the nested build() result should be stored as a string field
    AR_ASSERT(ar_data__get_type(own_map) == AR_DATA_TYPE__MAP,
              "Map literal result should be a map");
    ar_data_t *ref_payload = ar_data__get_map_data(own_map, "payload");
    _assert_string_value(ref_payload, "name=Ada");
    ar_data__destroy(own_map);

    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_build_returns_string_for_bad_inputs(void) {
    printf("Testing expression evaluator build returns string for bad inputs...\n");

    // Given a fixture with an expression evaluator
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_build_returns_string_for_bad_inputs");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // When the values argument is not a map
    ar_data_t *own_non_map_values = _evaluate_expression_text(
        own_fixture,
        "build(\"Name: {name}\", \"not a map\")"
    );

    // Then unresolved placeholders should remain unchanged
    _assert_string_value(own_non_map_values, "Name: {name}");
    ar_data__destroy(own_non_map_values);

    // When the template argument is not a string
    ar_data_t *own_non_string_template = _evaluate_expression_text(
        own_fixture,
        "build({template: \"Name: {name}\"}, {name: \"Ada\"})"
    );

    // Then build() should return a safe string fallback
    _assert_string_value(own_non_string_template, "");
    ar_data__destroy(own_non_string_template);

    // When a placeholder value cannot be converted for building
    ar_data_t *own_non_string_value = _evaluate_expression_text(
        own_fixture,
        "build(\"items={items}\", {items: [1, 2]})"
    );

    // Then the placeholder should remain unchanged
    _assert_string_value(own_non_string_value, "items={items}");
    ar_data__destroy(own_non_string_value);

    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_build_is_path_neutral_for_memory_self(void) {
    printf("Testing expression evaluator build path-neutral memory.self handling...\n");

    // Given memory with a field named self
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_build_is_path_neutral_for_memory_self");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_integer(mut_memory, "self", 7),
              "Memory setup should store self value");

    // When build() receives the evaluated memory map
    ar_data_t *own_map_path = _evaluate_expression_text(
        own_fixture,
        "build(\"{self}\", memory)"
    );

    // Then it should use the map's self field
    _assert_string_value(own_map_path, "7");
    ar_data__destroy(own_map_path);

    // When build() receives an equivalent literal map
    ar_data_t *own_literal_map = _evaluate_expression_text(
        own_fixture,
        "build(\"{self}\", {self: 7})"
    );

    // Then it should behave the same way
    _assert_string_value(own_literal_map, "7");
    ar_data__destroy(own_literal_map);

    // When build() receives the evaluated memory.self integer
    ar_data_t *own_memory_self = _evaluate_expression_text(
        own_fixture,
        "build(\"{self}\", memory.self)"
    );

    // Then the non-map value should leave the placeholder unresolved
    _assert_string_value(own_memory_self, "{self}");
    ar_data__destroy(own_memory_self);

    // When build() receives an equivalent integer literal
    ar_data_t *own_literal_integer = _evaluate_expression_text(
        own_fixture,
        "build(\"{self}\", 7)"
    );

    // Then it should behave the same way
    _assert_string_value(own_literal_integer, "{self}");
    ar_data__destroy(own_literal_integer);

    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_evaluate_build_preserves_borrowed_frame_maps(void) {
    printf("Testing expression evaluator build preserves borrowed frame maps...\n");

    // Given memory whose lifetime is owned by the frame
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_evaluate_build_preserves_borrowed_frame_maps");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_string(mut_memory, "sentinel", "alive"),
              "Memory setup should store sentinel value");

    // When build() reads the root memory map as values
    ar_data_t *own_result = _evaluate_expression_text(
        own_fixture,
        "build(\"sentinel={sentinel}\", memory)"
    );

    // Then it should build from the map without destroying frame memory
    _assert_string_value(own_result, "sentinel=alive");
    ar_data__destroy(own_result);
    const char *ref_sentinel = ar_data__get_map_string(mut_memory, "sentinel");
    AR_ASSERT(ref_sentinel != NULL, "Frame memory should retain sentinel");
    AR_ASSERT(strcmp(ref_sentinel, "alive") == 0,
              "Frame memory value should remain unchanged");

    // When build() reads the root memory map as a non-string template
    ar_data_t *own_template_result = _evaluate_expression_text(
        own_fixture,
        "build(memory, memory)"
    );

    // Then it should return the fallback string without destroying frame memory
    _assert_string_value(own_template_result, "");
    ar_data__destroy(own_template_result);
    ref_sentinel = ar_data__get_map_string(mut_memory, "sentinel");
    AR_ASSERT(ref_sentinel != NULL, "Frame memory should still retain sentinel");
    AR_ASSERT(strcmp(ref_sentinel, "alive") == 0,
              "Frame memory value should still remain unchanged");

    ar_evaluator_fixture__destroy(own_fixture);
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
    test_evaluate_empty_list_comparison();
    test_evaluate_type_mismatch_error_message();
    test_evaluate_list_literal();
    test_evaluate_map_literal();
    test_evaluate_parse_function_call();
    test_evaluate_parse_function_call_in_literals();
    test_evaluate_parse_returns_empty_map_for_bad_inputs();
    test_evaluate_parse_is_path_neutral_for_memory_self();
    test_evaluate_parse_preserves_borrowed_frame_maps();
    test_evaluate_build_function_call();
    test_evaluate_build_function_call_in_literals();
    test_evaluate_build_returns_string_for_bad_inputs();
    test_evaluate_build_is_path_neutral_for_memory_self();
    test_evaluate_build_preserves_borrowed_frame_maps();
    test_evaluate_if_function_call_truthiness();
    test_evaluate_if_function_call_composition();
    test_evaluate_if_function_call_is_lazy();
    test_evaluate_if_selected_invalid_branch_returns_zero();
    test_evaluate_if_is_path_neutral_for_memory_self();
    test_evaluate_head_function_call();
    test_evaluate_tail_function_call();
    test_evaluate_head_tail_sentinel_cases();
    test_evaluate_head_tail_nested_composition();
    test_evaluate_head_tail_function_call_in_literals();
    test_evaluate_head_tail_is_path_neutral_for_memory_self();
    
    printf("\nAll expression_evaluator tests passed!\n");
    return 0;
}
