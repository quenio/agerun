#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "ar_evaluator_fixture.h"
#include "ar_assignment_instruction_evaluator.h"
#include "ar_data.h"
#include "ar_event.h"
#include "ar_expression_ast.h"

static void test_assignment_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_create_destroy");
    assert(fixture != NULL);
    
    // When getting the evaluator from the fixture
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    // Then it should exist
    assert(evaluator != NULL);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__create_without_memory(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_create_without_memory");
    assert(fixture != NULL);
    
    // When getting the evaluator (which was created without memory parameter)
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    // Then it should exist
    assert(evaluator != NULL);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__evaluate_with_frame(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_with_frame");
    assert(fixture != NULL);
    
    // When evaluating an assignment instruction with frame: memory.count := 42
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_int(
        fixture, "memory.count", 42
    );
    assert(ast != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    // Evaluate with frame parameter
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    int count = ar_data__get_map_integer(memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__evaluate_with_instance(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_with_instance");
    assert(fixture != NULL);
    
    // When evaluating an assignment instruction: memory.count := 42
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_int(
        fixture, "memory.count", 42
    );
    assert(ast != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    int count = ar_data__get_map_integer(memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__evaluate_integer(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_integer");
    assert(fixture != NULL);
    
    // When evaluating an assignment instruction: memory.count := 42
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_int(
        fixture, "memory.count", 42
    );
    assert(ast != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    int count = ar_data__get_map_integer(memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__evaluate_string(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_string");
    assert(fixture != NULL);
    
    // When evaluating an assignment instruction: memory.name := "Alice"
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_string(
        fixture, "memory.name", "Alice"
    );
    assert(ast != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    const char *name = ar_data__get_map_string(memory, "name");
    assert(name != NULL);
    assert(strcmp(name, "Alice") == 0);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__evaluate_nested_path(void) {
    // Given a test fixture with memory containing a nested map
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_nested_path");
    assert(fixture != NULL);
    
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data_t *user = ar_data__create_map();
    assert(user != NULL);
    ar_data__set_map_data(memory, "user", user);
    
    // When evaluating an assignment to a nested path: memory.user.age := 25
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_int(
        fixture, "memory.user.age", 25
    );
    assert(ast != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in the nested map
    ar_data_t *stored_user = ar_data__get_map_data(memory, "user");
    assert(stored_user != NULL);
    int age = ar_data__get_map_integer(stored_user, "age");
    assert(age == 25);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__evaluate_expression(void) {
    // Given a test fixture with memory containing initial values
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_expression");
    assert(fixture != NULL);
    
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data__set_map_integer(memory, "x", 10);
    ar_data__set_map_integer(memory, "y", 5);
    
    // When evaluating an assignment with an expression: memory.sum := memory.x + memory.y
    // Create the expression AST for memory.x + memory.y
    const char *x_path[] = {"x"};
    ar_expression_ast_t *left_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    assert(left_ast != NULL);
    
    const char *y_path[] = {"y"};
    ar_expression_ast_t *right_ast = ar_expression_ast__create_memory_access("memory", y_path, 1);
    assert(right_ast != NULL);
    
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__ADD, left_ast, right_ast);
    assert(expr_ast != NULL);
    
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_expr(
        fixture, "memory.sum", expr_ast
    );
    assert(ast != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the computed value should be stored in memory
    int sum = ar_data__get_map_integer(memory, "sum");
    assert(sum == 15);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__evaluate_invalid_path(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_invalid_path");
    assert(fixture != NULL);
    
    // When evaluating an assignment with invalid path: invalid.path := 42
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_int(
        fixture, "invalid.path", 42
    );
    assert(ast != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    // Evaluator is managed by the test, not the fixture
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return false
    assert(result == false);
    
    // And error should be logged
    ar_event_t *error_event = ar_log__get_last_error(log);
    assert(error_event != NULL);
    const char *error_msg = ar_event__get_message(error_event);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "Assignment target must start with 'memory.'") != NULL);
    
    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}


int main(void) {
    printf("Starting assignment instruction_evaluator tests...\n");
    
    test_assignment_instruction_evaluator__create_destroy();
    printf("test_assignment_instruction_evaluator__create_destroy passed!\n");
    
    test_assignment_instruction_evaluator__create_without_memory();
    printf("test_assignment_instruction_evaluator__create_without_memory passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_with_frame();
    printf("test_assignment_instruction_evaluator__evaluate_with_frame passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_with_instance();
    printf("test_assignment_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_integer();
    printf("test_assignment_instruction_evaluator__evaluate_integer passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_string();
    printf("test_assignment_instruction_evaluator__evaluate_string passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_nested_path();
    printf("test_assignment_instruction_evaluator__evaluate_nested_path passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_expression();
    printf("test_assignment_instruction_evaluator__evaluate_expression passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_invalid_path();
    printf("test_assignment_instruction_evaluator__evaluate_invalid_path passed!\n");
    
    printf("All assignment instruction_evaluator tests passed!\n");
    
    return 0;
}

