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
#include "ar_agent_store_fixture.h"
#include "ar_data.h"
#include "ar_yaml_reader.h"
#include "ar_assert.h"

static void test_store_basics(void) {
    printf("Testing store basic operations...\n");
    
    // Given a store with empty fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_empty();
    assert(own_fixture != NULL);
    
    // When saving with no agents
    assert(ar_agent_store_fixture__save(own_fixture));

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Store basic operations test passed\n");
}

static void test_store_empty_save_load(void) {
    printf("Testing empty store save/load...\n");
    
    // Given a store with empty fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_empty();
    assert(own_fixture != NULL);
    
    // When saving empty state
    assert(ar_agent_store_fixture__save(own_fixture));

    // Then no agents should exist
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 0);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Empty store save/load test passed\n");
}

static void test_store_single_agent(void) {
    printf("Testing single agent persistence...\n");
    
    // Given an agent store with full test fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    // When creating an agent with memory data
    int64_t agent_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(agent_id > 0);
    
    ar_data_t *mut_memory = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_id);
    assert(mut_memory != NULL);
    ar_data__set_map_string(mut_memory, "name", "Test Agent");
    ar_data__set_map_integer(mut_memory, "count", 42);  
    ar_data__set_map_double(mut_memory, "value", 3.14);
    
    // Then saving should succeed
    assert(ar_agent_store_fixture__save(own_fixture));
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Single agent persistence test passed\n");
}

static void test_store_multiple_agents(void) {
    printf("Testing multiple agent persistence...\n");
    
    // Given an agent store with full test fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    ar_agent_store_fixture__delete_file(own_fixture);
    
    // When creating multiple agents with different methods
    int64_t echo1_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(echo1_id > 0);
    
    int64_t echo2_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(echo2_id > 0);
    
    int64_t calc1_id = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    assert(calc1_id > 0);
    
    ar_data_t *mut_memory1 = ar_agent_store_fixture__get_agent_memory(own_fixture, echo1_id);
    ar_data__set_map_string(mut_memory1, "name", "Echo One");
    ar_data__set_map_integer(mut_memory1, "id", 1);
    
    ar_data_t *mut_memory2 = ar_agent_store_fixture__get_agent_memory(own_fixture, echo2_id);
    ar_data__set_map_string(mut_memory2, "name", "Echo Two");
    ar_data__set_map_integer(mut_memory2, "id", 2);
    
    ar_data_t *mut_memory3 = ar_agent_store_fixture__get_agent_memory(own_fixture, calc1_id);
    ar_data__set_map_string(mut_memory3, "name", "Calculator");
    ar_data__set_map_double(mut_memory3, "pi", 3.14159);
    
    // Then saving should succeed
    assert(ar_agent_store_fixture__save(own_fixture));
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Multiple agent persistence test passed\n");
}

static void test_store_file_corruption(void) {
    printf("Testing store file corruption handling...\n");

    // Given a store with empty fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_empty();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);

    // When creating corrupted file (invalid YAML)
    const char *store_path = ar_agent_store_fixture__get_store_path(own_fixture);
    FILE *fp = fopen(store_path, "w");
    assert(fp != NULL);
    fprintf(fp, "invalid data\n");
    fprintf(fp, "more garbage\n");
    fclose(fp);

    // When loading corrupted file
    bool result = ar_agent_store_fixture__load(own_fixture);

    // Then should return false (YAML parsing fails on corrupted data)
    assert(result == false);

    // Then no agents should exist (nothing was loaded)
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 0);

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);

    printf("✓ Store file corruption handling test passed\n");
}

static void test_store_id_preservation(void) {
    printf("Testing agent ID preservation...\n");
    
    // Given an agent store with full test fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);
    
    // Create agents and remember their IDs
    int64_t id1 = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(id1 > 0);
    
    int64_t id2 = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    assert(id2 > 0);
    
    int64_t id3 = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(id3 > 0);
    
    // Save agents
    assert(ar_agent_store_fixture__save(own_fixture));
    
    // Destroy all agents
    int64_t agent_ids[] = {id1, id2, id3};
    ar_agent_store_fixture__destroy_agents(own_fixture, agent_ids, 3);
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 0);
    
    // Load agents back
    assert(ar_agent_store_fixture__load(own_fixture));
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 3);
    
    // Then the loaded agents should have the same IDs
    assert(ar_agent_store_fixture__verify_agent(own_fixture, id1, "echo"));
    assert(ar_agent_store_fixture__verify_agent(own_fixture, id2, "calculator"));
    assert(ar_agent_store_fixture__verify_agent(own_fixture, id3, "echo"));
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent ID preservation test passed\n");
}

