#include "ar_method_registry.h"
#include "ar_method.h"
#include <stdio.h>
#include <assert.h>

// Forward declarations
static void test_method_registry__create_destroy(void);
static void test_method_registry__register_method(void);
static void test_method_registry__get_counts(void);

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

int main(void) {
    printf("\n=== Running ar_method_registry tests ===\n");
    
    test_method_registry__create_destroy();
    test_method_registry__register_method();
    test_method_registry__get_counts();
    
    printf("\nAll 3 tests passed!\n");
    return 0;
}