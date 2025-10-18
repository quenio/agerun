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
#include "ar_send_evaluator_fixture.h"
#include "ar_frame.h"
#include "ar_assert.h"

static void test_send_instruction_evaluator__create_destroy(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_send_create_destroy");
    assert(fixture != NULL);

    // When creating a send instruction evaluator
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);

    // Then it should succeed
    assert(own_evaluator != NULL);

    // Cleanup
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_send_instruction_evaluator__evaluate_with_instance(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_send_evaluate_instance");
    assert(fixture != NULL);

    // When creating a send instruction evaluator
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    assert(own_evaluator != NULL);

    ar_frame_t *ref_frame = ar_send_evaluator_fixture__create_frame(fixture);

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
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_integer_message(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_send_integer_message");
    assert(fixture != NULL);

    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    assert(evaluator != NULL);

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);

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
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_string_message(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_send_string_message");
    assert(fixture != NULL);

    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    assert(evaluator != NULL);

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);

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
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_with_result(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_send_with_result");
    assert(fixture != NULL);

    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    assert(evaluator != NULL);

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);

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
    ar_data_t *memory = ar_frame__get_memory(frame);
    ar_data_t *result_value = ar_data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 1); // true as integer

    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_memory_reference(void) {
    // Given a send evaluator fixture with memory containing a message
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_send_memory_reference");
    assert(fixture != NULL);

    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    assert(evaluator != NULL);

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);

    ar_data_t *memory = ar_frame__get_memory(frame);
    ar_data_t *msg_value = ar_data__create_string("Hello from memory");
    ar_data__set_map_data(memory, "msg", msg_value);

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
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_instruction_evaluator__evaluate_send_invalid_args(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_send_invalid_args");
    assert(fixture != NULL);

    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    assert(evaluator != NULL);

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);

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
    ar_send_evaluator_fixture__destroy(fixture);
}

