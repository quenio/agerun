/**
 * @file agerun_agent_update_tests.c
 * @brief Tests for the agent update module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_agent_update.h"
#include "ar_system_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_heap.h"

static void test_update_compatibility(void) {
    printf("Testing update compatibility checks...\n");
    
    // Given a system with methods
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("test_compat");
    assert(own_fixture != NULL);
    assert(ar_system_fixture__initialize(own_fixture));
    
    // Register different versions
    ar_method_t *ref_v1_0 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_v1_0 != NULL);
    
    ar_method_t *ref_v1_1 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.1.0"
    );
    assert(ref_v1_1 != NULL);
    
    ar_method_t *ref_v2_0 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "2.0.0"
    );
    assert(ref_v2_0 != NULL);
    
    ar_method_t *ref_other = ar_system_fixture__register_method(
        own_fixture, "calc", "send(0, \"result\")", "1.0.0"
    );
    assert(ref_other != NULL);
    
    // When checking compatibility
    // Then compatible versions should work
    assert(ar_agent_update__are_compatible(ref_v1_0, ref_v1_1));
    assert(ar_agent_update__are_compatible(ref_v1_1, ref_v1_0));
    
    // Then incompatible versions should fail
    assert(!ar_agent_update__are_compatible(ref_v1_0, ref_v2_0));
    assert(!ar_agent_update__are_compatible(ref_v2_0, ref_v1_0));
    
    // Then different methods should fail
    assert(!ar_agent_update__are_compatible(ref_v1_0, ref_other));
    
    // Then null checks should work
    assert(!ar_agent_update__are_compatible(NULL, ref_v1_0));
    assert(!ar_agent_update__are_compatible(ref_v1_0, NULL));
    assert(!ar_agent_update__are_compatible(NULL, NULL));
    
    // Check for memory leaks
    assert(ar_system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
    
    printf("✓ Update compatibility test passed\n");
}

static void test_count_using_method(void) {
    printf("Testing count agents using method...\n");
    
    // Given a system with agents
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("test_count");
    assert(own_fixture != NULL);
    assert(ar_system_fixture__initialize(own_fixture));
    
    // Register methods
    ar_method_t *ref_echo = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_echo != NULL);
    
    ar_method_t *ref_calc = ar_system_fixture__register_method(
        own_fixture, "calc", "send(0, \"result\")", "1.0.0"
    );
    assert(ref_calc != NULL);
    
    // When no agents exist
    assert(ar_agency__count_agents_using_method(ref_echo) == 0);
    assert(ar_agency__count_agents_using_method(ref_calc) == 0);
    
    // When creating agents
    ar_agency__create_agent("echo", "1.0.0", NULL);
    ar_agency__create_agent("echo", "1.0.0", NULL);
    ar_agency__create_agent("calc", "1.0.0", NULL);
    ar_system__process_all_messages(); // Process wake messages
    
    // Then counts should be correct
    assert(ar_agency__count_agents_using_method(ref_echo) == 2);
    assert(ar_agency__count_agents_using_method(ref_calc) == 1);
    
    // When checking with null
    assert(ar_agency__count_agents_using_method(NULL) == 0);
    
    // Check for memory leaks
    assert(ar_system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
    
    printf("✓ Count using method test passed\n");
}

static void test_update_without_lifecycle(void) {
    printf("Testing update without lifecycle events...\n");
    
    // Given a system with agents
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("test_no_lifecycle");
    assert(own_fixture != NULL);
    assert(ar_system_fixture__initialize(own_fixture));
    
    // Register two versions
    ar_method_t *ref_v1_0 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, \"v1.0: \" + message)", "1.0.0"
    );
    assert(ref_v1_0 != NULL);
    
    ar_method_t *ref_v1_1 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, \"v1.1: \" + message)", "1.1.0"
    );
    assert(ref_v1_1 != NULL);
    
    // Register calc method for the third agent
    ar_system_fixture__register_method(
        own_fixture, "calc", "send(0, \"result\")", "1.0.0"
    );
    
    // Create agents with v1.0
    int64_t agent1 = ar_agency__create_agent("echo", "1.0.0", NULL);
    int64_t agent2 = ar_agency__create_agent("echo", "1.0.0", NULL);
    ar_agency__create_agent("calc", "1.0.0", NULL); // Different method - won't be updated
    ar_system__process_all_messages(); // Process wake messages
    
    // Verify initial state
    assert(ar_agency__get_agent_method(agent1) == ref_v1_0);
    assert(ar_agency__get_agent_method(agent2) == ref_v1_0);
    
    // When updating without lifecycle events
    int count = ar_agency__update_agent_methods(ref_v1_0, ref_v1_1, false);
    
    // Then correct number should be updated
    assert(count == 2);
    
    // Then methods should be updated
    assert(ar_agency__get_agent_method(agent1) == ref_v1_1);
    assert(ar_agency__get_agent_method(agent2) == ref_v1_1);
    
    // Then no messages should be queued
    assert(!ar_agency__agent_has_messages(agent1));
    assert(!ar_agency__agent_has_messages(agent2));
    
    // Check for memory leaks
    assert(ar_system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
    
    printf("✓ Update without lifecycle test passed\n");
}

static void test_update_with_lifecycle(void) {
    printf("Testing update with lifecycle events...\n");
    
    // Given a system with agents
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("test_lifecycle");
    assert(own_fixture != NULL);
    assert(ar_system_fixture__initialize(own_fixture));
    
    // Register two simple versions
    ar_method_t *ref_v1_0 = ar_system_fixture__register_method(
        own_fixture, "echo", 
        "send(sender, \"v1.0: \" + message)", 
        "1.0.0"
    );
    assert(ref_v1_0 != NULL);
    
    ar_method_t *ref_v1_1 = ar_system_fixture__register_method(
        own_fixture, "echo", 
        "send(sender, \"v1.1: \" + message)", 
        "1.1.0"
    );
    assert(ref_v1_1 != NULL);
    
    // Create agents
    int64_t agent1 = ar_agency__create_agent("echo", "1.0.0", NULL);
    int64_t agent2 = ar_agency__create_agent("echo", "1.0.0", NULL);
    ar_system__process_all_messages(); // Process initial wake messages
    
    // Verify initial state
    assert(ar_agency__agent_has_messages(agent1) == false);
    assert(ar_agency__agent_has_messages(agent2) == false);
    
    // When updating with lifecycle events
    int count = ar_agency__update_agent_methods(ref_v1_0, ref_v1_1, true);
    assert(count == 2);
    
    // Then messages should be queued
    assert(ar_agency__agent_has_messages(agent1) == true);
    assert(ar_agency__agent_has_messages(agent2) == true);
    
    // Process sleep messages
    ar_system__process_next_message(); // agent1 sleep
    ar_system__process_next_message(); // agent1 wake
    ar_system__process_next_message(); // agent2 sleep  
    ar_system__process_next_message(); // agent2 wake
    
    // Then no more messages should be queued
    assert(ar_agency__agent_has_messages(agent1) == false);
    assert(ar_agency__agent_has_messages(agent2) == false);
    
    // And methods should be updated
    assert(ar_agency__get_agent_method(agent1) == ref_v1_1);
    assert(ar_agency__get_agent_method(agent2) == ref_v1_1);
    
    // Check for memory leaks
    assert(ar_system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
    
    printf("✓ Update with lifecycle test passed\n");
}

static void test_update_incompatible(void) {
    printf("Testing update with incompatible versions...\n");
    
    // Given a system with agents
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("test_incompat");
    assert(own_fixture != NULL);
    assert(ar_system_fixture__initialize(own_fixture));
    
    // Register incompatible versions
    ar_method_t *ref_v1_0 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_v1_0 != NULL);
    
    ar_method_t *ref_v2_0 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "2.0.0"
    );
    assert(ref_v2_0 != NULL);
    
    ar_method_t *ref_other = ar_system_fixture__register_method(
        own_fixture, "calc", "send(0, \"result\")", "1.0.0"
    );
    assert(ref_other != NULL);
    
    // Create agents
    ar_agency__create_agent("echo", "1.0.0", NULL);
    ar_agency__create_agent("echo", "1.0.0", NULL);
    ar_system__process_all_messages();
    
    // When attempting incompatible updates
    // Then major version change should fail
    assert(ar_agency__update_agent_methods(ref_v1_0, ref_v2_0, false) == 0);
    
    // Then different method should fail
    assert(ar_agency__update_agent_methods(ref_v1_0, ref_other, false) == 0);
    
    // Then null methods should fail
    assert(ar_agency__update_agent_methods(NULL, ref_v1_0, false) == 0);
    assert(ar_agency__update_agent_methods(ref_v1_0, NULL, false) == 0);
    
    // Check for memory leaks
    assert(ar_system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
    
    printf("✓ Update incompatible test passed\n");
}

static void test_update_no_agents(void) {
    printf("Testing update with no matching agents...\n");
    
    // Given a system with no agents
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("test_no_agents");
    assert(own_fixture != NULL);
    assert(ar_system_fixture__initialize(own_fixture));
    
    // Register methods
    ar_method_t *ref_v1_0 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_v1_0 != NULL);
    
    ar_method_t *ref_v1_1 = ar_system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.1.0"
    );
    assert(ref_v1_1 != NULL);
    
    // When updating with no agents
    int count = ar_agency__update_agent_methods(ref_v1_0, ref_v1_1, true);
    
    // Then no agents should be updated
    assert(count == 0);
    
    // Check for memory leaks
    assert(ar_system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
    
    printf("✓ Update no agents test passed\n");
}

int main(void) {
    printf("Running agent update tests...\n\n");
    
    test_update_compatibility();
    test_count_using_method();
    test_update_without_lifecycle();
    test_update_with_lifecycle();
    test_update_incompatible();
    test_update_no_agents();
    
    printf("All 6 tests passed!\n");
    return 0;
}
