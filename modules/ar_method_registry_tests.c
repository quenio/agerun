#include "ar_method_registry.h"
#include "ar_method.h"
#include "ar_list.h"
#include "ar_heap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Forward declarations
static void test_method_registry__create_destroy(void);
static void test_method_registry__register_method(void);
static void test_method_registry__get_counts(void);
static void test_method_registry__find_method_index(void);
static void test_method_registry__get_method_by_exact_match(void);
static void test_method_registry__get_latest_version(void);
static void test_method_registry__unregister_method(void);
static void test_method_registry__method_exists(void);
static void test_method_registry__get_all_methods(void);
static void test_method_registry__storage_growth(void);

static void test_method_registry__create_destroy(void) {
    printf("Testing ar_method_registry__create() and ar_method_registry__destroy()...\n");
    
    // When we create a method registry
    ar_method_registry_t *own_registry = ar_method_registry__create();
    
    // Then the registry should be created successfully
    assert(own_registry != NULL);
    
    // When we destroy the registry
    ar_method_registry__destroy(own_registry);
    own_registry = NULL;
    
    // Then no memory leaks should occur (verified by sanitizer)
    
    printf("test_method_registry__create_destroy passed\n");
}

static void test_method_registry__register_method(void) {
    printf("Testing ar_method_registry__register_method()...\n");
    
    // Given a method registry
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // And a test method
    ar_method_t *own_method = ar_method__create("test_method", "message -> \"Hello\"", "1.0.0");
    assert(own_method != NULL);
    
    // When we register the method
    ar_method_registry__register_method(own_registry, own_method);
    // Ownership transferred to registry
    
    // Then the method should be stored in the registry
    // (We'll verify this in later tests when we add lookup functionality)
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__register_method passed\n");
}

static void test_method_registry__get_counts(void) {
    printf("Testing ar_method_registry__get_unique_name_count()...\n");
    
    // Given an empty registry
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Then the count should be 0
    assert(ar_method_registry__get_unique_name_count(own_registry) == 0);
    
    // When we register a method
    ar_method_t *own_method1 = ar_method__create("method1", "message -> \"Hello\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method1);
    
    // Then the count should be 1
    assert(ar_method_registry__get_unique_name_count(own_registry) == 1);
    
    // When we register another method with a different name
    ar_method_t *own_method2 = ar_method__create("method2", "message -> \"World\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method2);
    
    // Then the count should be 2
    assert(ar_method_registry__get_unique_name_count(own_registry) == 2);
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__get_counts passed\n");
}

static void test_method_registry__find_method_index(void) {
    printf("Testing ar_method_registry__find_method_index()...\n");
    
    // Given a registry with methods
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Initially, no methods should be found
    assert(ar_method_registry__find_method_index(own_registry, "nonexistent") == -1);
    
    // Register first method
    ar_method_t *own_method1 = ar_method__create("hello", "message -> \"Hello\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method1);
    
    // Should find it at index 0
    assert(ar_method_registry__find_method_index(own_registry, "hello") == 0);
    
    // Register second method  
    ar_method_t *own_method2 = ar_method__create("world", "message -> \"World\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method2);
    
    // Should find first at 0, second at 1
    assert(ar_method_registry__find_method_index(own_registry, "hello") == 0);
    assert(ar_method_registry__find_method_index(own_registry, "world") == 1);
    
    // Non-existent method should still return -1
    assert(ar_method_registry__find_method_index(own_registry, "nonexistent") == -1);
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__find_method_index passed\n");
}

static void test_method_registry__get_method_by_exact_match(void) {
    printf("Testing ar_method_registry__get_method_by_exact_match()...\n");
    
    // Given a registry
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Initially, no method should be found
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "test", "1.0.0") == NULL);
    
    // Register a method
    ar_method_t *own_method1 = ar_method__create("test", "message -> \"Test\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method1);
    
    // Should find it by exact match
    ar_method_t *found = ar_method_registry__get_method_by_exact_match(own_registry, "test", "1.0.0");
    assert(found != NULL);
    assert(found == own_method1); // Should be the same instance
    
    // Different version should not match
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "test", "2.0.0") == NULL);
    
    // Different name should not match
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "other", "1.0.0") == NULL);
    
    // Register another version of the same method
    ar_method_t *own_method2 = ar_method__create("test", "message -> \"Test v2\"", "2.0.0");
    ar_method_registry__register_method(own_registry, own_method2);
    
    // Both versions should be retrievable
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "test", "1.0.0") == own_method1);
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "test", "2.0.0") == own_method2);
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__get_method_by_exact_match passed\n");
}

