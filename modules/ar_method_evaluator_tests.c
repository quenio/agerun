/**
 * @file ar_method_evaluator_tests.c
 * @brief Test suite for method evaluator module
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_method_evaluator.h"
#include "ar_method.h"
#include "ar_method_ast.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_evaluator.h"
#include "ar_frame.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_io.h"

/**
 * Test creating and destroying a method evaluator
 */
static void test_method_evaluator__create_destroy(void) {
    printf("Testing method evaluator create/destroy...\n");
    
    // Given a method with parsed ASTs
    method_t *method = ar__method__create("test", "memory.x := 42", "1.0.0");
    assert(method != NULL);
    
    // When creating a method evaluator
    ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_method_evaluator__destroy(evaluator);
    
    // Clean up
    ar__method__destroy(method);
    
    printf("  ✓ Create and destroy method evaluator\n");
}

/**
 * Test evaluating a method with a single assignment
 */
static void test_method_evaluator__evaluate_simple_assignment(void) {
    printf("Testing method evaluator simple assignment...\n");
    
    // Given a method with an assignment instruction
    method_t *method = ar__method__create("assign_test", "memory.result := 100", "1.0.0");
    assert(method != NULL);
    
    // And a method evaluator
    ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);
    assert(evaluator != NULL);
    
    // And a frame with memory
    data_t *memory = ar__data__create_map();
    data_t *context = ar__data__create_map();
    data_t *message = ar__data__create_string("test");
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    // When evaluating the method
    bool result = ar_method_evaluator__evaluate(evaluator, frame);
    
    // Then evaluation should succeed
    assert(result == true);
    
    // And the value should be stored in memory
    data_t *value = ar__data__get_map_data(memory, "result");
    assert(value != NULL);
    assert(ar__data__get_type(value) == DATA_INTEGER);
    assert(ar__data__get_integer(value) == 100);
    
    // Clean up
    ar_method_evaluator__destroy(evaluator);
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    ar__method__destroy(method);
    
    printf("  ✓ Evaluate simple assignment\n");
}

/**
 * Test evaluating a method with multiple instructions
 */
static void test_method_evaluator__evaluate_multiple_instructions(void) {
    printf("Testing method evaluator multiple instructions...\n");
    
    // Given a method with multiple instructions
    const char *instructions = "memory.x := 10\n"
                             "memory.y := 20\n"
                             "memory.sum := memory.x + memory.y";
    method_t *method = ar__method__create("multi_test", instructions, "1.0.0");
    assert(method != NULL);
    
    // And a method evaluator
    ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);
    assert(evaluator != NULL);
    
    // And a frame
    data_t *memory = ar__data__create_map();
    data_t *context = ar__data__create_map();
    data_t *message = ar__data__create_string("test");
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    // When evaluating the method
    bool result = ar_method_evaluator__evaluate(evaluator, frame);
    
    // Then evaluation should succeed
    assert(result == true);
    
    // And all values should be correctly computed
    data_t *x = ar__data__get_map_data(memory, "x");
    assert(x != NULL && ar__data__get_integer(x) == 10);
    
    data_t *y = ar__data__get_map_data(memory, "y");
    assert(y != NULL && ar__data__get_integer(y) == 20);
    
    data_t *sum = ar__data__get_map_data(memory, "sum");
    assert(sum != NULL && ar__data__get_integer(sum) == 30);
    
    // Clean up
    ar_method_evaluator__destroy(evaluator);
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    ar__method__destroy(method);
    
    printf("  ✓ Evaluate multiple instructions\n");
}

/**
 * Test evaluating a method with conditional logic
 */
