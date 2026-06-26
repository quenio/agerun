#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ar_assert.h"
#include "ar_data.h"
#include "ar_frame.h"
#include "ar_log.h"
#include "ar_result_binding.h"

static ar_frame_t* _create_frame(
    ar_data_t **out_own_memory,
    ar_data_t **out_own_context,
    ar_data_t **out_own_message
) {
    *out_own_memory = ar_data__create_map();
    *out_own_context = ar_data__create_map();
    *out_own_message = ar_data__create_map();

    AR_ASSERT(*out_own_memory != NULL, "Memory map should be created");
    AR_ASSERT(*out_own_context != NULL, "Context map should be created");
    AR_ASSERT(*out_own_message != NULL, "Message map should be created");

    ar_frame_t *own_frame = ar_frame__create(
        *out_own_memory,
        *out_own_context,
        *out_own_message
    );
    AR_ASSERT(own_frame != NULL, "Frame should be created");
    return own_frame; // Ownership transferred to caller
}

static void _destroy_frame_data(
    ar_frame_t *own_frame,
    ar_data_t *own_memory,
    ar_data_t *own_context,
    ar_data_t *own_message
) {
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_memory);
    ar_data__destroy(own_context);
    ar_data__destroy(own_message);
}

static void test_result_binding__validate_target_accepts_memory_path(void) {
    printf("  test_result_binding__validate_target_accepts_memory_path...\n");

    // Given an effectful result path under memory
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");

    // When validating the target before effectful evaluation
    bool is_valid = ar_result_binding__validate_target(own_log, "memory.result");

    // Then the target should be accepted
    AR_ASSERT(is_valid, "memory.result should be a valid effectful result-binding target");

    // Cleanup
    ar_log__destroy(own_log);
}

static void test_result_binding__validate_target_rejects_memory_self(void) {
    printf("  test_result_binding__validate_target_rejects_memory_self...\n");

    // Given the protected agency-managed memory.self target
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");

    // When validating the target before effectful evaluation
    bool is_valid = ar_result_binding__validate_target(own_log, "memory.self");

    // Then the target should be rejected before side effects run
    AR_ASSERT(!is_valid, "memory.self should be rejected for effectful result binding");
    const char *ref_error = ar_log__get_last_error_message(own_log);
    AR_ASSERT(ref_error != NULL, "Protected target rejection should be logged");
    AR_ASSERT(
        strstr(ref_error, "memory.self is agency-managed") != NULL,
        "Protected target rejection should use the established error message"
    );

    // Cleanup
    ar_log__destroy(own_log);
}

static void test_result_binding__validate_target_rejects_nested_memory_self(void) {
    printf("  test_result_binding__validate_target_rejects_nested_memory_self...\n");

    // Given a nested path under protected agency-managed memory.self
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");

    // When validating the target before effectful evaluation
    bool is_valid = ar_result_binding__validate_target(own_log, "memory.self.agent");

    // Then the nested target should also be rejected
    AR_ASSERT(!is_valid, "memory.self.* should be rejected for effectful result binding");

    // Cleanup
    ar_log__destroy(own_log);
}

static void test_result_binding__bind_transfers_owned_result_to_frame_memory(void) {
    printf("  test_result_binding__bind_transfers_owned_result_to_frame_memory...\n");

    // Given an owned result value and a frame with mutable memory
    ar_data_t *own_memory = NULL;
    ar_data_t *own_context = NULL;
    ar_data_t *own_message = NULL;
    ar_frame_t *own_frame = _create_frame(&own_memory, &own_context, &own_message);
    ar_data_t *own_result = ar_data__create_integer(1);
    AR_ASSERT(own_result != NULL, "Result value should be created");

    // When binding the result into frame memory
    bool did_bind = ar_result_binding__bind(NULL, own_frame, "memory.ok", own_result);
    own_result = NULL;

    // Then memory should own the stored result value
    AR_ASSERT(did_bind, "Result binding should store memory.ok");
    const ar_data_t *ref_stored = ar_data__get_map_data(own_memory, "ok");
    AR_ASSERT(ref_stored != NULL, "Stored result should exist in frame memory");
    AR_ASSERT(ar_data__get_integer(ref_stored) == 1, "Stored result should preserve its value");
    AR_ASSERT(
        ar_data__is_owned_by(ref_stored, own_memory),
        "Stored result ownership should transfer to frame memory"
    );

    // Cleanup
    _destroy_frame_data(own_frame, own_memory, own_context, own_message);
}

