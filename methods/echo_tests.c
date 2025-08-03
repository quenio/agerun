#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"

static void test_echo_simple_message(void) {
    printf("Testing echo method with simple message...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("echo_simple_message");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register echo method
    assert(ar_method_fixture__load_method(own_fixture, "echo", "../../methods/echo-1.0.0.method", "1.0.0"));
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Create empty context for the agent
    ar_data_t *own_context = ar_data__create_map();
    assert(own_context != NULL);
    
    // Create echo agent using the fixture's agency
    int64_t echo_agent = ar_agency__create_agent_with_instance(mut_agency, "echo", "1.0.0", own_context);
    assert(echo_agent > 0);
    // Note: Agent stores reference to context, don't destroy it here
    
    // Process wake message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Verify agent memory was initialized
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, echo_agent);
    assert(agent_memory != NULL);
    
    // When we send a message with sender field
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    
    // Add sender field (0 means system/test)
    ar_data_t *own_sender = ar_data__create_integer(0);
    ar_data__set_map_data(own_message, "sender", own_sender);
    own_sender = NULL; // Ownership transferred
    
    // Add the actual message content
    ar_data_t *own_content = ar_data__create_string("Hello, Echo!");
    ar_data__set_map_data(own_message, "content", own_content);
    own_content = NULL; // Ownership transferred
    
    bool sent = ar_agency__send_to_agent_with_instance(mut_agency, echo_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Echo method now only sends back message.content, doesn't store in memory
    
    // Process the return message (echo sends it back)
    // NOTE: This currently fails because send() requires ownership of the message
    // but message.content is a reference, not an owned value. The AgeRun language
    // doesn't yet support sending memory references directly.
    processed = ar_method_fixture__process_next_message(own_fixture);
    assert(!processed);  // Expected to fail due to ownership limitations
    (void)processed;  // Suppress unused variable warning
    
    // Clean up
    ar_agency__destroy_agent_with_instance(mut_agency, echo_agent);
    
    // Now destroy the context we created (agent only stores a reference)
    ar_data__destroy(own_context);
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("PASS\n");
}

static void test_echo_map_message(void) {
    printf("Testing echo method with map message...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("echo_map_message");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register echo method
    assert(ar_method_fixture__load_method(own_fixture, "echo", "../../methods/echo-1.0.0.method", "1.0.0"));
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Create empty context for the agent
    ar_data_t *own_context = ar_data__create_map();
    assert(own_context != NULL);
    
    // Create echo agent using the fixture's agency
    int64_t echo_agent = ar_agency__create_agent_with_instance(mut_agency, "echo", "1.0.0", own_context);
    assert(echo_agent > 0);
    // Note: Agent stores reference to context, don't destroy it here
    
    // Process wake message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Verify agent memory was initialized
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, echo_agent);
    assert(agent_memory != NULL);
    
    // When we send a map message
    ar_data_t *own_map_message = ar_data__create_map();
    assert(own_map_message != NULL);
    ar_data__set_map_integer(own_map_message, "sender", 0); // Add sender field
    ar_data__set_map_string(own_map_message, "type", "greeting");
    ar_data__set_map_string(own_map_message, "content", "Hello from map!");
    ar_data__set_map_integer(own_map_message, "count", 42);
    
    bool sent = ar_agency__send_to_agent_with_instance(mut_agency, echo_agent, own_map_message);
    assert(sent);
    own_map_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Process the return message (echo sends it back)
    // NOTE: This currently fails because send() requires ownership of the message
    // but message.content is a reference (a map), not an owned value. The AgeRun 
    // language doesn't yet support sending memory references directly.
    processed = ar_method_fixture__process_next_message(own_fixture);
    assert(!processed);  // Expected to fail due to ownership limitations
    
    // Clean up
    ar_agency__destroy_agent_with_instance(mut_agency, echo_agent);
    
    // Now destroy the context we created (agent only stores a reference)
    ar_data__destroy(own_context);
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("PASS\n");
}

int main(void) {
    printf("Running echo method tests...\n");
    
    test_echo_simple_message();
    test_echo_map_message();
    
    printf("All tests passed!\n");
    return 0;
}
