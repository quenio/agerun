#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ar_method_resolver.h"
#include "ar_method_registry.h"
#include "ar_method.h"

static void test_method_resolver__create_and_destroy(void) {
    printf("test_method_resolver__create_and_destroy\n");
    
    // Given a method registry
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // When we create a method resolver with the registry
    ar_method_resolver_t *own_resolver = ar_method_resolver__create(own_registry);
    
    // Then the resolver should be created successfully
    assert(own_resolver != NULL);
    
    // And we can destroy the resolver
    ar_method_resolver__destroy(own_resolver);
    
    // And destroy the registry
    ar_method_registry__destroy(own_registry);
}

static void test_method_resolver__resolve_null_version_returns_latest(void) {
    printf("test_method_resolver__resolve_null_version_returns_latest\n");
    
    // Given a registry with multiple versions of a method
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Register multiple versions
    ar_method_t *own_method1 = ar_method__create("test_method", "memory.result = \"v1\"", "1.0.0");
    assert(own_method1 != NULL);
    ar_method_registry__register_method(own_registry, own_method1);
    
    ar_method_t *own_method2 = ar_method__create("test_method", "memory.result = \"v2\"", "2.0.0");
    assert(own_method2 != NULL);
    ar_method_registry__register_method(own_registry, own_method2);
    
    ar_method_t *own_method3 = ar_method__create("test_method", "memory.result = \"v1.5\"", "1.5.0");
    assert(own_method3 != NULL);
    ar_method_registry__register_method(own_registry, own_method3);
    
    // And a resolver using this registry
    ar_method_resolver_t *own_resolver = ar_method_resolver__create(own_registry);
    assert(own_resolver != NULL);
    
    // When we resolve with NULL version
    ar_method_t *ref_resolved = ar_method_resolver__resolve_method(own_resolver, "test_method", NULL);
    
    // Then we should get the latest version (2.0.0)
    assert(ref_resolved != NULL);
    assert(strcmp(ar_method__get_version(ref_resolved), "2.0.0") == 0);
    
    // Clean up
    ar_method_resolver__destroy(own_resolver);
    ar_method_registry__destroy(own_registry);
}

static void test_method_resolver__resolve_full_version_returns_exact_match(void) {
    printf("test_method_resolver__resolve_full_version_returns_exact_match\n");
    
    // Given a registry with multiple versions of a method
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Register multiple versions
    ar_method_t *own_method1 = ar_method__create("test_method", "memory.result = \"v1\"", "1.0.0");
    assert(own_method1 != NULL);
    ar_method_registry__register_method(own_registry, own_method1);
    
    ar_method_t *own_method2 = ar_method__create("test_method", "memory.result = \"v2\"", "2.0.0");
    assert(own_method2 != NULL);
    ar_method_registry__register_method(own_registry, own_method2);
    
    ar_method_t *own_method3 = ar_method__create("test_method", "memory.result = \"v1.5\"", "1.5.0");
    assert(own_method3 != NULL);
    ar_method_registry__register_method(own_registry, own_method3);
    
    // And a resolver using this registry
    ar_method_resolver_t *own_resolver = ar_method_resolver__create(own_registry);
    assert(own_resolver != NULL);
    
    // When we resolve with full version "1.5.0"
    ar_method_t *ref_resolved = ar_method_resolver__resolve_method(own_resolver, "test_method", "1.5.0");
    
    // Then we should get exact match
    assert(ref_resolved != NULL);
    assert(strcmp(ar_method__get_version(ref_resolved), "1.5.0") == 0);
    assert(strcmp(ar_method__get_instructions(ref_resolved), "memory.result = \"v1.5\"") == 0);
    
    // Clean up
    ar_method_resolver__destroy(own_resolver);
    ar_method_registry__destroy(own_registry);
}

