#include "ar_shell_session.h"
#include "ar_assert.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void test_shell_session__create_initializes_memory_mode_and_transcript_labels(void);
static void test_shell_session__activate_records_agent_id(void);
static void test_shell_session__store_value_updates_owned_memory(void);
static void test_shell_session__load_value_returns_loaded_value_message(void);
static void test_shell_session__load_value_reports_missing_path_failure(void);
static void test_shell_session__render_output_prefixes_labeled_transcript_lines(void);

int main(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            return 1;
        }
    }

    printf("Running shell session tests...\n");
    test_shell_session__create_initializes_memory_mode_and_transcript_labels();
    test_shell_session__activate_records_agent_id();
    test_shell_session__store_value_updates_owned_memory();
    test_shell_session__load_value_returns_loaded_value_message();
    test_shell_session__load_value_reports_missing_path_failure();
    test_shell_session__render_output_prefixes_labeled_transcript_lines();
    printf("All shell session tests passed!\n");
    return 0;
}

static void test_shell_session__create_initializes_memory_mode_and_transcript_labels(void) {
    ar_shell_session_t *own_session;

    printf("  test_shell_session__create_initializes_memory_mode_and_transcript_labels...\n");

    own_session = ar_shell_session__create(7, AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");
    AR_ASSERT(ar_shell_session__get_session_id(own_session) == 7,
              "Shell session should retain its identifier");
    AR_ASSERT(ar_shell_session__get_mode(own_session) == AR_SHELL_MODE__VERBOSE,
              "Shell session should retain its startup mode");
    AR_ASSERT(ar_shell_session__get_memory(own_session) != NULL,
              "Shell session should create an owned session memory map");
    AR_ASSERT(!ar_shell_session__is_active(own_session),
              "Fresh shell session should not be active before activation");
    AR_ASSERT(!ar_shell_session__get_transcript_labels_enabled(own_session),
              "Fresh shell sessions should start with transcript labels disabled");

    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}

static void test_shell_session__activate_records_agent_id(void) {
    ar_shell_session_t *own_session;

    printf("  test_shell_session__activate_records_agent_id...\n");

    own_session = ar_shell_session__create(8, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");
    AR_ASSERT(ar_shell_session__activate(own_session, 42),
              "Activation should succeed with a receiving-agent identifier");
    AR_ASSERT(ar_shell_session__is_active(own_session),
              "Activated shell session should become active");
    AR_ASSERT(ar_shell_session__get_agent_id(own_session) == 42,
              "Activated shell session should retain the receiving-agent identifier");

    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}

static void test_shell_session__store_value_updates_owned_memory(void) {
    ar_shell_session_t *own_session;
    ar_data_t *own_value;

    printf("  test_shell_session__store_value_updates_owned_memory...\n");

    own_session = ar_shell_session__create(9, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");

    own_value = ar_data__create_string("Ready");
    AR_ASSERT(own_value != NULL, "Shell session value creation should succeed");
    AR_ASSERT(ar_shell_session__store_value(own_session, "memory.prompt", own_value),
              "Shell session should store values addressed through the memory root");
    own_value = NULL;

    AR_ASSERT(strcmp(ar_data__get_map_string(ar_shell_session__get_memory(own_session), "prompt"), "Ready") == 0,
              "Stored shell-session values should land in the owned session memory map");

    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}

static void test_shell_session__load_value_returns_loaded_value_message(void) {
    ar_shell_session_t *own_session;
    ar_data_t *own_value;
    ar_data_t *own_response;
    ar_data_t *ref_loaded_value;

    printf("  test_shell_session__load_value_returns_loaded_value_message...\n");

    own_session = ar_shell_session__create(10, AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");

    own_value = ar_data__create_string("Ready");
    AR_ASSERT(own_value != NULL, "Shell session value creation should succeed");
    AR_ASSERT(ar_shell_session__store_value(own_session, "memory.prompt", own_value),
              "Shell session should store the value before it is loaded");
    own_value = NULL;

    own_response = ar_shell_session__load_value(own_session, "memory.prompt", 77);
    AR_ASSERT(own_response != NULL, "Shell session load should produce a reply map");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_response, "action"), "ar_shell_session__return_loaded_value") == 0,
              "Successful shell session loads should return the loaded-value action");
    AR_ASSERT(ar_data__get_map_integer(own_response, "request_id") == 77,
              "Successful shell session loads should preserve the request correlation value");

    ref_loaded_value = ar_data__get_map_data(own_response, "value");
    AR_ASSERT(ref_loaded_value != NULL, "Successful shell session loads should include the loaded value");
    AR_ASSERT(strcmp(ar_data__get_string(ref_loaded_value), "Ready") == 0,
              "Loaded shell-session values should preserve their stored content");

    ar_data__destroy(own_response);
    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}

static void test_shell_session__load_value_reports_missing_path_failure(void) {
    ar_shell_session_t *own_session;
    ar_data_t *own_response;

    printf("  test_shell_session__load_value_reports_missing_path_failure...\n");

    own_session = ar_shell_session__create(11, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");

    own_response = ar_shell_session__load_value(own_session, "memory.unknown", 91);
    AR_ASSERT(own_response != NULL, "Missing shell-session paths should still return a reply map");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_response, "action"), "ar_shell_session__report_operation_failure") == 0,
              "Missing shell-session paths should return the operation-failure action");
    AR_ASSERT(ar_data__get_map_integer(own_response, "request_id") == 91,
              "Failure replies should preserve the request correlation value");
    AR_ASSERT(ar_data__get_map_string(own_response, "reason") != NULL,
              "Failure replies should include a reason string");

    ar_data__destroy(own_response);
    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}

static void test_shell_session__render_output_prefixes_labeled_transcript_lines(void) {
    ar_shell_session_t *own_session;
    ar_data_t *own_reply;
    FILE *own_output_stream;
    char ref_output[128];
    size_t output_bytes_read;

    printf("  test_shell_session__render_output_prefixes_labeled_transcript_lines...\n");

    own_session = ar_shell_session__create(12, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");
    AR_ASSERT(ar_shell_session__activate(own_session, 33),
              "Shell session activation should succeed before reply rendering");
    ar_shell_session__set_transcript_labels_enabled(own_session, true);
    AR_ASSERT(ar_shell_session__get_transcript_labels_enabled(own_session),
              "Transcript label enabling should persist on the shell session");

    if (errno != 0) {
        errno = 0;
    }
    own_output_stream = tmpfile();
    AR_ASSERT(own_output_stream != NULL, "Output stream creation should succeed");
    AR_ASSERT(errno == 0, "Output stream creation should not leave errno set on success");
    ar_shell_session__bind_output(own_session, own_output_stream);

    own_reply = ar_data__create_string("Ready");
    AR_ASSERT(own_reply != NULL, "Reply creation should succeed");
    AR_ASSERT(ar_shell_session__render_output(own_session, own_reply, 5),
              "Shell session should render labeled reply output successfully");

    rewind(own_output_stream);
    if (errno != 0) {
        errno = 0;
    }
    output_bytes_read = fread(ref_output, 1, sizeof(ref_output) - 1, own_output_stream);
    AR_ASSERT(errno == 0, "Rendered labeled reply read should not leave errno set on success");
    AR_ASSERT(output_bytes_read > 0, "Rendered labeled reply output should be readable");
    ref_output[output_bytes_read] = '\0';
    AR_ASSERT(strcmp(ref_output, "OUT: reply sender_id=5 text=Ready\n") == 0,
              "Labeled shell-session replies should use the OUT prefix");

    ar_data__destroy(own_reply);
    fclose(own_output_stream);
    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}
