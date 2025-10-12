/* AgeRun Delegation Tests */
#include "ar_delegation.h"
#include "ar_log.h"
#include "ar_assert.h"
#include <stdio.h>

static void test_delegation__create_and_destroy(void) {
    // Given a log instance
    ar_log_t *ref_log = ar_log__create();
    AR_ASSERT(ref_log != NULL, "Log creation should succeed");

    // When we create a delegation
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);

    // Then delegation should be created successfully
    AR_ASSERT(own_delegation != NULL, "Delegation creation should succeed");

    // Clean up
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__has_registry(void) {
    // Given a delegation instance
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *mut_delegation = ar_delegation__create(ref_log);
    AR_ASSERT(mut_delegation != NULL, "Delegation creation should succeed");

    // When we get the registry
    ar_delegate_registry_t *ref_registry = ar_delegation__get_registry(mut_delegation);

    // Then the registry should exist
    AR_ASSERT(ref_registry != NULL, "Delegation should have a registry");

    // Clean up
    ar_delegation__destroy(mut_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__register_delegate(void) {
    // Given a delegation and a delegate
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *mut_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When we register the delegate
    bool result = ar_delegation__register_delegate(mut_delegation, -100, own_delegate);

    // Then registration should succeed
    AR_ASSERT(result, "Delegate registration should succeed");

    // And the delegate should be findable in the registry
    ar_delegate_registry_t *ref_registry = ar_delegation__get_registry(mut_delegation);
    ar_delegate_t *ref_found = ar_delegate_registry__find(ref_registry, -100);
    AR_ASSERT(ref_found != NULL, "Registered delegate should be found in registry");
    AR_ASSERT(ref_found == own_delegate, "Found delegate should be the same delegate we registered");

    // Clean up
    ar_delegation__destroy(mut_delegation);
    ar_log__destroy(ref_log);
}

int main(void) {
    printf("Running ar_delegation tests...\n");

    test_delegation__create_and_destroy();
    test_delegation__has_registry();
    test_delegation__register_delegate();

    printf("All ar_delegation tests passed!\n");
    return 0;
}
