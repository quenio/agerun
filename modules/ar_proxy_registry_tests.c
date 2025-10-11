#include "ar_proxy_registry.h"
#include "ar_proxy.h"
#include "ar_log.h"
#include "ar_assert.h"
#include <stdio.h>
#include <stdlib.h>

/* Test function prototypes */
static void test_proxy_registry__create_and_destroy(void);
static void test_proxy_registry__register_and_find(void);
static void test_proxy_registry__unregister(void);
static void test_proxy_registry__count(void);
static void test_proxy_registry__duplicate_registration(void);
static void test_proxy_registry__clear(void);

static void test_proxy_registry__create_and_destroy(void) {
    printf("  test_proxy_registry__create_and_destroy...\n");

    // Given we want to create a proxy registry
    // When we create the registry
    ar_proxy_registry_t *own_registry = ar_proxy_registry__create();

    // Then it should be created successfully
    AR_ASSERT(own_registry != NULL, "Proxy registry creation should succeed");

    // When we destroy it
    ar_proxy_registry__destroy(own_registry);

    // Then cleanup should complete without errors
    printf("    PASS\n");
}

static void test_proxy_registry__register_and_find(void) {
    printf("  test_proxy_registry__register_and_find...\n");

    // Given a proxy registry and a test proxy
    ar_proxy_registry_t *own_registry = ar_proxy_registry__create();
    AR_ASSERT(own_registry != NULL, "Proxy registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_proxy_t *own_proxy = ar_proxy__create(own_log, "test");
    AR_ASSERT(own_proxy != NULL, "Proxy creation should succeed");

    // When we register the proxy with ID -100
    int64_t proxy_id = -100;
    bool register_result = ar_proxy_registry__register(own_registry, proxy_id, own_proxy);

    // Then registration should succeed
    AR_ASSERT(register_result == true, "Proxy registration should succeed");

    // When we find the proxy by ID
    ar_proxy_t *found_proxy = ar_proxy_registry__find(own_registry, proxy_id);

    // Then we should find the same proxy
    AR_ASSERT(found_proxy != NULL, "Should find registered proxy");
    AR_ASSERT(found_proxy == own_proxy, "Found proxy should match original");

    // Clean up - registry should destroy the proxy
    ar_proxy_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_proxy_registry__unregister(void) {
    printf("  test_proxy_registry__unregister...\n");

    // Given a registry with a registered proxy
    ar_proxy_registry_t *own_registry = ar_proxy_registry__create();
    AR_ASSERT(own_registry != NULL, "Proxy registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_proxy_t *own_proxy = ar_proxy__create(own_log, "test");
    AR_ASSERT(own_proxy != NULL, "Proxy creation should succeed");

    int64_t proxy_id = -200;
    bool register_result = ar_proxy_registry__register(own_registry, proxy_id, own_proxy);
    AR_ASSERT(register_result == true, "Proxy registration should succeed");

    // When we unregister the proxy
    bool unregister_result = ar_proxy_registry__unregister(own_registry, proxy_id);

    // Then unregistration should succeed
    AR_ASSERT(unregister_result == true, "Proxy unregistration should succeed");

    // And the proxy should no longer be found
    ar_proxy_t *found_proxy = ar_proxy_registry__find(own_registry, proxy_id);
    AR_ASSERT(found_proxy == NULL, "Should not find unregistered proxy");

    // And it should not be registered
    bool is_registered = ar_proxy_registry__is_registered(own_registry, proxy_id);
    AR_ASSERT(is_registered == false, "Proxy should not be registered after unregister");

    // Clean up
    ar_proxy_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_proxy_registry__count(void) {
    printf("  test_proxy_registry__count...\n");

    // Given an empty registry
    ar_proxy_registry_t *own_registry = ar_proxy_registry__create();
    AR_ASSERT(own_registry != NULL, "Proxy registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    // When we check count
    int count = ar_proxy_registry__count(own_registry);

    // Then count should be zero
    AR_ASSERT(count == 0, "Empty registry should have count 0");

    // When we register proxies
    ar_proxy_t *own_proxy1 = ar_proxy__create(own_log, "test1");
    ar_proxy_t *own_proxy2 = ar_proxy__create(own_log, "test2");
    ar_proxy_t *own_proxy3 = ar_proxy__create(own_log, "test3");

    ar_proxy_registry__register(own_registry, -100, own_proxy1);
    ar_proxy_registry__register(own_registry, -101, own_proxy2);
    ar_proxy_registry__register(own_registry, -102, own_proxy3);

    count = ar_proxy_registry__count(own_registry);

    // Then count should be 3
    AR_ASSERT(count == 3, "Registry with 3 proxies should have count 3");

    // Clean up
    ar_proxy_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_proxy_registry__duplicate_registration(void) {
    printf("  test_proxy_registry__duplicate_registration...\n");

    // Given a registry with a registered proxy
    ar_proxy_registry_t *own_registry = ar_proxy_registry__create();
    AR_ASSERT(own_registry != NULL, "Proxy registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_proxy_t *own_proxy1 = ar_proxy__create(own_log, "test1");
    AR_ASSERT(own_proxy1 != NULL, "First proxy creation should succeed");

    int64_t proxy_id = -300;
    bool register_result1 = ar_proxy_registry__register(own_registry, proxy_id, own_proxy1);
    AR_ASSERT(register_result1 == true, "First proxy registration should succeed");

    // When we try to register another proxy with the same ID
    ar_proxy_t *own_proxy2 = ar_proxy__create(own_log, "test2");
    AR_ASSERT(own_proxy2 != NULL, "Second proxy creation should succeed");

    bool register_result2 = ar_proxy_registry__register(own_registry, proxy_id, own_proxy2);

    // Then the second registration should fail
    AR_ASSERT(register_result2 == false, "Duplicate proxy registration should fail");

    // And the original proxy should still be found
    ar_proxy_t *found_proxy = ar_proxy_registry__find(own_registry, proxy_id);
    AR_ASSERT(found_proxy == own_proxy1, "Original proxy should still be registered");

    // Clean up - destroy the second proxy manually since it wasn't registered
    ar_proxy__destroy(own_proxy2);
    ar_proxy_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_proxy_registry__clear(void) {
    printf("  test_proxy_registry__clear...\n");

    // Given a registry with multiple proxies
    ar_proxy_registry_t *own_registry = ar_proxy_registry__create();
    AR_ASSERT(own_registry != NULL, "Proxy registry creation should succeed");

    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");

    ar_proxy_t *own_proxy1 = ar_proxy__create(own_log, "test1");
    ar_proxy_t *own_proxy2 = ar_proxy__create(own_log, "test2");
    ar_proxy_t *own_proxy3 = ar_proxy__create(own_log, "test3");

    ar_proxy_registry__register(own_registry, -100, own_proxy1);
    ar_proxy_registry__register(own_registry, -101, own_proxy2);
    ar_proxy_registry__register(own_registry, -102, own_proxy3);

    int count_before = ar_proxy_registry__count(own_registry);
    AR_ASSERT(count_before == 3, "Registry should have 3 proxies before clear");

    // When we clear the registry
    ar_proxy_registry__clear(own_registry);

    // Then count should be zero
    int count_after = ar_proxy_registry__count(own_registry);
    AR_ASSERT(count_after == 0, "Registry should have 0 proxies after clear");

    // And none of the proxies should be found
    AR_ASSERT(ar_proxy_registry__find(own_registry, -100) == NULL, "Proxy -100 should not be found");
    AR_ASSERT(ar_proxy_registry__find(own_registry, -101) == NULL, "Proxy -101 should not be found");
    AR_ASSERT(ar_proxy_registry__find(own_registry, -102) == NULL, "Proxy -102 should not be found");

    // Clean up
    ar_proxy_registry__destroy(own_registry);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

int main(void) {
    printf("Running ar_proxy_registry tests...\n");

    test_proxy_registry__create_and_destroy();
    test_proxy_registry__register_and_find();
    test_proxy_registry__unregister();
    test_proxy_registry__count();
    test_proxy_registry__duplicate_registration();
    test_proxy_registry__clear();

    printf("All ar_proxy_registry tests passed!\n");
    return 0;
}
