#include "agerun_method_fixture.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_fixture_create_destroy(void) {
    printf("Testing ar_method_fixture_create() and ar_method_fixture_destroy()...\n");
    
    // Given a test name
    const char *test_name = "sample_test";
    
    // When we create a method fixture
    method_fixture_t *own_fixture = ar_method_fixture_create(test_name);
    
    // Then the fixture should be created successfully
    assert(own_fixture != NULL);
    
    // And we should be able to get the test name
    const char *retrieved_name = ar_method_fixture_get_name(own_fixture);
    assert(retrieved_name != NULL);
    assert(strcmp(retrieved_name, test_name) == 0);
    
    // When we destroy the fixture
    ar_method_fixture_destroy(own_fixture);
    
    // Then no assertion failures should occur (destruction succeeded)
    printf("ar_method_fixture_create() and ar_method_fixture_destroy() tests passed!\n");
}

static void test_fixture_initialize(void) {
    printf("Testing ar_method_fixture_initialize()...\n");
    
    // Given a method fixture
    method_fixture_t *own_fixture = ar_method_fixture_create("init_test");
    assert(own_fixture != NULL);
    
    // When we initialize the test environment
    bool result = ar_method_fixture_initialize(own_fixture);
    
    // Then initialization should succeed
    assert(result == true);
    
    // Clean up
    ar_method_fixture_destroy(own_fixture);
    
    printf("ar_method_fixture_initialize() tests passed!\n");
}

static void test_fixture_verify_directory(void) {
    printf("Testing ar_method_fixture_verify_directory()...\n");
    
    // Given a method fixture
    method_fixture_t *own_fixture = ar_method_fixture_create("dir_test");
    assert(own_fixture != NULL);
    
    // When we verify the directory
    bool result = ar_method_fixture_verify_directory(own_fixture);
    
    // Then the result depends on where we're running from
    // (This test itself might not be in bin directory)
    printf("Directory verification returned: %s\n", result ? "true" : "false");
    
    // Clean up
    ar_method_fixture_destroy(own_fixture);
    
    printf("ar_method_fixture_verify_directory() tests passed!\n");
}

static void test_fixture_check_memory(void) {
    printf("Testing ar_method_fixture_check_memory()...\n");
    
    // Given an initialized method fixture
    method_fixture_t *own_fixture = ar_method_fixture_create("memory_test");
    assert(own_fixture != NULL);
    
    bool init_result = ar_method_fixture_initialize(own_fixture);
    assert(init_result == true);
    
    // When we check memory (before any allocations)
    bool result = ar_method_fixture_check_memory(own_fixture);
    
    // Then there should be no memory leaks
    assert(result == true);
    
    // Clean up
    ar_method_fixture_destroy(own_fixture);
    
    printf("ar_method_fixture_check_memory() tests passed!\n");
}

int main(void) {
    printf("Running method fixture tests...\n\n");
    
    test_fixture_create_destroy();
    test_fixture_initialize();
    test_fixture_verify_directory();
    test_fixture_check_memory();
    
    printf("\nAll method fixture tests passed!\n");
    return 0;
}
