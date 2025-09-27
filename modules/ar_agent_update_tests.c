/**
 * @file ar_agent_update_tests.c
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
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // When no agents exist
    assert(ar_agency__count_agents_using_method(mut_agency, ref_echo) == 0);
    assert(ar_agency__count_agents_using_method(mut_agency, ref_calc) == 0);
    
    // When creating agents
    ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    ar_agency__create_agent(mut_agency, "calc", "1.0.0", NULL);
    ar_system_fixture__process_all_messages(own_fixture); // Process any messages
    
    // Then counts should be correct
    assert(ar_agency__count_agents_using_method(mut_agency, ref_echo) == 2);
    assert(ar_agency__count_agents_using_method(mut_agency, ref_calc) == 1);
    
    // When checking with null
    assert(ar_agency__count_agents_using_method(mut_agency, NULL) == 0);
    
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
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Create agents with v1.0
    int64_t agent1 = ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    int64_t agent2 = ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    ar_agency__create_agent(mut_agency, "calc", "1.0.0", NULL); // Different method - won't be updated
    ar_system_fixture__process_all_messages(own_fixture); // Process any messages
    
    // Verify initial state
    assert(ar_agency__get_agent_method(mut_agency, agent1) == ref_v1_0);
    assert(ar_agency__get_agent_method(mut_agency, agent2) == ref_v1_0);
    
    // When updating
    int count = ar_agency__update_agent_methods(mut_agency, ref_v1_0, ref_v1_1);
    
    // Then correct number should be updated
    assert(count == 2);
    
    // Then methods should be updated
    assert(ar_agency__get_agent_method(mut_agency, agent1) == ref_v1_1);
    assert(ar_agency__get_agent_method(mut_agency, agent2) == ref_v1_1);
    
    // Then no messages should be queued
    assert(!ar_agency__agent_has_messages(mut_agency, agent1));
    assert(!ar_agency__agent_has_messages(mut_agency, agent2));
    
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
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Create agents using instance APIs
    int64_t agent1 = ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    int64_t agent2 = ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    ar_system_fixture__process_all_messages(own_fixture); // Process any messages
    
    // Verify initial state using instance APIs
    assert(ar_agency__agent_has_messages(mut_agency, agent1) == false);
    assert(ar_agency__agent_has_messages(mut_agency, agent2) == false);
    
    // When updating using instance API
    // (Note: lifecycle events are no longer sent)
    int count = ar_agency__update_agent_methods(mut_agency, ref_v1_0, ref_v1_1);
    assert(count == 2);
    
    // Then no messages should be queued
    assert(ar_agency__agent_has_messages(mut_agency, agent1) == false);
    assert(ar_agency__agent_has_messages(mut_agency, agent2) == false);
    
    // Then no messages to process
    assert(ar_agency__agent_has_messages(mut_agency, agent1) == false);
    assert(ar_agency__agent_has_messages(mut_agency, agent2) == false);
    
    // And methods should be updated using instance API
    assert(ar_agency__get_agent_method(mut_agency, agent1) == ref_v1_1);
    assert(ar_agency__get_agent_method(mut_agency, agent2) == ref_v1_1);
    
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
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // Create agents using instance APIs
    ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    ar_agency__create_agent(mut_agency, "echo", "1.0.0", NULL);
    ar_system_fixture__process_all_messages(own_fixture);
    
    // When attempting incompatible updates using instance API
    // Then major version change should fail
    assert(ar_agency__update_agent_methods(mut_agency, ref_v1_0, ref_v2_0) == 0);
    
    // Then different method should fail
    assert(ar_agency__update_agent_methods(mut_agency, ref_v1_0, ref_other) == 0);
    
    // Then null methods should fail
    assert(ar_agency__update_agent_methods(mut_agency, NULL, ref_v1_0) == 0);
    assert(ar_agency__update_agent_methods(mut_agency, ref_v1_0, NULL) == 0);
    
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
    
    // Get the fixture's agency
    ar_agency_t *mut_agency = ar_system_fixture__get_agency(own_fixture);
    assert(mut_agency != NULL);
    
    // When updating with no agents using instance API
    int count = ar_agency__update_agent_methods(mut_agency, ref_v1_0, ref_v1_1);
    
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
