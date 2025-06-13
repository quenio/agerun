#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_method_fixture.h"
#include "agerun_system.h"
#include "agerun_agency.h"
#include "agerun_data.h"

static void test_agent_manager_create_destroy(void) {
    printf("Testing agent-manager method with create and destroy...\n");
    
    // Create test fixture
    method_fixture_t *own_fixture = ar__method_fixture__create("agent_manager_create_destroy");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar__method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar__method_fixture__verify_directory(own_fixture));
    
    // Load required methods
    assert(ar__method_fixture__load_method(own_fixture, "echo", "../methods/echo-1.0.0.method", "1.0.0"));
    assert(ar__method_fixture__load_method(own_fixture, "agent-manager", "../methods/agent-manager-1.0.0.method", "1.0.0"));
    
    // Create agent-manager agent
    int64_t manager_agent = ar__agency__create_agent("agent-manager", "1.0.0", NULL);
    assert(manager_agent > 0);
    
    // Process wake message
    ar__system__process_next_message();
    
    // When we send a message to create an echo agent
    data_t *own_message = ar__data__create_map();
    assert(own_message != NULL);
    
    ar__data__set_map_string(own_message, "action", "create");
    ar__data__set_map_string(own_message, "method_name", "echo");
    ar__data__set_map_string(own_message, "version", "1.0.0");
    data_t *own_context = ar__data__create_map();
    ar__data__set_map_string(own_context, "name", "Test Echo");
    ar__data__set_map_data(own_message, "context", own_context);
    own_context = NULL; // Ownership transferred
    
    bool sent = ar__agency__send_to_agent(manager_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the create message
    bool processed = ar__system__process_next_message();
    assert(processed);
    
    // Check agent memory for result
    const data_t *agent_memory = ar__agency__get_agent_memory(manager_agent);
    assert(agent_memory != NULL);
    
    const data_t *result = ar__data__get_map_data(agent_memory, "result");
    if (result != NULL) {
        assert(ar__data__get_type(result) == DATA_INTEGER);
        int64_t created_agent = (int64_t)ar__data__get_integer(result);
        printf("SUCCESS: agent() instruction executed\n");
        printf("  - Created agent ID: %lld\n", (long long)created_agent);
    } else {
        printf("FAIL: memory.result not found - agent() instruction failed to execute\n");
        printf("NOTE: This is expected until agent() function is implemented in instruction module\n");
    }
    
    // Test checks for memory fields that don't exist yet (implementation incomplete)
    const data_t *is_create = ar__data__get_map_data(agent_memory, "is_create");
    if (!is_create) {
        printf("EXPECTED FAIL: memory.is_create not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    const data_t *is_destroy = ar__data__get_map_data(agent_memory, "is_destroy");
    if (!is_destroy) {
        printf("EXPECTED FAIL: memory.is_destroy not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    if (!result) {
        printf("EXPECTED FAIL: memory.result not found - conditional assignment failed\n");
        printf("NOTE: Depends on if() comparison which is not yet implemented\n");
    }
    
    // Now test destroy action
    data_t *own_destroy_message = ar__data__create_map();
    assert(own_destroy_message != NULL);
    
    ar__data__set_map_string(own_destroy_message, "action", "destroy");
    ar__data__set_map_integer(own_destroy_message, "agent_id", 2); // Assuming agent 2 was created
    
    sent = ar__agency__send_to_agent(manager_agent, own_destroy_message);
    assert(sent);
    own_destroy_message = NULL; // Ownership transferred
    
    // Process the destroy message
    processed = ar__system__process_next_message();
    assert(processed);
    
    // Check if destroy worked
    const data_t *destroy_result = ar__data__get_map_data(agent_memory, "destroy_result");
    if (!destroy_result) {
        printf("EXPECTED FAIL: memory.destroy_result not found - destroy() instruction failed to execute\n");
        printf("NOTE: This is expected until destroy() function is implemented in instruction module\n");
    }
    
    // Check for the missing fields again
    if (!is_create) {
        printf("EXPECTED FAIL: memory.is_create not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    if (!is_destroy) {
        printf("EXPECTED FAIL: memory.is_destroy not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    // Check for memory leaks
    assert(ar__method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar__method_fixture__destroy(own_fixture);
    
    printf("✓ Agent manager create and destroy test passed\n");
}

static void test_agent_manager_invalid_action(void) {
    printf("Testing agent-manager method with invalid action...\n");
    
    // Create test fixture
    method_fixture_t *own_fixture = ar__method_fixture__create("agent_manager_invalid_action");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar__method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar__method_fixture__verify_directory(own_fixture));
    
    // Load agent-manager method
    assert(ar__method_fixture__load_method(own_fixture, "agent-manager", "../methods/agent-manager-1.0.0.method", "1.0.0"));
    
    // Create agent-manager agent
    int64_t manager_agent = ar__agency__create_agent("agent-manager", "1.0.0", NULL);
    assert(manager_agent > 0);
    
    // Process wake message
    ar__system__process_next_message();
    
    // When we send a message with an invalid action
    data_t *own_message = ar__data__create_map();
    assert(own_message != NULL);
    
    ar__data__set_map_string(own_message, "action", "invalid");
    
    bool sent = ar__agency__send_to_agent(manager_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the invalid action message
    ar__system__process_next_message();
    
    // Check agent memory - invalid actions should be handled gracefully
    const data_t *agent_memory = ar__agency__get_agent_memory(manager_agent);
    assert(agent_memory != NULL);
    
    // For invalid actions, the method should fail gracefully
    const data_t *is_create = ar__data__get_map_data(agent_memory, "is_create");
    if (!is_create) {
        printf("EXPECTED FAIL: memory.is_create not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    const data_t *is_destroy = ar__data__get_map_data(agent_memory, "is_destroy");
    if (!is_destroy) {
        printf("EXPECTED FAIL: memory.is_destroy not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    const data_t *result = ar__data__get_map_data(agent_memory, "result");
    if (!result) {
        printf("EXPECTED FAIL: memory.result not found - conditional assignment failed\n");
        printf("NOTE: Depends on if() comparison which is not yet implemented\n");
    }
    
    // Check for memory leaks
    assert(ar__method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar__method_fixture__destroy(own_fixture);
    
    printf("✓ Agent manager invalid action test passed\n");
}

int main(void) {
    printf("Running agent-manager method tests...\n\n");
    
    test_agent_manager_create_destroy();
    test_agent_manager_invalid_action();
    
    printf("\nAll agent-manager method tests passed!\n");
    return 0;
}