static void test_store_memory_persistence(void) {
    printf("Testing agent memory persistence...\n");
    
    // Given an agent store with full test fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);
    
    // Create agent with specific memory values
    int64_t agent_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(agent_id > 0);
    
    ar_data_t *mut_memory = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data__set_map_string(mut_memory, "name", "Memory Test");
    ar_data__set_map_integer(mut_memory, "count", 99);
    ar_data__set_map_double(mut_memory, "ratio", 2.718);
    
    // Save agent
    assert(ar_agent_store_fixture__save(own_fixture));
    
    // Destroy and reload
    ar_agent_store_fixture__destroy_agent(own_fixture, agent_id);
    assert(ar_agent_store_fixture__load(own_fixture));
    
    // Then memory should be restored
    ar_data_t *mut_restored = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_id);
    assert(mut_restored != NULL);
    
    const char *name_str = ar_data__get_map_string(mut_restored, "name");
    assert(name_str != NULL);
    assert(strcmp(name_str, "Memory Test") == 0);
    
    int count_val = ar_data__get_map_integer(mut_restored, "count");
    assert(count_val == 99);
    
    double ratio_val = ar_data__get_map_double(mut_restored, "ratio");
    assert(ratio_val == 2.718);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent memory persistence test passed\n");
}

static void test_store_method_preservation(void) {
    printf("Testing method information preservation...\n");
    
    // Given an agent store with full test fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);
    
    // Create agents with different methods
    int64_t echo_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    int64_t calc_id = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    
    // Save agents
    assert(ar_agent_store_fixture__save(own_fixture));
    
    // Destroy and reload
    int64_t ids[] = {echo_id, calc_id};
    ar_agent_store_fixture__destroy_agents(own_fixture, ids, 2);
    assert(ar_agent_store_fixture__load(own_fixture));
    
    // Then agents should have correct methods
    assert(ar_agent_store_fixture__verify_agent(own_fixture, echo_id, "echo"));
    assert(ar_agent_store_fixture__verify_agent(own_fixture, calc_id, "calculator"));
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Method information preservation test passed\n");
}

static void test_store_id_continuity(void) {
    printf("Testing agent ID continuity after load...\n");
    
    // Given an agent store with full test fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);
    
    // Create agents with IDs
    int64_t id1 = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    int64_t id2 = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    
    // Save agents
    assert(ar_agent_store_fixture__save(own_fixture));
    
    // Destroy and reload
    int64_t ids[] = {id1, id2};
    ar_agent_store_fixture__destroy_agents(own_fixture, ids, 2);
    assert(ar_agent_store_fixture__load(own_fixture));
    
    // Then new agent should get ID after existing ones
    int64_t id3 = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(id3 > id2);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent ID continuity after load test passed\n");
}

static void test_store_invalid_yaml_structure(void) {
    printf("Testing invalid YAML structure handling...\n");

    // Given a store with empty fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_empty();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);

    // When creating YAML file with wrong structure (no "agents" list)
    const char *store_path = ar_agent_store_fixture__get_store_path(own_fixture);
    FILE *fp = fopen(store_path, "w");
    assert(fp != NULL);
    fprintf(fp, "version: 1.0\n");
    fprintf(fp, "wrong_key: value\n");
    fclose(fp);

    // When loading file with invalid structure
    bool result = ar_agent_store_fixture__load(own_fixture);

    // Then should return false (YAML parsed but structure invalid)
    assert(result == false);

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);

    printf("✓ Invalid YAML structure handling test passed\n");
}

static void test_store_load_creates_single_agent(void) {
    printf("Testing store load creates single agent...\n");

    // Given a fixture with full methodology (echo, calculator)
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);

    // When creating YAML file with single agent (with memory data)
    const char *store_path = ar_agent_store_fixture__get_store_path(own_fixture);
    bool created = ar_agent_store_fixture__create_yaml_file_single(own_fixture, store_path);
    assert(created == true);

    // When loading store
    bool result = ar_agent_store_fixture__load(own_fixture);
    assert(result == true);

    // Then first agent ID should be 42
    int64_t first_id = ar_agent_store_fixture__get_first_agent_id(own_fixture);
    assert(first_id == 42);

    // Then agent should have correct method
    assert(ar_agent_store_fixture__verify_agent(own_fixture, 42, "echo"));
    
    // Then agent memory should contain restored data
    ar_data_t *mut_memory = ar_agent_store_fixture__get_agent_memory(own_fixture, 42);
    assert(mut_memory != NULL);
    
    int64_t count = ar_data__get_map_integer(mut_memory, "count");
    assert(count == 5);
    
    const char *ref_name = ar_data__get_map_string(mut_memory, "name");
    assert(ref_name != NULL);
    assert(strcmp(ref_name, "test_agent") == 0);
    
    // Then registry next_id should be updated to prevent collisions
    int64_t next_id = ar_agent_store_fixture__get_next_agent_id(own_fixture);
    assert(next_id == 43);

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);

    printf("✓ Store load creates single agent test passed\n");
}

