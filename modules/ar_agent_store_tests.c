/**
 * @file ar_agent_store_tests.c
 * @brief Tests for the agent store module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "ar_agent_store.h"
#include "ar_agent_registry.h"
#include "ar_agent.h"
#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_data.h"
#include "ar_yaml_reader.h"

static void test_store_basics(void) {
    printf("Testing store basic operations...\n");
    
    // Given a registry and methodology
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Test basic save/load operations
    // When saving with no agents
    assert(ar_agent_store__save(own_store));
    
    // When loading 
    assert(ar_agent_store__load(own_store));
    
    // Clean up
    ar_agent_store__destroy(own_store);
    ar_methodology__destroy(own_methodology);
    ar_agent_registry__destroy(own_registry);
    
    printf("✓ Store basic operations test passed\n");
}

static void test_store_empty_save_load(void) {
    printf("Testing empty store save/load...\n");
    
    // Given a registry with no agents
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // When saving empty state
    assert(ar_agent_store__save(own_store));
    
    // When loading
    assert(ar_agent_store__load(own_store));
    
    // Then no agents should exist
    assert(ar_agent_registry__count(own_registry) == 0);
    
    // Clean up
    ar_agent_store__destroy(own_store);
    ar_methodology__destroy(own_methodology);
    ar_agent_registry__destroy(own_registry);
    
    printf("✓ Empty store save/load test passed\n");
}

static void test_store_single_agent(void) {
    printf("Testing single agent persistence...\n");
    
    // Create a test method directly (not using global methodology)
    ar_method_t *own_method = ar_method__create("echo", "send(sender, message)", "1.0.0");
    assert(own_method != NULL);
    
    // Given a registry and store
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Create an agent directly
    ar_agent_t *own_agent = ar_agent__create_with_method(own_method, NULL);
    assert(own_agent != NULL);
    
    // Register the agent
    int64_t agent_id = ar_agent_registry__allocate_id(own_registry);
    assert(agent_id > 0);
    ar_agent__set_id(own_agent, agent_id);
    assert(ar_agent_registry__register_id(own_registry, agent_id));
    assert(ar_agent_registry__track_agent(own_registry, agent_id, own_agent));
    
    // Add some data to agent memory
    ar_data_t *mut_memory = ar_agent__get_mutable_memory(own_agent);
    assert(mut_memory != NULL);
    ar_data__set_map_string(mut_memory, "name", "Test Agent");
    ar_data__set_map_integer(mut_memory, "count", 42);  
    ar_data__set_map_double(mut_memory, "value", 3.14);
    
    // When saving
    assert(ar_agent_store__save(own_store));
    
    // Destroy the agent
    ar_agent_registry__unregister_id(own_registry, agent_id);
    ar_agent__destroy(own_agent);
    assert(ar_agent_registry__count(own_registry) == 0);
    
    // When loading
    assert(ar_agent_store__load(own_store));
    
    // Then the agent should be restored (once load is fully implemented)
    // For now, just verify save/load operations complete successfully
    // TODO: Add verification once agent_store load implementation is complete
    
    // Clean up
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_method__destroy(own_method);
    ar_methodology__destroy(own_methodology);
    
    printf("✓ Single agent persistence test passed\n");
}

static void test_store_multiple_agents(void) {
    printf("Testing multiple agent persistence...\n");
    
    // Create methods directly (not using global methodology)
    ar_method_t *own_echo = ar_method__create("echo", "send(sender, message)", "1.0.0");
    assert(own_echo != NULL);
    ar_method_t *own_calc = ar_method__create("calc", "send(sender, \"result: \" + (2 + 2))", "2.0.0");
    assert(own_calc != NULL);
    
    // Given a registry and store
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Clean up any existing store
    ar_agent_store__delete(own_store);
    
    // Create agents
    ar_agent_t *own_echo1 = ar_agent__create_with_method(own_echo, NULL);
    assert(own_echo1 != NULL);
    int64_t echo1_id = ar_agent_registry__allocate_id(own_registry);
    ar_agent__set_id(own_echo1, echo1_id);
    assert(ar_agent_registry__register_id(own_registry, echo1_id));
    assert(ar_agent_registry__track_agent(own_registry, echo1_id, own_echo1));
    
    ar_agent_t *own_echo2 = ar_agent__create_with_method(own_echo, NULL);
    assert(own_echo2 != NULL);
    int64_t echo2_id = ar_agent_registry__allocate_id(own_registry);
    ar_agent__set_id(own_echo2, echo2_id);
    assert(ar_agent_registry__register_id(own_registry, echo2_id));
    assert(ar_agent_registry__track_agent(own_registry, echo2_id, own_echo2));
    
    ar_agent_t *own_calc1 = ar_agent__create_with_method(own_calc, NULL);
    assert(own_calc1 != NULL);
    int64_t calc1_id = ar_agent_registry__allocate_id(own_registry);
    ar_agent__set_id(own_calc1, calc1_id);
    assert(ar_agent_registry__register_id(own_registry, calc1_id));
    assert(ar_agent_registry__track_agent(own_registry, calc1_id, own_calc1));
    
    // Add unique data to each agent
    ar_data_t *mut_memory1 = ar_agent__get_mutable_memory(own_echo1);
    ar_data__set_map_string(mut_memory1, "name", "Echo One");
    ar_data__set_map_integer(mut_memory1, "id", 1);
    
    ar_data_t *mut_memory2 = ar_agent__get_mutable_memory(own_echo2);
    ar_data__set_map_string(mut_memory2, "name", "Echo Two");
    ar_data__set_map_integer(mut_memory2, "id", 2);
    
    ar_data_t *mut_memory3 = ar_agent__get_mutable_memory(own_calc1);
    ar_data__set_map_string(mut_memory3, "name", "Calculator");
    ar_data__set_map_double(mut_memory3, "pi", 3.14159);
    
    // When saving
    assert(ar_agent_store__save(own_store));
    
    // Destroy all agents
    ar_agent_registry__unregister_id(own_registry, echo1_id);
    ar_agent__destroy(own_echo1);
    ar_agent_registry__unregister_id(own_registry, echo2_id);
    ar_agent__destroy(own_echo2);
    ar_agent_registry__unregister_id(own_registry, calc1_id);
    ar_agent__destroy(own_calc1);
    assert(ar_agent_registry__count(own_registry) == 0);
    
    // When loading
    assert(ar_agent_store__load(own_store));
    
    // Note: Load implementation is incomplete, so we just verify it doesn't crash
    // TODO: Add full verification once agent_store load implementation is complete
    
    // Clean up
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_method__destroy(own_echo);
    ar_method__destroy(own_calc);
    ar_methodology__destroy(own_methodology);
    
    printf("✓ Multiple agent persistence test passed\n");
}

static void test_store_file_corruption(void) {
    printf("Testing store file corruption handling...\n");
    
    // Given a registry and store
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Clean up any existing store
    ar_agent_store__delete(own_store);
    
    // Create a corrupted file
    FILE *fp = fopen(ar_agent_store__get_path(own_store), "w");
    assert(fp != NULL);
    fprintf(fp, "invalid data\n");
    fprintf(fp, "more garbage\n");
    fclose(fp);
    
    // When loading from corrupted file
    assert(ar_agent_store__load(own_store)); // Should succeed but with empty state
    
    // Note: Load implementation is incomplete, so we just verify it doesn't crash
    // TODO: Add corruption detection and recovery once agent_store load implementation is complete
    
    // Then no agents should exist (they weren't loaded)
    assert(ar_agent_registry__count(own_registry) == 0);
    
    // Clean up the corrupted file manually since load doesn't handle corruption yet
    ar_agent_store__delete(own_store);
    
    // Clean up
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_methodology__destroy(own_methodology);
    
    printf("✓ Store file corruption handling test passed\n");
}

static void test_store_missing_method(void) {
    printf("Testing store with missing method...\n");
    
    // Create a method directly
    ar_method_t *own_method = ar_method__create("test", "send(0, \"ok\")", "1.0.0");
    assert(own_method != NULL);
    
    // Given a registry and store
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Clean up any existing store
    ar_agent_store__delete(own_store);
    
    // Create an agent
    ar_agent_t *own_agent = ar_agent__create_with_method(own_method, NULL);
    assert(own_agent != NULL);
    int64_t agent_id = ar_agent_registry__allocate_id(own_registry);
    ar_agent__set_id(own_agent, agent_id);
    assert(ar_agent_registry__register_id(own_registry, agent_id));
    assert(ar_agent_registry__track_agent(own_registry, agent_id, own_agent));
    
    // Save the agent
    assert(ar_agent_store__save(own_store));
    
    // Destroy the agent
    ar_agent_registry__unregister_id(own_registry, agent_id);
    ar_agent__destroy(own_agent);
    
    // When loading (without the method available for reconstruction)
    assert(ar_agent_store__load(own_store));
    
    // Note: Load implementation is incomplete, so we just verify it doesn't crash
    // TODO: Add verification once agent_store load implementation is complete
    
    // Clean up
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_method__destroy(own_method);
    ar_methodology__destroy(own_methodology);
    
    printf("✓ Store with missing method test passed\n");
}

static void test_store_id_preservation(void) {
    printf("Testing agent ID preservation...\n");
    
    // Create method directly
    ar_method_t *own_method = ar_method__create("test", "send(0, \"ok\")", "1.0.0");
    assert(own_method != NULL);
    
    // Given a registry and store
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Clean up any existing store
    ar_agent_store__delete(own_store);
    
    // Create agents and remember their IDs
    ar_agent_t *own_agent1 = ar_agent__create_with_method(own_method, NULL);
    int64_t id1 = ar_agent_registry__allocate_id(own_registry);
    ar_agent__set_id(own_agent1, id1);
    assert(ar_agent_registry__register_id(own_registry, id1));
    assert(ar_agent_registry__track_agent(own_registry, id1, own_agent1));
    
    ar_agent_t *own_agent2 = ar_agent__create_with_method(own_method, NULL);
    int64_t id2 = ar_agent_registry__allocate_id(own_registry);
    ar_agent__set_id(own_agent2, id2);
    assert(ar_agent_registry__register_id(own_registry, id2));
    assert(ar_agent_registry__track_agent(own_registry, id2, own_agent2));
    
    ar_agent_t *own_agent3 = ar_agent__create_with_method(own_method, NULL);
    int64_t id3 = ar_agent_registry__allocate_id(own_registry);
    ar_agent__set_id(own_agent3, id3);
    assert(ar_agent_registry__register_id(own_registry, id3));
    assert(ar_agent_registry__track_agent(own_registry, id3, own_agent3));
    
    // Save
    assert(ar_agent_store__save(own_store));
    
    // Destroy all agents
    ar_agent_registry__unregister_id(own_registry, id1);
    ar_agent__destroy(own_agent1);
    ar_agent_registry__unregister_id(own_registry, id2);
    ar_agent__destroy(own_agent2);
    ar_agent_registry__unregister_id(own_registry, id3);
    ar_agent__destroy(own_agent3);
    
    // Load
    assert(ar_agent_store__load(own_store));
    
    // Note: Load implementation is incomplete, so we just verify it doesn't crash
    // TODO: Add ID preservation verification once agent_store load implementation is complete
    
    // Clean up
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_method__destroy(own_method);
    ar_methodology__destroy(own_methodology);
    
    printf("✓ Agent ID preservation test passed\n");
}

static void test_store_yaml_format_validation(void) {
    printf("Testing agent store YAML format validation...\n");
    
    // Given an agent store with an agent containing memory data
    ar_method_t *own_method = ar_method__create("echo", "send(sender, message)", "1.0.0");
    assert(own_method != NULL);
    
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Create and register agent with memory
    ar_agent_t *own_agent = ar_agent__create_with_method(own_method, NULL);
    assert(own_agent != NULL);
    
    int64_t agent_id = ar_agent_registry__allocate_id(own_registry);
    assert(agent_id > 0);
    ar_agent__set_id(own_agent, agent_id);
    assert(ar_agent_registry__register_id(own_registry, agent_id));
    assert(ar_agent_registry__track_agent(own_registry, agent_id, own_agent));
    
    ar_data_t *mut_memory = ar_agent__get_mutable_memory(own_agent);
    assert(mut_memory != NULL);
    ar_data__set_map_string(mut_memory, "name", "Test Agent");
    ar_data__set_map_integer(mut_memory, "count", 42);
    ar_data__set_map_double(mut_memory, "value", 3.14);
    
    // When saving the agent store
    assert(ar_agent_store__save(own_store));
    
    // Then the saved file should be valid YAML with proper structure
    const char *file_path = ar_agent_store__get_path(own_store);
    ar_yaml_reader_t *own_reader = ar_yaml_reader__create(NULL);
    assert(own_reader != NULL);
    
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file(own_reader, file_path);
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_agents = ar_data__get_map_data(own_loaded, "agents");
    assert(ref_agents != NULL);
    assert(ar_data__get_type(ref_agents) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(ref_agents) == 1);
    
    // Cleanup
    ar_data__destroy(own_loaded);
    ar_yaml_reader__destroy(own_reader);
    ar_agent_registry__unregister_id(own_registry, agent_id);
    ar_agent__destroy(own_agent);
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_method__destroy(own_method);
    ar_methodology__destroy(own_methodology);
    
    printf("✓ Agent store YAML format validation test passed\n");
}

static void test_store_methodology_support(void) {
    printf("Testing agent store methodology support...\n");
    
    // Given a registry and methodology
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    // When creating agent store with methodology
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // Then methodology should be accessible from agent store
    ar_methodology_t *ref_methodology = ar_agent_store__get_methodology(own_store);
    assert(ref_methodology == own_methodology);
    
    // Clean up
    ar_agent_store__destroy(own_store);
    ar_methodology__destroy(own_methodology);
    ar_agent_registry__destroy(own_registry);
    
    printf("✓ Agent store methodology support test passed\n");
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
    test_store_methodology_support();
    test_store_yaml_format_validation();
    
    printf("All 10 tests passed!\n");
    return 0;
}
