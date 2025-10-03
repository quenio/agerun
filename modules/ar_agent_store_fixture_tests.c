#include "ar_agent_store_fixture.h"
#include "ar_agent_store.h"
#include "ar_agent_registry.h"
#include "ar_methodology.h"
#include "ar_agent.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_fixture_create_destroy(void) {
    printf("Testing ar_agent_store_fixture__create() and ar_agent_store_fixture__destroy()...\n");
    
    // Given we create an agent store fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create();
    
    // Then the fixture should be created successfully
    assert(own_fixture != NULL);
    
    // When we destroy the fixture
    ar_agent_store_fixture__destroy(own_fixture);
    
    // Then no assertion failures should occur (destruction succeeded)
    printf("✓ Agent store fixture create/destroy test passed\n");
}

static void test_fixture_create_test_methodology(void) {
    printf("Testing ar_agent_store_fixture__create_test_methodology()...\n");
    
    // Given an agent store fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create();
    assert(own_fixture != NULL);
    
    // When we create a test methodology
    ar_methodology_t *own_methodology = ar_agent_store_fixture__create_test_methodology(own_fixture);
    
    // Then the methodology should be created with echo and calculator methods
    assert(own_methodology != NULL);
    
    ar_method_t *ref_echo = ar_methodology__get_method(own_methodology, "echo", "1.0.0");
    assert(ref_echo != NULL);
    assert(strcmp(ar_method__get_name(ref_echo), "echo") == 0);
    
    ar_method_t *ref_calc = ar_methodology__get_method(own_methodology, "calculator", "1.0.0");
    assert(ref_calc != NULL);
    assert(strcmp(ar_method__get_name(ref_calc), "calculator") == 0);
    
    // Clean up
    ar_methodology__destroy(own_methodology);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture create test methodology test passed\n");
}

static void test_fixture_create_multiple_agents_yaml(void) {
    printf("Testing ar_agent_store_fixture__create_multiple_agents_yaml()...\n");
    
    // Given an agent store fixture
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create();
    assert(own_fixture != NULL);
    
    const char *test_path = "test_agents.yaml";
    
    // When we create a YAML file with multiple agents
    bool result = ar_agent_store_fixture__create_multiple_agents_yaml(own_fixture, test_path);
    
    // Then the YAML file should be created successfully
    assert(result == true);
    
    // And the file should exist
    FILE *fp = fopen(test_path, "r");
    assert(fp != NULL);
    fclose(fp);
    
    // Clean up
    remove(test_path);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture create multiple agents YAML test passed\n");
}

static void test_fixture_verify_agent(void) {
    printf("Testing ar_agent_store_fixture__verify_agent()...\n");
    
    // Given an agent store fixture and test methodology
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create();
    assert(own_fixture != NULL);
    
    ar_methodology_t *own_methodology = ar_agent_store_fixture__create_test_methodology(own_fixture);
    assert(own_methodology != NULL);
    
    // And a registry with an agent
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_method_t *ref_method = ar_methodology__get_method(own_methodology, "echo", "1.0.0");
    assert(ref_method != NULL);
    
    ar_agent_t *own_agent = ar_agent__create_with_method(ref_method, NULL);
    assert(own_agent != NULL);
    
    int64_t agent_id = 100;
    ar_agent_registry__register_id(own_registry, agent_id);
    ar_agent_registry__track_agent(own_registry, agent_id, own_agent);
    assert(agent_id > 0);
    
    // When we verify the agent has the expected method
    bool result = ar_agent_store_fixture__verify_agent(own_registry, agent_id, "echo");
    
    // Then verification should succeed
    assert(result == true);
    
    // And verification with wrong method name should fail
    bool wrong_result = ar_agent_store_fixture__verify_agent(own_registry, agent_id, "calculator");
    assert(wrong_result == false);
    
    // Clean up
    ar_agent_t *own_cleanup_agent = (ar_agent_t*)ar_agent_registry__find_agent(own_registry, agent_id);
    if (own_cleanup_agent) {
        ar_agent_registry__unregister_id(own_registry, agent_id);
        ar_agent__destroy(own_cleanup_agent);
    }
    ar_agent_registry__destroy(own_registry);
    ar_methodology__destroy(own_methodology);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture verify agent test passed\n");
}

static void test_fixture_destroy_all_agents(void) {
    printf("Testing ar_agent_store_fixture__destroy_all_agents()...\n");
    
    // Given an agent store fixture, methodology, and registry with multiple agents
    ar_agent_store_fixture_t *own_fixture = ar_agent_store_fixture__create();
    assert(own_fixture != NULL);
    
    ar_methodology_t *own_methodology = ar_agent_store_fixture__create_test_methodology(own_fixture);
    assert(own_methodology != NULL);
    
    ar_agent_registry_t *own_registry = ar_agent_registry__create();
    assert(own_registry != NULL);
    
    ar_method_t *ref_echo = ar_methodology__get_method(own_methodology, "echo", "1.0.0");
    ar_method_t *ref_calc = ar_methodology__get_method(own_methodology, "calculator", "1.0.0");
    
    ar_agent_t *own_agent1 = ar_agent__create_with_method(ref_echo, NULL);
    ar_agent_t *own_agent2 = ar_agent__create_with_method(ref_calc, NULL);
    ar_agent_t *own_agent3 = ar_agent__create_with_method(ref_echo, NULL);
    
    int64_t id1 = 100;
    int64_t id2 = 101;
    int64_t id3 = 102;
    ar_agent_registry__register_id(own_registry, id1);
    ar_agent_registry__register_id(own_registry, id2);
    ar_agent_registry__register_id(own_registry, id3);
    ar_agent_registry__track_agent(own_registry, id1, own_agent1);
    ar_agent_registry__track_agent(own_registry, id2, own_agent2);
    ar_agent_registry__track_agent(own_registry, id3, own_agent3);
    
    int64_t agent_ids[] = {id1, id2, id3};
    
    // When we destroy all agents
    ar_agent_store_fixture__destroy_all_agents(own_registry, agent_ids, 3);
    
    // Then the agents should be removed from registry
    assert(ar_agent_registry__find_agent(own_registry, id1) == NULL);
    assert(ar_agent_registry__find_agent(own_registry, id2) == NULL);
    assert(ar_agent_registry__find_agent(own_registry, id3) == NULL);
    
    // Clean up
    ar_agent_registry__destroy(own_registry);
    ar_methodology__destroy(own_methodology);
    ar_agent_store_fixture__destroy(own_fixture);
    
    printf("✓ Agent store fixture destroy all agents test passed\n");
}

int main(void) {
    printf("\n=== Running Agent Store Fixture Tests ===\n\n");
    
    test_fixture_create_destroy();
    test_fixture_create_test_methodology();
    test_fixture_create_multiple_agents_yaml();
    test_fixture_verify_agent();
    test_fixture_destroy_all_agents();
    
    printf("\n=== All Agent Store Fixture Tests Passed ===\n");
    return 0;
}