static void test_store_load_creates_multiple_agents(void) {
    printf("Testing store load creates multiple agents...\n");

    // Given a fixture with full methodology
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);
    
    // When creating YAML file with multiple agents
    const char *store_path = ar_agent_store_fixture__get_store_path(own_fixture);
    bool created = ar_agent_store_fixture__create_yaml_file(own_fixture, store_path);
    assert(created == true);
    
    // When loading store
    bool result = ar_agent_store_fixture__load(own_fixture);
    assert(result == true);
    
    // Then first agent ID should be 10
    int64_t first_id = ar_agent_store_fixture__get_first_agent_id(own_fixture);
    assert(first_id == 10);
    
    // Then all agents should exist with correct methods
    assert(ar_agent_store_fixture__verify_agent(own_fixture, 10, "echo"));
    assert(ar_agent_store_fixture__verify_agent(own_fixture, 20, "calculator"));
    assert(ar_agent_store_fixture__verify_agent(own_fixture, 30, "echo"));

    // Then memory should be restored for all agents
    ar_data_t *mut_memory_10 = ar_agent_store_fixture__get_agent_memory(own_fixture, 10);
    AR_ASSERT(mut_memory_10 != NULL, "Agent 10 should have memory");
    const char *msg_10 = ar_data__get_map_string(mut_memory_10, "message");
    AR_ASSERT(msg_10 != NULL, "Agent 10 should have 'message' key");
    AR_ASSERT(strcmp(msg_10, "first_agent") == 0, "Agent 10 message should be 'first_agent'");

    ar_data_t *mut_memory_20 = ar_agent_store_fixture__get_agent_memory(own_fixture, 20);
    AR_ASSERT(mut_memory_20 != NULL, "Agent 20 should have memory");
    int64_t result_20 = ar_data__get_map_integer(mut_memory_20, "result");
    AR_ASSERT(result_20 == 100, "Agent 20 result should be 100");

    ar_data_t *mut_memory_30 = ar_agent_store_fixture__get_agent_memory(own_fixture, 30);
    AR_ASSERT(mut_memory_30 != NULL, "Agent 30 should have memory");
    const char *msg_30 = ar_data__get_map_string(mut_memory_30, "message");
    AR_ASSERT(msg_30 != NULL, "Agent 30 should have 'message' key");
    AR_ASSERT(strcmp(msg_30, "third_agent") == 0, "Agent 30 message should be 'third_agent'");

    // Then next ID should be after all loaded agents
    int64_t next_id = ar_agent_store_fixture__get_next_agent_id(own_fixture);
    assert(next_id == 31);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Store load creates multiple agents test passed\n");
}

static void test_store_yaml_format_validation(void) {
    printf("Testing agent store YAML format validation...\n");
    
    // Given an agent store with full fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);
    
    // Create and save agent with memory
    int64_t agent_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(agent_id > 0);
    
    ar_data_t *mut_memory = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_id);
    assert(mut_memory != NULL);
    ar_data__set_map_string(mut_memory, "name", "Test Agent");
    ar_data__set_map_integer(mut_memory, "count", 42);
    ar_data__set_map_double(mut_memory, "value", 3.14);
    
    // When saving the agent store
    assert(ar_agent_store_fixture__save(own_fixture));
    
    // Then the saved file should be valid YAML with proper structure
    const char *file_path = ar_agent_store_fixture__get_store_path(own_fixture);
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
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store YAML format validation test passed\n");
}

