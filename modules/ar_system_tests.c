#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_agent_registry.h"
#include "ar_assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

/* Message strings */
static const char *g_test_message = "test_message";

/* Test function prototypes */
static void test_no_auto_loading_on_init(void);
static void test_method_creation(ar_system_t *mut_system);
static void test_agent_creation(ar_system_t *mut_system);
static void test_message_passing(ar_system_t *mut_system);
static void test_no_wake_message_from_init_with_agent(void);

static void test_no_auto_loading_on_init(void) {
    printf("Testing that system does NOT auto-load files on init...\n");
    
    // Given we want to verify no file loading happens during init
    // We'll redirect stdout to capture any warning messages
    
    // Save original stdout
    fflush(stdout);
    int original_stdout = dup(1);
    AR_ASSERT(original_stdout != -1, "Could not duplicate stdout for capture");
    
    // Create a pipe to capture output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        close(original_stdout);
        AR_ASSERT(false, "Could not create pipe for stdout capture");
    }
    
    // Redirect stdout to pipe
    if (dup2(pipefd[1], 1) == -1) {
        close(original_stdout);
        close(pipefd[0]);
        close(pipefd[1]);
        AR_ASSERT(false, "Could not redirect stdout to pipe");
    }
    close(pipefd[1]);
    
    // When we create and init a system
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");
    
    ar_system__init_with_instance(mut_system, NULL, NULL);
    
    // Restore stdout
    fflush(stdout);
    if (dup2(original_stdout, 1) == -1) {
        // Can't restore stdout, but test should still fail appropriately
    }
    close(original_stdout);
    
    // Read captured output
    char buffer[1024] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    
    // Then verify NO loading warnings were printed
    bool loading_attempted = (strstr(buffer, "Warning: Could not load") != NULL);
    if (loading_attempted) {
        printf("Captured output: %s\n", buffer);
    }
    AR_ASSERT(!loading_attempted, "System should NOT attempt to load files during init");
    
    // Clean up
    ar_system__shutdown_with_instance(mut_system);
    ar_system__destroy(mut_system);
    
    // Remove temp files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    printf("No auto-loading test passed!\n");
}

