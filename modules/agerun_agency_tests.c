#include "agerun_agency.h"
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_agency_init_state(void);
static void test_agency_count_agents(void);
static void test_agency_next_id(void);
static void test_agency_persistence(void);
static void test_agency_reset(void);

static void test_agency_init_state(void) {
    printf("Testing agency initialization state...\n");
    
    // Given the agency system has been initialized
    
    // When we get the agents array
    agent_t *agents = ar_agency_get_agents();
    
    // Then the array should exist
    assert(agents != NULL);
    
    // When we get the next agent ID
    agent_id_t next_id = ar_agency_get_next_id();
    
    // Then it should be set to a positive value
    assert(next_id > 0);
    
    printf("Agency initialization state test passed!\n");
}

static void test_agency_count_agents(void) {
    printf("Testing ar_agency_count_agents()...\n");
    
    // Given we have a count of existing agents
    int initial_count = ar_agency_count_agents();
    
    // And we have a test method
    const char *method_name = "count_test_method";
    const char *instructions = "message -> \"Count Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // When we create several agents
    int num_agents_to_create = 3;
    agent_id_t agent_ids[3];
    
    for (int i = 0; i < num_agents_to_create; i++) {
        agent_ids[i] = ar_agent_create(method_name, version, NULL);
        assert(agent_ids[i] > 0);
    }
    
    // Then the agent count should increase by the number of agents created
    int new_count = ar_agency_count_agents();
    assert(new_count == initial_count + num_agents_to_create);
    
    // When we destroy one agent
    bool result = ar_agent_destroy(agent_ids[0]);
    
    // Then the destruction should succeed
    assert(result);
    
    // And the agent count should decrease by one
    int after_destroy_count = ar_agency_count_agents();
    assert(after_destroy_count == new_count - 1);
    
    // When we destroy the remaining agents
    for (int i = 1; i < num_agents_to_create; i++) {
        ar_agent_destroy(agent_ids[i]);
    }
    
    // Then the agent count should return to the initial value
    int final_count = ar_agency_count_agents();
    assert(final_count == initial_count);
    
    printf("ar_agency_count_agents() test passed!\n");
}

static void test_agency_next_id(void) {
    printf("Testing ar_agency_get_next_id() and ar_agency_set_next_id()...\n");
    
    // Given we have the current next agent ID
    agent_id_t current_next_id = ar_agency_get_next_id();
    
    // When we set a new next ID
    agent_id_t new_next_id = current_next_id + 1000;
    ar_agency_set_next_id(new_next_id);
    
    // Then the next ID should be updated correctly
    agent_id_t retrieved_next_id = ar_agency_get_next_id();
    assert(retrieved_next_id == new_next_id);
    
    // Given we have a test method
    const char *method_name = "next_id_test_method";
    const char *instructions = "message -> \"Next ID Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // When we create a new agent
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    
    // Then the agent should be assigned the current next ID
    assert(agent_id == new_next_id);
    
    // And the next ID should be incremented
    retrieved_next_id = ar_agency_get_next_id();
    assert(retrieved_next_id == new_next_id + 1);
    
    // Cleanup
    ar_agent_destroy(agent_id);
    
    // Restore the original next ID value
    ar_agency_set_next_id(current_next_id);
    
    printf("ar_agency_get_next_id() and ar_agency_set_next_id() tests passed!\n");
}

static void test_agency_persistence(void) {
    printf("Testing agency persistence...\n");
    
    // Given a persistent method
    const char *method_name = "agency_persistence_method";
    const char *instructions = "message -> \"Agency Persistence Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, true);
    assert(version > 0);
    
    // And an agent created with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we save agents to disk
    bool save_result = ar_agency_save_agents();
    
    // Then the save operation should succeed
    assert(save_result);
    
    // Given we capture the current next ID
    agent_id_t next_id = ar_agency_get_next_id();
    
    // When we simulate a system restart
    ar_system_shutdown();
    ar_system_init("agency_persistence_method", version);
    
    // And restore the next ID
    ar_agency_set_next_id(next_id);
    
    // And load the methods and agents
    bool load_methods_result = ar_methodology_load_methods();
    assert(load_methods_result);
    
    bool load_agents_result = ar_agency_load_agents();
    
    // Then the load operations should succeed
    assert(load_agents_result);
    
    // And our persistent agent should still exist
    bool exists = ar_agent_exists(agent_id);
    assert(exists);
    
    // Cleanup
    ar_agent_destroy(agent_id);
    
    printf("Agency persistence test passed!\n");
}

static void test_agency_reset(void) {
    printf("Testing ar_agency_reset()...\n");
    
    // Given a test method
    const char *method_name = "reset_test_method";
    const char *instructions = "message -> \"Reset Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // And an agent created with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // And the agent exists in the system
    bool exists = ar_agent_exists(agent_id);
    assert(exists);
    
    // When we reset the agency state
    ar_agency_reset();
    
    // Then the agent should no longer exist
    exists = ar_agent_exists(agent_id);
    assert(!exists);
    
    // And the agent count should be zero
    int count = ar_agency_count_agents();
    assert(count == 0);
    
    printf("ar_agency_reset() test passed!\n");
}

int main(void) {
    printf("Starting Agency Module Tests...\n");
    
    // Given a test method and initialized system
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    agent_id_t init_agent_id = ar_system_init(method_name, version);
    assert(init_agent_id > 0);
    
    // When we run all agency tests
    test_agency_init_state();
    test_agency_count_agents();
    test_agency_next_id();
    test_agency_persistence();
    test_agency_reset();
    
    // Then clean up the system
    ar_system_shutdown();
    
    // And report success
    printf("All agency tests passed!\n");
    return 0;
}

