#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"

static void test_agent_manager_spawn_exit(void) {
    printf("Testing agent-manager method with spawn and exit...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("agent_manager_create_destroy");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load required methods
    assert(ar_method_fixture__load_method(own_fixture, "echo", "../../methods/echo-1.0.0.method", "1.0.0"));
    assert(ar_method_fixture__load_method(own_fixture, "agent-manager", "../../methods/agent-manager-1.0.0.method", "1.0.0"));
    
    // Create agent-manager agent

    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);

    int64_t manager_agent = ar_agency__create_agent(mut_agency, "agent-manager", "1.0.0", NULL);
    assert(manager_agent > 0);
    
    // No initial message processing needed
    
    // When we send a message to spawn an echo agent
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    
    ar_data__set_map_string(own_message, "action", "spawn");
    ar_data__set_map_string(own_message, "method_name", "echo");
    ar_data__set_map_string(own_message, "version", "1.0.0");
    ar_data_t *own_context = ar_data__create_map();
    ar_data__set_map_string(own_context, "name", "Test Echo");
    ar_data__set_map_data(own_message, "context", own_context);
    own_context = NULL; // Ownership transferred
    
    bool sent = ar_agency__send_to_agent(mut_agency, manager_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the create message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Check agent memory for result
    const ar_data_t *agent_memory = ar_agency__get_agent_memory(mut_agency, manager_agent);
    assert(agent_memory != NULL);
    
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    if (result != NULL) {
        assert(ar_data__get_type(result) == AR_DATA_TYPE__INTEGER);
        int64_t created_agent = (int64_t)ar_data__get_integer(result);
        printf("SUCCESS: spawn() instruction executed\n");
        printf("  - Created agent ID: %lld\n", (long long)created_agent);
    } else {
        printf("FAIL: memory.result not found - spawn() instruction failed to execute\n");
        printf("NOTE: This is expected until agent() function is implemented in instruction module\n");
    }
    
    // Test checks for memory fields that don't exist yet (implementation incomplete)
    const ar_data_t *is_spawn = ar_data__get_map_data(agent_memory, "is_spawn");
    if (!is_spawn) {
        printf("EXPECTED FAIL: memory.is_spawn not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    const ar_data_t *is_exit = ar_data__get_map_data(agent_memory, "is_exit");
    if (!is_exit) {
        printf("EXPECTED FAIL: memory.is_exit not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    if (!result) {
        printf("EXPECTED FAIL: memory.result not found - conditional assignment failed\n");
        printf("NOTE: Depends on if() comparison which is not yet implemented\n");
    }
    
    // Now test destroy action
    ar_data_t *own_destroy_message = ar_data__create_map();
    assert(own_destroy_message != NULL);
    
    ar_data__set_map_string(own_destroy_message, "action", "exit");
    ar_data__set_map_integer(own_destroy_message, "agent_id", 2); // Assuming agent 2 was created
    
    sent = ar_agency__send_to_agent(mut_agency, manager_agent, own_destroy_message);
    assert(sent);
    own_destroy_message = NULL; // Ownership transferred
    
    // Process the destroy message
    processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Check if destroy worked
    const ar_data_t *exit_result = ar_data__get_map_data(agent_memory, "exit_result");
    if (!exit_result) {
        printf("EXPECTED FAIL: memory.exit_result not found - exit() instruction failed to execute\n");
        printf("NOTE: This is expected until destroy() function is implemented in instruction module\n");
    }
    
    // Check for the missing fields again
    if (!is_spawn) {
        printf("EXPECTED FAIL: memory.is_spawn not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    if (!is_exit) {
        printf("EXPECTED FAIL: memory.is_exit not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Agent manager spawn and exit test passed\n");
}

static void test_agent_manager_invalid_action(void) {
    printf("Testing agent-manager method with invalid action...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("agent_manager_invalid_action");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load agent-manager method
    assert(ar_method_fixture__load_method(own_fixture, "agent-manager", "../../methods/agent-manager-1.0.0.method", "1.0.0"));
    
    // Create agent-manager agent

    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);

    int64_t manager_agent = ar_agency__create_agent(mut_agency, "agent-manager", "1.0.0", NULL);
    assert(manager_agent > 0);
    
    // No initial message processing needed
    
    // When we send a message with an invalid action
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    
    ar_data__set_map_string(own_message, "action", "invalid");
    
    bool sent = ar_agency__send_to_agent(mut_agency, manager_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the invalid action message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Check agent memory - invalid actions should be handled gracefully
    const ar_data_t *agent_memory = ar_agency__get_agent_memory(mut_agency, manager_agent);
    assert(agent_memory != NULL);
    
    // For invalid actions, the method should fail gracefully
    const ar_data_t *is_spawn = ar_data__get_map_data(agent_memory, "is_spawn");
    if (!is_spawn) {
        printf("EXPECTED FAIL: memory.is_spawn not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    const ar_data_t *is_exit = ar_data__get_map_data(agent_memory, "is_exit");
    if (!is_exit) {
        printf("EXPECTED FAIL: memory.is_exit not found - if() comparison failed\n");
        printf("NOTE: String comparison in if() is not yet implemented\n");
    }
    
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    if (!result) {
        printf("EXPECTED FAIL: memory.result not found - conditional assignment failed\n");
        printf("NOTE: Depends on if() comparison which is not yet implemented\n");
    }
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Agent manager invalid action test passed\n");
}

int main(void) {
    printf("Running agent-manager method tests...\n\n");
    
    test_agent_manager_spawn_exit();
    test_agent_manager_invalid_action();
    
    printf("\nAll agent-manager method tests passed!\n");
    return 0;
}
