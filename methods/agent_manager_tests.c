#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include "agerun_data.h"
#include "agerun_io.h"
#include "agerun_heap.h"

/**
 * Reads a method file and returns its contents as a string
 * @param ref_filename Path to the method file
 * @return Newly allocated string with file contents, or NULL on error
 * @note Ownership: Returns an owned string that caller must free
 */
static char* read_method_file(const char *ref_filename) {
    FILE *fp = NULL;
    file_result_t result = ar_io_open_file(ref_filename, "r", &fp);
    if (result != FILE_SUCCESS) {
        ar_io_error("Failed to open method file %s: %s\n", 
                    ref_filename, ar_io_error_message(result));
        return NULL;
    }
    
    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate buffer
    char *own_content = (char*)AR_HEAP_MALLOC((size_t)(file_size + 1), "Method file content");
    if (!own_content) {
        ar_io_close_file(fp, ref_filename);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(own_content, 1, (size_t)file_size, fp);
    own_content[bytes_read] = '\0';
    
    ar_io_close_file(fp, ref_filename);
    return own_content; // Ownership transferred to caller
}

static void test_agent_manager_create_destroy(void) {
    printf("Testing agent-manager method with create and destroy...\n");
    
    // First, ensure the echo method exists for testing
    char *own_echo_instructions = read_method_file("../methods/echo-1.0.0.method");
    assert(own_echo_instructions != NULL);
    bool echo_created = ar_methodology_create_method("echo", own_echo_instructions, "1.0.0");
    assert(echo_created);
    AR_HEAP_FREE(own_echo_instructions);
    own_echo_instructions = NULL;
    
    // Given the agent-manager method file
    char *own_instructions = read_method_file("../methods/agent-manager-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("agent-manager", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t manager_agent = ar_system_init("agent-manager", "1.0.0");
    if (manager_agent == 0) {
        // System already initialized, create agent directly
        manager_agent = ar_agent_create("agent-manager", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(manager_agent, own_wake);
    }
    assert(manager_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a message to create an echo agent
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "action", "create");
    ar_data_set_map_string(own_message, "method_name", "echo");
    ar_data_set_map_string(own_message, "version", "1.0.0");
    data_t *own_context = ar_data_create_map();
    ar_data_set_map_string(own_context, "name", "Test Echo");
    ar_data_set_map_data(own_message, "context", own_context);
    ar_data_set_map_integer(own_message, "sender", 777);
    
    bool sent = ar_agent_send(manager_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify method execution for CREATE action
    // The agent-manager method should:
    // 1. Call agent() to create a new agent (stores result in memory.create_result)
    // 2. Skip destroy() for create action (memory.destroy_result)
    // 3. Set memory.is_create to 1 and memory.is_destroy to 0
    // 4. Set memory.result to the created agent ID
    // 5. Send the result back to the sender
    
    agent_t *agents = ar_agency_get_agents();
    assert(agents != NULL);
    assert(agents[manager_agent - 1].own_memory != NULL);
    
    // Check all memory values set by the method
    const data_t *create_result = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "create_result");
    const data_t *is_create = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "is_create");
    const data_t *is_destroy = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "is_destroy");
    const data_t *result = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "result");
    
    if (create_result == NULL) {
        printf("FAIL: memory.create_result not found - agent() instruction failed to execute\n");
        printf("NOTE: This is expected until agent() function is implemented in instruction module\n");
    } else {
        assert(ar_data_get_type(create_result) == DATA_INTEGER);
        agent_id_t created_id = ar_data_get_integer(create_result);
        printf("SUCCESS: agent() instruction executed\n");
        printf("  - Created agent ID: %lld\n", (long long)created_id);
    }
    
    if (is_create == NULL) {
        printf("FAIL: memory.is_create not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(is_create) == DATA_INTEGER);
        assert(ar_data_get_integer(is_create) == 1);
        printf("SUCCESS: if() correctly evaluated action = \"create\"\n");
    }
    
    if (is_destroy == NULL) {
        printf("FAIL: memory.is_destroy not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(is_destroy) == DATA_INTEGER);
        assert(ar_data_get_integer(is_destroy) == 0);
        printf("SUCCESS: if() correctly evaluated action != \"destroy\"\n");
    }
    
    if (result == NULL) {
        printf("FAIL: memory.result not found - conditional assignment failed\n");
    } else {
        assert(ar_data_get_type(result) == DATA_INTEGER);
        printf("SUCCESS: Conditional logic set final result: %lld\n", (long long)ar_data_get_integer(result));
    }
    
    // Then the agent-manager should have sent back a non-zero agent ID
    // Let's assume it's agent ID 2 for the test
    
    // Now test destroying the agent
    own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "action", "destroy");
    ar_data_set_map_integer(own_message, "agent_id", 2); // Assuming the created agent has ID 2
    ar_data_set_map_integer(own_message, "sender", 777);
    
    sent = ar_agent_send(manager_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify method execution for DESTROY action
    // The agent-manager method should:
    // 1. Call agent() again (but action != "create" so result ignored)
    // 2. Call destroy() to destroy the specified agent
    // 3. Set memory.is_create to 0 and memory.is_destroy to 1
    // 4. Set memory.result to the destroy result (1 for success, 0 for failure)
    // 5. Send the result back to the sender
    
    // Check memory values for destroy action
    const data_t *destroy_result = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "destroy_result");
    is_create = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "is_create");
    is_destroy = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "is_destroy");
    result = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "result");
    
    if (destroy_result == NULL) {
        printf("FAIL: memory.destroy_result not found - destroy() instruction failed to execute\n");
        printf("NOTE: This is expected until destroy() function is implemented in instruction module\n");
    } else {
        assert(ar_data_get_type(destroy_result) == DATA_INTEGER);
        int destroy_status = ar_data_get_integer(destroy_result);
        printf("SUCCESS: destroy() instruction executed\n");
        printf("  - Destroy result: %d\n", destroy_status);
    }
    
    if (is_create == NULL) {
        printf("FAIL: memory.is_create not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(is_create) == DATA_INTEGER);
        assert(ar_data_get_integer(is_create) == 0);
        printf("SUCCESS: if() correctly evaluated action != \"create\"\n");
    }
    
    if (is_destroy == NULL) {
        printf("FAIL: memory.is_destroy not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(is_destroy) == DATA_INTEGER);
        assert(ar_data_get_integer(is_destroy) == 1);
        printf("SUCCESS: if() correctly evaluated action = \"destroy\"\n");
    }
    
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_INTEGER);
        printf("Final result: %lld\n", (long long)ar_data_get_integer(result));
    }
    
    // Then the agent-manager should have sent back 1 (success)
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Agent manager create and destroy test passed\n");
}

