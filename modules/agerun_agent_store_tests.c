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
#include "agerun_agency.h"
#include "agerun_data.h"
#include "agerun_method.h"
#include "agerun_heap.h"

static void test_store_basics(void) {
    printf("Testing store basic operations...\n");
    
    // Given a system
    system_fixture_t *own_fixture = ar__system_fixture__create("test_basics");
    assert(own_fixture != NULL);
    assert(ar__system_fixture__initialize(own_fixture));
    
    // Given a clean environment
    ar__agent_store__delete();
    assert(!ar__agent_store__exists());
    
    // When checking the path
    const char *path = ar__agent_store__get_path();
    assert(path != NULL);
    assert(strcmp(path, "agency.agerun") == 0);
    
    // When saving with no agents (except the initial agent)
    assert(ar__agent_store__save());
    
    // Then the file should exist
    assert(ar__agent_store__exists());
    
    // When deleting
    assert(ar__agent_store__delete());
    
    // Then the file should not exist
    assert(!ar__agent_store__exists());
    
    // Check for memory leaks
    assert(ar__system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar__system_fixture__destroy(own_fixture);
    
    printf("✓ Store basic operations test passed\n");
}

static void test_store_empty_save_load(void) {
    printf("Testing empty store save/load...\n");
    
    // Given a system with no agents
    system_fixture_t *own_fixture = ar__system_fixture__create("test_empty");
    assert(own_fixture != NULL);
    assert(ar__system_fixture__initialize(own_fixture));
    
    // Clean up any existing store
    ar__agent_store__delete();
    
    // When saving empty state
    assert(ar__agent_store__save());
    assert(ar__agent_store__exists());
    
    // When loading
    assert(ar__agent_store__load());
    
    // Then no agents should exist
    assert(ar__agency__count_active_agents() == 0);
    
    // Check for memory leaks
    assert(ar__system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar__agent_store__delete();
    ar__system_fixture__destroy(own_fixture);
    
    printf("✓ Empty store save/load test passed\n");
}

static void test_store_single_agent(void) {
    printf("Testing single agent persistence...\n");
    
    // Given a system with one agent
    system_fixture_t *own_fixture = ar__system_fixture__create("test_single");
    assert(own_fixture != NULL);
    assert(ar__system_fixture__initialize(own_fixture));
    
    // Clean up any existing store
    ar__agent_store__delete();
    
    // Register a test method
    method_t *ref_method = ar__system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_method != NULL);
    
    // Create an agent
    int64_t agent_id = ar__agency__create_agent("echo", "1.0.0", NULL);
    assert(agent_id > 0);
    
    // Process wake message
    ar__system__process_next_message();
    
    // Add some data to agent memory
    data_t *mut_memory = ar__agency__get_agent_mutable_memory(agent_id);
    assert(mut_memory != NULL);
    ar__data__set_map_string(mut_memory, "name", "Test Agent");
    ar__data__set_map_integer(mut_memory, "count", 42);
    ar__data__set_map_double(mut_memory, "value", 3.14);
    
    // When saving
    assert(ar__agent_store__save());
    
    // Destroy all agents
    ar__agency__destroy_agent(agent_id);
    ar__system__process_next_message(); // Process sleep message
    assert(ar__agency__count_active_agents() == 0);
    
    // When loading
    assert(ar__agent_store__load());
    
    // Then the agent should be restored
    assert(ar__agency__count_active_agents() == 1);
    
    // Find the restored agent
    int64_t restored_id = ar__agency__get_first_agent();
    assert(restored_id > 0);
    
    // Verify the method
    const method_t *ref_restored_method = ar__agency__get_agent_method(restored_id);
    assert(ref_restored_method != NULL);
    assert(strcmp(ar__method__get_name(ref_restored_method), "echo") == 0);
    assert(strcmp(ar__method__get_version(ref_restored_method), "1.0.0") == 0);
    
    // Verify the memory was persisted
    data_t *ref_restored_memory = ar__agency__get_agent_mutable_memory(restored_id);
    assert(ref_restored_memory != NULL);
    assert(ar__data__get_type(ref_restored_memory) == DATA_MAP);
    
    // Check all persisted values
    const char *name = ar__data__get_map_string(ref_restored_memory, "name");
    assert(name != NULL);
    assert(strcmp(name, "Test Agent") == 0);
    
    int count = ar__data__get_map_integer(ref_restored_memory, "count");
    assert(count == 42);
    
    double value = ar__data__get_map_double(ref_restored_memory, "value");
    assert(value == 3.14);
    
    // Check for memory leaks
    assert(ar__system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar__agent_store__delete();
    ar__system_fixture__destroy(own_fixture);
    
    printf("✓ Single agent persistence test passed\n");
}

static void test_store_multiple_agents(void) {
    printf("Testing multiple agent persistence...\n");
    
    // Given a system with multiple agents
    system_fixture_t *own_fixture = ar__system_fixture__create("test_multiple");
    assert(own_fixture != NULL);
    assert(ar__system_fixture__initialize(own_fixture));
    
    // Clean up any existing store
    ar__agent_store__delete();
    
    // Register methods
    method_t *ref_echo = ar__system_fixture__register_method(
        own_fixture, "echo", "send(sender, message)", "1.0.0"
    );
    assert(ref_echo != NULL);
    
    method_t *ref_calc = ar__system_fixture__register_method(
        own_fixture, "calc", "send(sender, \"result: \" + (2 + 2))", "2.0.0"
    );
    assert(ref_calc != NULL);
    
    // Create agents
    int64_t echo1 = ar__agency__create_agent("echo", "1.0.0", NULL);
    assert(echo1 > 0);
    
    int64_t echo2 = ar__agency__create_agent("echo", "1.0.0", NULL);
    assert(echo2 > 0);
    
    int64_t calc1 = ar__agency__create_agent("calc", "2.0.0", NULL);
    assert(calc1 > 0);
    
    // Process wake messages
    ar__system__process_all_messages();
    
    // Add unique data to each agent
    data_t *mut_memory1 = ar__agency__get_agent_mutable_memory(echo1);
    ar__data__set_map_string(mut_memory1, "name", "Echo One");
    ar__data__set_map_integer(mut_memory1, "id", 1);
    
    data_t *mut_memory2 = ar__agency__get_agent_mutable_memory(echo2);
    ar__data__set_map_string(mut_memory2, "name", "Echo Two");
    ar__data__set_map_integer(mut_memory2, "id", 2);
    
    data_t *mut_memory3 = ar__agency__get_agent_mutable_memory(calc1);
    ar__data__set_map_string(mut_memory3, "name", "Calculator");
    ar__data__set_map_double(mut_memory3, "pi", 3.14159);
    
    // When saving
    assert(ar__agent_store__save());
    
    // Destroy all agents
    ar__agency__destroy_agent(echo1);
    ar__agency__destroy_agent(echo2);
    ar__agency__destroy_agent(calc1);
    ar__system__process_all_messages(); // Process sleep messages
    assert(ar__agency__count_active_agents() == 0);
    
    // When loading
    assert(ar__agent_store__load());
    
    // Then all agents should be restored
    assert(ar__agency__count_active_agents() == 3);
    
    // Verify agents were restored with correct methods and memory
    int echo_count = 0, calc_count = 0;
    int found_echo_one = 0, found_echo_two = 0, found_calculator = 0;
    
    int64_t agent_id = ar__agency__get_first_agent();
    while (agent_id != 0) {
        const method_t *ref_method = ar__agency__get_agent_method(agent_id);
        const char *method_name = ar__method__get_name(ref_method);
        
        // Get agent memory
        data_t *ref_memory = ar__agency__get_agent_mutable_memory(agent_id);
        assert(ref_memory != NULL);
        
        if (strcmp(method_name, "echo") == 0) {
            echo_count++;
            
            // Check which echo agent this is based on memory
            const char *name = ar__data__get_map_string(ref_memory, "name");
            int id = ar__data__get_map_integer(ref_memory, "id");
            
            if (name && strcmp(name, "Echo One") == 0 && id == 1) {
                found_echo_one = 1;
            } else if (name && strcmp(name, "Echo Two") == 0 && id == 2) {
                found_echo_two = 1;
            }
        } else if (strcmp(method_name, "calc") == 0) {
            calc_count++;
            
            // Check calculator memory
            const char *name = ar__data__get_map_string(ref_memory, "name");
            double pi = ar__data__get_map_double(ref_memory, "pi");
            
            if (name && strcmp(name, "Calculator") == 0 && pi == 3.14159) {
                found_calculator = 1;
            }
        }
        
        agent_id = ar__agency__get_next_agent(agent_id);
    }
    
    assert(echo_count == 2);
    assert(calc_count == 1);
    assert(found_echo_one);
    assert(found_echo_two);
    assert(found_calculator);
    
    // Check for memory leaks
    assert(ar__system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar__agent_store__delete();
    ar__system_fixture__destroy(own_fixture);
    
    printf("✓ Multiple agent persistence test passed\n");
}

static void test_store_file_corruption(void) {
    printf("Testing store file corruption handling...\n");
    
    // Given a system
    system_fixture_t *own_fixture = ar__system_fixture__create("test_corrupt");
    assert(own_fixture != NULL);
    assert(ar__system_fixture__initialize(own_fixture));
    
    // Clean up any existing store
    ar__agent_store__delete();
    
    // Create a corrupted file
    FILE *fp = fopen(ar__agent_store__get_path(), "w");
    assert(fp != NULL);
    fprintf(fp, "invalid data\n");
    fprintf(fp, "more garbage\n");
    fclose(fp);
    
    // When loading from corrupted file
    assert(ar__agent_store__load()); // Should succeed but with empty state
    
    // Then no agents should exist
    assert(ar__agency__count_active_agents() == 0);
    
    // And the corrupted file should be gone
    assert(!ar__agent_store__exists());
    
    // But a backup should exist
    char backup_path[256];
    snprintf(backup_path, sizeof(backup_path), "%s.bak", ar__agent_store__get_path());
    assert(access(backup_path, F_OK) == 0);
    
    // Clean up backup
    remove(backup_path);
    
    // Check for memory leaks
    assert(ar__system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar__system_fixture__destroy(own_fixture);
    
    printf("✓ Store file corruption handling test passed\n");
}

static void test_store_missing_method(void) {
    printf("Testing store with missing method...\n");
    
    // Given a system with an agent
    system_fixture_t *own_fixture = ar__system_fixture__create("test_missing");
    assert(own_fixture != NULL);
    assert(ar__system_fixture__initialize(own_fixture));
    
    // Clean up any existing store
    ar__agent_store__delete();
    
    // Register a method and create an agent
    method_t *ref_method = ar__system_fixture__register_method(
        own_fixture, "test", "send(0, \"ok\")", "1.0.0"
    );
    assert(ref_method != NULL);
    
    int64_t agent_id = ar__agency__create_agent("test", "1.0.0", NULL);
    assert(agent_id > 0);
    ar__system__process_next_message();
    
    // Save the agent
    assert(ar__agent_store__save());
    
    // Reset the system (loses method registration)
    ar__system_fixture__reset_system(own_fixture);
    
    // When loading without the method registered
    assert(ar__agent_store__load());
    
    // Then no agents should be created (method doesn't exist)
    assert(ar__agency__count_active_agents() == 0);
    
    // Check for memory leaks
    assert(ar__system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar__agent_store__delete();
    ar__system_fixture__destroy(own_fixture);
    
    printf("✓ Store with missing method test passed\n");
}

static void test_store_id_preservation(void) {
    printf("Testing agent ID preservation...\n");
    
    // Given a system
    system_fixture_t *own_fixture = ar__system_fixture__create("test_ids");
    assert(own_fixture != NULL);
    assert(ar__system_fixture__initialize(own_fixture));
    
    // Clean up any existing store
    ar__agent_store__delete();
    
    // Register method
    method_t *ref_method = ar__system_fixture__register_method(
        own_fixture, "test", "send(0, \"ok\")", "1.0.0"
    );
    assert(ref_method != NULL);
    
    // Create agents and remember their IDs
    int64_t id1 = ar__agency__create_agent("test", "1.0.0", NULL);
    int64_t id2 = ar__agency__create_agent("test", "1.0.0", NULL);
    int64_t id3 = ar__agency__create_agent("test", "1.0.0", NULL);
    ar__system__process_all_messages();
    
    // Save
    assert(ar__agent_store__save());
    
    // Destroy all agents
    ar__agency__destroy_agent(id1);
    ar__agency__destroy_agent(id2);
    ar__agency__destroy_agent(id3);
    ar__system__process_all_messages();
    
    // Load
    assert(ar__agent_store__load());
    
    // Then the same IDs should be preserved
    assert(ar__agency__agent_exists(id1));
    assert(ar__agency__agent_exists(id2));
    assert(ar__agency__agent_exists(id3));
    
    // And next ID should be set correctly to avoid collisions
    int64_t new_id = ar__agency__create_agent("test", "1.0.0", NULL);
    assert(new_id > id1 && new_id > id2 && new_id > id3);
    
    // Check for memory leaks
    assert(ar__system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar__agent_store__delete();
    ar__system_fixture__destroy(own_fixture);
    
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