static void test_method_evaluator__evaluate_conditional(void) {
    printf("Testing method evaluator conditional logic...\n");
    
    // Given a method with conditional instruction
    const char *instructions = "memory.flag := 1\n"
                             "memory.result := if(memory.flag, \"yes\", \"no\")";
    method_t *method = ar__method__create("cond_test", instructions, "1.0.0");
    assert(method != NULL);
    
    // And a method evaluator
    ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);
    assert(evaluator != NULL);
    
    // And a frame
    data_t *memory = ar__data__create_map();
    data_t *context = ar__data__create_map();
    data_t *message = ar__data__create_string("test");
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    // When evaluating the method
    bool result = ar_method_evaluator__evaluate(evaluator, frame);
    
    // Then evaluation should succeed
    assert(result == true);
    
    // And the conditional should have been evaluated correctly
    data_t *value = ar__data__get_map_data(memory, "result");
    assert(value != NULL);
    assert(ar__data__get_type(value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(value), "yes") == 0);
    
    // Clean up
    ar_method_evaluator__destroy(evaluator);
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    ar__method__destroy(method);
    
    printf("  ✓ Evaluate conditional logic\n");
}

/**
 * Test that method evaluator reuses evaluators efficiently
 */
static void test_method_evaluator__reuses_evaluators(void) {
    printf("Testing method evaluator reuses evaluators...\n");
    
    // Given a method
    method_t *method = ar__method__create("reuse_test", "memory.x := 42", "1.0.0");
    assert(method != NULL);
    
    // When creating a method evaluator
    ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);
    assert(evaluator != NULL);
    
    // Then we can evaluate the method multiple times with different frames
    for (int i = 0; i < 3; i++) {
        data_t *memory = ar__data__create_map();
        data_t *context = ar__data__create_map();
        data_t *message = ar__data__create_integer(i);
        ar_frame_t *frame = ar_frame__create(memory, context, message);
        
        bool result = ar_method_evaluator__evaluate(evaluator, frame);
        assert(result == true);
        
        // Verify the assignment worked
        data_t *value = ar__data__get_map_data(memory, "x");
        assert(value != NULL && ar__data__get_integer(value) == 42);
        
        ar_frame__destroy(frame);
        ar__data__destroy(memory);
        ar__data__destroy(context);
        ar__data__destroy(message);
    }
    
    // Clean up - single evaluator handled multiple evaluations
    ar_method_evaluator__destroy(evaluator);
    ar__method__destroy(method);
    
    printf("  ✓ Method evaluator reuses evaluators efficiently\n");
}

/**
 * Test error handling for invalid instruction
 */
static void test_method_evaluator__handles_evaluation_error(void) {
    printf("Testing method evaluator error handling...\n");
    
    // Given a method with an instruction that will fail
    // (trying to add a string to a number)
    const char *instructions = "memory.text := \"hello\"\n"
                             "memory.bad := memory.text + 5";  // This should fail
    method_t *method = ar__method__create("error_test", instructions, "1.0.0");
    assert(method != NULL);
    
    // And a method evaluator
    ar_method_evaluator_t *evaluator = ar_method_evaluator__create(method);
    assert(evaluator != NULL);
    
    // And a frame
    data_t *memory = ar__data__create_map();
    data_t *context = ar__data__create_map();
    data_t *message = ar__data__create_string("test");
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    // When evaluating the method
    bool result = ar_method_evaluator__evaluate(evaluator, frame);
    
    // Then evaluation should fail
    assert(result == false);
    
    // But the first instruction should have succeeded
    data_t *text = ar__data__get_map_data(memory, "text");
    assert(text != NULL);
    assert(strcmp(ar__data__get_string(text), "hello") == 0);
    
    // Clean up
    ar_method_evaluator__destroy(evaluator);
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    ar__method__destroy(method);
    
    printf("  ✓ Method evaluator handles errors correctly\n");
}

int main(void) {
    printf("\n=== Method Evaluator Tests ===\n\n");
    
    test_method_evaluator__create_destroy();
    test_method_evaluator__evaluate_simple_assignment();
    test_method_evaluator__evaluate_multiple_instructions();
    test_method_evaluator__evaluate_conditional();
    test_method_evaluator__reuses_evaluators();
    test_method_evaluator__handles_evaluation_error();
    
    printf("\nAll method evaluator tests passed!\n");
    return 0;
}