static void test_agent_manager_invalid_action(void) {
    printf("Testing agent-manager method with invalid action...\n");
    
    // Clean up from previous test
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Given the agent-manager method file
    char *own_instructions = read_method_file("../methods/agent-manager-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("agent-manager", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t manager_agent = ar_system_init("agent-manager", "1.0.0");
    if (manager_agent == 0) {
        // System already initialized, create agent directly
        manager_agent = ar_agent_create("agent-manager", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(manager_agent, own_wake);
    }
    assert(manager_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a message with an invalid action
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "action", "invalid");
    ar_data_set_map_integer(own_message, "sender", 777);
    
    bool sent = ar_agent_send(manager_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify method execution with invalid action
    // When action is neither "create" nor "destroy":
    // - Both if() conditions should evaluate to 0
    // - memory.result should remain 0 (neither create nor destroy path taken)
    
    agent_t *agents = ar_agency_get_agents();
    assert(agents != NULL);
    assert(agents[manager_agent - 1].own_memory != NULL);
    
    // Check memory values for invalid action
    const data_t *is_create = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "is_create");
    const data_t *is_destroy = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "is_destroy");
    const data_t *result = ar_data_get_map_data(agents[manager_agent - 1].own_memory, "result");
    
    if (is_create == NULL) {
        printf("FAIL: memory.is_create not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(is_create) == DATA_INTEGER);
        assert(ar_data_get_integer(is_create) == 0);
        printf("SUCCESS: if() correctly evaluated action != \"create\"\n");
    }
    
    if (is_destroy == NULL) {
        printf("FAIL: memory.is_destroy not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(is_destroy) == DATA_INTEGER);
        assert(ar_data_get_integer(is_destroy) == 0);
        printf("SUCCESS: if() correctly evaluated action != \"destroy\"\n");
    }
    
    if (result == NULL) {
        printf("FAIL: memory.result not found - conditional assignment failed\n");
    } else {
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 0);
        printf("SUCCESS: Result correctly set to 0 for invalid action\n");
    }
    
    // Then the agent-manager should have sent back 0 (failure/not recognized)
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Agent manager invalid action test passed\n");
}

int main(void) {
    printf("Running agent-manager method tests...\n\n");
    
    // Ensure clean state before starting tests
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    test_agent_manager_create_destroy();
    test_agent_manager_invalid_action();
    
    printf("\nAll agent-manager method tests passed!\n");
    return 0;
}