static void test_store_load_missing_method(void) {
    printf("Testing store load with missing method...\n");

    // Given a fixture with full methodology
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);

    // When creating YAML file with non-existent method
    const char *store_path = ar_agent_store_fixture__get_store_path(own_fixture);
    FILE *file = fopen(store_path, "w");
    assert(file != NULL);

    fprintf(file, "version: \"1.0\"\n");
    fprintf(file, "agents:\n");
    fprintf(file, "  - id: 99\n");
    fprintf(file, "    method_name: \"nonexistent\"\n");
    fprintf(file, "    method_version: \"1.0.0\"\n");
    fprintf(file, "    memory: {}\n");

    fclose(file);

    // When loading store with missing method
    bool result = ar_agent_store_fixture__load(own_fixture);

    // Then load should succeed (skip bad agents gracefully)
    AR_ASSERT(result == true, "Load should succeed despite missing method");

    // Then agent should not be created
    int64_t agent_count = ar_agent_store_fixture__get_agent_count(own_fixture);
    AR_ASSERT(agent_count == 0, "No agents should be created for missing methods");

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);

    printf("✓ Store load missing method test passed\n");
}

static void test_store_load_corrupt_yaml(void) {
    printf("Testing store load with corrupt YAML structure...\n");

    // Given a fixture with full methodology
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);

    // When creating YAML file with invalid structure (agents is a map instead of list)
    const char *store_path = ar_agent_store_fixture__get_store_path(own_fixture);
    FILE *file = fopen(store_path, "w");
    assert(file != NULL);

    fprintf(file, "version: \"1.0\"\n");
    fprintf(file, "agents: {bad: structure}\n");  // Should be a list, not a map

    fclose(file);

    // When loading store with corrupt YAML
    bool result = ar_agent_store_fixture__load(own_fixture);

    // Then load should fail
    AR_ASSERT(result == false, "Load should fail for corrupt YAML structure");

    // Then no agents should be created
    int64_t agent_count = ar_agent_store_fixture__get_agent_count(own_fixture);
    AR_ASSERT(agent_count == 0, "No agents should be created for corrupt YAML");

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);

    printf("✓ Store load corrupt YAML test passed\n");
}

static void test_store_load_missing_required_fields(void) {
    printf("Testing store load with missing required fields...\n");

    // Given a fixture with full methodology
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);

    // When creating YAML file with agents missing required fields
    const char *store_path = ar_agent_store_fixture__get_store_path(own_fixture);
    FILE *file = fopen(store_path, "w");
    assert(file != NULL);

    fprintf(file, "version: \"1.0\"\n");
    fprintf(file, "agents:\n");
    fprintf(file, "  - id: 0\n");  // Invalid ID (must be > 0)
    fprintf(file, "    method_name: \"echo\"\n");
    fprintf(file, "    method_version: \"1.0.0\"\n");
    fprintf(file, "    memory: {}\n");
    fprintf(file, "  - id: 10\n");
    fprintf(file, "    method_version: \"1.0.0\"\n");  // Missing method_name
    fprintf(file, "    memory: {}\n");
    fprintf(file, "  - id: 20\n");
    fprintf(file, "    method_name: \"echo\"\n");  // Missing method_version
    fprintf(file, "    memory: {}\n");

    fclose(file);

    // When loading store with missing fields
    bool result = ar_agent_store_fixture__load(own_fixture);

    // Then load should succeed (skip bad agents gracefully)
    AR_ASSERT(result == true, "Load should succeed and skip agents with missing fields");

    // Then no agents should be created (all had missing/invalid fields)
    int64_t agent_count = ar_agent_store_fixture__get_agent_count(own_fixture);
    AR_ASSERT(agent_count == 0, "No agents should be created when fields are missing");

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);

    printf("✓ Store load missing required fields test passed\n");
}

