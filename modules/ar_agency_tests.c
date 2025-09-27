#include "ar_agency.h"
#include "ar_system.h"
#include "ar_agent.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_system_fixture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_agency_count_agents(ar_system_fixture_t *own_fixture);
// static void test_agency_persistence(ar_system_fixture_t *own_fixture);
static void test_agency_reset(ar_system_fixture_t *own_fixture);
static void test_agency_instance_api(ar_system_fixture_t *own_fixture);


static void test_agency_count_agents(ar_system_fixture_t *own_fixture) {
    printf("Testing ar_agency__count_agents()...\n");
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Given we have a count of existing agents
    int initial_count = ar_agency__count_agents(mut_agency);
    
    // And we have a test method
    const char *method_name = "count_test_method";
    const char *instructions = "send(0, \"Count Test\")";
    const char *version = "1.0.0";
    
    // Register method with the fixture (which handles methodology)
    ar_method_t *ref_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(ref_method != NULL);
    
    // When we create several agents
    int num_agents_to_create = 3;
    int64_t agent_ids[3];
    
    for (int i = 0; i < num_agents_to_create; i++) {
        agent_ids[i] = ar_agency__create_agent(mut_agency, method_name, version, NULL);
        assert(agent_ids[i] > 0);
    }
    
    // Then the agent count should increase by the number of agents created
    int new_count = ar_agency__count_agents(mut_agency);
    assert(new_count == initial_count + num_agents_to_create);
    
    // When we destroy one agent
    bool result = ar_agency__destroy_agent(mut_agency, agent_ids[0]);
    
    // Then the destruction should succeed
    assert(result);
    
    // And the agent count should decrease by one
    int after_destroy_count = ar_agency__count_agents(mut_agency);
    assert(after_destroy_count == new_count - 1);
    
    // When we destroy the remaining agents
    for (int i = 1; i < num_agents_to_create; i++) {
        ar_agency__destroy_agent(mut_agency, agent_ids[i]);
    }
    
    // Then the agent count should return to the initial value
    int final_count = ar_agency__count_agents(mut_agency);
    assert(final_count == initial_count);
    
    printf("ar_agency__count_agents() test passed!\n");
}


/* TODO: Fix persistence test - needs proper instance-based save/load
static void test_agency_persistence(ar_system_fixture_t *own_fixture) {
    printf("Testing agency persistence...\n");
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Given a persistent method
    const char *method_name = "agency_persistence_method";
    const char *instructions = "send(0, \"Agency Persistence Test\")";
    const char *version = "1.0.0";
    
    // Register method with the fixture
    ar_method_t *ref_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(ref_method != NULL);
    
    // And an agent created with this method
    int64_t agent_id = ar_agency__create_agent(mut_agency, method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we save agents to disk
    bool save_result = ar_agency__save_agents(mut_agency, NULL);
    
    // Then the save operation should succeed
    assert(save_result);
    
    // When we simulate a system restart
    ar_system_fixture__shutdown_preserve_files(own_fixture);
    ar_system_fixture__reset_system(own_fixture);
    
    // Debug: ensure fixture is reinitialized
    printf("After reset, checking fixture state...\n");
    
    // Get the new agency after reset
    ar_agency_t *mut_agency_after_reset = ar_system_fixture__get_agency(own_fixture);
    if (!mut_agency_after_reset) {
        printf("ERROR: Agency is NULL after reset!\n");
    }
    assert(mut_agency_after_reset != NULL);
    
    // Get the methodology from the new agency
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency_after_reset);
    
    // And load the methods and agents
    bool load_methods_result = ar_methodology__load_methods(mut_methodology, NULL);
    assert(load_methods_result);
    
    bool load_agents_result = ar_agency__load_agents(mut_agency_after_reset, NULL);
    
    // Then the load operations should succeed
    assert(load_agents_result);
    
    // And our persistent agent should still exist
    ar_agent_registry_t *ref_registry = ar_agency__get_registry(mut_agency_after_reset);
    bool exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    assert(exists);
    
    // Cleanup
    ar_agency__destroy_agent(mut_agency_after_reset, agent_id);
    
    printf("Agency persistence test passed!\n");
}
*/