static void test_result_binding__bind_rejects_protected_target_and_preserves_memory_self(void) {
    printf("  test_result_binding__bind_rejects_protected_target_and_preserves_memory_self...\n");

    // Given frame memory with the agency-managed self value already present
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log should be created");
    ar_data_t *own_memory = NULL;
    ar_data_t *own_context = NULL;
    ar_data_t *own_message = NULL;
    ar_frame_t *own_frame = _create_frame(&own_memory, &own_context, &own_message);
    ar_data_t *own_self = ar_data__create_integer(7);
    AR_ASSERT(own_self != NULL, "Self value should be created");
    AR_ASSERT(ar_data__set_map_data(own_memory, "self", own_self), "Self value should be stored");
    own_self = NULL;
    ar_data_t *own_result = ar_data__create_integer(99);
    AR_ASSERT(own_result != NULL, "Result value should be created");

    // When binding a result to memory.self
    bool did_bind = ar_result_binding__bind(own_log, own_frame, "memory.self", own_result);
    own_result = NULL;

    // Then binding should fail and the existing self value should remain unchanged
    AR_ASSERT(!did_bind, "Result binding should reject memory.self");
    AR_ASSERT(ar_data__get_map_integer(own_memory, "self") == 7, "memory.self should be preserved");

    // Cleanup
    _destroy_frame_data(own_frame, own_memory, own_context, own_message);
    ar_log__destroy(own_log);
}

static void test_result_binding__bind_destroys_result_when_storage_fails(void) {
    printf("  test_result_binding__bind_destroys_result_when_storage_fails...\n");

    // Given an owned result value and a non-memory binding path
    ar_data_t *own_memory = NULL;
    ar_data_t *own_context = NULL;
    ar_data_t *own_message = NULL;
    ar_frame_t *own_frame = _create_frame(&own_memory, &own_context, &own_message);
    ar_data_t *own_result = ar_data__create_integer(5);
    AR_ASSERT(own_result != NULL, "Result value should be created");

    // When binding the result to a path outside frame memory
    bool did_bind = ar_result_binding__bind(NULL, own_frame, "context.result", own_result);
    own_result = NULL;

    // Then storage should fail without writing to memory
    AR_ASSERT(!did_bind, "Non-memory result binding should fail storage");
    AR_ASSERT(
        ar_data__get_map_data(own_memory, "result") == NULL,
        "Failed result binding should not write to frame memory"
    );

    // Cleanup
    _destroy_frame_data(own_frame, own_memory, own_context, own_message);
}

int main(void) {
    char own_cwd[1024];
    if (getcwd(own_cwd, sizeof(own_cwd)) != NULL) {
        if (!strstr(own_cwd, "/bin/") && !strstr(own_cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", own_cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_result_binding_tests\n");
            return 1;
        }
    }

    printf("Running result binding tests...\n");

    test_result_binding__validate_target_accepts_memory_path();
    test_result_binding__validate_target_rejects_memory_self();
    test_result_binding__validate_target_rejects_nested_memory_self();
    test_result_binding__bind_transfers_owned_result_to_frame_memory();
    test_result_binding__bind_rejects_protected_target_and_preserves_memory_self();
    test_result_binding__bind_destroys_result_when_storage_fails();

    printf("All result binding tests passed!\n");
    return 0;
}
