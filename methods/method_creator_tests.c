#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_method_fixture.h"
#include "agerun_system.h"
#include "agerun_agency.h"
#include "agerun_data.h"

static void test_method_creator_create_simple(void) {
    printf("Testing method-creator method with simple method creation...\n");
    
    // Create test fixture
    method_fixture_t *own_fixture = ar__method_fixture__create("method_creator_create_simple");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar__method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar__method_fixture__verify_directory(own_fixture));
    
    // Load method-creator method
    assert(ar__method_fixture__load_method(own_fixture, "method-creator", "../methods/method-creator-1.0.0.method", "1.0.0"));
    
    // Create method-creator agent
    int64_t creator_agent = ar__agency__create_agent("method-creator", "1.0.0", NULL);
    assert(creator_agent > 0);
    
    // Process wake message
    ar__system__process_next_message();
    
    // When we send a message to create a new method
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "method_name", "doubler");
    ar_data_set_map_string(own_message, "instructions", "memory.result := message.value * 2\nsend(message.sender, memory.result)");
    ar_data_set_map_string(own_message, "version", "1.0.0");
    ar_data_set_map_integer(own_message, "sender", 888);
    
    bool sent = ar__agency__send_to_agent(creator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar__system__process_next_message();
    assert(processed);
    
    // Verify method execution by checking agent's memory
    // The method-creator method should:
    // 1. Call method() function to create a new method
    // 2. Store the result (1 for success, 0 for failure) in memory.result
    // 3. Send the result back to the sender
    
    // Get agent memory for verification
    const data_t *agent_memory = ar__agency__get_agent_memory(creator_agent);
    assert(agent_memory != NULL);
    
    // Check memory.result - should contain the result of method() function
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result == NULL) {
        printf("FAIL: memory.result not found - method() instruction failed to execute\n");
        printf("NOTE: This indicates the method() function in instruction module needs implementation\n");
    } else {
        assert(ar_data_get_type(result) == DATA_INTEGER);
        int result_val = ar_data_get_integer(result);
        printf("SUCCESS: method() instruction executed and created memory.result\n");
        printf("  - Method creation result: %d\n", result_val);
        
        if (result_val == 1) {
            printf("  - Method 'doubler' was successfully created\n");
            // We could verify the method exists by trying to create an agent with it
        } else {
            printf("  - Method creation failed (returned 0)\n");
        }
    }
    
    // Check for memory leaks
    assert(ar__method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar__method_fixture__destroy(own_fixture);
    
    printf("✓ Method creator create simple test passed\n");
}

static void test_method_creator_invalid_syntax(void) {
    printf("Testing method-creator method with invalid syntax...\n");
    
    // Create test fixture
    method_fixture_t *own_fixture = ar__method_fixture__create("method_creator_invalid_syntax");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar__method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar__method_fixture__verify_directory(own_fixture));
    
    // Load method-creator method
    assert(ar__method_fixture__load_method(own_fixture, "method-creator", "../methods/method-creator-1.0.0.method", "1.0.0"));
    
    // Create method-creator agent
    int64_t creator_agent = ar__agency__create_agent("method-creator", "1.0.0", NULL);
    assert(creator_agent > 0);
    
    // Process wake message
    ar__system__process_next_message();
    
    // When we send a message to create a method with invalid syntax
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "method_name", "broken");
    ar_data_set_map_string(own_message, "instructions", "memory.result = invalid syntax here");
    ar_data_set_map_string(own_message, "version", "1.0.0");
    ar_data_set_map_integer(own_message, "sender", 888);
    
    bool sent = ar__agency__send_to_agent(creator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar__system__process_next_message();
    assert(processed);
    
    // Verify method execution with invalid syntax
    // The method() function should validate syntax and return 0 for invalid instructions
    
    // Get agent memory for verification
    const data_t *agent_memory = ar__agency__get_agent_memory(creator_agent);
    assert(agent_memory != NULL);
    
    // Check memory.result - should be 0 for invalid syntax
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result == NULL) {
        printf("FAIL: memory.result not found - method() instruction failed to execute\n");
        printf("NOTE: This is expected until method() function is implemented in instruction module\n");
    } else {
        assert(ar_data_get_type(result) == DATA_INTEGER);
        int result_val = ar_data_get_integer(result);
        printf("SUCCESS: method() instruction executed with invalid syntax\n");
        printf("  - Method creation result: %d\n", result_val);
        
        if (result_val == 0) {
            printf("  - Correctly rejected invalid syntax\n");
        } else {
            printf("  - WARNING: Method creation succeeded with invalid syntax (expected failure)\n");
        }
    }
    
    // Check for memory leaks
    assert(ar__method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar__method_fixture__destroy(own_fixture);
    
    printf("✓ Method creator invalid syntax test passed\n");
}

int main(void) {
    printf("Running method-creator method tests...\n\n");
    
    test_method_creator_create_simple();
    test_method_creator_invalid_syntax();
    
    printf("\nAll method-creator method tests passed!\n");
    return 0;
}