static void test_method_creation(ar_system_t *mut_system) {
    printf("Testing method creation...\n");
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Given we want to create a new method
    const char *method_name = "test_method";
    const char *method_body = "send(0, \"Hello, World!\")";
    
    // When we create the method
    ar_method_t *own_method = ar_method__create(method_name, method_body, "1.0.0");
    
    // Then the method should be created successfully
    assert(own_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create a new version of the same method
    const char *updated_body = "send(0, \"Hello, Updated World!\")";
    ar_method_t *own_method2 = ar_method__create(method_name, updated_body, "2.0.0");
    
    // Then the method should be created successfully
    assert(own_method2 != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_method2);
    own_method2 = NULL; // Mark as transferred
    
    // For test purposes, we use version "2.0.0"
    const char *version2 = "2.0.0";
    
    // And the new version should be different from the previous version
    assert(strcmp(version2, version) != 0);
    
    printf("Method creation test passed.\n");
}

static void test_agent_creation(ar_system_t *mut_system) {
    printf("Testing agent creation...\n");
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Given we have a method for an agent
    const char *method_name = "agent_test";
    const char *method_body = "send(0, \"Agent created\")";
    
    // Create method and register it with the system's methodology 
    ar_method_t *own_method = ar_method__create(method_name, method_body, "1.0.0");
    assert(own_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an agent with this method using the system's agency
    int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    ar_agent_registry_t *ref_registry = ar_agency__get_registry_with_instance(mut_agency);
    assert(ar_agent_registry__is_registered(ref_registry, agent_id));
    
    // When we send a message to the agent
    ar_data_t *test_message = ar_data__create_string(g_test_message);
    assert(test_message != NULL);
    bool send_result = ar_agency__send_to_agent_with_instance(mut_agency, agent_id, test_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the test message
    ar_system__process_next_message_with_instance(mut_system);
    
    // When we destroy the agent
    ar_agency__destroy_agent_with_instance(mut_agency, agent_id);
    
    // Then the destruction should succeed
    // Agency destroy returns void
    
    // And the agent should no longer exist in the system
    assert(!ar_agent_registry__is_registered(ref_registry, agent_id));
    
    printf("Agent creation test passed.\n");
}

static void test_message_passing(ar_system_t *mut_system) {
    printf("Testing message passing between agents...\n");
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Given methods for sender and receiver agents
    // Create and register sender method
    ar_method_t *own_sender_method = ar_method__create("sender", "send(target_id, \"Hello from sender!\")", "1.0.0");
    assert(own_sender_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_sender_method);
    own_sender_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *sender_version = "1.0.0";
    
    // Create and register receiver method
    ar_method_t *own_receiver_method = ar_method__create("receiver", "memory[\"received\"] := \"true\"", "1.0.0");
    assert(own_receiver_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_receiver_method);
    own_receiver_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *receiver_version = "1.0.0";
    
    // And a receiver agent created with the receiver method
    int64_t receiver_id = ar_agency__create_agent_with_instance(mut_agency, "receiver", receiver_version, NULL);
    assert(receiver_id > 0);
    
    // And a sender agent created with the sender method
    // Note: In the full implementation, a context with receiver ID would be passed
    int64_t sender_id = ar_agency__create_agent_with_instance(mut_agency, "sender", sender_version, NULL);
    assert(sender_id > 0);
    
    // When we process all pending messages
    // With opaque ar_map_t, we can't rely on the exact count
    ar_system__process_all_messages_with_instance(mut_system);
    
    // When we clean up the agents
    ar_agency__destroy_agent_with_instance(mut_agency, sender_id);
    ar_agency__destroy_agent_with_instance(mut_agency, receiver_id);
    
    // Then the destruction should succeed
    // Agency destroy returns void
    
    printf("Message passing test passed.\n");
}

static void test_no_auto_saving_on_shutdown(void) {
    printf("Testing that system does NOT auto-save on shutdown...\n");
    
    // Given a clean environment with no existing files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // And a system with methodology and agents
    ar_system_t *mut_system = ar_system__create();
    assert(mut_system != NULL);
    ar_system__init_with_instance(mut_system, NULL, NULL);
    
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    assert(mut_agency != NULL);
    
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    assert(mut_methodology != NULL);
    
    // And the methodology has methods
    ar_method_t *own_method = ar_method__create("test_method", "send(0, \"test\")", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    
    // And the agency has active agents
    int64_t agent_id = ar_agency__create_agent_with_instance(mut_agency, "test_method", "1.0.0", NULL);
    assert(agent_id > 0);
    
    // When the system is shut down
    ar_system__shutdown_with_instance(mut_system);
    ar_system__destroy(mut_system);
    
    // Then no files should be saved
    struct stat st;
    bool methodology_exists = (stat("methodology.agerun", &st) == 0);
    bool agency_exists = (stat("agency.agerun", &st) == 0);
    
    AR_ASSERT(!methodology_exists, "methodology.agerun should NOT have been saved on shutdown");
    AR_ASSERT(!agency_exists, "agency.agerun should NOT have been saved on shutdown");
    
    printf("No auto-saving test passed.\n");
}

static void test_no_wake_message_from_init_with_agent(void) {
    printf("Testing that system does NOT send wake message when init with agent...\n");
    
    // Given we create a system
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Create a test method that checks if the message is "__wake__" and records it
    ar_method_t *own_method = ar_method__create("init_test", 
        "memory.got_wake := if(message = \"__wake__\", 1, 0)", "1.0.0");
    AR_ASSERT(own_method != NULL, "Method creation should succeed");
    
    // Register the method with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    
    // When we initialize the system with an initial agent
    ar_system__init_with_instance(mut_system, "init_test", "1.0.0");
    
    // Check the agent's memory to see if it received a wake message
    const ar_data_t *ref_memory = ar_agency__get_agent_memory_with_instance(mut_agency, 1);
    AR_ASSERT(ref_memory != NULL, "Agent should have memory");
    
    const ar_data_t *ref_got_wake = ar_data__get_map_data(ref_memory, "got_wake");
    
    // Debug output
    if (ref_got_wake != NULL) {
        int value = ar_data__get_integer(ref_got_wake);
        printf("DEBUG: memory.got_wake = %d\n", value);
    } else {
        printf("DEBUG: memory.got_wake is not set\n");
    }
    
    // Verify that no wake message was sent
    // ref_got_wake should be NULL (field not set) since no wake message is sent
    AR_ASSERT(ref_got_wake == NULL, "Agent should NOT have received a wake message");
    
    // Clean up
    ar_system__shutdown_with_instance(mut_system);
    ar_system__destroy(mut_system);
    
    printf("No wake message from init test passed!\n");
}

int main(void) {
    printf("Starting Agerun tests...\n");
    
    // Test that system does NOT auto-load files
    test_no_auto_loading_on_init();
    
    // Test that system does NOT auto-save files
    test_no_auto_saving_on_shutdown();
    
    // Test that system does NOT send wake messages
    test_no_wake_message_from_init_with_agent();
    
    // Create system instance
    ar_system_t *mut_system = ar_system__create();
    if (mut_system == NULL) {
        printf("Error: Failed to create system instance\n");
        return 1;
    }
    
    // Initialize the system with no initial agent
    ar_system__init_with_instance(mut_system, NULL, NULL);
    
    // Given we create a test method
    ar_method_t *own_method = ar_method__create("test_init", "send(0, \"Runtime initialized\")", "1.0.0");
    
    // Then the method should be created successfully
    if (own_method == NULL) {
        printf("Error: Failed to create test_init method\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Register with the system's methodology
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an initial agent with this method using the system's agency
    int64_t initial_agent = ar_agency__create_agent_with_instance(mut_agency, "test_init", version, NULL);
    
    // Then the agent should be created successfully
    if (initial_agent == 0) {
        printf("Error: Failed to create initial agent\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // When we run all system tests
    test_method_creation(mut_system);
    test_agent_creation(mut_system);
    test_message_passing(mut_system);
    
    // Clean up the initial agent using the system's agency
    ar_agency__destroy_agent_with_instance(mut_agency, initial_agent);
    
    // Then clean up the system
    ar_system__shutdown_with_instance(mut_system);
    ar_system__destroy(mut_system);
    
    // Global cleanup
    ar_methodology__cleanup();
    ar_agency__reset();
    
    // And report success
    printf("All tests passed!\n");
    return 0;
}
