#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_method_test_fixture.h"
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_data.h"

static void test_message_router_routing(void) {
    printf("Testing message-router method with routing...\n");
    
    // Create test fixture
    method_test_fixture_t *own_fixture = ar_method_test_fixture_create("message_router_routing");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_test_fixture_initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_test_fixture_verify_directory(own_fixture));
    
    // Load required methods
    assert(ar_method_test_fixture_load_method(own_fixture, "echo", "../methods/echo-1.0.0.method", "1.0.0"));
    assert(ar_method_test_fixture_load_method(own_fixture, "calculator", "../methods/calculator-1.0.0.method", "1.0.0"));
    assert(ar_method_test_fixture_load_method(own_fixture, "message-router", "../methods/message-router-1.0.0.method", "1.0.0"));
    
    // Create router agent
    agent_id_t router_agent = ar_agent_create("message-router", "1.0.0", NULL);
    assert(router_agent > 0);
    
    // Create echo agent
    agent_id_t echo_agent = ar_agent_create("echo", "1.0.0", NULL);
    assert(echo_agent > 0);
    
    // Create calculator agent
    agent_id_t calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
    assert(calc_agent > 0);
    
    // Process wake messages
    ar_system_process_next_message(); // router wake
    ar_system_process_next_message(); // echo wake
    ar_system_process_next_message(); // calc wake
    
    // Test routing to echo agent
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "route", "echo");
    ar_data_set_map_integer(own_message, "echo_agent", (int)echo_agent);
    ar_data_set_map_integer(own_message, "calc_agent", (int)calc_agent);
    ar_data_set_map_string(own_message, "content", "Hello from router!");
    
    bool sent = ar_agent_send(router_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the routing message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Check router memory
    const data_t *router_memory = ar_agent_get_memory(router_agent);
    assert(router_memory != NULL);
    
    const data_t *is_echo = ar_data_get_map_data(router_memory, "is_echo");
    if (is_echo && ar_data_get_type(is_echo) == DATA_INTEGER && ar_data_get_integer(is_echo) == 1) {
        printf("SUCCESS: if() correctly identified route = \"echo\"\n");
    }
    
    const data_t *target = ar_data_get_map_data(router_memory, "target");
    if (target && ar_data_get_type(target) == DATA_INTEGER && ar_data_get_integer(target) == echo_agent) {
        printf("SUCCESS: Target correctly set to echo agent ID %lld\n", (long long)echo_agent);
    }
    
    const data_t *sent_result = ar_data_get_map_data(router_memory, "sent");
    if (!sent_result) {
        printf("FAIL: memory.sent not found - send() function failed\n");
    }
    
    // Test routing to calculator agent
    data_t *own_message2 = ar_data_create_map();
    assert(own_message2 != NULL);
    
    ar_data_set_map_string(own_message2, "route", "calc");
    ar_data_set_map_integer(own_message2, "echo_agent", (int)echo_agent);
    ar_data_set_map_integer(own_message2, "calc_agent", (int)calc_agent);
    ar_data_set_map_string(own_message2, "operation", "add");
    ar_data_set_map_integer(own_message2, "a", 10);
    ar_data_set_map_integer(own_message2, "b", 20);
    
    sent = ar_agent_send(router_agent, own_message2);
    assert(sent);
    own_message2 = NULL; // Ownership transferred
    
    // Process the second routing message
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Test invalid route
    data_t *own_message3 = ar_data_create_map();
    assert(own_message3 != NULL);
    
    ar_data_set_map_string(own_message3, "route", "invalid");
    ar_data_set_map_integer(own_message3, "echo_agent", (int)echo_agent);
    ar_data_set_map_integer(own_message3, "calc_agent", (int)calc_agent);
    
    sent = ar_agent_send(router_agent, own_message3);
    assert(sent);
    own_message3 = NULL; // Ownership transferred
    
    // Process the invalid route message
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Check for memory leaks
    assert(ar_method_test_fixture_check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_test_fixture_destroy(own_fixture);
    
    printf("✓ Message router routing test passed\n");
}

int main(void) {
    printf("Running message-router method tests...\n\n");
    
    test_message_router_routing();
    
    printf("\nAll message-router method tests passed!\n");
    return 0;
}
