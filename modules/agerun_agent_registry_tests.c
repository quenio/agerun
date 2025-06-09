/**
 * @file agerun_agent_registry_tests.c
 * @brief Tests for the agent registry module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_agent_registry.h"
#include "agerun_heap.h"

static void test_registry_create_destroy(void) {
    printf("Testing registry create and destroy...\n");
    
    // Given a new registry
    agent_registry_t *own_registry = ar__agent_registry__create();
    assert(own_registry != NULL);
    
    // When checking initial state
    assert(ar__agent_registry__count(own_registry) == 0);
    assert(ar__agent_registry__get_first(own_registry) == 0);
    assert(ar__agent_registry__get_next_id(own_registry) == 1);
    
    // When destroying the registry
    ar__agent_registry__destroy(own_registry);
    own_registry = NULL;
    
    // Then no crash should occur
    ar__agent_registry__destroy(NULL); // Should handle NULL gracefully
    
    printf("✓ Registry create/destroy test passed\n");
}

static void test_registry_id_management(void) {
    printf("Testing registry ID management...\n");
    
    // Given a new registry
    agent_registry_t *own_registry = ar__agent_registry__create();
    assert(own_registry != NULL);
    
    // When allocating IDs
    int64_t id1 = ar__agent_registry__allocate_id(own_registry);
    assert(id1 == 1);
    
    int64_t id2 = ar__agent_registry__allocate_id(own_registry);
    assert(id2 == 2);
    
    int64_t id3 = ar__agent_registry__allocate_id(own_registry);
    assert(id3 == 3);
    
    // Then next ID should be updated
    assert(ar__agent_registry__get_next_id(own_registry) == 4);
    
    // When setting next ID
    ar__agent_registry__set_next_id(own_registry, 100);
    assert(ar__agent_registry__get_next_id(own_registry) == 100);
    
    int64_t id4 = ar__agent_registry__allocate_id(own_registry);
    assert(id4 == 100);
    assert(ar__agent_registry__get_next_id(own_registry) == 101);
    
    ar__agent_registry__destroy(own_registry);
    
    printf("✓ Registry ID management test passed\n");
}

static void test_registry_registration(void) {
    printf("Testing registry registration...\n");
    
    // Given a new registry
    agent_registry_t *own_registry = ar__agent_registry__create();
    assert(own_registry != NULL);
    
    // When registering agent IDs
    assert(ar__agent_registry__register_id(own_registry, 10));
    assert(ar__agent_registry__register_id(own_registry, 20));
    assert(ar__agent_registry__register_id(own_registry, 30));
    
    // Then registry should track them
    assert(ar__agent_registry__count(own_registry) == 3);
    assert(ar__agent_registry__is_registered(own_registry, 10));
    assert(ar__agent_registry__is_registered(own_registry, 20));
    assert(ar__agent_registry__is_registered(own_registry, 30));
    assert(!ar__agent_registry__is_registered(own_registry, 40));
    
    // When trying to register duplicate
    assert(!ar__agent_registry__register_id(own_registry, 20));
    assert(ar__agent_registry__count(own_registry) == 3);
    
    // When unregistering
    assert(ar__agent_registry__unregister_id(own_registry, 20));
    assert(ar__agent_registry__count(own_registry) == 2);
    assert(!ar__agent_registry__is_registered(own_registry, 20));
    
    // When trying to unregister non-existent
    assert(!ar__agent_registry__unregister_id(own_registry, 20));
    assert(!ar__agent_registry__unregister_id(own_registry, 999));
    
    ar__agent_registry__destroy(own_registry);
    
    printf("✓ Registry registration test passed\n");
}

static void test_registry_iteration(void) {
    printf("Testing registry iteration...\n");
    
    // Given a new registry with some agents
    agent_registry_t *own_registry = ar__agent_registry__create();
    assert(own_registry != NULL);
    
    assert(ar__agent_registry__register_id(own_registry, 5));
    assert(ar__agent_registry__register_id(own_registry, 3));
    assert(ar__agent_registry__register_id(own_registry, 8));
    assert(ar__agent_registry__register_id(own_registry, 1));
    
    // When iterating through agents (insertion order)
    int64_t first = ar__agent_registry__get_first(own_registry);
    assert(first == 5); // First inserted
    
    int64_t next = ar__agent_registry__get_next(own_registry, first);
    assert(next == 3);
    
    next = ar__agent_registry__get_next(own_registry, next);
    assert(next == 8);
    
    next = ar__agent_registry__get_next(own_registry, next);
    assert(next == 1);
    
    next = ar__agent_registry__get_next(own_registry, next);
    assert(next == 0); // End of iteration
    
    ar__agent_registry__destroy(own_registry);
    
    printf("✓ Registry iteration test passed\n");
}

static void test_registry_agent_tracking(void) {
    printf("Testing registry agent tracking...\n");
    
    // Given a new registry
    agent_registry_t *own_registry = ar__agent_registry__create();
    assert(own_registry != NULL);
    
    // Create some dummy agent pointers (not real agents)
    int dummy_agent1 = 111;
    int dummy_agent2 = 222;
    int dummy_agent3 = 333;
    
    // When tracking agents
    assert(ar__agent_registry__register_id(own_registry, 10));
    assert(ar__agent_registry__track_agent(own_registry, 10, &dummy_agent1));
    
    assert(ar__agent_registry__register_id(own_registry, 20));
    assert(ar__agent_registry__track_agent(own_registry, 20, &dummy_agent2));
    
    assert(ar__agent_registry__register_id(own_registry, 30));
    assert(ar__agent_registry__track_agent(own_registry, 30, &dummy_agent3));
    
    // Then we should be able to find them
    assert(ar__agent_registry__find_agent(own_registry, 10) == &dummy_agent1);
    assert(ar__agent_registry__find_agent(own_registry, 20) == &dummy_agent2);
    assert(ar__agent_registry__find_agent(own_registry, 30) == &dummy_agent3);
    assert(ar__agent_registry__find_agent(own_registry, 40) == NULL);
    
    // When untracking an agent
    void *untracked = ar__agent_registry__untrack_agent(own_registry, 20);
    assert(untracked == &dummy_agent2);
    assert(ar__agent_registry__find_agent(own_registry, 20) == NULL);
    
    // But the ID is still registered
    assert(ar__agent_registry__is_registered(own_registry, 20));
    
    ar__agent_registry__destroy(own_registry);
    
    printf("✓ Registry agent tracking test passed\n");
}

static void test_registry_clear(void) {
    printf("Testing registry clear...\n");
    
    // Given a registry with agents
    agent_registry_t *own_registry = ar__agent_registry__create();
    assert(own_registry != NULL);
    
    // Register and track some agents
    int dummy1 = 1, dummy2 = 2, dummy3 = 3;
    assert(ar__agent_registry__register_id(own_registry, 10));
    assert(ar__agent_registry__track_agent(own_registry, 10, &dummy1));
    assert(ar__agent_registry__register_id(own_registry, 20));
    assert(ar__agent_registry__track_agent(own_registry, 20, &dummy2));
    assert(ar__agent_registry__register_id(own_registry, 30));
    assert(ar__agent_registry__track_agent(own_registry, 30, &dummy3));
    
    ar__agent_registry__set_next_id(own_registry, 100);
    
    // When clearing the registry
    ar__agent_registry__clear(own_registry);
    
    // Then everything should be reset
    assert(ar__agent_registry__count(own_registry) == 0);
    assert(ar__agent_registry__get_first(own_registry) == 0);
    assert(ar__agent_registry__get_next_id(own_registry) == 1);
    assert(!ar__agent_registry__is_registered(own_registry, 10));
    assert(ar__agent_registry__find_agent(own_registry, 10) == NULL);
    
    ar__agent_registry__destroy(own_registry);
    
    printf("✓ Registry clear test passed\n");
}

static void test_registry_edge_cases(void) {
    printf("Testing registry edge cases...\n");
    
    // Test NULL registry operations
    assert(ar__agent_registry__count(NULL) == 0);
    assert(ar__agent_registry__get_first(NULL) == 0);
    assert(ar__agent_registry__get_next(NULL, 1) == 0);
    assert(ar__agent_registry__get_next_id(NULL) == 0);
    assert(ar__agent_registry__allocate_id(NULL) == 0);
    assert(!ar__agent_registry__register_id(NULL, 1));
    assert(!ar__agent_registry__unregister_id(NULL, 1));
    assert(!ar__agent_registry__is_registered(NULL, 1));
    assert(!ar__agent_registry__track_agent(NULL, 1, (void*)1));
    assert(ar__agent_registry__untrack_agent(NULL, 1) == NULL);
    assert(ar__agent_registry__find_agent(NULL, 1) == NULL);
    ar__agent_registry__clear(NULL); // Should not crash
    ar__agent_registry__set_next_id(NULL, 100); // Should not crash
    
    printf("✓ Registry edge cases test passed\n");
}

int main(void) {
    printf("Running agent registry tests...\n\n");
    
    test_registry_create_destroy();
    test_registry_id_management();
    test_registry_registration();
    test_registry_iteration();
    test_registry_agent_tracking();
    test_registry_clear();
    test_registry_edge_cases();
    
    // Check for memory leaks
    ar__heap__memory_report();
    
    printf("\nAll agent registry tests passed!\n");
    return 0;
}
