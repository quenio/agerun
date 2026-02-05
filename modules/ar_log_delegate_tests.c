#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_log_delegate.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_data.h"
#include "ar_assert.h"

// Test function declarations
static void test_log_delegate__create_and_destroy(void);
static void test_log_delegate__handle_log_message(void);
static void test_log_delegate__filters_by_level(void);
static void test_log_delegate__rejects_agent_id_mismatch(void);
static void test_log_delegate__uses_sender_id_when_agent_id_missing(void);

static ar_data_t* _create_log_message(const char *ref_level, const char *ref_message, int64_t agent_id) {
    ar_data_t *own_message = ar_data__create_map();
    if (!own_message) {
        return NULL;
    }

    if (!ar_data__set_map_string(own_message, "level", ref_level)) {
        ar_data__destroy(own_message);
        return NULL;
    }

    if (!ar_data__set_map_string(own_message, "message", ref_message)) {
        ar_data__destroy(own_message);
        return NULL;
    }

    if (!ar_data__set_map_integer(own_message, "agent_id", (int)agent_id)) {
        ar_data__destroy(own_message);
        return NULL;
    }

    return own_message;
}

int main(void) {
    // Directory check
    char mut_cwd[1024];
    if (getcwd(mut_cwd, sizeof(mut_cwd)) != NULL) {
        if (!strstr(mut_cwd, "/bin/") && !strstr(mut_cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", mut_cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_log_delegate_tests\n");
            return 1;
        }
    }

    printf("Running log_delegate module tests...\n");

    test_log_delegate__create_and_destroy();
    test_log_delegate__handle_log_message();
    test_log_delegate__filters_by_level();
    test_log_delegate__rejects_agent_id_mismatch();
    test_log_delegate__uses_sender_id_when_agent_id_missing();

    printf("All log_delegate tests passed!\n");
    return 0;
}

static void test_log_delegate__create_and_destroy(void) {
    printf("  test_log_delegate__create_and_destroy...\n");

    ar_log_t *own_log = ar_log__create();
    ar_log_delegate_t *own_delegate = ar_log_delegate__create(own_log, "info");

    AR_ASSERT(own_delegate != NULL, "Delegate should be created");

    ar_log_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_log_delegate__handle_log_message(void) {
    printf("  test_log_delegate__handle_log_message...\n");

    ar_log_t *own_log = ar_log__create();
    ar_log_delegate_t *own_delegate = ar_log_delegate__create(own_log, "info");
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = _create_log_message("info", "hello", 123);
    AR_ASSERT(own_message != NULL, "Setup: message created");

    ar_data_t *own_response = ar_log_delegate__handle_message(own_delegate, own_message, 123);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "success") == 0, "Status should be success");

    ar_event_t *ref_event = ar_log__get_last_info(own_log);
    AR_ASSERT(ref_event != NULL, "Log should contain info event");

    const char *ref_message = ar_event__get_message(ref_event);
    AR_ASSERT(ref_message != NULL, "Event should have message");
    AR_ASSERT(strstr(ref_message, "level=info") != NULL, "Message should include level");
    AR_ASSERT(strstr(ref_message, "agent=123") != NULL, "Message should include agent");
    AR_ASSERT(strstr(ref_message, "message=hello") != NULL, "Message should include content");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_log_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_log_delegate__filters_by_level(void) {
    printf("  test_log_delegate__filters_by_level...\n");

    ar_log_t *own_log = ar_log__create();
    ar_log_delegate_t *own_delegate = ar_log_delegate__create(own_log, "warning");
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = _create_log_message("info", "ignored", 42);
    AR_ASSERT(own_message != NULL, "Setup: message created");

    ar_data_t *own_response = ar_log_delegate__handle_message(own_delegate, own_message, 42);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_message = ar_data__get_map_string(own_response, "message");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "error") == 0, "Status should be error");
    AR_ASSERT(ref_message != NULL, "Message should be set");
    AR_ASSERT(strcmp(ref_message, "Log level below minimum") == 0, "Message should mention level");

    ar_event_t *ref_event = ar_log__get_last_info(own_log);
    AR_ASSERT(ref_event == NULL, "Info event should not be logged");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_log_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_log_delegate__rejects_agent_id_mismatch(void) {
    printf("  test_log_delegate__rejects_agent_id_mismatch...\n");

    ar_log_t *own_log = ar_log__create();
    ar_log_delegate_t *own_delegate = ar_log_delegate__create(own_log, "info");
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = _create_log_message("info", "spoofed", 999);
    AR_ASSERT(own_message != NULL, "Setup: message created");

    ar_data_t *own_response = ar_log_delegate__handle_message(own_delegate, own_message, 7);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    const char *ref_message = ar_data__get_map_string(own_response, "message");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "error") == 0, "Status should be error");
    AR_ASSERT(ref_message != NULL, "Message should be set");
    AR_ASSERT(
        strcmp(ref_message, "agent_id does not match sender_id") == 0,
        "Message should indicate sender mismatch");

    ar_event_t *ref_event = ar_log__get_last_info(own_log);
    AR_ASSERT(ref_event == NULL, "Spoofed message should not be logged");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_log_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_log_delegate__uses_sender_id_when_agent_id_missing(void) {
    printf("  test_log_delegate__uses_sender_id_when_agent_id_missing...\n");

    ar_log_t *own_log = ar_log__create();
    ar_log_delegate_t *own_delegate = ar_log_delegate__create(own_log, "info");
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Setup: message created");
    AR_ASSERT(ar_data__set_map_string(own_message, "level", "info"), "Setup: level set");
    AR_ASSERT(ar_data__set_map_string(own_message, "message", "fallback"), "Setup: message set");

    ar_data_t *own_response = ar_log_delegate__handle_message(own_delegate, own_message, 77);
    AR_ASSERT(own_response != NULL, "Response should be returned");

    const char *ref_status = ar_data__get_map_string(own_response, "status");
    AR_ASSERT(ref_status != NULL, "Status should be set");
    AR_ASSERT(strcmp(ref_status, "success") == 0, "Status should be success");

    ar_event_t *ref_event = ar_log__get_last_info(own_log);
    AR_ASSERT(ref_event != NULL, "Message should be logged");

    const char *ref_log_message = ar_event__get_message(ref_event);
    AR_ASSERT(ref_log_message != NULL, "Event should have message");
    AR_ASSERT(strstr(ref_log_message, "agent=77") != NULL, "Log should use trusted sender_id");

    ar_data__destroy(own_response);
    ar_data__destroy(own_message);
    ar_log_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}
