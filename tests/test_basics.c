#include "../src/agerun_system.h"
#include "../src/agerun_method.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test function prototypes */
static void test_method_creation(void);
static void test_agent_creation(void);
static void test_message_passing(void);

static void test_method_creation(void) {
    printf("Testing method creation...\n");
    
    // Create a new method
    version_t version = ar_method_create("test_method", "send(0, \"Hello, World!\")", 0, true, false);
    assert(version > 0);
    
    // Create a new version of the method
    version_t version2 = ar_method_create("test_method", "send(0, \"Hello, Updated World!\")", version, true, false);
    assert(version2 > 0);
    assert(version2 > version);
    
    printf("Method creation test passed.\n");
}

static void test_agent_creation(void) {
    printf("Testing agent creation...\n");
    
    // Create a method
    version_t version = ar_method_create("agent_test", "send(0, \"Agent created\")", 0, true, false);
    assert(version > 0);
    
    // Create an agent
    agent_id_t agent_id = ar_create("agent_test", version, NULL);
    assert(agent_id > 0);
    
    // Check if agent exists
    assert(ar_agent_exists(agent_id));
    
    // Send a message to the agent
    assert(ar_send(agent_id, "test_message"));
    
    // Process the message
    assert(ar_process_next_message());
    
    // Destroy the agent
    assert(ar_destroy(agent_id));
    
    // Check if agent no longer exists
    assert(!ar_agent_exists(agent_id));
    
    printf("Agent creation test passed.\n");
}

static void test_message_passing(void) {
    printf("Testing message passing between agents...\n");
    
    // Create sender method
    version_t sender_version = ar_method_create("sender", "send(target_id, \"Hello from sender!\")", 0, true, false);
    assert(sender_version > 0);
    
    // Create receiver method
    version_t receiver_version = ar_method_create("receiver", "memory[\"received\"] := \"true\"", 0, true, false);
    assert(receiver_version > 0);
    
    // Create receiver agent
    agent_id_t receiver_id = ar_create("receiver", receiver_version, NULL);
    assert(receiver_id > 0);
    
    // Create sender agent with context that includes receiver ID
    // For now this just demonstrates the API, the actual context handling
    // will be implemented later
    agent_id_t sender_id = ar_create("sender", sender_version, NULL);
    assert(sender_id > 0);
    
    // Set the target_id (this is a placeholder for now)
    // In the full implementation, this would be through memory manipulation
    
    // Send __wake__ message to both agents
    assert(ar_send(receiver_id, "__wake__"));
    assert(ar_send(sender_id, "__wake__"));
    
    // Process all messages
    int count = ar_process_all_messages();
    assert(count >= 2);
    
    // Clean up
    assert(ar_destroy(sender_id));
    assert(ar_destroy(receiver_id));
    
    printf("Message passing test passed.\n");
}

int main(void) {
    printf("Starting Agerun tests...\n");
    
    // First initialize the runtime
    agent_id_t initial_agent = ar_init(NULL, 0);
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        ar_shutdown();
        return 1;
    }
    
    // Now create our test method
    version_t version = ar_method_create("test_init", "send(0, \"Runtime initialized\")", 0, true, false);
    if (version == 0) {
        printf("Error: Failed to create test_init method\n");
        ar_shutdown();
        return 1;
    }
    
    // Create our initial agent
    initial_agent = ar_create("test_init", version, NULL);
    if (initial_agent == 0) {
        printf("Error: Failed to create initial agent\n");
        ar_shutdown();
        return 1;
    }
    
    // Send wake message to initial agent
    if (!ar_send(initial_agent, "__wake__")) {
        printf("Error: Failed to send wake message\n");
        ar_shutdown();
        return 1;
    }
    
    // Process the message
    if (ar_process_next_message() == false) {
        printf("Error: Failed to process message\n");
        ar_shutdown();
        return 1;
    }
    
    // Run tests
    test_method_creation();
    test_agent_creation();
    test_message_passing();
    
    // Shutdown the runtime
    ar_shutdown();
    
    printf("All tests passed!\n");
    return 0;
}