static void test_agency_reset(ar_system_fixture_t *own_fixture) {
    printf("Testing ar_agency__reset()...\n");
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Given a test method
    const char *method_name = "reset_test_method";
    const char *instructions = "send(0, \"Reset Test\")";
    const char *version = "1.0.0";
    
    // Register method with the fixture
    ar_method_t *ref_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(ref_method != NULL);
    
    // And an agent created with this method
    int64_t agent_id = ar_agency__create_agent(mut_agency, method_name, version, NULL);
    assert(agent_id > 0);
    
    // And the agent exists in the system
    ar_agent_registry_t *ref_registry = ar_agency__get_registry(mut_agency);
    bool exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    assert(exists);
    
    // When we reset the agency state
    ar_agency__reset(mut_agency);
    
    // Then the agent should no longer exist
    exists = ar_agent_registry__is_registered(ref_registry, agent_id);
    assert(!exists);
    
    // And the agent count should be zero
    int count = ar_agency__count_agents(mut_agency);
    assert(count == 0);
    
    printf("ar_agency__reset() test passed!\n");
}

int main(void) {
    printf("Starting Agency Module Tests...\n");
    
    // Create fixture for testing
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("agency_tests");
    assert(own_fixture != NULL);
    
    // Initialize the system
    bool init_result = ar_system_fixture__initialize(own_fixture);
    assert(init_result);
    
    // Given a test method
    const char *method_name = "test_method";
    const char *instructions = "send(0, \"Test\")";
    const char *version = "1.0.0";
    
    // Register the test method
    ar_method_t *ref_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, version);
    assert(ref_method != NULL);
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Create initial agent
    int64_t init_agent_id = ar_agency__create_agent(mut_agency, method_name, version, NULL);
    assert(init_agent_id > 0);
    
    // When we run all agency tests
    test_agency_count_agents(own_fixture);
    // TODO: Fix persistence test - needs proper instance-based save/load
    // test_agency_persistence(own_fixture);
    test_agency_reset(own_fixture);
    test_agency_instance_api(own_fixture);
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
    
    // And report success
    printf("All tests passed!\n");
    return 0;
}

static void test_agency_instance_api(ar_system_fixture_t *own_fixture) {
    printf("Testing instance-based API...\n");
    
    // Use the fixture's agency to test instance-based APIs
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Test count agents with instance
    int initial_count = ar_agency__count_agents(mut_agency);
    
    // Create method and register it with the fixture
    const char *method_name = "instance_test_method";
    const char *instructions = "send(0, \"Instance Test\")";
    ar_method_t *ref_method = ar_system_fixture__register_method(own_fixture, method_name, instructions, "1.0.0");
    assert(ref_method != NULL);
    
    // Create an agent using instance API
    int64_t agent_id = ar_agency__create_agent(mut_agency, method_name, "1.0.0", NULL);
    assert(agent_id > 0);
    
    // Verify count increased
    int count = ar_agency__count_agents(mut_agency);
    assert(count == initial_count + 1);
    
    // Test send message with instance
    ar_data_t *own_message = ar_data__create_string("test");
    bool sent = ar_agency__send_to_agent(mut_agency, agent_id, own_message);
    assert(sent);
    
    // Process the message
    ar_system_fixture__process_next_message(own_fixture);
    
    // Get agent memory
    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, agent_id);
    assert(ref_memory != NULL);
    
    // Destroy agent
    bool destroyed = ar_agency__destroy_agent(mut_agency, agent_id);
    assert(destroyed);
    
    // Process any cleanup messages
    ar_system_fixture__process_next_message(own_fixture);
    
    // Verify count decreased
    count = ar_agency__count_agents(mut_agency);
    assert(count == initial_count);
    
    printf("Instance-based API test passed!\n");
}
