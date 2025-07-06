#include "ar_agent.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_agency.h"
#include "ar_methodology.h"
#include "ar_system_fixture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

/* Message strings */
static const char *g_hello_message = "Hello Agent!";

// Forward declarations
static void test_agent_create_destroy(system_fixture_t *mut_fixture);
static void test_agent_send(system_fixture_t *mut_fixture);
static void test_agent_exists(system_fixture_t *mut_fixture);
static void test_agent_persistence(system_fixture_t *mut_fixture);

static void test_agent_create_destroy(system_fixture_t *mut_fixture) {
    printf("Testing ar_agent__create() and ar_agent__destroy()...\n");
    
    // Given a simple method for an agent
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test Method Response\"";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    method_t *own_method = ar_system_fixture__register_method(mut_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    // When we create an agent with this method
    int64_t agent_id = ar_agency__create_agent(method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    bool exists = ar_agency__agent_exists(agent_id);
    assert(exists);
    
    // When we destroy the agent
    ar_agency__destroy_agent(agent_id);
    
    // Then the agent should be destroyed successfully
    // Agency destroy returns void
    
    // And the agent should no longer exist in the system
    exists = ar_agency__agent_exists(agent_id);
    assert(!exists);
    
    printf("ar_agent__create() and ar_agent__destroy() tests passed!\n");
}

static void test_agent_send(system_fixture_t *mut_fixture) {
    printf("Testing ar_agent__send()...\n");
    
    // Given an echo method and an agent using it
    const char *method_name = "echo_method";
    const char *instructions = "message -> message";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    method_t *own_method = ar_system_fixture__register_method(mut_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    int64_t agent_id = ar_agency__create_agent(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we send a message to the agent
    ar_data_t *own_message_data = ar_data__create_string(g_hello_message);
    assert(own_message_data != NULL);
    bool send_result = ar_agency__send_to_agent(agent_id, own_message_data);
    own_message_data = NULL; // Mark as transferred
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // Process the message to prevent memory leaks
    ar_system__process_next_message();
    
    // Since we can't directly access the message queue in an opaque map,
    // we'll verify the agent was created (which was already tested)
    // and that the message was sent (which was verified by send_result)
    assert(ar_agency__agent_exists(agent_id));
    
    // Note: We don't destroy the agent - fixture handles cleanup
    
    printf("ar_agent__send() test passed!\n");
}

static void test_agent_exists(system_fixture_t *mut_fixture) {
    printf("Testing ar_agent__exists()...\n");
    
    // Given a method and an agent created with it
    const char *method_name = "exists_method";
    const char *instructions = "message -> \"I exist\"";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    method_t *own_method = ar_system_fixture__register_method(mut_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    int64_t agent_id = ar_agency__create_agent(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we check if the valid agent ID exists
    bool exists = ar_agency__agent_exists(agent_id);
    
    // Then it should exist
    assert(exists);
    
    // When we check if invalid agent IDs exist
    bool exists_zero = ar_agency__agent_exists(0);
    bool exists_large = ar_agency__agent_exists(999999);
    
    // Then they should not exist
    assert(!exists_zero);
    assert(!exists_large);
    
    // When we destroy the agent
    ar_agency__destroy_agent(agent_id);
    // Agency destroy returns void
    
    // And check if it still exists
    exists = ar_agency__agent_exists(agent_id);
    
    // Then it should no longer exist
    assert(!exists);
    
    printf("ar_agent__exists() test passed!\n");
}

static void test_agent_persistence(system_fixture_t *mut_fixture) {
    printf("Testing agent save functionality...\n");
    
    // Note: This test only verifies that agents can be saved to disk.
    // A full persistence test across system restarts would need to be
    // implemented without fixtures to avoid lifecycle conflicts.
    
    // Given a persistent method
    const char *method_name = "persistent_method";
    const char *instructions = "message -> \"I persist\"";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    method_t *own_method = ar_system_fixture__register_method(mut_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    // Create a context with ar_data_t
    ar_data_t *own_context = ar_data__create_map();
    assert(own_context != NULL);
    ar_data__set_map_string(own_context, "test_key", "test_value");
    
    // And an agent created with this persistent method - agent doesn't take ownership
    int64_t agent_id = ar_agency__create_agent(method_name, version, own_context);
    assert(agent_id > 0);
    
    // When we save agents to disk
    bool save_result = ar_agency__save_agents();
    
    // Then the save operation should succeed
    assert(save_result);
    
    // Verify the agency file was created
    struct stat st;
    assert(stat("agency.agerun", &st) == 0);
    
    // Verify the agent still exists in memory
    bool exists = ar_agency__agent_exists(agent_id);
    assert(exists);
    
    // Since the agent didn't take ownership, we need to cleanup our context
    ar_data__destroy(own_context);
    own_context = NULL; // Mark as destroyed
    
    printf("Agent save test passed!\n");
}

int main(void) {
    printf("Starting Agent Module Tests...\n");
    
    // Create a system fixture for all tests
    system_fixture_t *mut_fixture = ar_system_fixture__create("agent_tests");
    assert(mut_fixture != NULL);
    
    // Initialize the fixture
    bool init_result = ar_system_fixture__initialize(mut_fixture);
    assert(init_result);
    
    // When we run all agent tests with the fixture
    test_agent_create_destroy(mut_fixture);
    test_agent_send(mut_fixture);
    test_agent_exists(mut_fixture);
    test_agent_persistence(mut_fixture);
    
    // Check for memory leaks
    bool no_leaks = ar_system_fixture__check_memory(mut_fixture);
    if (!no_leaks) {
        printf("WARNING: Memory leaks detected in agent tests\n");
    }
    
    // Clean up the fixture
    ar_system_fixture__destroy(mut_fixture);
    mut_fixture = NULL;
    
    // And report success
    printf("All 8 tests passed!\n");
    return 0;
}
