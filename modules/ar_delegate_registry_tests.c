#include "ar_delegate_registry.h"
#include "ar_delegate.h"
#include "ar_log.h"
#include "ar_assert.h"
#include <stdio.h>
#include <stdlib.h>

/* Test function prototypes */
static void test_delegate_registry__create_and_destroy(void);
static void test_delegate_registry__register_and_find(void);
static void test_delegate_registry__unregister(void);
static void test_delegate_registry__count(void);
static void test_delegate_registry__duplicate_registration(void);
static void test_delegate_registry__clear(void);

static void test_delegate_registry__create_and_destroy(void) {
    printf("  test_delegate_registry__create_and_destroy...\n");

    // Given we want to create a delegate registry
    // When we create the registry
    ar_delegate_registry_t *own_registry = ar_delegate_registry__create();

    // Then it should be created successfully
    AR_ASSERT(own_registry != NULL, "Delegate registry creation should succeed");

    // When we destroy it
    ar_delegate_registry__destroy(own_registry);

    // Then cleanup should complete without errors
    printf("    PASS\n");
}

static void test_delegate_registry__register_and_find(void) {
    printf("  test_delegate_registry__register_and_find...\n");

    // Given a delegate registry and a test delegate
    ar_delegate_registry_t *own_registry = ar_delegate_registry__create();
    AR_ASSERT(own_registry != NULL, "Delegate registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_delegate_t *own_delegate = ar_delegate__create(own_log, "test");
    AR_ASSERT(own_delegate != NULL, "Delegate creation should succeed");

    // When we register the delegate with ID -100
    int64_t delegate_id = -100;
    bool register_result = ar_delegate_registry__register(own_registry, delegate_id, own_delegate);

    // Then registration should succeed
    AR_ASSERT(register_result == true, "Delegate registration should succeed");

    // When we find the delegate by ID
    ar_delegate_t *found_delegate = ar_delegate_registry__find(own_registry, delegate_id);

    // Then we should find the same delegate
    AR_ASSERT(found_delegate != NULL, "Should find registered delegate");
    AR_ASSERT(found_delegate == own_delegate, "Found delegate should match original");

    // Clean up - registry should destroy the delegate
    ar_delegate_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_delegate_registry__unregister(void) {
    printf("  test_delegate_registry__unregister...\n");

    // Given a registry with a registered delegate
    ar_delegate_registry_t *own_registry = ar_delegate_registry__create();
    AR_ASSERT(own_registry != NULL, "Delegate registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_delegate_t *own_delegate = ar_delegate__create(own_log, "test");
    AR_ASSERT(own_delegate != NULL, "Delegate creation should succeed");

    int64_t delegate_id = -200;
    bool register_result = ar_delegate_registry__register(own_registry, delegate_id, own_delegate);
    AR_ASSERT(register_result == true, "Delegate registration should succeed");

    // When we unregister the delegate
    bool unregister_result = ar_delegate_registry__unregister(own_registry, delegate_id);

    // Then unregistration should succeed
    AR_ASSERT(unregister_result == true, "Delegate unregistration should succeed");

    // And the delegate should no longer be found
    ar_delegate_t *found_delegate = ar_delegate_registry__find(own_registry, delegate_id);
    AR_ASSERT(found_delegate == NULL, "Should not find unregistered delegate");

    // And it should not be registered
    bool is_registered = ar_delegate_registry__is_registered(own_registry, delegate_id);
    AR_ASSERT(is_registered == false, "Delegate should not be registered after unregister");

    // Clean up
    ar_delegate_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_delegate_registry__count(void) {
    printf("  test_delegate_registry__count...\n");

    // Given an empty registry
    ar_delegate_registry_t *own_registry = ar_delegate_registry__create();
    AR_ASSERT(own_registry != NULL, "Delegate registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    // When we check count
    int count = ar_delegate_registry__count(own_registry);

    // Then count should be zero
    AR_ASSERT(count == 0, "Empty registry should have count 0");

    // When we register proxies
    ar_delegate_t *own_delegate1 = ar_delegate__create(own_log, "test1");
    ar_delegate_t *own_delegate2 = ar_delegate__create(own_log, "test2");
    ar_delegate_t *own_delegate3 = ar_delegate__create(own_log, "test3");

    ar_delegate_registry__register(own_registry, -100, own_delegate1);
    ar_delegate_registry__register(own_registry, -101, own_delegate2);
    ar_delegate_registry__register(own_registry, -102, own_delegate3);

    count = ar_delegate_registry__count(own_registry);

    // Then count should be 3
    AR_ASSERT(count == 3, "Registry with 3 proxies should have count 3");

    // Clean up
    ar_delegate_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_delegate_registry__duplicate_registration(void) {
    printf("  test_delegate_registry__duplicate_registration...\n");

    // Given a registry with a registered delegate
    ar_delegate_registry_t *own_registry = ar_delegate_registry__create();
    AR_ASSERT(own_registry != NULL, "Delegate registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_delegate_t *own_delegate1 = ar_delegate__create(own_log, "test1");
    AR_ASSERT(own_delegate1 != NULL, "First delegate creation should succeed");

    int64_t delegate_id = -300;
    bool register_result1 = ar_delegate_registry__register(own_registry, delegate_id, own_delegate1);
    AR_ASSERT(register_result1 == true, "First delegate registration should succeed");

    // When we try to register another delegate with the same ID
    ar_delegate_t *own_delegate2 = ar_delegate__create(own_log, "test2");
    AR_ASSERT(own_delegate2 != NULL, "Second delegate creation should succeed");

    bool register_result2 = ar_delegate_registry__register(own_registry, delegate_id, own_delegate2);

    // Then the second registration should fail
    AR_ASSERT(register_result2 == false, "Duplicate delegate registration should fail");

    // And the original delegate should still be found
    ar_delegate_t *found_delegate = ar_delegate_registry__find(own_registry, delegate_id);
    AR_ASSERT(found_delegate == own_delegate1, "Original delegate should still be registered");

    // Clean up - destroy the second delegate manually since it wasn't registered
    ar_delegate__destroy(own_delegate2);
    ar_delegate_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_delegate_registry__clear(void) {
    printf("  test_delegate_registry__clear...\n");

    // Given a registry with multiple proxies
    ar_delegate_registry_t *own_registry = ar_delegate_registry__create();
    AR_ASSERT(own_registry != NULL, "Delegate registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_delegate_t *own_delegate1 = ar_delegate__create(own_log, "test1");
    ar_delegate_t *own_delegate2 = ar_delegate__create(own_log, "test2");
    ar_delegate_t *own_delegate3 = ar_delegate__create(own_log, "test3");

    ar_delegate_registry__register(own_registry, -100, own_delegate1);
    ar_delegate_registry__register(own_registry, -101, own_delegate2);
    ar_delegate_registry__register(own_registry, -102, own_delegate3);

    int count_before = ar_delegate_registry__count(own_registry);
    AR_ASSERT(count_before == 3, "Registry should have 3 proxies before clear");

    // When we clear the registry
    ar_delegate_registry__clear(own_registry);

    // Then count should be zero
    int count_after = ar_delegate_registry__count(own_registry);
    AR_ASSERT(count_after == 0, "Registry should have 0 proxies after clear");

    // And none of the proxies should be found
    AR_ASSERT(ar_delegate_registry__find(own_registry, -100) == NULL, "Delegate -100 should not be found");
    AR_ASSERT(ar_delegate_registry__find(own_registry, -101) == NULL, "Delegate -101 should not be found");
    AR_ASSERT(ar_delegate_registry__find(own_registry, -102) == NULL, "Delegate -102 should not be found");

    // Clean up
    ar_delegate_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

int main(void) {
    printf("Running ar_delegate_registry tests...\n");

    test_delegate_registry__create_and_destroy();
    test_delegate_registry__register_and_find();
    test_delegate_registry__unregister();
    test_delegate_registry__count();
    test_delegate_registry__duplicate_registration();
    test_delegate_registry__clear();

    printf("All ar_delegate_registry tests passed!\n");
    return 0;
}
