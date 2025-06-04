/**
 * @file agerun_agent_registry_tests.c
 * @brief Tests for the agent registry module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "agerun_agent_registry.h"
#include "agerun_system_fixture.h"
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_data.h"
#include "agerun_method.h"
#include "agerun_heap.h"

static void test_registry_initialization(void) {
    printf("Testing registry initialization...\n");
    
    // Given an uninitialized registry
    assert(!ar_agent_registry_is_initialized());
    
    // When initializing the registry
    assert(ar_agent_registry_initialize());
    
    // Then the registry should be initialized
    assert(ar_agent_registry_is_initialized());
    
    // When initializing again
    assert(ar_agent_registry_initialize());
    
    // Then it should still succeed (idempotent)
    assert(ar_agent_registry_is_initialized());
    
    // When shutting down
    ar_agent_registry_shutdown();
    
    // Then the registry should not be initialized
    assert(!ar_agent_registry_is_initialized());
    
    printf("✓ Registry initialization test passed\n");
}

static void test_registry_operations_uninitialized(void) {
    printf("Testing registry operations when uninitialized...\n");
    
    // Given an uninitialized registry
    assert(!ar_agent_registry_is_initialized());
    
    // When calling registry operations
    // Then they should return safe defaults
    assert(ar_agent_registry_count() == 0);
    assert(ar_agent_registry_get_first() == 0);
    assert(ar_agent_registry_get_next(1) == 0);
    
    // When resetting
    ar_agent_registry_reset_all(); // Should not crash
    
    printf("✓ Uninitialized operations test passed\n");
}

static void test_registry_with_agents(void) {
    printf("Testing registry with agents...\n");
    
    // Given an initialized system and registry
    system_fixture_t *own_fixture = ar_system_fixture_create("test_registry");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    ar_agent_registry_initialize();
    
    // Register a test method
    method_t *ref_method = ar_system_fixture_register_method(
        own_fixture, "test", "send(0, \"ok\")", "1.0.0"
    );
    assert(ref_method != NULL);
    
    // When no agents exist
    assert(ar_agent_registry_count() == 0);
    assert(ar_agent_registry_get_first() == 0);
    
    // When creating agents
    int64_t agent1 = ar_agent_create("test", "1.0.0", NULL);
    assert(agent1 > 0);
    
    int64_t agent2 = ar_agent_create("test", "1.0.0", NULL);
    assert(agent2 > 0);
    
    // Then the registry should track them
    assert(ar_agent_registry_count() == 2);
    
    // When iterating through agents
    int64_t first = ar_agent_registry_get_first();
    assert(first == agent1);
    
    int64_t next = ar_agent_registry_get_next(first);
    assert(next == agent2);
    
    int64_t end = ar_agent_registry_get_next(next);
    assert(end == 0);
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_agent_registry_shutdown();
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Registry with agents test passed\n");
}


static void test_registry_reset(void) {
    printf("Testing registry reset...\n");
    
    // Given an initialized system and registry with agents
    system_fixture_t *own_fixture = ar_system_fixture_create("test_reset");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    ar_agent_registry_initialize();
    
    method_t *ref_method = ar_system_fixture_register_method(
        own_fixture, "test", "send(0, \"ok\")", "1.0.0"
    );
    assert(ref_method != NULL);
    
    ar_agent_create("test", "1.0.0", NULL);
    ar_agent_create("test", "1.0.0", NULL);
    assert(ar_agent_registry_count() == 2);
    
    // When resetting the registry
    ar_agent_registry_reset_all();
    
    // Then all agents should be gone
    assert(ar_agent_registry_count() == 0);
    assert(ar_agent_registry_get_first() == 0);
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_agent_registry_shutdown();
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Registry reset test passed\n");
}

int main(void) {
    printf("Running agent registry tests...\n\n");
    
    test_registry_initialization();
    test_registry_operations_uninitialized();
    test_registry_with_agents();
    test_registry_reset();
    
    printf("\nAll agent registry tests passed!\n");
    return 0;
}
