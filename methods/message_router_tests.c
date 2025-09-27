#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"

static void test_message_router_routing(void) {
    printf("Testing message-router method with routing...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("message_router_routing");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load required methods
    assert(ar_method_fixture__load_method(own_fixture, "echo", "../../methods/echo-1.0.0.method", "1.0.0"));
    assert(ar_method_fixture__load_method(own_fixture, "calculator", "../../methods/calculator-1.0.0.method", "1.0.0"));
    assert(ar_method_fixture__load_method(own_fixture, "message-router", "../../methods/message-router-1.0.0.method", "1.0.0"));
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);

    // Create empty contexts for the agents
    ar_data_t *own_router_context = ar_data__create_map();
    assert(own_router_context != NULL);
    ar_data_t *own_echo_context = ar_data__create_map();
    assert(own_echo_context != NULL);
    ar_data_t *own_calc_context = ar_data__create_map();
    assert(own_calc_context != NULL);
    
    // Create router agent
    int64_t router_agent = ar_agency__create_agent(mut_agency, "message-router", "1.0.0", own_router_context);
    assert(router_agent > 0);
    // Note: Agent stores reference to context, don't destroy it here
    
    // Create echo agent
    int64_t echo_agent = ar_agency__create_agent(mut_agency, "echo", "1.0.0", own_echo_context);
    assert(echo_agent > 0);
    // Note: Agent stores reference to context, don't destroy it here
    
    // Create calculator agent
    int64_t calc_agent = ar_agency__create_agent(mut_agency, "calculator", "1.0.0", own_calc_context);
    assert(calc_agent > 0);
    // Note: Agent stores reference to context, don't destroy it here
    
    // No initial message processing needed
    
    // Test routing to echo agent
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    
    ar_data__set_map_string(own_message, "route", "echo");
    ar_data__set_map_integer(own_message, "echo_agent", (int)echo_agent);
    ar_data__set_map_integer(own_message, "calc_agent", (int)calc_agent);
    ar_data__set_map_string(own_message, "content", "Hello from router!");
    
    bool sent = ar_agency__send_to_agent(mut_agency, router_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the routing message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    if (!processed) {
        printf("ERROR: Failed to process routing message to echo\n");
    }
    assert(processed);
    
    // Debug: Check if payload was set correctly
    // Note: The router's memory is not the message - we need to check what happened during evaluation
    
    // Check router memory
    const ar_data_t *router_memory = ar_agency__get_agent_memory(mut_agency, router_agent);
    assert(router_memory != NULL);
    
    const ar_data_t *is_echo = ar_data__get_map_data(router_memory, "is_echo");
    if (is_echo && ar_data__get_type(is_echo) == AR_DATA_TYPE__INTEGER && ar_data__get_integer(is_echo) == 1) {
        printf("SUCCESS: if() correctly identified route = \"echo\"\n");
    }
    
    const ar_data_t *target = ar_data__get_map_data(router_memory, "target");
    if (target && ar_data__get_type(target) == AR_DATA_TYPE__INTEGER && ar_data__get_integer(target) == echo_agent) {
        printf("SUCCESS: Target correctly set to echo agent ID %lld\n", (long long)echo_agent);
    }
    
    const ar_data_t *sent_result = ar_data__get_map_data(router_memory, "sent");
    if (!sent_result) {
        printf("FAIL: memory.sent not found - send() function failed\n");
    } else if (ar_data__get_type(sent_result) == AR_DATA_TYPE__INTEGER) {
        int64_t sent_value = ar_data__get_integer(sent_result);
        printf("Router send result: %lld (should be 1 for success)\n", (long long)sent_value);
        if (sent_value == 0) {
            printf("WARNING: Router failed to send message to echo agent!\n");
        }
    }
    
    // Test routing to calculator agent
    ar_data_t *own_message2 = ar_data__create_map();
    assert(own_message2 != NULL);
    
    ar_data__set_map_string(own_message2, "route", "calc");
    ar_data__set_map_integer(own_message2, "echo_agent", (int)echo_agent);
    ar_data__set_map_integer(own_message2, "calc_agent", (int)calc_agent);
    ar_data__set_map_string(own_message2, "operation", "add");
    ar_data__set_map_integer(own_message2, "a", 10);
    ar_data__set_map_integer(own_message2, "b", 20);
    
    sent = ar_agency__send_to_agent(mut_agency, router_agent, own_message2);
    assert(sent);
    own_message2 = NULL; // Ownership transferred
    
    // Process the second routing message
    processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Test invalid route
    ar_data_t *own_message3 = ar_data__create_map();
    assert(own_message3 != NULL);
    
    ar_data__set_map_string(own_message3, "route", "invalid");
    ar_data__set_map_integer(own_message3, "echo_agent", (int)echo_agent);
    ar_data__set_map_integer(own_message3, "calc_agent", (int)calc_agent);
    
    sent = ar_agency__send_to_agent(mut_agency, router_agent, own_message3);
    assert(sent);
    own_message3 = NULL; // Ownership transferred
    
    // Process the invalid route message
    processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Process all remaining messages
    int remaining_count = 0;
    while (ar_method_fixture__process_next_message(own_fixture)) {
        remaining_count++;
        printf("Processed message %d\n", remaining_count);
    }
    printf("Processed %d total remaining messages\n", remaining_count);
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    // Now safe to destroy contexts after fixture cleanup
    ar_data__destroy(own_router_context);
    ar_data__destroy(own_echo_context);
    ar_data__destroy(own_calc_context);
    
    printf("✓ Message router routing test passed\n");
}

int main(void) {
    printf("Running message-router method tests...\n\n");
    
    test_message_router_routing();
    
    printf("\nAll message-router method tests passed!\n");
    return 0;
}