static void test_method_resolver__resolve_single_digit_partial_version(void) {
    printf("test_method_resolver__resolve_single_digit_partial_version\n");
    
    // Given a registry with multiple versions across major versions
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Register versions: 1.0.0, 1.2.0, 1.5.3, 2.0.0, 2.1.0
    ar_method_t *own_method1 = ar_method__create("test_method", "memory.result = \"v1.0\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method1);
    
    ar_method_t *own_method2 = ar_method__create("test_method", "memory.result = \"v1.2\"", "1.2.0");
    ar_method_registry__register_method(own_registry, own_method2);
    
    ar_method_t *own_method3 = ar_method__create("test_method", "memory.result = \"v1.5.3\"", "1.5.3");
    ar_method_registry__register_method(own_registry, own_method3);
    
    ar_method_t *own_method4 = ar_method__create("test_method", "memory.result = \"v2.0\"", "2.0.0");
    ar_method_registry__register_method(own_registry, own_method4);
    
    ar_method_t *own_method5 = ar_method__create("test_method", "memory.result = \"v2.1\"", "2.1.0");
    ar_method_registry__register_method(own_registry, own_method5);
    
    // And a resolver using this registry
    ar_method_resolver_t *own_resolver = ar_method_resolver__create(own_registry);
    assert(own_resolver != NULL);
    
    // When we resolve with partial version "1"
    ar_method_t *ref_resolved = ar_method_resolver__resolve_method(own_resolver, "test_method", "1");
    
    // Then we should get the latest 1.x.x version (1.5.3)
    assert(ref_resolved != NULL);
    assert(strcmp(ar_method__get_version(ref_resolved), "1.5.3") == 0);
    
    // When we resolve with partial version "2"
    ref_resolved = ar_method_resolver__resolve_method(own_resolver, "test_method", "2");
    
    // Then we should get the latest 2.x.x version (2.1.0)
    assert(ref_resolved != NULL);
    assert(strcmp(ar_method__get_version(ref_resolved), "2.1.0") == 0);
    
    // Clean up
    ar_method_resolver__destroy(own_resolver);
    ar_method_registry__destroy(own_registry);
}

static void test_method_resolver__resolve_two_digit_partial_version(void) {
    printf("test_method_resolver__resolve_two_digit_partial_version\n");
    
    // Given a registry with multiple versions in the same minor version
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Register versions: 1.2.0, 1.2.3, 1.2.7, 1.3.0, 1.3.5
    ar_method_t *own_method1 = ar_method__create("test_method", "memory.result = \"v1.2.0\"", "1.2.0");
    ar_method_registry__register_method(own_registry, own_method1);
    
    ar_method_t *own_method2 = ar_method__create("test_method", "memory.result = \"v1.2.3\"", "1.2.3");
    ar_method_registry__register_method(own_registry, own_method2);
    
    ar_method_t *own_method3 = ar_method__create("test_method", "memory.result = \"v1.2.7\"", "1.2.7");
    ar_method_registry__register_method(own_registry, own_method3);
    
    ar_method_t *own_method4 = ar_method__create("test_method", "memory.result = \"v1.3.0\"", "1.3.0");
    ar_method_registry__register_method(own_registry, own_method4);
    
    ar_method_t *own_method5 = ar_method__create("test_method", "memory.result = \"v1.3.5\"", "1.3.5");
    ar_method_registry__register_method(own_registry, own_method5);
    
    // And a resolver using this registry
    ar_method_resolver_t *own_resolver = ar_method_resolver__create(own_registry);
    assert(own_resolver != NULL);
    
    // When we resolve with partial version "1.2"
    ar_method_t *ref_resolved = ar_method_resolver__resolve_method(own_resolver, "test_method", "1.2");
    
    // Then we should get the latest 1.2.x version (1.2.7)
    assert(ref_resolved != NULL);
    assert(strcmp(ar_method__get_version(ref_resolved), "1.2.7") == 0);
    
    // When we resolve with partial version "1.3"
    ref_resolved = ar_method_resolver__resolve_method(own_resolver, "test_method", "1.3");
    
    // Then we should get the latest 1.3.x version (1.3.5)
    assert(ref_resolved != NULL);
    assert(strcmp(ar_method__get_version(ref_resolved), "1.3.5") == 0);
    
    // Clean up
    ar_method_resolver__destroy(own_resolver);
    ar_method_registry__destroy(own_registry);
}

static void test_method_resolver__resolve_non_existent_versions(void) {
    printf("test_method_resolver__resolve_non_existent_versions\n");
    
    // Given a registry with some methods
    ar_method_registry_t *own_registry = ar_method_registry__create();
    assert(own_registry != NULL);
    
    // Register a method
    ar_method_t *own_method1 = ar_method__create("existing_method", "memory.result = \"exists\"", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method1);
    
    // And a resolver using this registry
    ar_method_resolver_t *own_resolver = ar_method_resolver__create(own_registry);
    assert(own_resolver != NULL);
    
    // When we resolve a non-existent method name
    ar_method_t *ref_resolved = ar_method_resolver__resolve_method(own_resolver, "non_existent", NULL);
    
    // Then we should get NULL
    assert(ref_resolved == NULL);
    
    // When we resolve an existing method with non-matching partial version
    ref_resolved = ar_method_resolver__resolve_method(own_resolver, "existing_method", "2");
    
    // Then we should get NULL (no 2.x.x versions exist)
    assert(ref_resolved == NULL);
    
    // When we resolve an existing method with non-existent exact version
    ref_resolved = ar_method_resolver__resolve_method(own_resolver, "existing_method", "1.5.0");
    
    // Then we should get NULL
    assert(ref_resolved == NULL);
    
    // Clean up
    ar_method_resolver__destroy(own_resolver);
    ar_method_registry__destroy(own_registry);
}

int main(void) {
    printf("Running ar_method_resolver tests...\n");
    
    test_method_resolver__create_and_destroy();
    test_method_resolver__resolve_null_version_returns_latest();
    test_method_resolver__resolve_full_version_returns_exact_match();
    test_method_resolver__resolve_single_digit_partial_version();
    test_method_resolver__resolve_two_digit_partial_version();
    test_method_resolver__resolve_non_existent_versions();
    
    printf("All 6 tests passed!\n");
    return 0;
}