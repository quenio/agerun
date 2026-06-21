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
#include "ar_expression_parser.h"
#include "ar_frame.h"
#include "ar_assert.h"

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

static void test_assignment_instruction_evaluator__evaluate_message_expression(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_message");
    assert(fixture != NULL);
    
    // Create memory, context and message manually
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_string("Hello from message");
    
    // Create frame with our message
    ar_frame_t *frame = ar_frame__create(mut_memory, own_context, own_message);
    assert(frame != NULL);
    
    // Create expression AST for "message" 
    ar_expression_ast_t *message_expr = ar_expression_ast__create_memory_access("message", NULL, 0);
    assert(message_expr != NULL);
    
    // When evaluating an assignment instruction: memory.result := message
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_expr(
        fixture, "memory.result", message_expr
    );
    assert(ast != NULL);
    
    // Create and set evaluator
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the message value should be stored in memory.result
    const char *stored_result = ar_data__get_map_string(mut_memory, "result");
    assert(stored_result != NULL);
    assert(strcmp(stored_result, "Hello from message") == 0);
    
    // Cleanup
    ar_frame__destroy(frame);
    // Frame doesn't take ownership of context, so we must destroy it
    ar_data__destroy(own_context);
    // Don't destroy own_message - the assignment evaluator took ownership of it
    // when evaluating "message" expression (it was unowned, so evaluator claimed it)
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__rejects_assignment_to_memory_self(void) {
    // Given a test fixture with agency-managed memory.self
    ar_evaluator_fixture_t *fixture =
        ar_evaluator_fixture__create("test_reject_memory_self_assignment");
    assert(fixture != NULL);

    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data__set_map_integer(memory, "self", 7);

    // When evaluating an assignment instruction: memory.self := 99
    ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_int(
        fixture, "memory.self", 99
    );
    assert(ast != NULL);

    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(log, expr_eval);

    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);

    // Then the assignment should be rejected and the existing self ID preserved
    assert(result == false);
    assert(ar_data__get_map_integer(memory, "self") == 7);

    ar_event_t *error_event = ar_log__get_last_error(log);
    assert(error_event != NULL);
    const char *error_msg = ar_event__get_message(error_event);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "memory.self is agency-managed") != NULL);

    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static ar_expression_ast_t *_parse_assignment_test_expression(
    ar_log_t *ref_log,
    const char *ref_expression
) {
    ar_expression_parser_t *own_parser =
        ar_expression_parser__create(ref_log, ref_expression);
    assert(own_parser != NULL);
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    assert(own_ast != NULL);
    ar_expression_parser__destroy(own_parser);

    // Ownership transferred to caller
    return own_ast;
}

