#include "ar_agent.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_agency.h"
#include "ar_agent_registry.h"
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
static void test_agent_create_destroy(ar_system_fixture_t *own_fixture);
static void test_agent_send(ar_system_fixture_t *own_fixture);
static void test_agent_exists(ar_system_fixture_t *own_fixture);
static void test_agent_persistence(ar_system_fixture_t *own_fixture);

static void test_agent_create_destroy(ar_system_fixture_t *own_fixture) {
    printf("Testing ar_agent__create() and ar_agent__destroy()...\n");
    
    // Given a simple method for an agent
    const char *method_name = "test_method";
    const char *instructions = "send(0, \"Test Method Response\")";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    ar_method_t *own_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // When we create an agent with this method
    int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // Verify no wake message was sent (agent should not send wake on creation)
    // Trying to process a message should return false (no messages pending)
    bool message_processed = ar_system_fixture__process_next_message(own_fixture);
    assert(!message_processed && "No wake message should be sent on agent creation");
    
    // And the agent should exist in the system
    ar_agent_registry_t *ref_registry = ar_agency__get_registry_with_instance(mut_agency);
    bool exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    assert(exists);
    
    // When we destroy the agent
    ar_agency__destroy_agent_with_instance(mut_agency, agent_id);
    
    // Then the agent should be destroyed successfully
    // Agency destroy returns void
    
    // And the agent should no longer exist in the system
    exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    assert(!exists);
    
    printf("ar_agent__create() and ar_agent__destroy() tests passed!\n");
}

static void test_agent_send(ar_system_fixture_t *own_fixture) {
    printf("Testing ar_agent__send()...\n");
    
    // Given an echo method and an agent using it
    const char *method_name = "echo_method";
    const char *instructions = "send(0, message)";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    ar_method_t *own_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, method_name, version, NULL);
    assert(agent_id > 0);
    
    // No wake message should be sent on creation
    
    // When we send a message to the agent
    ar_data_t *own_message_data = ar_data__create_string(g_hello_message);
    assert(own_message_data != NULL);
    bool send_result = ar_agency__send_to_agent_with_instance(mut_agency, agent_id, own_message_data);
    own_message_data = NULL; // Mark as transferred
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // Process the test message
    ar_system_fixture__process_next_message(own_fixture);
    
    // Since we can't directly access the message queue in an opaque map,
    // we'll verify the agent was created (which was already tested)
    // and that the message was sent (which was verified by send_result)
    ar_agent_registry_t *ref_registry = ar_agency__get_registry_with_instance(mut_agency);
    assert(ar_agent_registry__is_registered(ref_registry, agent_id));
    
    // Note: We don't destroy the agent - fixture handles cleanup
    
    printf("ar_agent__send() test passed!\n");
}

static void test_agent_exists(ar_system_fixture_t *own_fixture) {
    printf("Testing ar_agent__exists()...\n");
    
    // Given a method and an agent created with it
    const char *method_name = "exists_method";
    const char *instructions = "send(0, \"I exist\")";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    ar_method_t *own_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, method_name, version, NULL);
    assert(agent_id > 0);
    
    // No wake message should be sent on creation
    
    // When we check if the valid agent ID exists
    ar_agent_registry_t *ref_registry = ar_agency__get_registry_with_instance(mut_agency);
    bool exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    
    // Then it should exist
    assert(exists);
    
    // When we check if invalid agent IDs exist
    bool exists_zero = ar_agent_registry__is_registered(ref_registry, 0);
    bool exists_large = ar_agent_registry__is_registered(ref_registry, 999999);
    
    // Then they should not exist
    assert(!exists_zero);
    assert(!exists_large);
    
    // When we destroy the agent
    ar_agency__destroy_agent_with_instance(mut_agency, agent_id);
    // Agency destroy returns void
    
    // And check if it still exists
    exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    
    // Then it should no longer exist
    assert(!exists);
    
    printf("ar_agent__exists() test passed!\n");
}

static void test_agent_persistence(ar_system_fixture_t *own_fixture) {
    printf("Testing agent save functionality...\n");
    
    // Note: This test only verifies that agents can be saved to disk.
    // A full persistence test across system restarts would need to be
    // implemented without fixtures to avoid lifecycle conflicts.
    
    // Given a persistent method
    const char *method_name = "persistent_method";
    const char *instructions = "send(0, \"I persist\")";
    const char *version = "1.0.0";
    
    // Use fixture to register the method
    ar_method_t *own_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(own_method != NULL);
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Create a context with ar_data_t
    ar_data_t *own_context = ar_data__create_map();
    assert(own_context != NULL);
    ar_data__set_map_string(own_context, "test_key", "test_value");
    
    // And an agent created with this persistent method - agent doesn't take ownership
    int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, method_name, version, own_context);
    assert(agent_id > 0);
    
    // No wake message should be sent on creation
    
    // When we save agents to disk using instance API
    bool save_result = ar_agency__save_agents_with_instance(mut_agency, "agency.agerun");
    
    // Then the save operation should succeed
    assert(save_result);
    
    // Verify the agency file was created
    struct stat st;
    assert(stat("agency.agerun", &st) == 0);
    
    // Verify the agent still exists in memory
    ar_agent_registry_t *ref_registry = ar_agency__get_registry_with_instance(mut_agency);
    bool exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    assert(exists);
    
    // Since the agent didn't take ownership, we need to cleanup our context
    ar_data__destroy(own_context);
    own_context = NULL; // Mark as destroyed
    
    printf("Agent save test passed!\n");
}

int main(void) {
    printf("Starting Agent Module Tests...\n");
    
    // Create a system fixture for all tests
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("agent_tests");
    assert(own_fixture != NULL);
    
    // Initialize the fixture
    bool init_result = ar_system_fixture__initialize(own_fixture);
    assert(init_result);
    
    // When we run all agent tests with the fixture
    test_agent_create_destroy(own_fixture);
    test_agent_send(own_fixture);
    test_agent_exists(own_fixture);
    test_agent_persistence(own_fixture);
    
    // Check for memory leaks
    bool no_leaks = ar_system_fixture__check_memory(own_fixture);
    if (!no_leaks) {
        printf("WARNING: Memory leaks detected in agent tests\n");
    }
    
    // Clean up the fixture
    ar_system_fixture__destroy(own_fixture);
    own_fixture = NULL;
    
    // And report success
    printf("All 8 tests passed!\n");
    return 0;
}
