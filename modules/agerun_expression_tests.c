#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test expression evaluation indirectly through the message passing system
static void test_echo_method(void);
static void test_simple_method(void);

int main(void) {
    printf("Starting Expression Tests...\n");
    
    // Given we initialize the runtime
    agent_id_t initial_agent = ar_system_init(NULL, 0);
    
    // Then no agent should be created during initialization
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        ar_system_shutdown();
        return 1;
    }
    
    // When we run all expression tests
    test_echo_method();
    test_simple_method();
    
    // Then we clean up the system
    ar_system_shutdown();
    
    // And report success
    printf("All expression tests passed!\n");
    return 0;
}

static void test_echo_method(void) {
    printf("Testing echo method...\n");
    
    // Given a simple echo method that returns the input message
    version_t version = ar_method_create("echo", "send(0, message)", 0, true, false);
    assert(version > 0);
    printf("Created echo method version %d\n", version);
    
    // And an agent created with this method
    agent_id_t agent_id = ar_agent_create("echo", version, NULL);
    assert(agent_id > 0);
    printf("Created agent %lld using echo method\n", (long long)agent_id);
    
    // When we send a test message to the agent
    const char *test_message = "Hello, Echo!";
    printf("Sending message: \"%s\"\n", test_message);
    bool send_result = ar_agent_send(agent_id, test_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the next message
    printf("Processing message...\n");
    bool process_result = ar_system_process_next_message();
    
    // Then the message should be processed successfully
    assert(process_result);
    
    // When we check for a response
    printf("Processing response...\n");
    bool response = ar_system_process_next_message();
    
    // Then there may be a response depending on the implementation
    printf("Response received: %s\n", response ? "yes" : "no");
    
    // When we clean up the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    
    // Then the agent should be destroyed successfully
    assert(destroy_result);
    
    printf("Echo method test passed.\n");
}

static void test_simple_method(void) {
    printf("Testing simple method...\n");
    
    // Given a simple method that stores a message in memory
    const char *simple_method = 
        "# Store message in memory\n"
        "memory[\"stored_message\"] := message";
    
    version_t version = ar_method_create("simple_test", simple_method, 0, true, false);
    assert(version > 0);
    printf("Created simple method version %d\n", version);
    
    // And an agent created with this method
    agent_id_t agent_id = ar_agent_create("simple_test", version, NULL);
    assert(agent_id > 0);
    printf("Created agent %lld using simple method\n", (long long)agent_id);
    
    // When we send a test message to the agent
    const char *test_message = "Test Data";
    printf("Sending message: \"%s\"\n", test_message);
    bool send_result = ar_agent_send(agent_id, test_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the next message
    printf("Processing message...\n");
    bool process_result = ar_system_process_next_message();
    
    // Then the message should be processed successfully
    assert(process_result);
    
    // When we clean up the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    
    // Then the agent should be destroyed successfully
    assert(destroy_result);
    
    printf("Simple method test passed.\n");
}
