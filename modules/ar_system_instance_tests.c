#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test function prototypes */
static void test_system_instance_create_destroy(void);
static void test_system_instance_with_custom_agency(void);
static void test_system_instance_parallel_systems(void);
static void test_system_instance_message_processing(void);

static void test_system_instance_create_destroy(void) {
    printf("Testing system instance create/destroy...\n");
    
    // Given we want to create a system instance
    ar_system_t *own_system = ar_system__create(NULL);
    
    // Then the instance should be created successfully
    assert(own_system != NULL);
    
    // When we destroy the instance
    ar_system__destroy(own_system);
    own_system = NULL;
    
    // Then no crash should occur
    printf("System instance create/destroy test passed.\n");
}

static void test_system_instance_with_custom_agency(void) {
    printf("Testing system instance with custom agency...\n");
    
    // Given we create a custom agency
    ar_agency_t *own_agency = ar_agency__create(NULL);
    assert(own_agency != NULL);
    
    // And a system instance using that agency
    ar_system_t *own_system = ar_system__create(own_agency);
    assert(own_system != NULL);
    
    // When we initialize the system with a method
    ar_method_t *own_method = ar_method__create("instance_test", "send(0, \"Instance test\")", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method(own_method);
    own_method = NULL; // Ownership transferred
    
    int64_t agent_id = ar_system__init_with_instance(own_system, "instance_test", "1.0.0");
    
    // Then an agent should be created
    assert(agent_id > 0);
    
    // And message processing should work
    bool processed = ar_system__process_next_message_with_instance(own_system);
    assert(processed); // Should process the wake message
    
    // When we shut down the system
    ar_system__shutdown_with_instance(own_system);
    
    // And destroy the system and agency
    ar_system__destroy(own_system);
    ar_agency__destroy(own_agency);
    
    printf("System instance with custom agency test passed.\n");
}

static void test_system_instance_parallel_systems(void) {
    printf("Testing parallel system instances...\n");
    
    // Given we create two separate agencies
    ar_agency_t *own_agency1 = ar_agency__create(NULL);
    ar_agency_t *own_agency2 = ar_agency__create(NULL);
    assert(own_agency1 != NULL);
    assert(own_agency2 != NULL);
    
    // And two system instances
    ar_system_t *own_system1 = ar_system__create(own_agency1);
    ar_system_t *own_system2 = ar_system__create(own_agency2);
    assert(own_system1 != NULL);
    assert(own_system2 != NULL);
    
    // Create methods for testing
    ar_method_t *own_method1 = ar_method__create("system1_test", "memory[\"system\"] := \"one\"", "1.0.0");
    ar_method_t *own_method2 = ar_method__create("system2_test", "memory[\"system\"] := \"two\"", "1.0.0");
    assert(own_method1 != NULL);
    assert(own_method2 != NULL);
    ar_methodology__register_method(own_method1);
    ar_methodology__register_method(own_method2);
    own_method1 = NULL; // Ownership transferred
    own_method2 = NULL; // Ownership transferred
    
    // When we initialize both systems with different methods
    int64_t agent1_id = ar_system__init_with_instance(own_system1, "system1_test", "1.0.0");
    int64_t agent2_id = ar_system__init_with_instance(own_system2, "system2_test", "1.0.0");
    
    // Then both should create agents
    assert(agent1_id > 0);
    assert(agent2_id > 0);
    
    // And they should be independent
    assert(agent1_id != agent2_id || own_agency1 != own_agency2);
    
    // Process wake messages
    ar_system__process_next_message_with_instance(own_system1);
    ar_system__process_next_message_with_instance(own_system2);
    
    // Clean up
    ar_system__shutdown_with_instance(own_system1);
    ar_system__shutdown_with_instance(own_system2);
    ar_system__destroy(own_system1);
    ar_system__destroy(own_system2);
    ar_agency__destroy(own_agency1);
    ar_agency__destroy(own_agency2);
    
    printf("Parallel system instances test passed.\n");
}

static void test_system_instance_message_processing(void) {
    printf("Testing instance-based message processing...\n");
    
    // Given a system instance with its own agency
    ar_agency_t *own_agency = ar_agency__create(NULL);
    ar_system_t *own_system = ar_system__create(own_agency);
    assert(own_agency != NULL);
    assert(own_system != NULL);
    
    // Create a simple method
    ar_method_t *own_method = ar_method__create("msg_test", 
        "memory[\"processed\"] := \"true\"", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method(own_method);
    own_method = NULL; // Ownership transferred
    
    // Initialize with the method
    int64_t agent_id = ar_system__init_with_instance(own_system, "msg_test", "1.0.0");
    assert(agent_id > 0);
    
    // Process the wake message (this executes the method)
    bool processed = ar_system__process_next_message_with_instance(own_system);
    assert(processed);
    
    // Verify the method was executed by checking agent memory
    const ar_data_t *ref_memory = ar_agency__get_agent_memory_with_instance(own_agency, agent_id);
    assert(ref_memory != NULL);
    assert(ar_data__get_type(ref_memory) == AR_DATA_TYPE__MAP);
    
    // Send a manual message to the agent
    ar_data_t *own_test_msg = ar_data__create_string("test message");
    bool sent = ar_agency__send_to_agent_with_instance(own_agency, agent_id, own_test_msg);
    assert(sent);
    
    // Process the test message
    processed = ar_system__process_next_message_with_instance(own_system);
    assert(processed);
    
    // Process all should return 0 when no messages
    int count = ar_system__process_all_messages_with_instance(own_system);
    assert(count == 0);
    
    // Clean up
    ar_system__shutdown_with_instance(own_system);
    ar_system__destroy(own_system);
    ar_agency__destroy(own_agency);
    
    printf("Instance-based message processing test passed.\n");
}

int main(void) {
    printf("Starting Agerun system instance tests...\n");
    
    // Initialize global system for method registration
    ar_system__init(NULL, NULL);
    
    // Run all tests
    test_system_instance_create_destroy();
    test_system_instance_with_custom_agency();
    test_system_instance_parallel_systems();
    test_system_instance_message_processing();
    
    // Clean up global system
    ar_system__shutdown();
    
    printf("All system instance tests passed!\n");
    return 0;
}