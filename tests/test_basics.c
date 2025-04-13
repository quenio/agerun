#include "../include/agerun.h"
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
    version_t version = agerun_method("test_method", "send(0, \"Hello, World!\")", 0, true, false);
    assert(version > 0);
    
    // Create a new version of the method
    version_t version2 = agerun_method("test_method", "send(0, \"Hello, Updated World!\")", version, true, false);
    assert(version2 > 0);
    assert(version2 > version);
    
    printf("Method creation test passed.\n");
}

static void test_agent_creation(void) {
    printf("Testing agent creation...\n");
    
    // Create a method
    version_t version = agerun_method("agent_test", "send(0, \"Agent created\")", 0, true, false);
    assert(version > 0);
    
    // Create an agent
    agent_id_t agent_id = agerun_create("agent_test", version, NULL);
    assert(agent_id > 0);
    
    // Check if agent exists
    assert(agerun_agent_exists(agent_id));
    
    // Send a message to the agent
    assert(agerun_send(agent_id, "test_message"));
    
    // Process the message
    assert(agerun_process_next_message());
    
    // Destroy the agent
    assert(agerun_destroy(agent_id));
    
    // Check if agent no longer exists
    assert(!agerun_agent_exists(agent_id));
    
    printf("Agent creation test passed.\n");
}

static void test_message_passing(void) {
    printf("Testing message passing between agents...\n");
    
    // Create sender method
    version_t sender_version = agerun_method("sender", "send(target_id, \"Hello from sender!\")", 0, true, false);
    assert(sender_version > 0);
    
    // Create receiver method
    version_t receiver_version = agerun_method("receiver", "memory[\"received\"] := \"true\"", 0, true, false);
    assert(receiver_version > 0);
    
    // Create receiver agent
    agent_id_t receiver_id = agerun_create("receiver", receiver_version, NULL);
    assert(receiver_id > 0);
    
    // Create sender agent with context that includes receiver ID
    // For now this just demonstrates the API, the actual context handling
    // will be implemented later
    agent_id_t sender_id = agerun_create("sender", sender_version, NULL);
    assert(sender_id > 0);
    
    // Set the target_id (this is a placeholder for now)
    // In the full implementation, this would be through memory manipulation
    
    // Send __wake__ message to both agents
    assert(agerun_send(receiver_id, "__wake__"));
    assert(agerun_send(sender_id, "__wake__"));
    
    // Process all messages
    int count = agerun_process_all_messages();
    assert(count >= 2);
    
    // Clean up
    assert(agerun_destroy(sender_id));
    assert(agerun_destroy(receiver_id));
    
    printf("Message passing test passed.\n");
}

int main(void) {
    printf("Starting Agerun tests...\n");
    
    // First initialize the runtime
    agent_id_t initial_agent = agerun_init(NULL, 0);
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        agerun_shutdown();
        return 1;
    }
    
    // Now create our test method
    version_t version = agerun_method("test_init", "send(0, \"Runtime initialized\")", 0, true, false);
    if (version == 0) {
        printf("Error: Failed to create test_init method\n");
        agerun_shutdown();
        return 1;
    }
    
    // Create our initial agent
    initial_agent = agerun_create("test_init", version, NULL);
    if (initial_agent == 0) {
        printf("Error: Failed to create initial agent\n");
        agerun_shutdown();
        return 1;
    }
    
    // Send wake message to initial agent
    if (!agerun_send(initial_agent, "__wake__")) {
        printf("Error: Failed to send wake message\n");
        agerun_shutdown();
        return 1;
    }
    
    // Process the message
    if (agerun_process_next_message() == false) {
        printf("Error: Failed to process message\n");
        agerun_shutdown();
        return 1;
    }
    
    // Run tests
    test_method_creation();
    test_agent_creation();
    test_message_passing();
    
    // Shutdown the runtime
    agerun_shutdown();
    
    printf("All tests passed!\n");
    return 0;
}