static void test_store_complete_lifecycle_integration(void) {
    printf("Testing complete agent lifecycle integration (end-to-end)...\n");

    // Step 1: Create empty fixture - Start fresh
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);

    // Clean up any existing store
    ar_agent_store_fixture__delete_file(own_fixture);

    // Step 2: Create 3 agents with different methods
    int64_t agent_1 = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    AR_ASSERT(agent_1 > 0, "Agent 1 should be created");

    int64_t agent_2 = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    AR_ASSERT(agent_2 > 0, "Agent 2 should be created");

    int64_t agent_3 = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    AR_ASSERT(agent_3 > 0, "Agent 3 should be created");

    // Step 3: Populate distinct memory for each agent
    ar_data_t *mut_memory_1 = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_1);
    AR_ASSERT(mut_memory_1 != NULL, "Agent 1 should have memory");
    ar_data__set_map_string(mut_memory_1, "message", "first");
    ar_data__set_map_integer(mut_memory_1, "count", 10);

    ar_data_t *mut_memory_2 = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_2);
    AR_ASSERT(mut_memory_2 != NULL, "Agent 2 should have memory");
    ar_data__set_map_integer(mut_memory_2, "result", 42);
    ar_data__set_map_double(mut_memory_2, "pi", 3.14);

    ar_data_t *mut_memory_3 = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_3);
    AR_ASSERT(mut_memory_3 != NULL, "Agent 3 should have memory");
    ar_data__set_map_string(mut_memory_3, "message", "third");
    ar_data__set_map_double(mut_memory_3, "value", 99.9);

    // Step 4: Save - Persist to agerun.agency
    bool save_result = ar_agent_store_fixture__save(own_fixture);
    AR_ASSERT(save_result == true, "Save should succeed");

    // Step 5: Destroy all - Simulate shutdown
    int64_t all_ids[] = {agent_1, agent_2, agent_3};
    ar_agent_store_fixture__destroy_agents(own_fixture, all_ids, 3);

    // Step 6: Verify empty - Count == 0
    int64_t empty_count = ar_agent_store_fixture__get_agent_count(own_fixture);
    AR_ASSERT(empty_count == 0, "All agents should be destroyed");

    // Step 7: Load - Restore from file
    bool load_result = ar_agent_store_fixture__load(own_fixture);
    AR_ASSERT(load_result == true, "Load should succeed");

    // Step 8: Enumerate verification (following KB Test Completeness Enumeration pattern)

    // Verify count
    int64_t loaded_count = ar_agent_store_fixture__get_agent_count(own_fixture);
    AR_ASSERT(loaded_count == 3, "Should restore exactly 3 agents");

    // Verify Agent 1: ID, method, memory
    bool agent_1_exists = ar_agent_store_fixture__verify_agent(own_fixture, agent_1, "echo");
    AR_ASSERT(agent_1_exists, "Agent 1 should exist with echo method");

    ar_data_t *mut_restored_1 = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_1);
    AR_ASSERT(mut_restored_1 != NULL, "Agent 1 should have restored memory");

    const char *message_1 = ar_data__get_map_string(mut_restored_1, "message");
    AR_ASSERT(message_1 != NULL, "Agent 1 should have 'message' field");
    AR_ASSERT(strcmp(message_1, "first") == 0, "Agent 1 message should be 'first'");

    int64_t count_1 = ar_data__get_map_integer(mut_restored_1, "count");
    AR_ASSERT(count_1 == 10, "Agent 1 count should be 10");

    // Verify Agent 2: ID, method, memory
    bool agent_2_exists = ar_agent_store_fixture__verify_agent(own_fixture, agent_2, "calculator");
    AR_ASSERT(agent_2_exists, "Agent 2 should exist with calculator method");

    ar_data_t *mut_restored_2 = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_2);
    AR_ASSERT(mut_restored_2 != NULL, "Agent 2 should have restored memory");

    int64_t result_2 = ar_data__get_map_integer(mut_restored_2, "result");
    AR_ASSERT(result_2 == 42, "Agent 2 result should be 42");

    double pi_2 = ar_data__get_map_double(mut_restored_2, "pi");
    AR_ASSERT(pi_2 == 3.14, "Agent 2 pi should be 3.14");

    // Verify Agent 3: ID, method, memory
    bool agent_3_exists = ar_agent_store_fixture__verify_agent(own_fixture, agent_3, "echo");
    AR_ASSERT(agent_3_exists, "Agent 3 should exist with echo method");

    ar_data_t *mut_restored_3 = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_3);
    AR_ASSERT(mut_restored_3 != NULL, "Agent 3 should have restored memory");

    const char *message_3 = ar_data__get_map_string(mut_restored_3, "message");
    AR_ASSERT(message_3 != NULL, "Agent 3 should have 'message' field");
    AR_ASSERT(strcmp(message_3, "third") == 0, "Agent 3 message should be 'third'");

    double value_3 = ar_data__get_map_double(mut_restored_3, "value");
    AR_ASSERT(value_3 == 99.9, "Agent 3 value should be 99.9");

    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);

    printf("✓ Complete lifecycle integration test passed\n");
}

int main(void) {
    printf("\n=== Running Agent Store Tests ===\n\n");

    test_store_basics();
    test_store_empty_save_load();
    test_store_single_agent();
    test_store_multiple_agents();
    test_store_file_corruption();
    test_store_id_preservation();
    test_store_memory_persistence();
    test_store_method_preservation();
    test_store_id_continuity();
    test_store_invalid_yaml_structure();
    test_store_load_creates_single_agent();
    test_store_load_creates_multiple_agents();
    test_store_yaml_format_validation();
    test_store_load_missing_method();
    test_store_load_corrupt_yaml();
    test_store_load_missing_required_fields();
    test_store_complete_lifecycle_integration();

    printf("\n=== All Agent Store Tests Passed ===\n");
    return 0;
}
