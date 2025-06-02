#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_method_fixture.h"
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_data.h"

static void test_echo_simple_message(void) {
    printf("Testing echo method with simple message...\n");
    
    // Create test fixture
    method_fixture_t *own_fixture = ar_method_fixture_create("echo_simple_message");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture_initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture_verify_directory(own_fixture));
    
    // Load and register echo method
    assert(ar_method_fixture_load_method(own_fixture, "echo", "../methods/echo-1.0.0.method", "1.0.0"));
    
    // Create echo agent
    agent_id_t echo_agent = ar_agent_create("echo", "1.0.0", NULL);
    assert(echo_agent != 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(echo_agent);
    assert(agent_memory != NULL);
    
    // When we send a message with sender field
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    // Add sender field (0 means system/test)
    data_t *own_sender = ar_data_create_integer(0);
    ar_data_set_map_data(own_message, "sender", own_sender);
    own_sender = NULL; // Ownership transferred
    
    // Add the actual message content
    data_t *own_content = ar_data_create_string("Hello, Echo!");
    ar_data_set_map_data(own_message, "content", own_content);
    own_content = NULL; // Ownership transferred
    
    bool sent = ar_agent_send(echo_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Echo method now only sends back message.content, doesn't store in memory
    
    // Process the return message (echo sends it back)
    processed = ar_system_process_next_message();
    
    // Clean up
    ar_agent_destroy(echo_agent);
    
    // Check for memory leaks
    assert(ar_method_fixture_check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture_destroy(own_fixture);
    
    printf("PASS\n");
}

static void test_echo_map_message(void) {
    printf("Testing echo method with map message...\n");
    
    // Create test fixture
    method_fixture_t *own_fixture = ar_method_fixture_create("echo_map_message");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture_initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture_verify_directory(own_fixture));
    
    // Load and register echo method
    assert(ar_method_fixture_load_method(own_fixture, "echo", "../methods/echo-1.0.0.method", "1.0.0"));
    
    // Create echo agent
    agent_id_t echo_agent = ar_agent_create("echo", "1.0.0", NULL);
    assert(echo_agent != 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(echo_agent);
    assert(agent_memory != NULL);
    
    // When we send a map message
    data_t *own_map_message = ar_data_create_map();
    assert(own_map_message != NULL);
    ar_data_set_map_integer(own_map_message, "sender", 0); // Add sender field
    ar_data_set_map_string(own_map_message, "type", "greeting");
    ar_data_set_map_string(own_map_message, "content", "Hello from map!");
    ar_data_set_map_integer(own_map_message, "count", 42);
    
    bool sent = ar_agent_send(echo_agent, own_map_message);
    assert(sent);
    own_map_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Process the return message (echo sends it back)
    processed = ar_system_process_next_message();
    
    // Clean up
    ar_agent_destroy(echo_agent);
    
    // Check for memory leaks
    assert(ar_method_fixture_check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture_destroy(own_fixture);
    
    printf("PASS\n");
}

int main(void) {
    printf("Running echo method tests...\n");
    
    test_echo_simple_message();
    test_echo_map_message();
    
    printf("All tests passed!\n");
    return 0;
}
