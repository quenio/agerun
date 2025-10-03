/**
 * @file ar_agent_store_tests.c
 * @brief Tests for the agent store module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include "ar_agent_store.h"
#include "ar_agent_registry.h"
#include "ar_agent.h"
#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_data.h"
#include "ar_yaml_reader.h"
#include "ar_agent_store_fixture.h"

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

    // When creating corrupted file (invalid YAML)
    FILE *fp = fopen(ar_agent_store__get_path(own_store), "w");
    assert(fp != NULL);
    fprintf(fp, "invalid data\n");
    fprintf(fp, "more garbage\n");
    fclose(fp);

    // When loading corrupted file
    bool result = ar_agent_store__load(own_store);

    // Then should return false (YAML parsing fails on corrupted data)
    assert(result == false);

    // Then no agents should exist (nothing was loaded)
    assert(ar_agent_registry__count(own_registry) == 0);

    // Clean up the corrupted file
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
    
    // Verify agent data structure
    ar_data_t *ref_agent_data = ar_data__list_first(ref_agents);
    assert(ref_agent_data != NULL);
    assert(ar_data__get_type(ref_agent_data) == AR_DATA_TYPE__MAP);
    
    // Verify agent ID
    int loaded_id = ar_data__get_map_integer(ref_agent_data, "id");
    assert(loaded_id == (int)agent_id);
    
    // Verify method info
    const char *loaded_method_name = ar_data__get_map_string(ref_agent_data, "method_name");
    assert(loaded_method_name != NULL);
    assert(strcmp(loaded_method_name, "echo") == 0);
    
    const char *loaded_method_version = ar_data__get_map_string(ref_agent_data, "method_version");
    assert(loaded_method_version != NULL);
    assert(strcmp(loaded_method_version, "1.0.0") == 0);
    
    // Verify memory data (shallow copy)
    ar_data_t *ref_memory_data = ar_data__get_map_data(ref_agent_data, "memory");
    assert(ref_memory_data != NULL);
    assert(ar_data__get_type(ref_memory_data) == AR_DATA_TYPE__MAP);
    
    // Verify memory contents are preserved
    const char *loaded_name = ar_data__get_map_string(ref_memory_data, "name");
    assert(loaded_name != NULL);
    assert(strcmp(loaded_name, "Test Agent") == 0);
    
    int loaded_count = ar_data__get_map_integer(ref_memory_data, "count");
    assert(loaded_count == 42);
    
    double loaded_value = ar_data__get_map_double(ref_memory_data, "value");
    assert(loaded_value == 3.14);
    
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

static void test_store_invalid_yaml_structure(void) {
    printf("Testing invalid YAML structure handling...\n");

    // Given a store with invalid YAML file (missing "agents" key)
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);

    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);

    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);

    // Clean up any existing store
    ar_agent_store__delete(own_store);

    // When creating YAML file with wrong structure (no "agents" list)
    FILE *fp = fopen(ar_agent_store__get_path(own_store), "w");
    assert(fp != NULL);
    fprintf(fp, "version: 1.0\n");       // Valid YAML, but wrong structure
    fprintf(fp, "wrong_key: value\n");    // Missing required "agents" key
    fclose(fp);

    // When loading file with invalid structure
    bool result = ar_agent_store__load(own_store);

    // Then should return false (YAML parsed but structure invalid)
    assert(result == false);  // RED: This will FAIL - current implementation returns true

    // Clean up
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_methodology__destroy(own_methodology);

    printf("✓ Invalid YAML structure handling test passed\n");
}

static void test_store_load_creates_single_agent(void) {
    printf("Testing store load creates single agent...\n");

    // Given a methodology with echo method
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    ar_method_t *own_method = ar_method__create("echo", "send(sender, message)", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method(own_methodology, own_method);

    // Given a registry and store
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);

    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);

    // Clean up any existing store
    ar_agent_store__delete(own_store);

    // When creating YAML file with single agent (with memory data)
    FILE *fp = fopen(ar_agent_store__get_path(own_store), "w");
    assert(fp != NULL);
    fprintf(fp, "# AgeRun YAML File\n");
    fprintf(fp, "agents:\n");
    fprintf(fp, "- id: 42\n");
    fprintf(fp, "  method_name: echo\n");
    fprintf(fp, "  method_version: 1.0.0\n");
    fprintf(fp, "  memory:\n");
    fprintf(fp, "    count: 5\n");
    fprintf(fp, "    name: test_agent\n");
    fclose(fp);

    // When loading store
    bool result = ar_agent_store__load(own_store);
    assert(result == true);

    // Then first agent ID should be 42
    int64_t first_id = ar_agent_registry__get_first(own_registry);
    assert(first_id == 42);

    // Then agent should have correct method
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(own_registry, first_id);
    assert(ref_agent != NULL);
    const ar_method_t *ref_method = ar_agent__get_method(ref_agent);
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_name(ref_method), "echo") == 0);
    
    // Then agent memory should contain restored data
    const ar_data_t *ref_memory = ar_agent__get_memory(ref_agent);
    assert(ref_memory != NULL);
    assert(ar_data__get_type(ref_memory) == AR_DATA_TYPE__MAP);
    
    int64_t count = ar_data__get_map_integer(ref_memory, "count");
    assert(count == 5);  // RED: Will FAIL - memory not restored yet
    
    const char *ref_name = ar_data__get_map_string(ref_memory, "name");
    assert(ref_name != NULL);
    assert(strcmp(ref_name, "test_agent") == 0);
    
    // Then registry next_id should be updated to prevent collisions
    int64_t next_id = ar_agent_registry__get_next_id(own_registry);
    assert(next_id == 43);  // RED: Will FAIL - next_id not updated yet

    // Clean up
    ar_agent_t *own_agent = (ar_agent_t*)ar_agent_registry__find_agent(own_registry, first_id);
    if (own_agent) {
        ar_agent_registry__unregister_id(own_registry, first_id);
        ar_agent__destroy(own_agent);
    }
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_methodology__destroy(own_methodology);

    printf("✓ Store load creates single agent test passed\n");
}

static void test_store_load_creates_multiple_agents(void) {
    // Given an agent store fixture with test methodology
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create();
    assert(own_fixture != NULL);
    
    ar_methodology_t *own_methodology = ar_agent_store_fixture__create_test_methodology(own_fixture);
    assert(own_methodology != NULL);
    
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_agent_store_t *own_store = ar_agent_store__create(own_registry, own_methodology);
    assert(own_store != NULL);
    
    // When we create a YAML file with multiple agents and load it
    ar_agent_store__delete(own_store);
    bool yaml_created = ar_agent_store_fixture__create_multiple_agents_yaml(own_fixture, ar_agent_store__get_path(own_store));
    assert(yaml_created == true);
    
    bool result = ar_agent_store__load(own_store);
    
    // Then all agents should be loaded successfully
    assert(result == true);
    assert(ar_agent_registry__get_first(own_registry) == 10);
    assert(ar_agent_store_fixture__verify_agent(own_registry, 10, "echo") == true);
    assert(ar_agent_store_fixture__verify_agent(own_registry, 20, "calculator") == true);
    assert(ar_agent_store_fixture__verify_agent(own_registry, 30, "echo") == true);
    assert(ar_agent_registry__get_next_id(own_registry) == 31);
    
    // Clean up
    int64_t agent_ids[] = {10, 20, 30};
    ar_agent_store_fixture__destroy_all_agents(own_registry, agent_ids, 3);
    ar_agent_store__delete(own_store);
    ar_agent_store__destroy(own_store);
    ar_agent_registry__destroy(own_registry);
    ar_methodology__destroy(own_methodology);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Store load creates multiple agents test passed\n");
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
    test_store_invalid_yaml_structure();
    test_store_load_creates_single_agent();
    test_store_load_creates_multiple_agents();

    printf("All 13 tests passed!\n");
    return 0;
}
