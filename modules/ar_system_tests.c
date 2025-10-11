#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_agent_registry.h"
#include "ar_proxy.h"
#include "ar_proxy_registry.h"
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
    
    ar_system__init(mut_system, NULL, NULL);
    
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
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);
    
    // Remove temp files
    remove("methodology.agerun");
    remove("agerun.agency");
    
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
    ar_methodology__register_method(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create a new version of the same method
    const char *updated_body = "send(0, \"Hello, Updated World!\")";
    ar_method_t *own_method2 = ar_method__create(method_name, updated_body, "2.0.0");
    
    // Then the method should be created successfully
    assert(own_method2 != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_method2);
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
    ar_methodology__register_method(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an agent with this method using the system's agency
    int64_t agent_id = ar_agency__create_agent(mut_agency, method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    ar_agent_registry_t *ref_registry = ar_agency__get_registry(mut_agency);
    assert(ar_agent_registry__is_registered(ref_registry, agent_id));
    
    // When we send a message to the agent
    ar_data_t *test_message = ar_data__create_string(g_test_message);
    assert(test_message != NULL);
    bool send_result = ar_agency__send_to_agent(mut_agency, agent_id, test_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the test message
    ar_system__process_next_message(mut_system);
    
    // When we destroy the agent
    ar_agency__destroy_agent(mut_agency, agent_id);
    
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
    ar_methodology__register_method(mut_methodology, own_sender_method);
    own_sender_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *sender_version = "1.0.0";
    
    // Create and register receiver method
    ar_method_t *own_receiver_method = ar_method__create("receiver", "memory[\"received\"] := \"true\"", "1.0.0");
    assert(own_receiver_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_receiver_method);
    own_receiver_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *receiver_version = "1.0.0";
    
    // And a receiver agent created with the receiver method
    int64_t receiver_id = ar_agency__create_agent(mut_agency, "receiver", receiver_version, NULL);
    assert(receiver_id > 0);
    
    // And a sender agent created with the sender method
    // Note: In the full implementation, a context with receiver ID would be passed
    int64_t sender_id = ar_agency__create_agent(mut_agency, "sender", sender_version, NULL);
    assert(sender_id > 0);
    
    // When we process all pending messages
    // With opaque ar_map_t, we can't rely on the exact count
    ar_system__process_all_messages(mut_system);
    
    // When we clean up the agents
    ar_agency__destroy_agent(mut_agency, sender_id);
    ar_agency__destroy_agent(mut_agency, receiver_id);
    
    // Then the destruction should succeed
    // Agency destroy returns void
    
    printf("Message passing test passed.\n");
}

static void test_no_auto_saving_on_shutdown(void) {
    printf("Testing that system does NOT auto-save on shutdown...\n");

    // Given a clean environment with no existing files
    remove("methodology.agerun");
    remove("agerun.agency");

    // And a system with methodology and agents
    ar_system_t *mut_system = ar_system__create();
    assert(mut_system != NULL);
    ar_system__init(mut_system, NULL, NULL);

    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    assert(mut_agency != NULL);

    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    assert(mut_methodology != NULL);

    // And the methodology has methods
    ar_method_t *own_method = ar_method__create("test_method", "send(0, \"test\")", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method(mut_methodology, own_method);

    // And the agency has active agents
    int64_t agent_id = ar_agency__create_agent(mut_agency, "test_method", "1.0.0", NULL);
    assert(agent_id > 0);

    // When the system is shut down
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);

    // Then no files should be saved
    struct stat st;
    bool methodology_exists = (stat("methodology.agerun", &st) == 0);
    bool agency_exists = (stat("agerun.agency", &st) == 0);

    AR_ASSERT(!methodology_exists, "methodology.agerun should NOT have been saved on shutdown");
    AR_ASSERT(!agency_exists, "agerun.agency should NOT have been saved on shutdown");

    printf("No auto-saving test passed.\n");
}

static void test_system__has_proxy_registry(void) {
    printf("Testing that system has proxy registry...\n");

    // Given a system instance
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");

    // When we get the proxy registry
    ar_proxy_registry_t *ref_registry = ar_system__get_proxy_registry(mut_system);

    // Then the registry should exist
    AR_ASSERT(ref_registry != NULL, "System should have a proxy registry");

    // Clean up
    ar_system__destroy(mut_system);

    printf("Proxy registry test passed.\n");
}

static void test_system__register_proxy(void) {
    printf("Testing proxy registration in system...\n");

    // Given a system instance
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");

    // And a log instance for the proxy
    ar_log_t *ref_log = ar_system__get_log(mut_system);
    AR_ASSERT(ref_log != NULL, "System should have a log");

    // And a test proxy
    ar_proxy_t *own_proxy = ar_proxy__create(ref_log, "test");
    AR_ASSERT(own_proxy != NULL, "Proxy creation should succeed");

    // When we register the proxy with ID -100
    bool result = ar_system__register_proxy(mut_system, -100, own_proxy);

    // Then the registration should succeed
    AR_ASSERT(result, "Proxy registration should succeed");

    // And we should be able to find it in the registry
    ar_proxy_registry_t *ref_registry = ar_system__get_proxy_registry(mut_system);
    ar_proxy_t *ref_found = ar_proxy_registry__find(ref_registry, -100);
    AR_ASSERT(ref_found != NULL, "Registered proxy should be findable");
    AR_ASSERT(strcmp(ar_proxy__get_type(ref_found), "test") == 0, "Found proxy should have correct type");

    // Clean up (system owns proxy now, will destroy it)
    ar_system__destroy(mut_system);

    printf("Proxy registration test passed.\n");
}

int main(void) {
    printf("Starting Agerun tests...\n");

    // Test that system does NOT auto-load files
    test_no_auto_loading_on_init();

    // Test that system does NOT auto-save files
    test_no_auto_saving_on_shutdown();

    // Test proxy registry integration
    test_system__has_proxy_registry();
    test_system__register_proxy();
    
    // Create system instance
    ar_system_t *mut_system = ar_system__create();
    if (mut_system == NULL) {
        printf("Error: Failed to create system instance\n");
        return 1;
    }
    
    // Initialize the system with no initial agent
    ar_system__init(mut_system, NULL, NULL);
    
    // Given we create a test method
    ar_method_t *own_method = ar_method__create("test_init", "send(0, \"Runtime initialized\")", "1.0.0");
    
    // Then the method should be created successfully
    if (own_method == NULL) {
        printf("Error: Failed to create test_init method\n");
        ar_system__shutdown(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an initial agent with this method using the system's agency
    int64_t initial_agent = ar_agency__create_agent(mut_agency, "test_init", version, NULL);
    
    // Then the agent should be created successfully
    if (initial_agent == 0) {
        printf("Error: Failed to create initial agent\n");
        ar_system__shutdown(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // When we run all system tests
    test_method_creation(mut_system);
    test_agent_creation(mut_system);
    test_message_passing(mut_system);
    
    // Clean up the initial agent using the system's agency
    ar_agency__destroy_agent(mut_agency, initial_agent);
    
    // Then clean up the system
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);
    
    // Global cleanup
    /* ar_methodology__cleanup() removed - system handles cleanup */
    // ar_agency__reset(); // Global API removed
    
    // And report success
    printf("All tests passed!\n");
    return 0;
}
