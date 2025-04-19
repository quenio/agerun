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
    
    // Create a simple method first
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test Method Response\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // Create an agent with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Verify agent exists
    bool exists = ar_agent_exists(agent_id);
    assert(exists);
    
    // Destroy the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    assert(destroy_result);
    
    // Verify agent no longer exists
    exists = ar_agent_exists(agent_id);
    assert(!exists);
    
    printf("ar_agent_create() and ar_agent_destroy() tests passed!\n");
}

static void test_agent_send(void) {
    printf("Testing ar_agent_send()...\n");
    
    // Create a simple echo method
    const char *method_name = "echo_method";
    const char *instructions = "message -> message";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // Create an agent with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Send a message to the agent
    const char *test_message = "Hello Agent!";
    bool send_result = ar_agent_send(agent_id, test_message);
    assert(send_result);
    
    // Get the agent to verify message queue
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
    
    // Clean up
    ar_agent_destroy(agent_id);
    
    printf("ar_agent_send() test passed!\n");
}

static void test_agent_exists(void) {
    printf("Testing ar_agent_exists()...\n");
    
    // Create a simple method
    const char *method_name = "exists_method";
    const char *instructions = "message -> \"I exist\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    // Create an agent
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Valid ID should exist
    bool exists = ar_agent_exists(agent_id);
    assert(exists);
    
    // Invalid ID should not exist
    exists = ar_agent_exists(0);
    assert(!exists);
    
    exists = ar_agent_exists(999999);
    assert(!exists);
    
    // Destroy and verify it no longer exists
    bool destroy_result = ar_agent_destroy(agent_id);
    assert(destroy_result);
    
    exists = ar_agent_exists(agent_id);
    assert(!exists);
    
    printf("ar_agent_exists() test passed!\n");
}

static void test_agent_persistence(void) {
    printf("Testing agent persistence...\n");
    
    // Create a persistent method
    const char *method_name = "persistent_method";
    const char *instructions = "message -> \"I persist\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, true);
    assert(version > 0);
    
    // Create an agent with this persistent method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    assert(agent_id > 0);
    
    // Save agents to disk
    bool save_result = ar_agency_save_agents();
    assert(save_result);
    
    // Reset system to simulate restart
    ar_system_shutdown();
    
    // Initialize with the default method
    ar_system_init(method_name, version);
    
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
    
    printf("Agent persistence test passed!\n");
}

int main(void) {
    printf("Starting Agent Module Tests...\n");
    
    // Initialize the system before running tests
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test\"";
    version_t version = ar_method_create(method_name, instructions, 0, false, false);
    assert(version > 0);
    
    agent_id_t init_agent_id = ar_system_init(method_name, version);
    assert(init_agent_id > 0);
    
    test_agent_create_destroy();
    test_agent_send();
    test_agent_exists();
    test_agent_persistence();
    
    // Shutdown the system after tests
    ar_system_shutdown();
    
    printf("All agent tests passed!\n");
    return 0;
}

