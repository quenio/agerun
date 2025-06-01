#include "agerun_system_test_fixture.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_fixture_create_destroy(void) {
    printf("Testing ar_system_test_fixture_create() and ar_system_test_fixture_destroy()...\n");
    
    // Given a test name
    const char *test_name = "sample_module_test";
    
    // When we create a system test fixture
    system_test_fixture_t *own_fixture = ar_system_test_fixture_create(test_name);
    
    // Then the fixture should be created successfully
    assert(own_fixture != NULL);
    
    // And we should be able to get the test name
    const char *retrieved_name = ar_system_test_fixture_get_name(own_fixture);
    assert(retrieved_name != NULL);
    assert(strcmp(retrieved_name, test_name) == 0);
    
    // When we destroy the fixture
    ar_system_test_fixture_destroy(own_fixture);
    
    // Then no assertion failures should occur (destruction succeeded)
    printf("ar_system_test_fixture_create() and ar_system_test_fixture_destroy() tests passed!\n");
}

static void test_fixture_initialize(void) {
    printf("Testing ar_system_test_fixture_initialize()...\n");
    
    // Given a system test fixture
    system_test_fixture_t *own_fixture = ar_system_test_fixture_create("init_test");
    assert(own_fixture != NULL);
    
    // When we initialize the test environment
    bool result = ar_system_test_fixture_initialize(own_fixture);
    
    // Then initialization should succeed
    assert(result == true);
    
    // Clean up
    ar_system_test_fixture_destroy(own_fixture);
    
    printf("ar_system_test_fixture_initialize() tests passed!\n");
}

static void test_fixture_register_method(void) {
    printf("Testing ar_system_test_fixture_register_method()...\n");
    
    // Given an initialized system test fixture
    system_test_fixture_t *own_fixture = ar_system_test_fixture_create("register_test");
    assert(own_fixture != NULL);
    
    bool init_result = ar_system_test_fixture_initialize(own_fixture);
    assert(init_result == true);
    
    // When we register a method
    method_t *ref_method = ar_system_test_fixture_register_method(
        own_fixture,
        "test_method",
        "send(0, \"Test Response\")",
        "1.0.0"
    );
    
    // Then the method should be registered successfully
    assert(ref_method != NULL);
    
    // Clean up
    ar_system_test_fixture_destroy(own_fixture);
    
    printf("ar_system_test_fixture_register_method() tests passed!\n");
}

static void test_fixture_reset_system(void) {
    printf("Testing ar_system_test_fixture_reset_system()...\n");
    
    // Given an initialized system test fixture
    system_test_fixture_t *own_fixture = ar_system_test_fixture_create("reset_test");
    assert(own_fixture != NULL);
    
    bool init_result = ar_system_test_fixture_initialize(own_fixture);
    assert(init_result == true);
    
    // When we reset the system
    ar_system_test_fixture_reset_system(own_fixture);
    
    // Then no assertion failures should occur (reset succeeded)
    // We could register a method again to verify the system is working
    method_t *ref_method = ar_system_test_fixture_register_method(
        own_fixture,
        "test_method_after_reset",
        "send(0, \"After Reset\")",
        "1.0.0"
    );
    assert(ref_method != NULL);
    
    // Clean up
    ar_system_test_fixture_destroy(own_fixture);
    
    printf("ar_system_test_fixture_reset_system() tests passed!\n");
}

static void test_fixture_check_memory(void) {
    printf("Testing ar_system_test_fixture_check_memory()...\n");
    
    // Given an initialized system test fixture
    system_test_fixture_t *own_fixture = ar_system_test_fixture_create("memory_test");
    assert(own_fixture != NULL);
    
    bool init_result = ar_system_test_fixture_initialize(own_fixture);
    assert(init_result == true);
    
    // When we check memory (before any allocations)
    bool result = ar_system_test_fixture_check_memory(own_fixture);
    
    // Then there should be no memory leaks
    assert(result == true);
    
    // Clean up
    ar_system_test_fixture_destroy(own_fixture);
    
    printf("ar_system_test_fixture_check_memory() tests passed!\n");
}

int main(void) {
    printf("Running system test fixture tests...\n\n");
    
    test_fixture_create_destroy();
    test_fixture_initialize();
    test_fixture_register_method();
    test_fixture_reset_system();
    test_fixture_check_memory();
    
    printf("\nAll system test fixture tests passed!\n");
    return 0;
}
