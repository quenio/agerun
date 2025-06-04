/**
 * @file agerun_agent_store_tests.c
 * @brief Tests for the agent store module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "agerun_agent_store.h"
#include "agerun_system_fixture.h"
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_data.h"
#include "agerun_method.h"
#include "agerun_heap.h"

static void test_store_basics(void) {
    printf("Testing store basic operations...\n");
    
    // Given a clean environment
    ar_agent_store_delete();
    assert(!ar_agent_store_exists());
    
    // When checking the path
    const char *path = ar_agent_store_get_path();
    assert(path != NULL);
    assert(strcmp(path, "agency.agerun") == 0);
    
    // When saving with no agents
    assert(ar_agent_store_save());
    
    // Then the file should exist
    assert(ar_agent_store_exists());
    
    // When deleting
    assert(ar_agent_store_delete());
    
    // Then the file should not exist
    assert(!ar_agent_store_exists());
    
    printf("✓ Store basic operations test passed\n");
}

static void test_store_empty_save_load(void) {
    printf("Testing empty store save/load...\n");
    
    // Given a system with no agents
    system_fixture_t *own_fixture = ar_system_fixture_create("test_empty");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    // Clean up any existing store
    ar_agent_store_delete();
    
    // When saving empty state
    assert(ar_agent_store_save());
    assert(ar_agent_store_exists());
    
    // When loading
    assert(ar_agent_store_load());
    
    // Then no agents should exist
    assert(ar_agent_count_active() == 0);
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_agent_store_delete();
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Empty store save/load test passed\n");
}

static void test_store_single_agent(void) {
    printf("Testing single agent persistence...\n");
    
    // Given a system with one agent
    system_fixture_t *own_fixture = ar_system_fixture_create("test_single");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    // Clean up any existing store
    ar_agent_store_delete();
    
    // Register a test method
    method_t *ref_method = ar_system_fixture_register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_method != NULL);
    
    // Create an agent
    int64_t agent_id = ar_agent_create("echo", "1.0.0", NULL);
    assert(agent_id > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // Add some data to agent memory
    data_t *mut_memory = ar_agent_get_mutable_memory(agent_id);
    assert(mut_memory != NULL);
    ar_data_set_map_string(mut_memory, "name", "Test Agent");
    ar_data_set_map_integer(mut_memory, "count", 42);
    ar_data_set_map_double(mut_memory, "value", 3.14);
    
    // When saving
    assert(ar_agent_store_save());
    
    // Destroy all agents
    ar_agent_destroy(agent_id);
    ar_system_process_next_message(); // Process sleep message
    assert(ar_agent_count_active() == 0);
    
    // When loading
    assert(ar_agent_store_load());
    
    // Then the agent should be restored
    assert(ar_agent_count_active() == 1);
    
    // Find the restored agent
    int64_t restored_id = ar_agent_get_first_active();
    assert(restored_id > 0);
    
    // Verify the method
    const method_t *ref_restored_method = ar_agent_get_method(restored_id);
    assert(ref_restored_method != NULL);
    assert(strcmp(ar_method_get_name(ref_restored_method), "echo") == 0);
    assert(strcmp(ar_method_get_version(ref_restored_method), "1.0.0") == 0);
    
    // Memory is not persisted in current implementation (no map iteration support)
    // This is documented as a limitation in the module documentation
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_agent_store_delete();
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Single agent persistence test passed\n");
}

static void test_store_multiple_agents(void) {
    printf("Testing multiple agent persistence...\n");
    
    // Given a system with multiple agents
    system_fixture_t *own_fixture = ar_system_fixture_create("test_multiple");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    // Clean up any existing store
    ar_agent_store_delete();
    
    // Register methods
    method_t *ref_echo = ar_system_fixture_register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_echo != NULL);
    
    method_t *ref_calc = ar_system_fixture_register_method(
        own_fixture, "calc", "send(sender, \"result: \" + (2 + 2))", "2.0.0"
    );
    assert(ref_calc != NULL);
    
    // Create agents
    int64_t echo1 = ar_agent_create("echo", "1.0.0", NULL);
    assert(echo1 > 0);
    
    int64_t echo2 = ar_agent_create("echo", "1.0.0", NULL);
    assert(echo2 > 0);
    
    int64_t calc1 = ar_agent_create("calc", "2.0.0", NULL);
    assert(calc1 > 0);
    
    // Process wake messages
    ar_system_process_all_messages();
    
    // Add unique data to each agent
    data_t *mut_memory1 = ar_agent_get_mutable_memory(echo1);
    ar_data_set_map_string(mut_memory1, "name", "Echo One");
    ar_data_set_map_integer(mut_memory1, "id", 1);
    
    data_t *mut_memory2 = ar_agent_get_mutable_memory(echo2);
    ar_data_set_map_string(mut_memory2, "name", "Echo Two");
    ar_data_set_map_integer(mut_memory2, "id", 2);
    
    data_t *mut_memory3 = ar_agent_get_mutable_memory(calc1);
    ar_data_set_map_string(mut_memory3, "name", "Calculator");
    ar_data_set_map_double(mut_memory3, "pi", 3.14159);
    
    // When saving
    assert(ar_agent_store_save());
    
    // Destroy all agents
    ar_agent_destroy(echo1);
    ar_agent_destroy(echo2);
    ar_agent_destroy(calc1);
    ar_system_process_all_messages(); // Process sleep messages
    assert(ar_agent_count_active() == 0);
    
    // When loading
    assert(ar_agent_store_load());
    
    // Then all agents should be restored
    assert(ar_agent_count_active() == 3);
    
    // Memory is not persisted in current implementation
    // Just verify agents were restored with correct methods
    int echo_count = 0, calc_count = 0;
    
    int64_t agent_id = ar_agent_get_first_active();
    while (agent_id != 0) {
        const method_t *ref_method = ar_agent_get_method(agent_id);
        const char *method_name = ar_method_get_name(ref_method);
        
        if (strcmp(method_name, "echo") == 0) {
            echo_count++;
        } else if (strcmp(method_name, "calc") == 0) {
            calc_count++;
        }
        
        agent_id = ar_agent_get_next_active(agent_id);
    }
    
    assert(echo_count == 2);
    assert(calc_count == 1);
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_agent_store_delete();
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Multiple agent persistence test passed\n");
}

static void test_store_file_corruption(void) {
    printf("Testing store file corruption handling...\n");
    
    // Given a system
    system_fixture_t *own_fixture = ar_system_fixture_create("test_corrupt");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    // Clean up any existing store
    ar_agent_store_delete();
    
    // Create a corrupted file
    FILE *fp = fopen(ar_agent_store_get_path(), "w");
    assert(fp != NULL);
    fprintf(fp, "invalid data\n");
    fprintf(fp, "more garbage\n");
    fclose(fp);
    
    // When loading from corrupted file
    assert(ar_agent_store_load()); // Should succeed but with empty state
    
    // Then no agents should exist
    assert(ar_agent_count_active() == 0);
    
    // And the corrupted file should be gone
    assert(!ar_agent_store_exists());
    
    // But a backup should exist
    char backup_path[256];
    snprintf(backup_path, sizeof(backup_path), "%s.bak", ar_agent_store_get_path());
    assert(access(backup_path, F_OK) == 0);
    
    // Clean up backup
    remove(backup_path);
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Store file corruption handling test passed\n");
}

static void test_store_missing_method(void) {
    printf("Testing store with missing method...\n");
    
    // Given a system with an agent
    system_fixture_t *own_fixture = ar_system_fixture_create("test_missing");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    // Clean up any existing store
    ar_agent_store_delete();
    
    // Register a method and create an agent
    method_t *ref_method = ar_system_fixture_register_method(
        own_fixture, "test", "send(0, \"ok\")", "1.0.0"
    );
    assert(ref_method != NULL);
    
    int64_t agent_id = ar_agent_create("test", "1.0.0", NULL);
    assert(agent_id > 0);
    ar_system_process_next_message();
    
    // Save the agent
    assert(ar_agent_store_save());
    
    // Reset the system (loses method registration)
    ar_system_fixture_reset_system(own_fixture);
    
    // When loading without the method registered
    assert(ar_agent_store_load());
    
    // Then no agents should be created (method doesn't exist)
    assert(ar_agent_count_active() == 0);
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_agent_store_delete();
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Store with missing method test passed\n");
}

static void test_store_id_preservation(void) {
    printf("Testing agent ID preservation...\n");
    
    // Given a system
    system_fixture_t *own_fixture = ar_system_fixture_create("test_ids");
    assert(own_fixture != NULL);
    assert(ar_system_fixture_initialize(own_fixture));
    
    // Clean up any existing store
    ar_agent_store_delete();
    
    // Register method
    method_t *ref_method = ar_system_fixture_register_method(
        own_fixture, "test", "send(0, \"ok\")", "1.0.0"
    );
    assert(ref_method != NULL);
    
    // Create agents and remember their IDs
    int64_t id1 = ar_agent_create("test", "1.0.0", NULL);
    int64_t id2 = ar_agent_create("test", "1.0.0", NULL);
    int64_t id3 = ar_agent_create("test", "1.0.0", NULL);
    ar_system_process_all_messages();
    
    // Save
    assert(ar_agent_store_save());
    
    // Destroy all agents
    ar_agent_destroy(id1);
    ar_agent_destroy(id2);
    ar_agent_destroy(id3);
    ar_system_process_all_messages();
    
    // Load
    assert(ar_agent_store_load());
    
    // Then the same IDs should be preserved
    assert(ar_agent_exists(id1));
    assert(ar_agent_exists(id2));
    assert(ar_agent_exists(id3));
    
    // And next ID should be set correctly to avoid collisions
    int64_t new_id = ar_agent_create("test", "1.0.0", NULL);
    assert(new_id > id1 && new_id > id2 && new_id > id3);
    
    // Check for memory leaks
    assert(ar_system_fixture_check_memory(own_fixture));
    
    // Clean up
    ar_agent_store_delete();
    ar_system_fixture_destroy(own_fixture);
    
    printf("✓ Agent ID preservation test passed\n");
}

int main(void) {
    printf("Running agent store tests...\n\n");
    
    test_store_basics();
    test_store_empty_save_load();
    test_store_single_agent();
    test_store_multiple_agents();
    test_store_file_corruption();
    test_store_missing_method();
    test_store_id_preservation();
    
    printf("\nAll agent store tests passed!\n");
    return 0;
}