static void test_assignment_instruction_evaluator__stores_build_expression_string(void) {
    // Given an assignment whose RHS is a pure build() expression
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_assignment_stores_build_expression_string");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_expression_evaluator_t *ref_expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_assignment_instruction_evaluator_t *own_evaluator =
        ar_assignment_instruction_evaluator__create(ref_log, ref_expr_eval);
    AR_ASSERT(own_evaluator != NULL, "Assignment evaluator creation should succeed");

    ar_expression_ast_t *own_build_ast = _parse_assignment_test_expression(
        ref_log,
        "build(\"Hello {name}!\", {name: \"Ada\"})"
    );
    ar_instruction_ast_t *ref_ast = ar_evaluator_fixture__create_assignment_expr(
        own_fixture, "memory.result", own_build_ast
    );
    AR_ASSERT(ref_ast != NULL, "Assignment AST creation should succeed");

    // When evaluating the assignment
    bool result = ar_assignment_instruction_evaluator__evaluate(
        own_evaluator, ref_frame, ref_ast
    );

    // Then the built string should be stored in memory
    AR_ASSERT(result == true, "Assignment evaluation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    const char *ref_result = ar_data__get_map_string(mut_memory, "result");
    AR_ASSERT(ref_result != NULL, "Assignment should store a result string");
    AR_ASSERT(strcmp(ref_result, "Hello Ada!") == 0,
              "Assignment should store the built string");

    ar_assignment_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_assignment_instruction_evaluator__stores_head_tail_expression_results(void) {
    // Given assignments whose RHS values are pure head() and tail() expressions
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_assignment_stores_head_tail_results");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // Given an assignment evaluator with a frame
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_expression_evaluator_t *ref_expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_assignment_instruction_evaluator_t *own_evaluator =
        ar_assignment_instruction_evaluator__create(ref_log, ref_expr_eval);
    AR_ASSERT(own_evaluator != NULL, "Assignment evaluator creation should succeed");

    // When building an assignment whose RHS is head()
    ar_expression_ast_t *own_head_ast = _parse_assignment_test_expression(
        ref_log,
        "head([9, 10])"
    );
    ar_instruction_ast_t *own_head_assignment = ar_evaluator_fixture__create_assignment_expr(
        own_fixture, "memory.first", own_head_ast
    );
    AR_ASSERT(own_head_assignment != NULL, "Head assignment AST creation should succeed");

    // When evaluating the head() assignment
    bool result = ar_assignment_instruction_evaluator__evaluate(
        own_evaluator, ref_frame, own_head_assignment
    );

    // Then the assignment should store the first item
    AR_ASSERT(result == true, "Head assignment evaluation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__get_map_integer(mut_memory, "first") == 9,
              "Assignment should store the head result");

    // When building an assignment whose RHS is tail()
    ar_expression_ast_t *own_tail_ast = _parse_assignment_test_expression(
        ref_log,
        "tail([9, 10, 11])"
    );
    ar_instruction_ast_t *own_tail_assignment = ar_evaluator_fixture__create_assignment_expr(
        own_fixture, "memory.rest", own_tail_ast
    );
    AR_ASSERT(own_tail_assignment != NULL, "Tail assignment AST creation should succeed");

    // When evaluating the tail() assignment
    result = ar_assignment_instruction_evaluator__evaluate(
        own_evaluator, ref_frame, own_tail_assignment
    );

    // Then the assignment should store the remaining items
    AR_ASSERT(result == true, "Tail assignment evaluation should succeed");
    ar_data_t *ref_rest = ar_data__get_map_data(mut_memory, "rest");
    AR_ASSERT(ref_rest != NULL, "Assignment should store the tail result");
    AR_ASSERT(ar_data__get_type(ref_rest) == AR_DATA_TYPE__LIST,
              "Tail assignment should store a list");
    AR_ASSERT(ar_data__list_count(ref_rest) == 2,
              "Tail assignment should store remaining items");
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_rest)) == 10,
              "Tail assignment first item should match");
    AR_ASSERT(ar_data__get_integer(ar_data__list_last(ref_rest)) == 11,
              "Tail assignment last item should match");

    // Cleanup
    ar_assignment_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_assignment_instruction_evaluator__stores_parse_self_fields_outside_memory_self(void) {
    // Given parse() expressions that produce self and nested self fields
    ar_evaluator_fixture_t *fixture =
        ar_evaluator_fixture__create("test_assignment_stores_parse_self_fields");
    assert(fixture != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    ar_expression_evaluator_t *expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator =
        ar_assignment_instruction_evaluator__create(log, expr_eval);
    assert(evaluator != NULL);

    ar_expression_ast_t *own_self_parse_ast = _parse_assignment_test_expression(
        log,
        "parse(\"self={self}\", \"self=7\")"
    );
    ar_instruction_ast_t *self_ast = ar_evaluator_fixture__create_assignment_expr(
        fixture, "memory.result_self", own_self_parse_ast
    );
    assert(self_ast != NULL);

    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, self_ast);

    assert(result == true);
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data_t *ref_self_result = ar_data__get_map_data(memory, "result_self");
    assert(ref_self_result != NULL);
    assert(ar_data__get_type(ref_self_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_integer(ref_self_result, "self") == 7);

    ar_expression_ast_t *own_nested_parse_ast = _parse_assignment_test_expression(
        log,
        "parse(\"nested={self.anything}\", \"nested=99\")"
    );
    ar_instruction_ast_t *nested_ast = ar_evaluator_fixture__create_assignment_expr(
        fixture, "memory.result_nested", own_nested_parse_ast
    );
    assert(nested_ast != NULL);

    result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, nested_ast);

    assert(result == true);
    ar_data_t *ref_nested_result = ar_data__get_map_data(memory, "result_nested");
    assert(ref_nested_result != NULL);
    assert(ar_data__get_type(ref_nested_result) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_map_integer(ref_nested_result, "self.anything") == 99);

    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__rejects_parse_expression_memory_self_writes(void) {
    // Given protected memory.self assignment targets and parse() expression RHS values
    ar_evaluator_fixture_t *fixture =
        ar_evaluator_fixture__create("test_assignment_rejects_parse_memory_self_writes");
    assert(fixture != NULL);

    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    assert(ar_data__set_map_integer(memory, "self", 7));

    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator =
        ar_assignment_instruction_evaluator__create(log, expr_eval);
    assert(evaluator != NULL);

    const char *targets[] = {"memory.self", "memory.self.anything"};
    for (size_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
        ar_expression_ast_t *own_parse_ast = _parse_assignment_test_expression(
            log,
            "parse(\"value={value}\", \"value=99\")"
        );
        ar_instruction_ast_t *ast = ar_evaluator_fixture__create_assignment_expr(
            fixture, targets[i], own_parse_ast
        );
        assert(ast != NULL);

        bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, frame, ast);

        assert(result == false);
        assert(ar_data__get_map_integer(memory, "self") == 7);

        ar_event_t *error_event = ar_log__get_last_error(log);
        assert(error_event != NULL);
        const char *error_msg = ar_event__get_message(error_event);
        assert(error_msg != NULL);
        assert(strstr(error_msg, "memory.self is agency-managed") != NULL);
    }

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

static void test_assignment_instruction_evaluator__stores_literal_containers(void) {
    ar_evaluator_fixture_t *fixture =
        ar_evaluator_fixture__create("test_stores_literal_containers");
    assert(fixture != NULL);

    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator =
        ar_assignment_instruction_evaluator__create(log, expr_eval);
    assert(evaluator != NULL);

    ar_expression_ast_t *own_items[2];
    own_items[0] = ar_expression_ast__create_literal_int(1);
    own_items[1] = ar_expression_ast__create_literal_int(2);
    ar_expression_ast_t *own_list_expr = ar_expression_ast__create_literal_list(own_items, 2);
    assert(own_list_expr != NULL);

    ar_instruction_ast_t *list_ast = ar_evaluator_fixture__create_assignment_expr(
        fixture, "memory.items", own_list_expr
    );
    assert(list_ast != NULL);
    assert(ar_assignment_instruction_evaluator__evaluate(evaluator, frame, list_ast) == true);

    const char *keys[] = {"name", "scores"};
    ar_expression_ast_t *own_scores_items[2];
    own_scores_items[0] = ar_expression_ast__create_literal_int(10);
    own_scores_items[1] = ar_expression_ast__create_literal_int(20);
    ar_expression_ast_t *own_values[2];
    own_values[0] = ar_expression_ast__create_literal_string("Ada");
    own_values[1] = ar_expression_ast__create_literal_list(own_scores_items, 2);
    ar_expression_ast_t *own_map_expr = ar_expression_ast__create_literal_map(keys, own_values, 2);
    assert(own_map_expr != NULL);

    ar_instruction_ast_t *map_ast = ar_evaluator_fixture__create_assignment_expr(
        fixture, "memory.profile", own_map_expr
    );
    assert(map_ast != NULL);
    assert(ar_assignment_instruction_evaluator__evaluate(evaluator, frame, map_ast) == true);

    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data_t *stored_items = ar_data__get_map_data(memory, "items");
    assert(stored_items != NULL);
    assert(ar_data__get_type(stored_items) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(stored_items) == 2);

    ar_data_t *stored_profile = ar_data__get_map_data(memory, "profile");
    assert(stored_profile != NULL);
    assert(ar_data__get_type(stored_profile) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(stored_profile, "name"), "Ada") == 0);
    ar_data_t *stored_scores = ar_data__get_map_data(stored_profile, "scores");
    assert(stored_scores != NULL);
    assert(ar_data__get_type(stored_scores) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(stored_scores) == 2);

    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_assignment_instruction_evaluator__deep_copies_nested_sources(void) {
    printf("Testing assignment deep-copies nested sources...\n");

    // Given nested values in memory, context, and message
    ar_evaluator_fixture_t *fixture =
        ar_evaluator_fixture__create("test_deep_copies_nested_sources");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data_t *own_memory_source = ar_data__create_map();
    ar_data_t *own_memory_inner = ar_data__create_list();
    AR_ASSERT(own_memory_source != NULL, "Memory source should be created");
    AR_ASSERT(own_memory_inner != NULL, "Memory inner list should be created");
    AR_ASSERT(ar_data__list_add_last_integer(own_memory_inner, 1), "Memory inner item should be stored");
    AR_ASSERT(ar_data__set_map_data(own_memory_source, "items", own_memory_inner), "Memory inner list should be stored");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "source", own_memory_source), "Memory source should be stored");

    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_context_value = ar_data__create_map();
    ar_data_t *own_context_inner = ar_data__create_list();
    AR_ASSERT(own_context != NULL, "Context should be created");
    AR_ASSERT(own_context_value != NULL, "Context value should be created");
    AR_ASSERT(own_context_inner != NULL, "Context inner list should be created");
    AR_ASSERT(ar_data__list_add_last_string(own_context_inner, "context-item"), "Context inner item should be stored");
    AR_ASSERT(ar_data__set_map_data(own_context_value, "items", own_context_inner), "Context inner list should be stored");
    AR_ASSERT(ar_data__set_map_data(own_context, "value", own_context_value), "Context value should be stored");

    ar_data_t *own_message = ar_data__create_map();
    ar_data_t *own_message_value = ar_data__create_map();
    ar_data_t *own_message_inner = ar_data__create_list();
    AR_ASSERT(own_message != NULL, "Message should be created");
    AR_ASSERT(own_message_value != NULL, "Message value should be created");
    AR_ASSERT(own_message_inner != NULL, "Message inner list should be created");
    AR_ASSERT(ar_data__list_add_last_string(own_message_inner, "message-item"), "Message inner item should be stored");
    AR_ASSERT(ar_data__set_map_data(own_message_value, "items", own_message_inner), "Message inner list should be stored");
    AR_ASSERT(ar_data__set_map_data(own_message, "value", own_message_value), "Message value should be stored");

    ar_frame_t *frame = ar_frame__create(mut_memory, own_context, own_message);
    AR_ASSERT(frame != NULL, "Frame creation should succeed");

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_assignment_instruction_evaluator_t *evaluator =
        ar_assignment_instruction_evaluator__create(log, expr_eval);
    AR_ASSERT(evaluator != NULL, "Evaluator creation should succeed");

    // When assigning those borrowed nested values into memory
    const char *memory_path[] = {"source"};
    ar_instruction_ast_t *memory_ast = ar_evaluator_fixture__create_assignment_expr(
        fixture,
        "memory.memory_copy",
        ar_expression_ast__create_memory_access("memory", memory_path, 1)
    );
    AR_ASSERT(memory_ast != NULL, "Memory assignment AST should be created");
    AR_ASSERT(ar_assignment_instruction_evaluator__evaluate(evaluator, frame, memory_ast) == true, "Memory assignment should succeed");

    const char *context_path[] = {"value"};
    ar_instruction_ast_t *context_ast = ar_evaluator_fixture__create_assignment_expr(
        fixture,
        "memory.context_copy",
        ar_expression_ast__create_memory_access("context", context_path, 1)
    );
    AR_ASSERT(context_ast != NULL, "Context assignment AST should be created");
    AR_ASSERT(ar_assignment_instruction_evaluator__evaluate(evaluator, frame, context_ast) == true, "Context assignment should succeed");

    const char *message_path[] = {"value"};
    ar_instruction_ast_t *message_ast = ar_evaluator_fixture__create_assignment_expr(
        fixture,
        "memory.message_copy",
        ar_expression_ast__create_memory_access("message", message_path, 1)
    );
    AR_ASSERT(message_ast != NULL, "Message assignment AST should be created");
    AR_ASSERT(ar_assignment_instruction_evaluator__evaluate(evaluator, frame, message_ast) == true, "Message assignment should succeed");

    // Then each stored value should be an independent deep copy
    ar_data_t *ref_memory_source = ar_data__get_map_data(mut_memory, "source");
    ar_data_t *ref_memory_copy = ar_data__get_map_data(mut_memory, "memory_copy");
    ar_data_t *ref_context_source = ar_data__get_map_data(own_context, "value");
    ar_data_t *ref_context_copy = ar_data__get_map_data(mut_memory, "context_copy");
    ar_data_t *ref_message_source = ar_data__get_map_data(own_message, "value");
    ar_data_t *ref_message_copy = ar_data__get_map_data(mut_memory, "message_copy");
    AR_ASSERT(ref_memory_copy != NULL, "Memory copy should be stored");
    AR_ASSERT(ref_context_copy != NULL, "Context copy should be stored");
    AR_ASSERT(ref_message_copy != NULL, "Message copy should be stored");
    AR_ASSERT(ref_memory_copy != ref_memory_source, "Memory source should be deep-copied");
    AR_ASSERT(ref_context_copy != ref_context_source, "Context source should be deep-copied");
    AR_ASSERT(ref_message_copy != ref_message_source, "Message source should be deep-copied");
    AR_ASSERT(ar_data__get_type(ar_data__get_map_data(ref_memory_copy, "items")) == AR_DATA_TYPE__LIST, "Memory copy should preserve nested list");
    AR_ASSERT(ar_data__get_type(ar_data__get_map_data(ref_context_copy, "items")) == AR_DATA_TYPE__LIST, "Context copy should preserve nested list");
    AR_ASSERT(ar_data__get_type(ar_data__get_map_data(ref_message_copy, "items")) == AR_DATA_TYPE__LIST, "Message copy should preserve nested list");

    AR_ASSERT(ar_data__list_add_last_integer(ar_data__get_map_data(ref_memory_source, "items"), 2), "Memory source should mutate");
    AR_ASSERT(ar_data__list_count(ar_data__get_map_data(ref_memory_copy, "items")) == 1, "Memory copy should remain independent");
    AR_ASSERT(ar_data__list_add_last_string(ar_data__get_map_data(ref_context_source, "items"), "new"), "Context source should mutate");
    AR_ASSERT(ar_data__list_count(ar_data__get_map_data(ref_context_copy, "items")) == 1, "Context copy should remain independent");
    AR_ASSERT(ar_data__list_add_last_string(ar_data__get_map_data(ref_message_source, "items"), "new"), "Message source should mutate");
    AR_ASSERT(ar_data__list_count(ar_data__get_map_data(ref_message_copy, "items")) == 1, "Message copy should remain independent");

    // Cleanup
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_frame__destroy(frame);
    ar_data__destroy(own_context);
    ar_data__destroy(own_message);
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
    
    test_assignment_instruction_evaluator__evaluate_message_expression();
    printf("test_assignment_instruction_evaluator__evaluate_message_expression passed!\n");
    
    test_assignment_instruction_evaluator__rejects_assignment_to_memory_self();
    printf("test_assignment_instruction_evaluator__rejects_assignment_to_memory_self passed!\n");

    test_assignment_instruction_evaluator__stores_build_expression_string();
    printf("test_assignment_instruction_evaluator__stores_build_expression_string passed!\n");

    test_assignment_instruction_evaluator__stores_head_tail_expression_results();
    printf("test_assignment_instruction_evaluator__stores_head_tail_expression_results passed!\n");

    test_assignment_instruction_evaluator__stores_parse_self_fields_outside_memory_self();
    printf("test_assignment_instruction_evaluator__stores_parse_self_fields_outside_memory_self passed!\n");

    test_assignment_instruction_evaluator__rejects_parse_expression_memory_self_writes();
    printf("test_assignment_instruction_evaluator__rejects_parse_expression_memory_self_writes passed!\n");

    test_assignment_instruction_evaluator__evaluate_invalid_path();
    printf("test_assignment_instruction_evaluator__evaluate_invalid_path passed!\n");

    test_assignment_instruction_evaluator__stores_literal_containers();
    printf("test_assignment_instruction_evaluator__stores_literal_containers passed!\n");

    test_assignment_instruction_evaluator__deep_copies_nested_sources();
    printf("test_assignment_instruction_evaluator__deep_copies_nested_sources passed!\n");
    
    printf("All assignment instruction_evaluator tests passed!\n");
    
    return 0;
}
