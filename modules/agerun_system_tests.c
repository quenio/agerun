#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_agent.h"
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
    
    // Given we want to create a new method
    const char *method_name = "test_method";
    const char *method_body = "send(0, \"Hello, World!\")";
    
    // When we create the method
    version_t version = ar_method_create(method_name, method_body, 0, true, false);
    
    // Then the method should be created successfully
    assert(version > 0);
    
    // When we create a new version of the same method
    const char *updated_body = "send(0, \"Hello, Updated World!\")";
    version_t version2 = ar_method_create(method_name, updated_body, version, true, false);
    
    // Then the new version should be created successfully
    assert(version2 > 0);
    
    // And the new version should be greater than the previous version
    assert(version2 > version);
    
    printf("Method creation test passed.\n");
}

static void test_agent_creation(void) {
    printf("Testing agent creation...\n");
    
    // Given we have a method for an agent
    const char *method_name = "agent_test";
    const char *method_body = "send(0, \"Agent created\")";
    version_t version = ar_method_create(method_name, method_body, 0, true, false);
    assert(version > 0);
    
    // When we create an agent with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    assert(ar_agent_exists(agent_id));
    
    // When we send a message to the agent
    bool send_result = ar_agent_send(agent_id, "test_message");
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the next message in the system
    bool process_result = ar_system_process_next_message();
    
    // Then the message should be processed successfully
    assert(process_result);
    
    // When we destroy the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    
    // Then the destruction should succeed
    assert(destroy_result);
    
    // And the agent should no longer exist in the system
    assert(!ar_agent_exists(agent_id));
    
    printf("Agent creation test passed.\n");
}

static void test_message_passing(void) {
    printf("Testing message passing between agents...\n");
    
    // Given methods for sender and receiver agents
    version_t sender_version = ar_method_create("sender", "send(target_id, \"Hello from sender!\")", 0, true, false);
    assert(sender_version > 0);
    
    version_t receiver_version = ar_method_create("receiver", "memory[\"received\"] := \"true\"", 0, true, false);
    assert(receiver_version > 0);
    
    // And a receiver agent created with the receiver method
    agent_id_t receiver_id = ar_agent_create("receiver", receiver_version, NULL);
    assert(receiver_id > 0);
    
    // And a sender agent created with the sender method
    // Note: In the full implementation, a context with receiver ID would be passed
    agent_id_t sender_id = ar_agent_create("sender", sender_version, NULL);
    assert(sender_id > 0);
    
    // When we send __wake__ messages to both agents
    bool receiver_send = ar_agent_send(receiver_id, "__wake__");
    bool sender_send = ar_agent_send(sender_id, "__wake__");
    
    // Then the messages should be sent successfully
    assert(receiver_send);
    assert(sender_send);
    
    // When we process all pending messages
    int count = ar_system_process_all_messages();
    
    // Then at least the two wake messages should be processed
    assert(count >= 2);
    
    // When we clean up the agents
    bool sender_destroy = ar_agent_destroy(sender_id);
    bool receiver_destroy = ar_agent_destroy(receiver_id);
    
    // Then the destruction should succeed
    assert(sender_destroy);
    assert(receiver_destroy);
    
    printf("Message passing test passed.\n");
}

int main(void) {
    printf("Starting Agerun tests...\n");
    
    // Given we initialize the runtime
    agent_id_t initial_agent = ar_system_init(NULL, 0);
    
    // Then no agent should be created during initialization
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        ar_system_shutdown();
        return 1;
    }
    
    // Given we create a test method
    version_t version = ar_method_create("test_init", "send(0, \"Runtime initialized\")", 0, true, false);
    
    // Then the method should be created successfully
    if (version == 0) {
        printf("Error: Failed to create test_init method\n");
        ar_system_shutdown();
        return 1;
    }
    
    // When we create an initial agent with this method
    initial_agent = ar_agent_create("test_init", version, NULL);
    
    // Then the agent should be created successfully
    if (initial_agent == 0) {
        printf("Error: Failed to create initial agent\n");
        ar_system_shutdown();
        return 1;
    }
    
    // When we send a wake message to the initial agent
    bool send_result = ar_agent_send(initial_agent, "__wake__");
    
    // Then the message should be sent successfully
    if (!send_result) {
        printf("Error: Failed to send wake message\n");
        ar_system_shutdown();
        return 1;
    }
    
    // When we process the message
    bool process_result = ar_system_process_next_message();
    
    // Then the message should be processed successfully
    if (process_result == false) {
        printf("Error: Failed to process message\n");
        ar_system_shutdown();
        return 1;
    }
    
    // When we run all system tests
    test_method_creation();
    test_agent_creation();
    test_message_passing();
    
    // Then clean up the system
    ar_system_shutdown();
    
    // And report success
    printf("All tests passed!\n");
    return 0;
}