static void test_method_registry__get_latest_version(void) {
    printf("Testing ar_method_registry__get_latest_version()...\n");
    
    // Given a registry
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Initially, no method should be found
    assert(ar_method_registry__get_latest_version(own_registry, "test") == NULL);
    
    // Register version 1.0.0
    ar_method_t *own_v1 = ar_method__create("test", "message -> \"v1\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_v1);
    
    // Latest should be 1.0.0
    assert(ar_method_registry__get_latest_version(own_registry, "test") == own_v1);
    
    // Register version 2.0.0
    ar_method_t *own_v2 = ar_method__create("test", "message -> \"v2\"", "2.0.0");
    ar_method_registry__register_method(own_registry, own_v2);
    
    // Latest should now be 2.0.0
    assert(ar_method_registry__get_latest_version(own_registry, "test") == own_v2);
    
    // Register version 1.5.0 (between existing versions)
    ar_method_t *own_v1_5 = ar_method__create("test", "message -> \"v1.5\"", "1.5.0");
    ar_method_registry__register_method(own_registry, own_v1_5);
    
    // Latest should still be 2.0.0
    assert(ar_method_registry__get_latest_version(own_registry, "test") == own_v2);
    
    // Register version 3.0.0-beta (pre-release)
    ar_method_t *own_v3_beta = ar_method__create("test", "message -> \"v3-beta\"", "3.0.0-beta");
    ar_method_registry__register_method(own_registry, own_v3_beta);
    
    // Latest should be 3.0.0-beta (semver considers pre-release versions)
    assert(ar_method_registry__get_latest_version(own_registry, "test") == own_v3_beta);
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__get_latest_version passed\n");
}

static void test_method_registry__unregister_method(void) {
    printf("Testing ar_method_registry__unregister_method()...\n");
    
    // Given a registry with multiple versions of methods
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Register multiple versions of method "test"
    ar_method_t *own_v1 = ar_method__create("test", "message -> \"v1\"", "1.0.0");
    ar_method_t *own_v2 = ar_method__create("test", "message -> \"v2\"", "2.0.0");
    ar_method_t *own_v3 = ar_method__create("test", "message -> \"v3\"", "3.0.0");
    ar_method_registry__register_method(own_registry, own_v1);
    ar_method_registry__register_method(own_registry, own_v2);
    ar_method_registry__register_method(own_registry, own_v3);
    
    // Register another method
    ar_method_t *own_other = ar_method__create("other", "message -> \"Other\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_other);
    
    // Verify initial state
    assert(ar_method_registry__get_unique_name_count(own_registry) == 2);
    assert(ar_method_registry__find_method_index(own_registry, "test") == 0);
    assert(ar_method_registry__find_method_index(own_registry, "other") == 1);
    
    // When we unregister a specific version
    ar_method_registry__unregister_method(own_registry, "test", "2.0.0");
    
    // Then that version should be gone
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "test", "2.0.0") == NULL);
    
    // But other versions should remain
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "test", "1.0.0") != NULL);
    assert(ar_method_registry__get_method_by_exact_match(own_registry, "test", "3.0.0") != NULL);
    
    // The method name count should remain the same
    assert(ar_method_registry__get_unique_name_count(own_registry) == 2);
    
    // When we unregister all versions of "test"
    ar_method_registry__unregister_method(own_registry, "test", "1.0.0");
    ar_method_registry__unregister_method(own_registry, "test", "3.0.0");
    
    // Then the method should no longer exist
    assert(ar_method_registry__find_method_index(own_registry, "test") == -1);
    assert(ar_method_registry__get_unique_name_count(own_registry) == 1);
    
    // The other method should still be there
    assert(ar_method_registry__find_method_index(own_registry, "other") == 0);
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__unregister_method passed\n");
}

