#include "agerun_method_fixture.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_fixture_create_destroy(void) {
    printf("Testing ar__method_fixture__create() and ar__method_fixture__destroy()...\n");
    
    // Given a test name
    const char *test_name = "sample_test";
    
    // When we create a method fixture
    method_fixture_t *own_fixture = ar__method_fixture__create(test_name);
    
    // Then the fixture should be created successfully
    assert(own_fixture != NULL);
    
    // And we should be able to get the test name
    const char *retrieved_name = ar__method_fixture__get_name(own_fixture);
    assert(retrieved_name != NULL);
    assert(strcmp(retrieved_name, test_name) == 0);
    
    // When we destroy the fixture
    ar__method_fixture__destroy(own_fixture);
    
    // Then no assertion failures should occur (destruction succeeded)
    printf("ar__method_fixture__create() and ar__method_fixture__destroy() tests passed!\n");
}

static void test_fixture_initialize(void) {
    printf("Testing ar__method_fixture__initialize()...\n");
    
    // Given a method fixture
    method_fixture_t *own_fixture = ar__method_fixture__create("init_test");
    assert(own_fixture != NULL);
    
    // When we initialize the test environment
    bool result = ar__method_fixture__initialize(own_fixture);
    
    // Then initialization should succeed
    assert(result == true);
    
    // Clean up
    ar__method_fixture__destroy(own_fixture);
    
    printf("ar__method_fixture__initialize() tests passed!\n");
}

static void test_fixture_verify_directory(void) {
    printf("Testing ar__method_fixture__verify_directory()...\n");
    
    // Given a method fixture
    method_fixture_t *own_fixture = ar__method_fixture__create("dir_test");
    assert(own_fixture != NULL);
    
    // When we verify the directory
    bool result = ar__method_fixture__verify_directory(own_fixture);
    
    // Then the result depends on where we're running from
    // (This test itself might not be in bin directory)
    printf("Directory verification returned: %s\n", result ? "true" : "false");
    
    // Clean up
    ar__method_fixture__destroy(own_fixture);
    
    printf("ar__method_fixture__verify_directory() tests passed!\n");
}

static void test_fixture_check_memory(void) {
    printf("Testing ar__method_fixture__check_memory()...\n");
    
    // Given an initialized method fixture
    method_fixture_t *own_fixture = ar__method_fixture__create("memory_test");
    assert(own_fixture != NULL);
    
    bool init_result = ar__method_fixture__initialize(own_fixture);
    assert(init_result == true);
    
    // When we check memory (before any allocations)
    bool result = ar__method_fixture__check_memory(own_fixture);
    
    // Then there should be no memory leaks
    assert(result == true);
    
    // Clean up
    ar__method_fixture__destroy(own_fixture);
    
    printf("ar__method_fixture__check_memory() tests passed!\n");
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
