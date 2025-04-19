#include "agerun_agent.h"
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_agent_create_destroy(void);
static void test_agent_send(void);
static void test_agent_exists(void);
static void test_agent_persistence(void);

static void test_agent_create_destroy(void) {
    printf("Testing ar_agent_create() and ar_agent_destroy()...\n");
    
    // Given a simple method for an agent
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test Method Response\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // When we create an agent with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    bool exists = ar_agent_exists(agent_id);
    assert(exists);
    
    // When we destroy the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    
    // Then the agent should be destroyed successfully
    assert(destroy_result);
    
    // And the agent should no longer exist in the system
    exists = ar_agent_exists(agent_id);
    assert(!exists);
    
    printf("ar_agent_create() and ar_agent_destroy() tests passed!\n");
}

static void test_agent_send(void) {
    printf("Testing ar_agent_send()...\n");
    
    // Given an echo method and an agent using it
    const char *method_name = "echo_method";
    const char *instructions = "message -> message";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we send a message to the agent
    const char *test_message = "Hello Agent!";
    bool send_result = ar_agent_send(agent_id, test_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // And the agent's message queue should contain the message
    agent_t *agents = ar_agency_get_agents();
    agent_t *agent = NULL;
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].id == agent_id) {
            agent = &agents[i];
            break;
        }
    }
    assert(agent != NULL);
    assert(agent->queue.size > 0);
    
    // Cleanup
    ar_agent_destroy(agent_id);
    
    printf("ar_agent_send() test passed!\n");
}

static void test_agent_exists(void) {
    printf("Testing ar_agent_exists()...\n");
    
    // Given a method and an agent created with it
    const char *method_name = "exists_method";
    const char *instructions = "message -> \"I exist\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we check if the valid agent ID exists
    bool exists = ar_agent_exists(agent_id);
    
    // Then it should exist
    assert(exists);
    
    // When we check if invalid agent IDs exist
    bool exists_zero = ar_agent_exists(0);
    bool exists_large = ar_agent_exists(999999);
    
    // Then they should not exist
    assert(!exists_zero);
    assert(!exists_large);
    
    // When we destroy the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    assert(destroy_result);
    
    // And check if it still exists
    exists = ar_agent_exists(agent_id);
    
    // Then it should no longer exist
    assert(!exists);
    
    printf("ar_agent_exists() test passed!\n");
}

static void test_agent_persistence(void) {
    printf("Testing agent persistence...\n");
    
    // Given a persistent method
    const char *method_name = "persistent_method";
    const char *instructions = "message -> \"I persist\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, true);
    assert(version > 0);
    
    // And an agent created with this persistent method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we save agents to disk
    bool save_result = ar_agency_save_agents();
    
    // Then the save operation should succeed
    assert(save_result);
    
    // When we simulate a system restart
    ar_system_shutdown();
    ar_system_init(method_name, version);
    
    // And load the methods and agents
    bool load_methods_result = ar_methodology_load_methods();
    assert(load_methods_result);
    
    bool load_agents_result = ar_agency_load_agents();
    
    // Then the load operations should succeed
    assert(load_agents_result);
    
    // When we check if our persistent agent still exists
    bool exists = ar_agent_exists(agent_id);
    
    // Then the agent should still exist
    assert(exists);
    
    // Cleanup
    ar_agent_destroy(agent_id);
    
    printf("Agent persistence test passed!\n");
}

int main(void) {
    printf("Starting Agent Module Tests...\n");
    
    // Given a test method and initialized system
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    agent_id_t init_agent_id = ar_system_init(method_name, version);
    assert(init_agent_id > 0);
    
    // When we run all agent tests
    test_agent_create_destroy();
    test_agent_send();
    test_agent_exists();
    test_agent_persistence();
    
    // Then clean up the system
    ar_system_shutdown();
    
    // And report success
    printf("All agent tests passed!\n");
    return 0;
}

