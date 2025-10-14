/* AgeRun Delegation Tests */
#include "ar_delegation.h"
#include "ar_data.h"
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

static void test_delegation__send_to_delegate_returns_true(void) {
    // Given a delegation with a registered delegate
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending a message to the delegate
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegation__send_to_delegate(own_delegation, -100, own_message);

    // Then send should return true
    AR_ASSERT(result, "Should return true");

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__send_actually_queues_message(void) {
    // Given a delegation with a registered delegate
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending a message via delegation and checking if queued
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("hello"));
    ar_delegate_registry_t *ref_registry = ar_delegation__get_registry(own_delegation);
    ar_delegate_t *ref_found = ar_delegate_registry__find(ref_registry, -100);
    bool has = ar_delegate__has_messages(ref_found);

    // Then the delegate should have the message queued
    AR_ASSERT(has, "Delegate should have queued message");

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__delegate_has_no_messages_initially(void) {
    // Given a delegation with a registered delegate (no messages sent)
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When checking if delegate has messages
    bool has = ar_delegation__delegate_has_messages(own_delegation, -100);

    // Then it should return false
    AR_ASSERT(!has, "Should have no messages initially");

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__delegate_has_messages_after_send(void) {
    // Given a delegation with a delegate that has received a message
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending a message and checking if delegate has messages
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("hello"));
    bool has = ar_delegation__delegate_has_messages(own_delegation, -100);

    // Then it should return true
    AR_ASSERT(has, "Should have messages after send");

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__take_delegate_message_null_when_empty(void) {
    // Given a delegation with a delegate that has no messages
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When taking a message from the delegate
    ar_data_t *own_msg = ar_delegation__take_delegate_message(own_delegation, -100);

    // Then it should return NULL
    AR_ASSERT(own_msg == NULL, "Should return NULL when empty");  // ← FAILS (stub returns fake)

    // Cleanup (no message to destroy - returned NULL)
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__take_delegate_message_returns_message(void) {
    // Given a delegation with a delegate that has a queued message
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("hello"));

    // When taking a message from the delegate
    ar_data_t *own_msg = ar_delegation__take_delegate_message(own_delegation, -100);

    // Then it should return the message
    AR_ASSERT(own_msg != NULL, "Should return message");

    // Cleanup - MUST destroy message (delegate dropped ownership)
    ar_data__destroy(own_msg);
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__send_to_nonexistent_returns_false(void) {
    // Given a delegation with no registered delegates
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);

    // When sending to a non-existent delegate
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegation__send_to_delegate(own_delegation, -999, own_message);

    // Then it should return false
    AR_ASSERT(!result, "Should return false for non-existent delegate");

    // Cleanup
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);
}

static void test_delegation__destroy_with_queued_messages(void) {
    // Given a delegation with a delegate that has queued messages
    ar_log_t *ref_log = ar_log__create();
    ar_delegation_t *own_delegation = ar_delegation__create(ref_log);
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");
    ar_delegation__register_delegate(own_delegation, -100, own_delegate);

    // When sending messages and destroying the delegation
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("msg1"));
    ar_delegation__send_to_delegate(own_delegation, -100, ar_data__create_string("msg2"));
    ar_delegation__destroy(own_delegation);
    ar_log__destroy(ref_log);

    // Then memory report should show zero leaks (verified by memory tracking)
}

int main(void) {
    printf("Running ar_delegation tests...\n");

    test_delegation__create_and_destroy();
    test_delegation__has_registry();
    test_delegation__register_delegate();
    test_delegation__send_to_delegate_returns_true();
    test_delegation__send_actually_queues_message();
    test_delegation__delegate_has_no_messages_initially();
    test_delegation__delegate_has_messages_after_send();
    test_delegation__take_delegate_message_null_when_empty();
    test_delegation__take_delegate_message_returns_message();
    test_delegation__send_to_nonexistent_returns_false();
    test_delegation__destroy_with_queued_messages();

    printf("All ar_delegation tests passed!\n");
    return 0;
}