static void test_method_registry__method_exists(void) {
    printf("Testing ar_method_registry__method_exists()...\n");
    
    // Given an empty registry
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Initially, no methods should exist
    assert(ar_method_registry__method_exists(own_registry, "test", NULL) == 0);
    assert(ar_method_registry__method_exists(own_registry, "test", "1.0.0") == 0);
    
    // Register a method
    ar_method_t *own_method = ar_method__create("test", "message -> \"Test\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method);
    
    // The method should exist (any version)
    assert(ar_method_registry__method_exists(own_registry, "test", NULL) == 1);
    
    // The specific version should exist
    assert(ar_method_registry__method_exists(own_registry, "test", "1.0.0") == 1);
    
    // A different version should not exist
    assert(ar_method_registry__method_exists(own_registry, "test", "2.0.0") == 0);
    
    // A different method should not exist
    assert(ar_method_registry__method_exists(own_registry, "other", NULL) == 0);
    
    // Register another version
    ar_method_t *own_method2 = ar_method__create("test", "message -> \"Test v2\"", "2.0.0");
    ar_method_registry__register_method(own_registry, own_method2);
    
    // Both versions should exist
    assert(ar_method_registry__method_exists(own_registry, "test", "1.0.0") == 1);
    assert(ar_method_registry__method_exists(own_registry, "test", "2.0.0") == 1);
    assert(ar_method_registry__method_exists(own_registry, "test", NULL) == 1);
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__method_exists passed\n");
}

static void test_method_registry__get_all_methods(void) {
    printf("Testing ar_method_registry__get_all_methods()...\n");
    
    // Given a registry with multiple methods and versions
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Initially empty
    ar_list_t *own_list = ar_method_registry__get_all_methods(own_registry);
    assert(own_list != NULL);
    assert(ar_list__count(own_list) == 0);
    ar_list__destroy(own_list);
    
    // Register methods
    ar_method_t *own_hello_v1 = ar_method__create("hello", "message -> \"v1\"", "1.0.0");
    ar_method_t *own_hello_v2 = ar_method__create("hello", "message -> \"v2\"", "2.0.0");
    ar_method_t *own_world_v1 = ar_method__create("world", "message -> \"world\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_hello_v1);
    ar_method_registry__register_method(own_registry, own_hello_v2);
    ar_method_registry__register_method(own_registry, own_world_v1);
    
    // Get all methods
    own_list = ar_method_registry__get_all_methods(own_registry);
    assert(own_list != NULL);
    assert(ar_list__count(own_list) == 3); // All 3 methods
    
    // Verify the methods are in the list (order not guaranteed)
    int found_hello_v1 = 0, found_hello_v2 = 0, found_world_v1 = 0;
    void **own_items = ar_list__items(own_list);
    assert(own_items != NULL);
    
    for (size_t i = 0; i < ar_list__count(own_list); i++) {
        ar_method_t *method = (ar_method_t*)own_items[i];
        const char *name = ar_method__get_name(method);
        const char *version = ar_method__get_version(method);
        
        if (strcmp(name, "hello") == 0 && strcmp(version, "1.0.0") == 0) {
            found_hello_v1 = 1;
        } else if (strcmp(name, "hello") == 0 && strcmp(version, "2.0.0") == 0) {
            found_hello_v2 = 1;
        } else if (strcmp(name, "world") == 0 && strcmp(version, "1.0.0") == 0) {
            found_world_v1 = 1;
        }
    }
    assert(found_hello_v1 && found_hello_v2 && found_world_v1);
    
    // Clean up the items array (ownership transferred)
    AR__HEAP__FREE(own_items);
    
    // Clean up
    ar_list__destroy(own_list);
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__get_all_methods passed\n");
}

static void test_method_registry__storage_growth(void) {
    printf("Testing ar_method_registry storage growth...\n");
    
    // Given a registry with initial capacity of 10
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Register more than 10 methods to test growth
    char name_buffer[32];
    char body_buffer[64];
    for (int i = 0; i < 15; i++) {
        snprintf(name_buffer, sizeof(name_buffer), "method_%d", i);
        snprintf(body_buffer, sizeof(body_buffer), "message -> \"Method %d\"", i);
        ar_method_t *own_method = ar_method__create(name_buffer, body_buffer, "1.0.0");
        ar_method_registry__register_method(own_registry, own_method);
    }
    
    // Verify all methods were registered
    assert(ar_method_registry__get_unique_name_count(own_registry) == 15);
    
    // Verify we can still find all methods
    for (int i = 0; i < 15; i++) {
        snprintf(name_buffer, sizeof(name_buffer), "method_%d", i);
        assert(ar_method_registry__find_method_index(own_registry, name_buffer) >= 0);
        assert(ar_method_registry__method_exists(own_registry, name_buffer, "1.0.0") == 1);
    }
    
    // Clean up
    ar_method_registry__destroy(own_registry);
    
    printf("test_method_registry__storage_growth passed\n");
}

int main(void) {
    printf("\n=== Running ar_method_registry tests ===\n");
    
    test_method_registry__create_destroy();
    test_method_registry__register_method();
    test_method_registry__get_counts();
    test_method_registry__find_method_index();
    test_method_registry__get_method_by_exact_match();
    test_method_registry__get_latest_version();
    test_method_registry__unregister_method();
    test_method_registry__method_exists();
    test_method_registry__get_all_methods();
    test_method_registry__storage_growth();
    
    printf("\nAll 10 tests passed!\n");
    return 0;
}