// Iteration 1.1: Send to delegate returns true (RED-GREEN-REFACTOR)
// RED PHASE: Test will FAIL because negative IDs currently route to agency (returns false for non-existent agent -1)
static void test_send_instruction_evaluator__routes_to_delegate(void) {
    // Given a send evaluator fixture with a registered delegate
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_route_to_delegate");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // Register delegate using fixture helper
    ar_delegate_t *delegate = ar_send_evaluator_fixture__create_and_register_delegate(fixture, -1, "test_delegate");
    AR_ASSERT(delegate != NULL, "Delegate registration should succeed");

    // Create evaluator and frame using fixture
    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    AR_ASSERT(evaluator != NULL, "Evaluator creation should succeed");

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);
    AR_ASSERT(frame != NULL, "Frame creation should succeed");

    // When creating a send AST node for "send(-1, \"test message\")"
    const char *args[] = {"-1", "\"test message\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    AR_ASSERT(ast != NULL, "AST creation should succeed");

    // Create and attach expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    AR_ASSERT(arg_asts != NULL, "Argument list creation should succeed");

    ar_expression_ast_t *delegate_id_ast = ar_expression_ast__create_literal_int(-1);
    AR_ASSERT(delegate_id_ast != NULL, "Delegate ID AST creation should succeed");
    ar_list__add_last(arg_asts, delegate_id_ast);

    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("test message");
    AR_ASSERT(msg_ast != NULL, "Message AST creation should succeed");
    ar_list__add_last(arg_asts, msg_ast);

    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    AR_ASSERT(ast_set == true, "Setting function arguments should succeed");

    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);

    // Then it should succeed (negative ID should route to delegation)
    AR_ASSERT(result == true, "Send to delegate should succeed");

    // Iteration 1.2: And the delegate should have received the message
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);
    bool has_messages = ar_delegation__delegate_has_messages(delegation, -1);
    AR_ASSERT(has_messages == true, "Delegate should have received message");

    // Iteration 1.3.1: Verify message actually exists and can be retrieved (not NULL)
    ar_data_t *own_received = ar_delegation__take_delegate_message(delegation, -1);
    AR_ASSERT(own_received != NULL, "Should be able to retrieve the queued message from delegate");

    // Iteration 1.3.2: Verify message type is correct (not corrupted to wrong type)
    ar_data_type_t received_type = ar_data__get_type(own_received);
    AR_ASSERT(received_type == AR_DATA_TYPE__STRING, "Message type should be STRING, but got something else");

    // Iteration 1.3.3: Verify message content matches what we sent
    const char *received_string = ar_data__get_string(own_received);
    AR_ASSERT(strcmp(received_string, "test message") == 0, "Message content should be 'test message' but got different content");  // ← WILL FAIL if content corrupted

    // Cleanup
    ar_data__destroy(own_received);
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_send_instruction_evaluator__routes_to_agent(void) {
    // Given a send evaluator fixture with a registered agent
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_route_to_agent");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // Register agent using fixture helper
    ar_agent_t *agent = ar_send_evaluator_fixture__create_and_register_agent(fixture, 1, NULL);
    AR_ASSERT(agent != NULL, "Agent registration should succeed");

    // Create evaluator and frame using fixture
    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    AR_ASSERT(evaluator != NULL, "Evaluator creation should succeed");

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);
    AR_ASSERT(frame != NULL, "Frame creation should succeed");

    // When creating a send AST node for "send(1, \"agent message\")"
    const char *args[] = {"1", "\"agent message\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    AR_ASSERT(ast != NULL, "AST creation should succeed");

    // Create and attach expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    AR_ASSERT(arg_asts != NULL, "Argument list creation should succeed");

    ar_expression_ast_t *agent_id_ast = ar_expression_ast__create_literal_int(1);
    AR_ASSERT(agent_id_ast != NULL, "Agent ID AST creation should succeed");
    ar_list__add_last(arg_asts, agent_id_ast);

    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("agent message");
    AR_ASSERT(msg_ast != NULL, "Message AST creation should succeed");
    ar_list__add_last(arg_asts, msg_ast);

    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    AR_ASSERT(ast_set == true, "Setting function arguments should succeed");

    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);

    // Then it should succeed (positive ID should route to agency)
    AR_ASSERT(result == true, "Send to agent should succeed");

    // And the agent should have received the message
    ar_agency_t *agency = ar_send_evaluator_fixture__get_agency(fixture);
    bool has_messages = ar_agency__agent_has_messages(agency, 1);
    AR_ASSERT(has_messages == true, "Agent should have received message");

    // Verify message content and type
    ar_data_t *own_received = ar_agency__get_agent_message(agency, 1);
    AR_ASSERT(own_received != NULL, "Should be able to retrieve message from agent");
    AR_ASSERT(ar_data__get_type(own_received) == AR_DATA_TYPE__STRING, "Message should be STRING type");
    AR_ASSERT(strcmp(ar_data__get_string(own_received), "agent message") == 0,
              "Message content should match sent value");

    // Cleanup
    ar_data__destroy(own_received);
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_send_instruction_evaluator__nonexistent_delegate_returns_false(void) {
    // Given a send evaluator fixture WITHOUT registering any delegates
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_nonexistent_delegate");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // Create evaluator and frame using fixture
    ar_send_instruction_evaluator_t *evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);
    AR_ASSERT(evaluator != NULL, "Evaluator creation should succeed");

    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);
    AR_ASSERT(frame != NULL, "Frame creation should succeed");

    // When creating a send AST node for "send(-99, \"message\")" (delegate -99 doesn't exist)
    const char *args[] = {"-99", "\"message\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", args, 2, NULL
    );
    AR_ASSERT(ast != NULL, "AST creation should succeed");

    // Create and attach expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    AR_ASSERT(arg_asts != NULL, "Argument list creation should succeed");

    ar_expression_ast_t *delegate_id_ast = ar_expression_ast__create_literal_int(-99);
    AR_ASSERT(delegate_id_ast != NULL, "Delegate ID AST creation should succeed");
    ar_list__add_last(arg_asts, delegate_id_ast);

    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("message");
    AR_ASSERT(msg_ast != NULL, "Message AST creation should succeed");
    ar_list__add_last(arg_asts, msg_ast);

    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    AR_ASSERT(ast_set == true, "Setting function arguments should succeed");

    // When evaluating the send to non-existent delegate
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, frame, ast);

    // Then it should fail (return false) for non-existent delegate
    AR_ASSERT(result == false, "Send to non-existent delegate should return false");

    // Verify no messages were queued anywhere
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);
    bool delegate_has_messages = ar_delegation__delegate_has_messages(delegation, -99);
    AR_ASSERT(delegate_has_messages == false, "Non-existent delegate should have no messages");

    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
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

    // Iteration 1.1-1.3.3: Routes to delegate
    test_send_instruction_evaluator__routes_to_delegate();
    printf("test_send_instruction_evaluator__routes_to_delegate passed!\n");

    // Iteration 2: Verify agent routing
    test_send_instruction_evaluator__routes_to_agent();
    printf("test_send_instruction_evaluator__routes_to_agent passed!\n");

    // Iteration 3: Error handling for non-existent delegate
    test_send_instruction_evaluator__nonexistent_delegate_returns_false();
    printf("test_send_instruction_evaluator__nonexistent_delegate_returns_false passed!\n");

    printf("All send instruction_evaluator tests passed!\n");

    return 0;
}
