#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ar_send_instruction_evaluator.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"

static void test_send_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_send_create_destroy");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(own_fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(own_fixture);

    // When creating a send instruction evaluator
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_instruction_evaluator__create(
        ref_log, ref_expr_eval, mut_agency, ref_delegation
    );
    
    // Then it should succeed
    assert(own_evaluator != NULL);
    
    // Cleanup
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_send_instruction_evaluator__evaluate_with_instance(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_send_evaluate_instance");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(own_fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);

    // When creating a send instruction evaluator
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_instruction_evaluator__create(
        ref_log, ref_expr_eval, mut_agency, ref_delegation
    );
    assert(own_evaluator != NULL);
    
    // When creating a send AST node for "send(0, 42)"
    const char *args[] = {"0", "42"};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *own_arg_asts = ar_list__create();
    assert(own_arg_asts != NULL);
    
    ar_expression_ast_t *own_agent_ast = ar_expression_ast__create_literal_int(0);
    assert(own_agent_ast != NULL);
    ar_list__add_last(own_arg_asts, own_agent_ast);
    
    ar_expression_ast_t *own_msg_ast = ar_expression_ast__create_literal_int(42);
    assert(own_msg_ast != NULL);
    ar_list__add_last(own_arg_asts, own_msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send with frame
    bool result = ar_send_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_send_integer_message(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create("test_send_integer_message");
    assert(fixture != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);

    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        log, expr_eval, mut_agency, ref_delegation
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, 42)"
    const char *args[] = {"0", "42"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_int(42);
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send with frame
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed (send to agent 0 is a no-op that returns true)
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_string_message(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create("test_send_string_message");
    assert(fixture != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);

    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        log, expr_eval, mut_agency, ref_delegation
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, \"hello\")"
    const char *args[] = {"0", "\"hello\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("hello");
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send with frame
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_with_result(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create("test_send_with_result");
    assert(fixture != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);

    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        log, expr_eval, mut_agency, ref_delegation
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "memory.result := send(0, \"test\")"
    const char *args[] = {"0", "\"test\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("test");
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send with frame
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be stored in memory (send returns true for agent 0)
    ar_data_t *result_value = ar_data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 1); // true as integer
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_memory_reference(void) {
    // Given a test fixture with memory containing a message
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create("test_send_memory_reference");
    assert(fixture != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);

    ar_data_t *msg_value = ar_data__create_string("Hello from memory");
    ar_data__set_map_data(memory, "msg", msg_value);

    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        log, expr_eval, mut_agency, ref_delegation
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, memory.msg)"
    const char *args[] = {"0", "memory.msg"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    const char *msg_path[] = {"msg"};
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_memory_access("memory", msg_path, 1);
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send with frame
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_invalid_args(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create("test_send_invalid_args");
    assert(fixture != NULL);

    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_delegation_t *ref_delegation = ar_evaluator_fixture__get_delegation(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);

    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        log, expr_eval, mut_agency, ref_delegation
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node with only one argument
    const char *args[] = {"0"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 1, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments (only one arg)
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send with frame
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should fail (send requires 2 arguments)
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

int main(void) {
    printf("Starting send instruction_evaluator tests...\n");
    
    test_send_instruction_evaluator__create_destroy();
    printf("test_send_instruction_evaluator__create_destroy passed!\n");
    
    test_send_instruction_evaluator__evaluate_with_instance();
    printf("test_send_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_instruction_evaluator__evaluate_send_integer_message();
    printf("test_instruction_evaluator__evaluate_send_integer_message passed!\n");
    
    test_instruction_evaluator__evaluate_send_string_message();
    printf("test_instruction_evaluator__evaluate_send_string_message passed!\n");
    
    test_instruction_evaluator__evaluate_send_with_result();
    printf("test_instruction_evaluator__evaluate_send_with_result passed!\n");
    
    test_instruction_evaluator__evaluate_send_memory_reference();
    printf("test_instruction_evaluator__evaluate_send_memory_reference passed!\n");
    
    test_instruction_evaluator__evaluate_send_invalid_args();
    printf("test_instruction_evaluator__evaluate_send_invalid_args passed!\n");

    printf("All send instruction_evaluator tests passed!\n");

    return 0;
}
