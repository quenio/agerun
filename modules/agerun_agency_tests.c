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
    
    // Get the agents array
    agent_t *agents = ar_agency_get_agents();
    assert(agents != NULL);
    
    // Check that next_id is set to a positive value
    agent_id_t next_id = ar_agency_get_next_id();
    assert(next_id > 0);
    
    printf("Agency initialization state test passed!\n");
}

static void test_agency_count_agents(void) {
    printf("Testing ar_agency_count_agents()...\n");
    
    // Get initial count
    int initial_count = ar_agency_count_agents();
    
    // Create a test method
    const char *method_name = "count_test_method";
    const char *instructions = "message -> \"Count Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // Create some agents
    int num_agents_to_create = 3;
    agent_id_t agent_ids[3];
    
    for (int i = 0; i < num_agents_to_create; i++) {
        agent_ids[i] = ar_agent_create(method_name, version, NULL);
        assert(agent_ids[i] > 0);
    }
    
    // Verify the count increased
    int new_count = ar_agency_count_agents();
    assert(new_count == initial_count + num_agents_to_create);
    
    // Destroy one agent
    bool result = ar_agent_destroy(agent_ids[0]);
    assert(result);
    
    // Verify count decreased
    int after_destroy_count = ar_agency_count_agents();
    assert(after_destroy_count == new_count - 1);
    
    // Clean up remaining agents
    for (int i = 1; i < num_agents_to_create; i++) {
        ar_agent_destroy(agent_ids[i]);
    }
    
    // Verify count went back to initial
    int final_count = ar_agency_count_agents();
    assert(final_count == initial_count);
    
    printf("ar_agency_count_agents() test passed!\n");
}

static void test_agency_next_id(void) {
    printf("Testing ar_agency_get_next_id() and ar_agency_set_next_id()...\n");
    
    // Get current next_id
    agent_id_t current_next_id = ar_agency_get_next_id();
    
    // Set a new next_id
    agent_id_t new_next_id = current_next_id + 1000;
    ar_agency_set_next_id(new_next_id);
    
    // Verify it was set correctly
    agent_id_t retrieved_next_id = ar_agency_get_next_id();
    assert(retrieved_next_id == new_next_id);
    
    // Creating an agent should use and increment this ID
    const char *method_name = "next_id_test_method";
    const char *instructions = "message -> \"Next ID Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id == new_next_id);
    
    // Next ID should have incremented
    retrieved_next_id = ar_agency_get_next_id();
    assert(retrieved_next_id == new_next_id + 1);
    
    // Clean up
    ar_agent_destroy(agent_id);
    
    // Reset next_id to original value
    ar_agency_set_next_id(current_next_id);
    
    printf("ar_agency_get_next_id() and ar_agency_set_next_id() tests passed!\n");
}

static void test_agency_persistence(void) {
    printf("Testing agency persistence...\n");
    
    // Create a persistent method
    const char *method_name = "agency_persistence_method";
    const char *instructions = "message -> \"Agency Persistence Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, true);
    assert(version > 0);
    
    // Create an agent with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Save agents to disk
    bool save_result = ar_agency_save_agents();
    assert(save_result);
    
    // Record the next_id before reset
    agent_id_t next_id = ar_agency_get_next_id();
    
    // Reset system
    ar_system_shutdown();
    
    // Initialize with the default method
    ar_system_init("agency_persistence_method", version);
    
    // Set the next_id back to what it was
    ar_agency_set_next_id(next_id);
    
    // Load methods first
    bool load_methods_result = ar_methodology_load_methods();
    assert(load_methods_result);
    
    // Load agents
    bool load_agents_result = ar_agency_load_agents();
    assert(load_agents_result);
    
    // Verify our agent still exists
    bool exists = ar_agent_exists(agent_id);
    assert(exists);
    
    // Clean up
    ar_agent_destroy(agent_id);
    
    printf("Agency persistence test passed!\n");
}

static void test_agency_reset(void) {
    printf("Testing ar_agency_reset()...\n");
    
    // Create a test method
    const char *method_name = "reset_test_method";
    const char *instructions = "message -> \"Reset Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // Create an agent
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Verify agent exists
    bool exists = ar_agent_exists(agent_id);
    assert(exists);
    
    // Reset agency state
    ar_agency_reset();
    
    // Verify agent no longer exists
    exists = ar_agent_exists(agent_id);
    assert(!exists);
    
    // Count should be 0
    int count = ar_agency_count_agents();
    assert(count == 0);
    
    printf("ar_agency_reset() test passed!\n");
}

int main(void) {
    printf("Starting Agency Module Tests...\n");
    
    // Initialize the system before running tests
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    agent_id_t init_agent_id = ar_system_init(method_name, version);
    assert(init_agent_id > 0);
    
    test_agency_init_state();
    test_agency_count_agents();
    test_agency_next_id();
    test_agency_persistence();
    test_agency_reset();
    
    // Shutdown the system after tests
    ar_system_shutdown();
    
    printf("All agency tests passed!\n");
    return 0;
}

