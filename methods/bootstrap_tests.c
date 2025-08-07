#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_assert.h"
#include "ar_methodology.h"
#include "ar_method.h"

// TODO: Re-enable this test once system can load methods from files (Cycle 5)
// This test verifies that bootstrap can spawn echo, but that requires
// the system to be able to load methods from the filesystem first.
/*
static void test_bootstrap_spawns_echo(void) {
    printf("Testing bootstrap spawns echo agent...\n");
    
    // Given a bootstrap agent with echo message as context
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("bootstrap_spawns_echo");
    AR_ASSERT(own_fixture != NULL, "Fixture should be created");
    
    // Initialize test environment
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    
    // Verify correct directory
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Should be in bin directory");
    
    // Load bootstrap method
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "bootstrap", "../../methods/bootstrap-1.0.0.method", "1.0.0"),
              "Bootstrap method should load");
    
    // Load echo method (needed by bootstrap)
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "echo", "../../methods/echo-1.0.0.method", "1.0.0"),
              "Echo method should load");
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Agency should be available");
    
    // Create context that is a valid echo message
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Context map should be created");
    
    ar_data_t *own_sender = ar_data__create_integer(0);
    ar_data__set_map_data(own_context, "sender", own_sender);
    own_sender = NULL; // Ownership transferred
    
    ar_data_t *own_content = ar_data__create_string("Test message from bootstrap");
    ar_data__set_map_data(own_context, "content", own_content);
    own_content = NULL; // Ownership transferred
    
    // When creating bootstrap agent with echo message as context
    int64_t bootstrap_agent = ar_agency__create_agent_with_instance(mut_agency, "bootstrap", "1.0.0", own_context);
    AR_ASSERT(bootstrap_agent > 0, "Bootstrap agent should be created");
    // Note: Agent stores reference to context, don't destroy it here
    
    // Process wake message (bootstrap should spawn echo and send context)
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Wake message should be processed");
    
    // Process the message bootstrap sends to echo
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Bootstrap should send message to echo");
    
    // Process echo's wake message
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Echo wake message should be processed");
    
    // Process the actual echo response (echo sends back to sender=0)
    processed = ar_method_fixture__process_next_message(own_fixture);
    // Note: This might fail since sender=0 goes to system, not another agent
    
    // Clean up
    ar_agency__destroy_agent_with_instance(mut_agency, bootstrap_agent);
    
    // Now destroy the context we created (agent only stores a reference)
    ar_data__destroy(own_context);
    
    // Check for memory leaks
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "No memory leaks should be detected");
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("PASS\n");
}
*/

static void test_bootstrap_handles_wake_message(void) {
    printf("Testing bootstrap handles wake message...\n");
    
    // Given a bootstrap agent
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("bootstrap_wake");
    AR_ASSERT(own_fixture != NULL, "Fixture should be created");
    
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Should be in bin directory");
    
    // Load bootstrap method
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "bootstrap", "../../methods/bootstrap-1.0.0.method", "1.0.0"),
              "Bootstrap method should load");
    
    // Verify the method was parsed successfully (has AST)
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Agency should be available");
    
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Methodology should be available");
    
    ar_method_t *ref_method = ar_methodology__get_method_with_instance(mut_methodology, "bootstrap", "1.0.0");
    AR_ASSERT(ref_method != NULL, "Bootstrap method should be found");
    AR_ASSERT(ar_method__get_ast(ref_method) != NULL, "Bootstrap method should have valid AST (no parse errors)");
    
    // Echo method not needed for basic wake message test
    // Will be needed once spawn functionality is enabled
    
    // Create empty context for bootstrap
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Context should be created");
    
    // When creating bootstrap agent
    int64_t bootstrap_agent = ar_agency__create_agent_with_instance(mut_agency, "bootstrap", "1.0.0", own_context);
    AR_ASSERT(bootstrap_agent > 0, "Bootstrap agent should be created");
    
    // Then wake message should be processed without error
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Wake message should be processed");
    
    // Verify agent memory was initialized
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, bootstrap_agent);
    AR_ASSERT(agent_memory != NULL, "Bootstrap agent memory should be initialized");
    
    // Clean up
    ar_agency__destroy_agent_with_instance(mut_agency, bootstrap_agent);
    ar_data__destroy(own_context);
    
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "No memory leaks");
    ar_method_fixture__destroy(own_fixture);
    
    printf("PASS\n");
}


int main(void) {
    printf("Bootstrap Method Tests\n");
    printf("======================\n\n");
    
    test_bootstrap_handles_wake_message();
    // TODO: Re-enable once system can load methods from files (Cycle 5)
    // test_bootstrap_spawns_echo();
    
    printf("\nAll bootstrap tests passed!\n");
    return 0;
}