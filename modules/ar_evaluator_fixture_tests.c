#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ar_evaluator_fixture.h"
#include "ar_assignment_instruction_evaluator.h"
#include "ar_data.h"

static void test_fixture__create_destroy(void) {
    // Given a test name
    const char *test_name = "test_fixture";
    
    // When creating a fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create(test_name);
    
    // Then it should be created successfully
    assert(fixture != NULL);
    
    // And all components should be available
    assert(ar_evaluator_fixture__get_expression_evaluator(fixture) != NULL);
    assert(ar_evaluator_fixture__get_memory(fixture) != NULL);
    assert(ar_evaluator_fixture__get_log(fixture) != NULL);
    
    // Test can create an evaluator using these components
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    assert(evaluator != NULL);
    
    // When destroying the fixture
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Then no memory should be leaked (checked by fixture itself)
}

static void test_fixture__create_frame(void) {
    // Given a fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_frame");
    assert(fixture != NULL);
    
    // When creating a frame
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    
    // Then it should be created successfully
    assert(frame != NULL);
    
    // Cleanup (frame is tracked by fixture)
    ar_evaluator_fixture__destroy(fixture);
}

static void test_fixture__create_assignment_int(void) {
    // Given a fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_int_assignment");
    assert(fixture != NULL);
    
    // When creating an integer assignment
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_int(
        fixture, "memory.count", 42
    );
    
    // Then it should be created successfully
    assert(ast != NULL);
    
    // Create evaluator for testing
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    
    // And evaluation should work
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    assert(result == true);
    
    // And the value should be stored
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    int count = ar_data__get_map_integer(memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_fixture__memory_leak_detection(void) {
    // Given a fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_memory");
    assert(fixture != NULL);
    
    // When performing operations
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_string(
        fixture, "memory.name", "test"
    );
    
    assert(frame != NULL);
    assert(ast != NULL);
    
    // Then memory check should pass
    bool no_leaks = ar_evaluator_fixture__check_memory(fixture);
    assert(no_leaks == true);
    
    // Cleanup
    ar_evaluator_fixture__destroy(fixture);
}

int main(void) {
    printf("Starting assignment instruction evaluator fixture tests...\n");
    
    test_fixture__create_destroy();
    printf("test_fixture__create_destroy passed!\n");
    
    test_fixture__create_frame();
    printf("test_fixture__create_frame passed!\n");
    
    test_fixture__create_assignment_int();
    printf("test_fixture__create_assignment_int passed!\n");
    
    test_fixture__memory_leak_detection();
    printf("test_fixture__memory_leak_detection passed!\n");
    
    printf("All assignment instruction evaluator fixture tests passed!\n");
    return 0;
}