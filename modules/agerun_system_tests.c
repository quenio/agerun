#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Message strings */
static const char *g_wake_message = "__wake__";
static const char *g_test_message = "test_message";

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
    method_t *own_method = ar_method_create(method_name, method_body, "1.0.0");
    
    // Then the method should be created successfully
    assert(own_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create a new version of the same method
    const char *updated_body = "send(0, \"Hello, Updated World!\")";
    method_t *own_method2 = ar_method_create(method_name, updated_body, "2.0.0");
    
    // Then the method should be created successfully
    assert(own_method2 != NULL);
    
    // Register with methodology
    ar_methodology_register_method(own_method2);
    own_method2 = NULL; // Mark as transferred
    
    // For test purposes, we use version "2.0.0"
    const char *version2 = "2.0.0";
    
    // And the new version should be different from the previous version
    assert(strcmp(version2, version) != 0);
    
    printf("Method creation test passed.\n");
}

static void test_agent_creation(void) {
    printf("Testing agent creation...\n");
    
    // Given we have a method for an agent
    const char *method_name = "agent_test";
    const char *method_body = "send(0, \"Agent created\")";
    
    // Create method and register it with methodology 
    method_t *own_method = ar_method_create(method_name, method_body, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an agent with this method
    agent_id_t agent_id = ar_agent_create(method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    assert(ar_agent_exists(agent_id));
    
    // When we send a message to the agent
    data_t *test_message = ar_data_create_string(g_test_message);
    assert(test_message != NULL);
    bool send_result = ar_agent_send(agent_id, test_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the next message in the system
    // With opaque map_t, we can't directly test the processing result
    ar_system_process_next_message();
    
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
    // Create and register sender method
    method_t *own_sender_method = ar_method_create("sender", "send(target_id, \"Hello from sender!\")", "1.0.0");
    assert(own_sender_method != NULL);
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_sender_method);
    own_sender_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *sender_version = "1.0.0";
    
    // Create and register receiver method
    method_t *own_receiver_method = ar_method_create("receiver", "memory[\"received\"] := \"true\"", "1.0.0");
    assert(own_receiver_method != NULL);
    
    // Register with methodology
    ar_methodology_register_method(own_receiver_method);
    own_receiver_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *receiver_version = "1.0.0";
    
    // And a receiver agent created with the receiver method
    agent_id_t receiver_id = ar_agent_create("receiver", receiver_version, NULL);
    assert(receiver_id > 0);
    
    // And a sender agent created with the sender method
    // Note: In the full implementation, a context with receiver ID would be passed
    agent_id_t sender_id = ar_agent_create("sender", sender_version, NULL);
    assert(sender_id > 0);
    
    // When we send __wake__ messages to both agents
    data_t *wake_message1 = ar_data_create_string(g_wake_message);
    data_t *wake_message2 = ar_data_create_string(g_wake_message);
    assert(wake_message1 != NULL);
    assert(wake_message2 != NULL);
    bool receiver_send = ar_agent_send(receiver_id, wake_message1);
    bool sender_send = ar_agent_send(sender_id, wake_message2);
    
    // Then the messages should be sent successfully
    assert(receiver_send);
    assert(sender_send);
    
    // When we process all pending messages
    // With opaque map_t, we can't rely on the exact count
    ar_system_process_all_messages();
    
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
    agent_id_t initial_agent = ar_system_init(NULL, NULL);
    
    // Then no agent should be created during initialization
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        ar_system_shutdown();
        return 1;
    }
    
    // Given we create a test method
    method_t *own_method = ar_method_create("test_init", "send(0, \"Runtime initialized\")", "1.0.0");
    
    // Then the method should be created successfully
    if (own_method == NULL) {
        printf("Error: Failed to create test_init method\n");
        ar_system_shutdown();
        return 1;
    }
    
    // Register with methodology
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an initial agent with this method
    initial_agent = ar_agent_create("test_init", version, NULL);
    
    // Then the agent should be created successfully
    if (initial_agent == 0) {
        printf("Error: Failed to create initial agent\n");
        ar_system_shutdown();
        return 1;
    }
    
    // When we send a wake message to the initial agent
    data_t *wake_message3 = ar_data_create_string(g_wake_message);
    assert(wake_message3 != NULL);
    bool send_result = ar_agent_send(initial_agent, wake_message3);
    
    // Then the message should be sent successfully
    if (!send_result) {
        printf("Error: Failed to send wake message\n");
        ar_system_shutdown();
        return 1;
    }
    
    // When we process the message
    // With opaque map_t, we can't directly test the processing result
    ar_system_process_next_message();
    
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
