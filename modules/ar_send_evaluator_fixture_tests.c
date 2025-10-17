#include <stdio.h>
#include <stdint.h>
#include "ar_send_evaluator_fixture.h"
#include "ar_assert.h"

static void test_send_evaluator_fixture__create_returns_non_null(void) {
    // Given test environment is initialized
    // (No explicit setup needed - system ready)

    // When creating a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_fixture_create");

    // Then it should return a valid fixture
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_send_evaluator_fixture__create_evaluator_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_create_evaluator");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating an evaluator from the fixture
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_evaluator_fixture__create_evaluator(fixture);

    // Then it should return a valid evaluator
    AR_ASSERT(own_evaluator != NULL, "Fixture should create evaluator");

    // Cleanup
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_send_evaluator_fixture__create_frame_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_create_frame");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating a frame from the fixture
    ar_frame_t *frame = ar_send_evaluator_fixture__create_frame(fixture);

    // Then it should return a valid frame
    AR_ASSERT(frame != NULL, "Fixture should create frame");

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_send_evaluator_fixture__get_delegation_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_get_delegation");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When getting the delegation from the fixture
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);

    // Then it should return a valid delegation
    AR_ASSERT(delegation != NULL, "Fixture should return delegation");

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

static void test_send_evaluator_fixture__get_agency_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_get_agency");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When getting the agency from the fixture
    ar_agency_t *agency = ar_send_evaluator_fixture__get_agency(fixture);

    // Then it should return a valid agency
    AR_ASSERT(agency != NULL, "Fixture should return agency");

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

// Iteration 0.6.1: create_and_register_delegate() returns non-NULL
static void test_send_evaluator_fixture__create_and_register_delegate_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_delegate_0_6_1");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering a delegate
    ar_delegate_t *delegate = ar_send_evaluator_fixture__create_and_register_delegate(fixture, -1, "test_delegate");

    // Then it should return a valid delegate (0.6.1 assertion)
    AR_ASSERT(delegate != NULL, "Delegate creation should succeed");

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

// Iteration 0.6.2: Delegate is registered in delegation
static void test_send_evaluator_fixture__delegate_is_registered(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_delegate_0_6_2");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering a delegate
    ar_delegate_t *delegate = ar_send_evaluator_fixture__create_and_register_delegate(fixture, -1, "test_delegate");
    AR_ASSERT(delegate != NULL, "Delegate creation should succeed");

    // Then the delegate should be registered in the delegation (verify by sending a message to it)
    // If delegate isn't registered, send_to_delegate returns false
    // If delegate IS registered, send_to_delegate returns true and queues the message
    ar_delegation_t *delegation = ar_send_evaluator_fixture__get_delegation(fixture);
    ar_data_t *test_message = ar_data__create_string("test");
    bool send_result = ar_delegation__send_to_delegate(delegation, -1, test_message);
    AR_ASSERT(send_result == true, "Delegate should be registered (send to registered delegate should succeed)");  // ← 0.6.2 assertion

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

// Iteration 0.7.1: create_and_register_agent() returns non-NULL
static void test_send_evaluator_fixture__create_and_register_agent_returns_non_null(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_agent_0_7_1");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering an agent
    ar_agent_t *agent = ar_send_evaluator_fixture__create_and_register_agent(fixture, 1, NULL);

    // Then it should return a valid agent (0.7.1 assertion)
    AR_ASSERT(agent != NULL, "Agent creation should succeed");

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

// Iteration 0.7.2: Agent is registered in agency
static void test_send_evaluator_fixture__agent_is_registered(void) {
    // Given a send evaluator fixture
    ar_send_evaluator_fixture_t *fixture = ar_send_evaluator_fixture__create("test_register_agent_0_7_2");
    AR_ASSERT(fixture != NULL, "Fixture creation should succeed");

    // When creating and registering an agent
    ar_agent_t *agent = ar_send_evaluator_fixture__create_and_register_agent(fixture, 1, NULL);
    AR_ASSERT(agent != NULL, "Agent creation should succeed");

    // Then the agent should be registered in the agency (verify it exists)
    // Note: The actual agent ID is encoded in the returned pointer
    int64_t actual_agent_id = (int64_t)(intptr_t)agent;
    ar_agency_t *agency = ar_send_evaluator_fixture__get_agency(fixture);
    bool agent_exists = ar_agency__agent_exists(agency, actual_agent_id);
    AR_ASSERT(agent_exists == true, "Agent should be registered in agency");  // ← 0.7.2 assertion

    // Cleanup
    ar_send_evaluator_fixture__destroy(fixture);
}

int main(void) {
    printf("Starting send evaluator fixture tests...\n");

    // Iteration 0.1
    test_send_evaluator_fixture__create_returns_non_null();
    printf("test_send_evaluator_fixture__create_returns_non_null (0.1) passed!\n");

    // Iteration 0.2
    test_send_evaluator_fixture__create_evaluator_returns_non_null();
    printf("test_send_evaluator_fixture__create_evaluator_returns_non_null (0.2) passed!\n");

    // Iteration 0.3
    test_send_evaluator_fixture__create_frame_returns_non_null();
    printf("test_send_evaluator_fixture__create_frame_returns_non_null (0.3) passed!\n");

    // Iteration 0.4
    test_send_evaluator_fixture__get_delegation_returns_non_null();
    printf("test_send_evaluator_fixture__get_delegation_returns_non_null (0.4) passed!\n");

    // Iteration 0.5
    test_send_evaluator_fixture__get_agency_returns_non_null();
    printf("test_send_evaluator_fixture__get_agency_returns_non_null (0.5) passed!\n");

    // Iteration 0.6.1
    test_send_evaluator_fixture__create_and_register_delegate_returns_non_null();
    printf("test_send_evaluator_fixture__create_and_register_delegate_returns_non_null (0.6.1) passed!\n");

    // Iteration 0.6.2
    test_send_evaluator_fixture__delegate_is_registered();
    printf("test_send_evaluator_fixture__delegate_is_registered (0.6.2) passed!\n");

    // Iteration 0.7.1
    test_send_evaluator_fixture__create_and_register_agent_returns_non_null();
    printf("test_send_evaluator_fixture__create_and_register_agent_returns_non_null (0.7.1) passed!\n");

    // Iteration 0.7.2
    test_send_evaluator_fixture__agent_is_registered();
    printf("test_send_evaluator_fixture__agent_is_registered (0.7.2) passed!\n");

    printf("All send evaluator fixture tests passed! (9 tests covering 0.1-0.8)\n");
    return 0;
}
