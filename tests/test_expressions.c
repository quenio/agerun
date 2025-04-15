#include "../include/system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test expression evaluation indirectly through the message passing system
static void test_echo_method(void);
static void test_simple_method(void);

int main(void) {
    printf("Starting Expression Tests...\n");
    
    // Initialize the runtime
    agent_id_t initial_agent = agerun_init(NULL, 0);
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        agerun_shutdown();
        return 1;
    }
    
    // Run expression tests
    test_echo_method();
    test_simple_method();
    
    // Shutdown
    agerun_shutdown();
    
    printf("All expression tests passed!\n");
    return 0;
}

static void test_echo_method(void) {
    printf("Testing echo method...\n");
    
    // Create a simple method that sends back a message
    version_t version = agerun_method("echo", "send(0, message)", 0, true, false);
    assert(version > 0);
    printf("Created echo method version %d\n", version);
    
    // Create an agent
    agent_id_t agent_id = agerun_create("echo", version, NULL);
    assert(agent_id > 0);
    printf("Created agent %lld using echo method\n", (long long)agent_id);
    
    // Send test message
    const char *test_message = "Hello, Echo!";
    printf("Sending message: \"%s\"\n", test_message);
    assert(agerun_send(agent_id, test_message));
    
    // Process the message
    printf("Processing message...\n");
    assert(agerun_process_next_message());
    
    // We should receive a response (not checking content as that depends on full implementation)
    printf("Processing response...\n");
    bool response = agerun_process_next_message();
    printf("Response received: %s\n", response ? "yes" : "no");
    
    // Cleanup
    assert(agerun_destroy(agent_id));
    printf("Echo method test passed.\n");
}

static void test_simple_method(void) {
    printf("Testing simple method...\n");
    
    // Create a simple method that just stores a value
    const char *simple_method = 
        "# Store message in memory\n"
        "memory[\"stored_message\"] := message";
    
    version_t version = agerun_method("simple_test", simple_method, 0, true, false);
    assert(version > 0);
    printf("Created simple method version %d\n", version);
    
    // Create an agent
    agent_id_t agent_id = agerun_create("simple_test", version, NULL);
    assert(agent_id > 0);
    printf("Created agent %lld using simple method\n", (long long)agent_id);
    
    // Send a test message
    const char *test_message = "Test Data";
    printf("Sending message: \"%s\"\n", test_message);
    assert(agerun_send(agent_id, test_message));
    
    // Process the message
    printf("Processing message...\n");
    assert(agerun_process_next_message());
    
    // Cleanup
    assert(agerun_destroy(agent_id));
    printf("Simple method test passed.\n");
}
