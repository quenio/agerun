#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"

static void test_calculator_add(void) {
    printf("Testing calculator method with add operation...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("calculator_add");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register calculator method
    assert(ar_method_fixture__load_method(own_fixture, "calculator", "../../methods/calculator-1.0.0.method", "1.0.0"));
    
    // Create initial memory for the agent
    ar_data_t *own_initial_memory = ar_data__create_map();
    assert(own_initial_memory != NULL);
    
    // Initialize memory.result to 0 as per the calculator method
    ar_data__set_map_integer(own_initial_memory, "result", 0);
    
    // Get the fixture\'s agency

    
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);

    
    assert(mut_agency != NULL);

    
    

    
    // Create calculator agent with initial memory using the fixture\'s agency
    int64_t calc_agent = ar_agency__create_agent_with_instance(mut_agency, "calculator", "1.0.0", own_initial_memory);
    assert(calc_agent != 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process wake message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Create and send add operation message
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    ar_data__set_map_integer(own_message, "sender", 0);
    ar_data__set_map_string(own_message, "operation", "add");
    ar_data__set_map_integer(own_message, "a", 5);
    ar_data__set_map_integer(own_message, "b", 3);
    
    printf("DEBUG: About to process calculator message\n");
    bool sent = ar_agency__send_to_agent_with_instance(mut_agency, calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculator message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Get agent memory to check result
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, calc_agent);
    assert(agent_memory != NULL);
    assert(ar_data__get_type(agent_memory) == AR_DATA_TYPE__MAP);
    
    // Check if result exists
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    assert(result != NULL);
    assert(ar_data__get_type(result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result) == 8);
    
    printf("SUCCESS: Calculator add operation: 5 + 3 = 8\n");
    
    // Clean up
    // Get the agent's context before destroying it (we created it, we must destroy it)
    const ar_data_t *ref_context = ar_agency__get_agent_context_with_instance(mut_agency, calc_agent);
    
    ar_agency__destroy_agent_with_instance(mut_agency, calc_agent);
    
    // Process any remaining messages (including sleep messages)
    while (ar_method_fixture__process_next_message(own_fixture)) {
        // Keep processing
    }
    
    // Destroy the context we created for this agent
    if (ref_context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        ar_data__destroy((ar_data_t*)ref_context);  // We created it, we destroy it
        #pragma GCC diagnostic pop
    }
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Calculator add operation test passed\n");
}

static void test_calculator_multiply(void) {
    printf("Testing calculator method with multiply operation...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("calculator_multiply");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register calculator method
    assert(ar_method_fixture__load_method(own_fixture, "calculator", "../../methods/calculator-1.0.0.method", "1.0.0"));
    
    // Create initial memory for the agent
    ar_data_t *own_initial_memory = ar_data__create_map();
    assert(own_initial_memory != NULL);
    
    // Initialize memory.result to 0 as per the calculator method
    ar_data__set_map_integer(own_initial_memory, "result", 0);
    
    // Get the fixture\'s agency

    
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);

    
    assert(mut_agency != NULL);

    
    

    
    // Create calculator agent with initial memory using the fixture\'s agency
    int64_t calc_agent = ar_agency__create_agent_with_instance(mut_agency, "calculator", "1.0.0", own_initial_memory);
    assert(calc_agent != 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process wake message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Create and send multiply operation message
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    ar_data__set_map_integer(own_message, "sender", 0);
    ar_data__set_map_string(own_message, "operation", "multiply");
    ar_data__set_map_integer(own_message, "a", 5);
    ar_data__set_map_integer(own_message, "b", 2);
    
    printf("DEBUG: About to process calculator message\n");
    bool sent = ar_agency__send_to_agent_with_instance(mut_agency, calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculator message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Get agent memory to check result
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, calc_agent);
    assert(agent_memory != NULL);
    
    // Check if result exists
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    assert(result != NULL);
    assert(ar_data__get_type(result) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result) == 10);
    
    printf("SUCCESS: Calculator multiply operation: 5 * 2 = 10\n");
    
    // Clean up
    // Get the agent's context before destroying it (we created it, we must destroy it)
    const ar_data_t *ref_context = ar_agency__get_agent_context_with_instance(mut_agency, calc_agent);
    
    ar_agency__destroy_agent_with_instance(mut_agency, calc_agent);
    
    // Process any remaining messages (including sleep messages)
    while (ar_method_fixture__process_next_message(own_fixture)) {
        // Keep processing
    }
    
    // Destroy the context we created for this agent
    if (ref_context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        ar_data__destroy((ar_data_t*)ref_context);  // We created it, we destroy it
        #pragma GCC diagnostic pop
    }
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Calculator multiply operation test passed\n");
}

static void test_calculator_subtract(void) {
    printf("Testing calculator method with subtract operation...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("calculator_subtract");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register calculator method
    assert(ar_method_fixture__load_method(own_fixture, "calculator", "../../methods/calculator-1.0.0.method", "1.0.0"));
    
    // Create initial memory for the agent
    ar_data_t *own_initial_memory = ar_data__create_map();
    assert(own_initial_memory != NULL);
    
    // Initialize memory.result to 0 as per the calculator method
    ar_data__set_map_integer(own_initial_memory, "result", 0);
    
    // Get the fixture\'s agency

    
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);

    
    assert(mut_agency != NULL);

    
    

    
    // Create calculator agent with initial memory using the fixture\'s agency
    int64_t calc_agent = ar_agency__create_agent_with_instance(mut_agency, "calculator", "1.0.0", own_initial_memory);
    assert(calc_agent != 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process wake message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Create and send subtract operation message
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    ar_data__set_map_integer(own_message, "sender", 0);
    ar_data__set_map_string(own_message, "operation", "subtract");
    ar_data__set_map_integer(own_message, "a", 10);
    ar_data__set_map_integer(own_message, "b", 7);
    
    printf("DEBUG: About to process calculator message\n");
    bool sent = ar_agency__send_to_agent_with_instance(mut_agency, calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculator message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Get agent memory to check result
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, calc_agent);
    assert(agent_memory != NULL);
    
    // Check if result exists
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    assert(result != NULL);
    assert(ar_data__get_type(result) == AR_DATA_TYPE__INTEGER);
    
    int subtract_result = ar_data__get_integer(result);
    printf("Subtract result: %d\n", subtract_result);
    assert(subtract_result == 3);
    
    // Clean up
    // Get the agent's context before destroying it (we created it, we must destroy it)
    const ar_data_t *ref_context = ar_agency__get_agent_context_with_instance(mut_agency, calc_agent);
    
    ar_agency__destroy_agent_with_instance(mut_agency, calc_agent);
    
    // Process any remaining messages (including sleep messages)
    while (ar_method_fixture__process_next_message(own_fixture)) {
        // Keep processing
    }
    
    // Destroy the context we created for this agent
    if (ref_context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        ar_data__destroy((ar_data_t*)ref_context);  // We created it, we destroy it
        #pragma GCC diagnostic pop
    }
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Calculator subtract operation test passed\n");
}

static void test_calculator_divide(void) {
    printf("Testing calculator method with divide operation...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("calculator_divide");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register calculator method
    assert(ar_method_fixture__load_method(own_fixture, "calculator", "../../methods/calculator-1.0.0.method", "1.0.0"));
    
    // Create initial memory for the agent
    ar_data_t *own_initial_memory = ar_data__create_map();
    assert(own_initial_memory != NULL);
    
    // Initialize memory.result to 0 as per the calculator method
    ar_data__set_map_integer(own_initial_memory, "result", 0);
    
    // Get the fixture\'s agency

    
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);

    
    assert(mut_agency != NULL);

    
    

    
    // Create calculator agent with initial memory using the fixture\'s agency
    int64_t calc_agent = ar_agency__create_agent_with_instance(mut_agency, "calculator", "1.0.0", own_initial_memory);
    assert(calc_agent != 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process wake message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Create and send divide operation message
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    ar_data__set_map_integer(own_message, "sender", 0);
    ar_data__set_map_string(own_message, "operation", "divide");
    ar_data__set_map_integer(own_message, "a", 10);
    ar_data__set_map_integer(own_message, "b", 2);
    
    printf("DEBUG: About to process calculator message\n");
    bool sent = ar_agency__send_to_agent_with_instance(mut_agency, calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculator message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Get agent memory to check result
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, calc_agent);
    assert(agent_memory != NULL);
    
    // Check if result exists (integer division)
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    assert(result != NULL);
    
    // Divide returns integer (integer division)
    assert(ar_data__get_type(result) == AR_DATA_TYPE__INTEGER);
    
    int divide_result = ar_data__get_integer(result);
    printf("Divide result: %d\n", divide_result);
    assert(divide_result == 5);
    
    // Clean up
    // Get the agent's context before destroying it (we created it, we must destroy it)
    const ar_data_t *ref_context = ar_agency__get_agent_context_with_instance(mut_agency, calc_agent);
    
    ar_agency__destroy_agent_with_instance(mut_agency, calc_agent);
    
    // Process any remaining messages (including sleep messages)
    while (ar_method_fixture__process_next_message(own_fixture)) {
        // Keep processing
    }
    
    // Destroy the context we created for this agent
    if (ref_context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        ar_data__destroy((ar_data_t*)ref_context);  // We created it, we destroy it
        #pragma GCC diagnostic pop
    }
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Calculator divide operation test passed\n");
}

static void test_calculator_unknown_operation(void) {
    printf("Testing calculator method with unknown operation...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("calculator_unknown");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register calculator method
    assert(ar_method_fixture__load_method(own_fixture, "calculator", "../../methods/calculator-1.0.0.method", "1.0.0"));
    
    // Create initial memory for the agent
    ar_data_t *own_initial_memory = ar_data__create_map();
    assert(own_initial_memory != NULL);
    
    // Initialize memory.result to 0 as per the calculator method
    ar_data__set_map_integer(own_initial_memory, "result", 0);
    
    // Get the fixture\'s agency

    
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);

    
    assert(mut_agency != NULL);

    
    

    
    // Create calculator agent with initial memory using the fixture\'s agency
    int64_t calc_agent = ar_agency__create_agent_with_instance(mut_agency, "calculator", "1.0.0", own_initial_memory);
    assert(calc_agent != 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process wake message
    ar_method_fixture__process_next_message(own_fixture);
    
    // Create and send unknown operation message
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    ar_data__set_map_integer(own_message, "sender", 0);
    ar_data__set_map_string(own_message, "operation", "modulo");
    ar_data__set_map_integer(own_message, "a", 10);
    ar_data__set_map_integer(own_message, "b", 3);
    
    printf("DEBUG: About to process calculator message\n");
    bool sent = ar_agency__send_to_agent_with_instance(mut_agency, calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculator message
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    assert(processed);
    
    // Get agent memory to check result
    const ar_data_t *agent_memory = ar_agency__get_agent_memory_with_instance(mut_agency, calc_agent);
    assert(agent_memory != NULL);
    
    // Check if result exists (should be 0 for unknown operations)
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    assert(result != NULL);
    assert(ar_data__get_type(result) == AR_DATA_TYPE__INTEGER);
    
    int unknown_result = ar_data__get_integer(result);
    printf("Unknown operation result: %d\n", unknown_result);
    assert(unknown_result == 0);
    
    // Clean up
    // Get the agent's context before destroying it (we created it, we must destroy it)
    const ar_data_t *ref_context = ar_agency__get_agent_context_with_instance(mut_agency, calc_agent);
    
    ar_agency__destroy_agent_with_instance(mut_agency, calc_agent);
    
    // Process any remaining messages (including sleep messages)
    while (ar_method_fixture__process_next_message(own_fixture)) {
        // Keep processing
    }
    
    // Destroy the context we created for this agent
    if (ref_context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        ar_data__destroy((ar_data_t*)ref_context);  // We created it, we destroy it
        #pragma GCC diagnostic pop
    }
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Calculator unknown operation test passed\n");
}

int main(void) {
    printf("Running calculator method tests...\n\n");
    
    test_calculator_add();
    test_calculator_multiply();
    test_calculator_subtract();
    test_calculator_divide();
    test_calculator_unknown_operation();
    
    printf("\nAll calculator method tests passed!\n");
    return 0;
}
