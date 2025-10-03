#include "ar_agent_store_fixture.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_fixture_create_full_destroy(void) {
    printf("Testing ar_agent_store_fixture__create_full() and ar_agent_store_fixture__destroy()...\n");
    
    // Given we create a full agent store fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    
    // Then the fixture should be created successfully
    assert(own_fixture != NULL);
    
    // When we destroy the fixture
    ar_agent_store_fixture__destroy(own_fixture);
    
    // Then no assertion failures should occur (destruction succeeded)
    printf("✓ Agent store fixture create full/destroy test passed\n");
}

static void test_fixture_create_empty_destroy(void) {
    printf("Testing ar_agent_store_fixture__create_empty() and ar_agent_store_fixture__destroy()...\n");
    
    // Given we create an empty agent store fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_empty();
    
    // Then the fixture should be created successfully
    assert(own_fixture != NULL);
    
    // When we destroy the fixture
    ar_agent_store_fixture__destroy(own_fixture);
    
    // Then no assertion failures should occur (destruction succeeded)
    printf("✓ Agent store fixture create empty/destroy test passed\n");
}

static void test_fixture_create_agent(void) {
    printf("Testing ar_agent_store_fixture__create_agent()...\n");
    
    // Given a full fixture with test methods
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    assert(own_fixture != NULL);
    
    // When we create an agent with echo method
    int64_t agent_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    
    // Then an agent ID should be allocated
    assert(agent_id > 0);
    
    // And the agent count should be 1
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 1);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture create agent test passed\n");
}

static void test_fixture_get_agent_memory(void) {
    printf("Testing ar_agent_store_fixture__get_agent_memory()...\n");
    
    // Given a fixture with an agent
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    int64_t agent_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    
    // When we get the agent's memory
    ar_data_t *mut_memory = ar_agent_store_fixture__get_agent_memory(own_fixture, agent_id);
    
    // Then memory should be accessible
    assert(mut_memory != NULL);
    assert(ar_data__get_type(mut_memory) == AR_DATA_TYPE__MAP);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture get agent memory test passed\n");
}

static void test_fixture_verify_agent(void) {
    printf("Testing ar_agent_store_fixture__verify_agent()...\n");
    
    // Given a fixture with an echo agent
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    int64_t agent_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    
    // When we verify the agent has the echo method
    bool result = ar_agent_store_fixture__verify_agent(own_fixture, agent_id, "echo");
    
    // Then verification should succeed
    assert(result == true);
    
    // And verification with wrong method name should fail
    bool wrong_result = ar_agent_store_fixture__verify_agent(own_fixture, agent_id, "calculator");
    assert(wrong_result == false);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture verify agent test passed\n");
}

static void test_fixture_destroy_agent(void) {
    printf("Testing ar_agent_store_fixture__destroy_agent()...\n");
    
    // Given a fixture with two agents
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    int64_t agent1_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    int64_t agent2_id = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 2);
    
    // When we destroy the first agent
    ar_agent_store_fixture__destroy_agent(own_fixture, agent1_id);
    
    // Then the agent count should be 1
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 1);
    
    // And the second agent should still exist
    assert(ar_agent_store_fixture__verify_agent(own_fixture, agent2_id, "calculator") == true);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture destroy agent test passed\n");
}

static void test_fixture_destroy_agents(void) {
    printf("Testing ar_agent_store_fixture__destroy_agents()...\n");
    
    // Given a fixture with three agents
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    int64_t agent1_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    int64_t agent2_id = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    int64_t agent3_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 3);
    
    // When we destroy all agents
    int64_t agent_ids[] = {agent1_id, agent2_id, agent3_id};
    ar_agent_store_fixture__destroy_agents(own_fixture, agent_ids, 3);
    
    // Then the agent count should be 0
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 0);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture destroy agents test passed\n");
}

static void test_fixture_save_load(void) {
    printf("Testing ar_agent_store_fixture__save() and ar_agent_store_fixture__load()...\n");
    
    // Given a fixture with agents
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    int64_t agent1_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    int64_t agent2_id = ar_agent_store_fixture__create_agent(own_fixture, "calculator", "1.0.0");
    
    // When we save the agents
    bool save_result = ar_agent_store_fixture__save(own_fixture);
    
    // Then save should succeed
    assert(save_result == true);
    
    // And when we destroy the agents and load them back
    ar_agent_store_fixture__destroy_agents(own_fixture, &agent1_id, 1);
    ar_agent_store_fixture__destroy_agents(own_fixture, &agent2_id, 1);
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 0);
    
    bool load_result = ar_agent_store_fixture__load(own_fixture);
    
    // Then load should succeed
    assert(load_result == true);
    
    // And agents should be restored
    assert(ar_agent_store_fixture__get_agent_count(own_fixture) == 2);
    
    // Clean up
    ar_agent_store_fixture__delete_file(own_fixture);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture save/load test passed\n");
}

static void test_fixture_create_yaml_file_single(void) {
    printf("Testing ar_agent_store_fixture__create_yaml_file_single()...\n");
    
    // Given a fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    const char *test_path = "test_single_agent.yaml";
    
    // When we create a YAML file with single test agent
    bool result = ar_agent_store_fixture__create_yaml_file_single(own_fixture, test_path);
    
    // Then the YAML file should be created successfully
    assert(result == true);
    
    // Clean up
    remove(test_path);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture create single agent YAML file test passed\n");
}

static void test_fixture_create_yaml_file(void) {
    printf("Testing ar_agent_store_fixture__create_yaml_file()...\n");
    
    // Given a fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    const char *test_path = "test_agents.yaml";
    
    // When we create a YAML file with multiple test agents
    bool result = ar_agent_store_fixture__create_yaml_file(own_fixture, test_path);
    
    // Then the YAML file should be created successfully
    assert(result == true);
    
    // Clean up
    remove(test_path);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture create multiple agents YAML file test passed\n");
}

static void test_fixture_get_agent_ids(void) {
    printf("Testing ar_agent_store_fixture__get_first_agent_id() and ar_agent_store_fixture__get_next_agent_id()...\n");
    
    // Given a fixture with one agent
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create_full();
    int64_t agent_id = ar_agent_store_fixture__create_agent(own_fixture, "echo", "1.0.0");
    
    // When we get the first agent ID
    int64_t first_id = ar_agent_store_fixture__get_first_agent_id(own_fixture);
    
    // Then it should match the created agent
    assert(first_id == agent_id);
    
    // And when we get the next agent ID
    int64_t next_id = ar_agent_store_fixture__get_next_agent_id(own_fixture);
    
    // Then it should be greater than the current agent ID
    assert(next_id > agent_id);
    
    // Clean up
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture get agent IDs test passed\n");
}

int main(void) {
    printf("\n=== Running Agent Store Fixture Tests ===\n\n");
    
    test_fixture_create_full_destroy();
    test_fixture_create_empty_destroy();
    test_fixture_create_agent();
    test_fixture_get_agent_memory();
    test_fixture_verify_agent();
    test_fixture_destroy_agent();
    test_fixture_destroy_agents();
    test_fixture_save_load();
    test_fixture_create_yaml_file_single();
    test_fixture_create_yaml_file();
    test_fixture_get_agent_ids();
    
    printf("\n=== All Agent Store Fixture Tests Passed ===\n");
    return 0;
}
