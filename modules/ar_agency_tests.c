#include "ar_agency.h"
#include "ar_system.h"
#include "ar_agent.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_agency_count_agents(void);
static void test_agency_persistence(void);
static void test_agency_reset(void);
static void test_agency_instance_api(void);


static void test_agency_count_agents(void) {
    printf("Testing ar_agency__count_agents()...\n");
    
    // Given we have a count of existing agents
    int initial_count = ar_agency__count_agents();
    
    // And we have a test method
    const char *method_name = "count_test_method";
    const char *instructions = "message -> \"Count Test\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create several agents
    int num_agents_to_create = 3;
    int64_t agent_ids[3];
    
    for (int i = 0; i < num_agents_to_create; i++) {
        agent_ids[i] = ar_agency__create_agent(method_name, version, NULL);
        assert(agent_ids[i] > 0);
        // Process the wake message
        ar_system__process_next_message();
    }
    
    // Then the agent count should increase by the number of agents created
    int new_count = ar_agency__count_agents();
    assert(new_count == initial_count + num_agents_to_create);
    
    // When we destroy one agent
    bool result = ar_agency__destroy_agent(agent_ids[0]);
    
    // Then the destruction should succeed
    assert(result);
    
    // And the agent count should decrease by one
    int after_destroy_count = ar_agency__count_agents();
    assert(after_destroy_count == new_count - 1);
    
    // When we destroy the remaining agents
    for (int i = 1; i < num_agents_to_create; i++) {
        ar_agency__destroy_agent(agent_ids[i]);
    }
    
    // Then the agent count should return to the initial value
    int final_count = ar_agency__count_agents();
    assert(final_count == initial_count);
    
    printf("ar_agency__count_agents() test passed!\n");
}


static void test_agency_persistence(void) {
    printf("Testing agency persistence...\n");
    
    // Given a persistent method
    const char *method_name = "agency_persistence_method";
    const char *instructions = "message -> \"Agency Persistence Test\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    // And an agent created with this method
    int64_t agent_id = ar_agency__create_agent(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Process the wake message
    ar_system__process_next_message();
    
    // When we save agents to disk
    bool save_result = ar_agency__save_agents();
    
    // Then the save operation should succeed
    assert(save_result);
    
    // When we simulate a system restart
    ar_system__shutdown();
    int64_t new_agent_id = ar_system__init("agency_persistence_method", version);
    
    // Process the wake message if an agent was created
    if (new_agent_id > 0) {
        ar_system__process_next_message();
    }
    
    // And load the methods and agents
    bool load_methods_result = ar_methodology__load_methods();
    assert(load_methods_result);
    
    bool load_agents_result = ar_agency__load_agents();
    
    // Then the load operations should succeed
    assert(load_agents_result);
    
    // Process wake messages for any agents created during load
    // Based on the output, 2 agents are created when loading
    ar_system__process_next_message();
    ar_system__process_next_message();
    
    // And our persistent agent should still exist
    bool exists = ar_agency__agent_exists(agent_id);
    assert(exists);
    
    // Cleanup
    ar_agency__destroy_agent(agent_id);
    
    printf("Agency persistence test passed!\n");
}

static void test_agency_reset(void) {
    printf("Testing ar_agency__reset()...\n");
    
    // Given a test method
    const char *method_name = "reset_test_method";
    const char *instructions = "message -> \"Reset Test\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    // And an agent created with this method
    int64_t agent_id = ar_agency__create_agent(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Process the wake message
    ar_system__process_next_message();
    
    // And the agent exists in the system
    bool exists = ar_agency__agent_exists(agent_id);
    assert(exists);
    
    // When we reset the agency state
    ar_agency__reset();
    
    // Then the agent should no longer exist
    exists = ar_agency__agent_exists(agent_id);
    assert(!exists);
    
    // And the agent count should be zero
    int count = ar_agency__count_agents();
    assert(count == 0);
    
    printf("ar_agency__reset() test passed!\n");
}

int main(void) {
    printf("Starting Agency Module Tests...\n");
    
    // Given a test method and initialized system
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    int64_t init_agent_id = ar_system__init(method_name, version);
    assert(init_agent_id > 0);
    
    // Process the wake message
    ar_system__process_next_message();
    
    // When we run all agency tests
    test_agency_count_agents();
    test_agency_persistence();
    test_agency_reset();
    test_agency_instance_api();
    
    // Then clean up the system
    ar_system__shutdown();
    
    // And report success
    printf("All 7 tests passed!\n");
    return 0;
}

static void test_agency_instance_api(void) {
    printf("Testing instance-based API...\n");
    
    // Create an agency instance
    ar_agency_t *own_agency = ar_agency__create(NULL); // NULL methodology for backward compatibility
    assert(own_agency != NULL);
    
    // Test count agents with instance
    int count = ar_agency__count_agents_with_instance(own_agency);
    assert(count == 0);
    
    // Create method and register it
    const char *method_name = "instance_test_method";
    const char *instructions = "message -> \"Instance Test\"";
    ar_method_t *own_method = ar_method__create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method(own_method);
    
    // Create an agent using instance API
    int64_t agent_id = ar_agency__create_agent_with_instance(own_agency, method_name, "1.0.0", NULL);
    assert(agent_id > 0);
    
    // Process wake message
    ar_system__process_next_message();
    
    // Verify count increased
    count = ar_agency__count_agents_with_instance(own_agency);
    assert(count == 1);
    
    // Test send message with instance
    ar_data_t *own_message = ar_data__create_string("test");
    bool sent = ar_agency__send_to_agent_with_instance(own_agency, agent_id, own_message);
    assert(sent);
    
    // Process the message
    ar_system__process_next_message();
    
    // Get agent memory
    const ar_data_t *ref_memory = ar_agency__get_agent_memory_with_instance(own_agency, agent_id);
    assert(ref_memory != NULL);
    
    // Destroy agent
    bool destroyed = ar_agency__destroy_agent_with_instance(own_agency, agent_id);
    assert(destroyed);
    
    // Process sleep message
    ar_system__process_next_message();
    
    // Verify count decreased
    count = ar_agency__count_agents_with_instance(own_agency);
    assert(count == 0);
    
    // Clean up
    ar_agency__destroy(own_agency);
    
    // Clean up the method
    ar_methodology__unregister_method(method_name, "1.0.0");
    
    printf("Instance-based API test passed!\n");
}
