#include "agerun_foundation_test_fixture.h"
#include "agerun_heap.h"
#include "agerun_list.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_fixture_create_destroy(void) {
    printf("Testing ar_foundation_test_fixture_create() and destroy()...\n");
    
    // Given a test name
    const char *test_name = "sample_foundation_test";
    
    // When we create a foundation test fixture
    foundation_test_fixture_t *own_fixture = ar_foundation_test_fixture_create(test_name);
    
    // Then the fixture should be created successfully
    assert(own_fixture != NULL);
    
    // And we should be able to get the test name
    const char *retrieved_name = ar_foundation_test_fixture_get_name(own_fixture);
    assert(retrieved_name != NULL);
    assert(strcmp(retrieved_name, test_name) == 0);
    
    // When we destroy the fixture
    ar_foundation_test_fixture_destroy(own_fixture);
    
    // Then no assertion failures should occur (destruction succeeded)
    printf("✓ Create and destroy tests passed\n");
}

static void test_expression_context_creation(void) {
    printf("Testing expression context creation...\n");
    
    // Given a foundation test fixture
    foundation_test_fixture_t *own_fixture = ar_foundation_test_fixture_create("expr_test");
    assert(own_fixture != NULL);
    
    // When we create an expression context
    expression_context_t *ref_ctx = ar_foundation_test_fixture_create_expression_context(
        own_fixture, "memory.count + 10"
    );
    
    // Then the context should be created with standard test data
    assert(ref_ctx != NULL);
    
    // And we can evaluate expressions using the test data
    const data_t *ref_result = ar_expression_evaluate(ref_ctx);
    assert(ref_result != NULL);
    assert(ar_data_get_type(ref_result) == DATA_INTEGER);
    assert(ar_data_get_integer(ref_result) == 52); // 42 + 10
    
    // Clean up
    ar_foundation_test_fixture_destroy(own_fixture);
    
    printf("✓ Expression context creation tests passed\n");
}

static void test_custom_expression_context(void) {
    printf("Testing custom expression context creation...\n");
    
    // Given a foundation test fixture and custom data
    foundation_test_fixture_t *own_fixture = ar_foundation_test_fixture_create("custom_expr");
    assert(own_fixture != NULL);
    
    data_t *own_memory = ar_data_create_map();
    ar_data_set_map_integer(own_memory, "x", 100);
    
    // When we create a custom expression context
    expression_context_t *ref_ctx = ar_foundation_test_fixture_create_custom_expression_context(
        own_fixture, own_memory, NULL, NULL, "memory.x * 2"
    );
    
    // Then the context should use our custom data
    assert(ref_ctx != NULL);
    const data_t *ref_result = ar_expression_evaluate(ref_ctx);
    assert(ref_result != NULL);
    assert(ar_data_get_integer(ref_result) == 200);
    
    // Clean up our custom data (fixture tracks the context)
    ar_data_destroy(own_memory);
    ar_foundation_test_fixture_destroy(own_fixture);
    
    printf("✓ Custom expression context tests passed\n");
}

static void test_map_creation(void) {
    printf("Testing test map creation...\n");
    
    // Given a foundation test fixture
    foundation_test_fixture_t *own_fixture = ar_foundation_test_fixture_create("map_test");
    assert(own_fixture != NULL);
    
    // When we create a user map
    data_t *ref_user = ar_foundation_test_fixture_create_test_map(own_fixture, "user");
    
    // Then it should have the expected values
    assert(ref_user != NULL);
    assert(strcmp(ar_data_get_map_string(ref_user, "username"), "alice") == 0);
    assert(strcmp(ar_data_get_map_string(ref_user, "role"), "admin") == 0);
    assert(ar_data_get_map_integer(ref_user, "id") == 123);
    
    // When we create a config map
    data_t *ref_config = ar_foundation_test_fixture_create_test_map(own_fixture, "config");
    
    // Then it should have different values
    assert(ref_config != NULL);
    assert(strcmp(ar_data_get_map_string(ref_config, "mode"), "test") == 0);
    assert(ar_data_get_map_integer(ref_config, "timeout") == 30);
    
    // Clean up (fixture tracks all maps)
    ar_foundation_test_fixture_destroy(own_fixture);
    
    printf("✓ Map creation tests passed\n");
}

static void test_list_creation(void) {
    printf("Testing test list creation...\n");
    
    // Given a foundation test fixture
    foundation_test_fixture_t *own_fixture = ar_foundation_test_fixture_create("list_test");
    assert(own_fixture != NULL);
    
    // When we create a test list
    data_t *ref_list = ar_foundation_test_fixture_create_test_list(own_fixture);
    
    // Then it should have the expected values
    assert(ref_list != NULL);
    // For now, just verify the list was created
    assert(ar_data_get_type(ref_list) == DATA_LIST);
    
    // Clean up
    ar_foundation_test_fixture_destroy(own_fixture);
    
    printf("✓ List creation tests passed\n");
}

static void test_resource_tracking(void) {
    printf("Testing resource tracking...\n");
    
    // Given a foundation test fixture
    foundation_test_fixture_t *own_fixture = ar_foundation_test_fixture_create("tracking_test");
    assert(own_fixture != NULL);
    
    // When we create data outside the fixture
    data_t *own_external_map = ar_data_create_map();
    ar_data_set_map_string(own_external_map, "external", "data");
    
    // And track it with the fixture
    ar_foundation_test_fixture_track_data(own_fixture, own_external_map);
    own_external_map = NULL; // Ownership transferred
    
    // When we create an expression context outside
    expression_context_t *own_external_ctx = ar_expression_create_context(
        NULL, NULL, NULL, "42"
    );
    ar_foundation_test_fixture_track_expression_context(own_fixture, own_external_ctx);
    own_external_ctx = NULL; // Ownership transferred
    
    // Then fixture destruction should clean up everything
    ar_foundation_test_fixture_destroy(own_fixture);
    
    printf("✓ Resource tracking tests passed\n");
}

static void test_memory_check(void) {
    printf("Testing memory check function...\n");
    
    // Given a foundation test fixture
    foundation_test_fixture_t *own_fixture = ar_foundation_test_fixture_create("memory_test");
    assert(own_fixture != NULL);
    
    // Create some test resources
    data_t *ref_map = ar_foundation_test_fixture_create_test_map(own_fixture, NULL);
    assert(ref_map != NULL);
    
    expression_context_t *ref_ctx = ar_foundation_test_fixture_create_expression_context(
        own_fixture, "1 + 1"
    );
    assert(ref_ctx != NULL);
    
    // When we check memory
    bool result = ar_foundation_test_fixture_check_memory(own_fixture);
    
    // Then it should pass (no leaks expected)
    assert(result == true);
    
    // Clean up
    ar_foundation_test_fixture_destroy(own_fixture);
    
    printf("✓ Memory check tests passed\n");
}

int main(void) {
    printf("Running foundation test fixture tests...\n\n");
    
    test_fixture_create_destroy();
    test_expression_context_creation();
    test_custom_expression_context();
    test_map_creation();
    test_list_creation();
    test_resource_tracking();
    test_memory_check();
    
    printf("\nAll foundation test fixture tests passed!\n");
    return 0;
}
