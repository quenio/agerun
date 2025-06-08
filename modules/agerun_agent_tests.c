#include "agerun_agent.h"
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Message strings */
static const char *g_hello_message = "Hello Agent!";

// Forward declarations
static void test_agent_create_destroy(void);
static void test_agent_send(void);
static void test_agent_exists(void);
static void test_agent_persistence(void);

static void test_agent_create_destroy(void) {
    printf("Testing ar__agent__create() and ar__agent__destroy()...\n");
    
    // Given a simple method for an agent
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test Method Response\"";
    
    // Create method and register it with methodology 
    method_t *own_method = ar_method_create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an agent with this method
    int64_t agent_id = ar__agency__create_agent(method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    bool exists = ar__agency__agent_exists(agent_id);
    assert(exists);
    
    // When we destroy the agent
    ar__agency__destroy_agent(agent_id);
    
    // Then the agent should be destroyed successfully
    // Agency destroy returns void
    
    // And the agent should no longer exist in the system
    exists = ar__agency__agent_exists(agent_id);
    assert(!exists);
    
    printf("ar__agent__create() and ar__agent__destroy() tests passed!\n");
}

static void test_agent_send(void) {
    printf("Testing ar__agent__send()...\n");
    
    // Given an echo method and an agent using it
    const char *method_name = "echo_method";
    const char *instructions = "message -> message";
    
    // Create method and register it with methodology 
    method_t *own_method = ar_method_create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    int64_t agent_id = ar__agency__create_agent(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we send a message to the agent
    data_t *own_message_data = ar_data_create_string(g_hello_message);
    assert(own_message_data != NULL);
    bool send_result = ar__agency__send_to_agent(agent_id, own_message_data);
    own_message_data = NULL; // Mark as transferred
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // Process the message to prevent memory leaks
    ar__system__process_next_message();
    
    // Since we can't directly access the message queue in an opaque map,
    // we'll verify the agent was created (which was already tested)
    // and that the message was sent (which was verified by send_result)
    assert(ar__agency__agent_exists(agent_id));
    
    // Cleanup
    ar__agency__destroy_agent(agent_id);
    
    printf("ar__agent__send() test passed!\n");
}

static void test_agent_exists(void) {
    printf("Testing ar__agent__exists()...\n");
    
    // Given a method and an agent created with it
    const char *method_name = "exists_method";
    const char *instructions = "message -> \"I exist\"";
    
    // Create method and register it with methodology 
    method_t *own_method = ar_method_create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    int64_t agent_id = ar__agency__create_agent(method_name, version, NULL);
    assert(agent_id > 0);
    
    // When we check if the valid agent ID exists
    bool exists = ar__agency__agent_exists(agent_id);
    
    // Then it should exist
    assert(exists);
    
    // When we check if invalid agent IDs exist
    bool exists_zero = ar__agency__agent_exists(0);
    bool exists_large = ar__agency__agent_exists(999999);
    
    // Then they should not exist
    assert(!exists_zero);
    assert(!exists_large);
    
    // When we destroy the agent
    ar__agency__destroy_agent(agent_id);
    // Agency destroy returns void
    
    // And check if it still exists
    exists = ar__agency__agent_exists(agent_id);
    
    // Then it should no longer exist
    assert(!exists);
    
    printf("ar__agent__exists() test passed!\n");
}

static void test_agent_persistence(void) {
    printf("Testing agent persistence...\n");
    
    // Given a persistent method
    const char *method_name = "persistent_method";
    const char *instructions = "message -> \"I persist\"";
    
    // Create method and register it with methodology 
    method_t *own_method = ar_method_create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    // Create a context with data_t
    data_t *own_context = ar_data_create_map();
    assert(own_context != NULL);
    ar_data_set_map_string(own_context, "test_key", "test_value");
    
    // And an agent created with this persistent method - agent doesn't take ownership
    int64_t agent_id = ar__agency__create_agent(method_name, version, own_context);
    assert(agent_id > 0);
    
    // When we save agents to disk
    bool save_result = ar__agency__save_agents();
    
    // Then the save operation should succeed
    assert(save_result);
    
    // When we simulate a system restart
    ar__system__shutdown();
    ar__system__init(method_name, version);
    
    // And load the methods and agents
    bool load_methods_result = ar_methodology_load_methods();
    assert(load_methods_result);
    
    bool load_agents_result = ar__agency__load_agents();
    
    // Then the load operations should succeed
    assert(load_agents_result);
    
    // When we check if our persistent agent still exists
    bool exists = ar__agency__agent_exists(agent_id);
    
    // Then the agent should still exist
    assert(exists);
    
    // Cleanup
    ar__agency__destroy_agent(agent_id);
    
    // Since the agent didn't take ownership, we need to cleanup our context
    ar_data_destroy(own_context);
    own_context = NULL; // Mark as destroyed
    
    printf("Agent persistence test passed!\n");
}

int main(void) {
    printf("Starting Agent Module Tests...\n");
    
    // Given a test method and initialized system
    const char *method_name = "test_method";
    const char *instructions = "message -> \"Test\"";
    
    // Create method and register it with methodology 
    method_t *own_method = ar_method_create(method_name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version "1.0.0"
    const char *version = "1.0.0";
    
    int64_t init_agent_id = ar__system__init(method_name, version);
    assert(init_agent_id > 0);
    
    // When we run all agent tests
    test_agent_create_destroy();
    test_agent_send();
    test_agent_exists();
    test_agent_persistence();
    
    // Then clean up the system
    ar__system__shutdown();
    
    // And report success
    printf("All agent tests passed!\n");
    return 0;
}
