#include "ar_shell_delegate.h"
#include "ar_shell.h"
#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_methodology.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void test_shell_delegate__create_input_envelope_preserves_text(void);
static void test_shell_delegate__forward_input_queues_wrapped_message(void);
static void test_shell_delegate__process_input_stream_forwards_repeated_lines(void);
static void test_shell_delegate__process_input_stream_renders_labeled_transcript_output(void);
static void test_shell_delegate__process_input_stream_reports_verbose_handoff_details(void);
static void test_shell_delegate__process_input_stream_closes_shell_session_on_eof(void);
static FILE* _create_stream_with_text(const char *ref_text);

int main(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            return 1;
        }
    }

    printf("Running shell delegate tests...\n");
    test_shell_delegate__create_input_envelope_preserves_text();
    test_shell_delegate__forward_input_queues_wrapped_message();
    test_shell_delegate__process_input_stream_forwards_repeated_lines();
    test_shell_delegate__process_input_stream_renders_labeled_transcript_output();
    test_shell_delegate__process_input_stream_reports_verbose_handoff_details();
    test_shell_delegate__process_input_stream_closes_shell_session_on_eof();
    printf("All shell delegate tests passed!\n");
    return 0;
}

static void test_shell_delegate__create_input_envelope_preserves_text(void) {
    ar_data_t *own_envelope;
    const char *ref_text;

    printf("  test_shell_delegate__create_input_envelope_preserves_text...\n");

    own_envelope = ar_shell_delegate__create_input_envelope("spawn(\"echo\", \"1.0.0\", context)");
    AR_ASSERT(own_envelope != NULL, "Delegate should wrap input into an envelope");
    AR_ASSERT(ar_data__get_type(own_envelope) == AR_DATA_TYPE__MAP,
              "Input envelope should be a map");

    ref_text = ar_data__get_map_string(own_envelope, "text");
    AR_ASSERT(ref_text != NULL, "Input envelope should contain the text field");
    AR_ASSERT(strcmp(ref_text, "spawn(\"echo\", \"1.0.0\", context)") == 0,
              "Input envelope should preserve exact input text");

    ar_data__destroy(own_envelope);
    printf("    PASS\n");
}

static void test_shell_delegate__forward_input_queues_wrapped_message(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_shell_session_t *own_session;
    ar_shell_delegate_t *own_delegate;
    ar_data_t *own_message;
    int64_t agent_id;
    const char *ref_text;

    printf("  test_shell_delegate__forward_input_queues_wrapped_message...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    ar_system__init(own_system, NULL, NULL);

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should expose its agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should expose its methodology");
    AR_ASSERT(ar_methodology__register_shell_method(mut_methodology),
              "Shell method should register for shell delegate tests");

    agent_id = ar_agency__create_agent(mut_agency, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION, NULL);
    AR_ASSERT(agent_id > 0, "Receiving agent should be created");

    own_session = ar_shell_session__create(1, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Session creation should succeed");
    AR_ASSERT(ar_shell_session__activate(own_session, agent_id),
              "Session should activate with receiving agent");

    own_delegate = ar_shell_delegate__create(ar_system__get_log(own_system), own_session, agent_id);
    AR_ASSERT(own_delegate != NULL, "Shell delegate creation should succeed");

    AR_ASSERT(ar_shell_delegate__forward_input(own_delegate, own_system, "memory.prompt := \"Ready\""),
              "Delegate should forward wrapped shell input");
    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, agent_id),
              "Receiving agent should have the queued wrapped message");

    own_message = ar_agency__get_agent_message(mut_agency, agent_id);
    AR_ASSERT(own_message != NULL, "Queued shell message should be retrievable");
    ref_text = ar_data__get_map_string(own_message, "text");
    AR_ASSERT(ref_text != NULL, "Queued message should preserve the text field");
    AR_ASSERT(strcmp(ref_text, "memory.prompt := \"Ready\"") == 0,
              "Queued message should preserve exact input text");

    ar_data__destroy(own_message);
    ar_shell_delegate__destroy(own_delegate);
    ar_shell_session__destroy(own_session);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);
    printf("    PASS\n");
}

static void test_shell_delegate__process_input_stream_forwards_repeated_lines(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_shell_session_t *own_session;
    ar_shell_delegate_t *own_delegate;
    ar_data_t *own_first_message;
    ar_data_t *own_second_message;
    FILE *own_input_stream;
    FILE *own_output_stream;
    char ref_output_line[128];
    char *ref_read_result;
    int output_stream_errno;
    int line_read_errno;
    int64_t agent_id;
    size_t processed_count;

    printf("  test_shell_delegate__process_input_stream_forwards_repeated_lines...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    ar_system__init(own_system, NULL, NULL);

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should expose its agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should expose its methodology");
    AR_ASSERT(ar_methodology__register_shell_method(mut_methodology),
              "Shell method should register for shell delegate tests");

    agent_id = ar_agency__create_agent(mut_agency, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION, NULL);
    AR_ASSERT(agent_id > 0, "Receiving agent should be created");

    own_session = ar_shell_session__create(2, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Session creation should succeed");
    AR_ASSERT(ar_shell_session__activate(own_session, agent_id),
              "Session should activate with receiving agent");

    own_delegate = ar_shell_delegate__create(ar_system__get_log(own_system), own_session, agent_id);
    AR_ASSERT(own_delegate != NULL, "Shell delegate creation should succeed");

    own_input_stream = _create_stream_with_text("memory.prompt := \"Ready\"\nspawn(\"echo\", \"1.0.0\", context)\n");
    AR_ASSERT(own_input_stream != NULL, "Input stream creation should succeed");
    if (errno != 0) {
        errno = 0;
    }
    own_output_stream = tmpfile();
    output_stream_errno = errno;
    AR_ASSERT(own_output_stream != NULL, "Output stream creation should succeed");
    AR_ASSERT(output_stream_errno == 0, "Output stream creation should not leave errno set on success");

    processed_count = ar_shell_delegate__process_input_stream(
        own_delegate,
        own_system,
        own_input_stream,
        own_output_stream);
    AR_ASSERT(processed_count == 2,
              "Delegate should keep reading until EOF and process both input lines");

    own_first_message = ar_agency__get_agent_message(mut_agency, agent_id);
    own_second_message = ar_agency__get_agent_message(mut_agency, agent_id);
    AR_ASSERT(own_first_message != NULL, "First queued shell message should exist");
    AR_ASSERT(own_second_message != NULL, "Second queued shell message should exist");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_first_message, "text"), "memory.prompt := \"Ready\"") == 0,
              "First queued message should trim the trailing newline before wrapping");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_second_message, "text"), "spawn(\"echo\", \"1.0.0\", context)") == 0,
              "Second queued message should preserve the next input line exactly");

    rewind(own_output_stream);
    if (errno != 0) {
        errno = 0;
    }
    ref_read_result = fgets(ref_output_line, sizeof(ref_output_line), own_output_stream);
    line_read_errno = errno;
    AR_ASSERT(ref_read_result != NULL,
              "Normal mode should report a handoff acknowledgement for the first line");
    AR_ASSERT(line_read_errno == 0, "First acknowledgement read should not leave errno set on success");
    AR_ASSERT(strcmp(ref_output_line, "handoff ok\n") == 0,
              "Normal mode should acknowledge successful handoff without verbose details");
    if (errno != 0) {
        errno = 0;
    }
    ref_read_result = fgets(ref_output_line, sizeof(ref_output_line), own_output_stream);
    line_read_errno = errno;
    AR_ASSERT(ref_read_result != NULL,
              "Normal mode should report a handoff acknowledgement for the second line");
    AR_ASSERT(line_read_errno == 0, "Second acknowledgement read should not leave errno set on success");
    AR_ASSERT(strcmp(ref_output_line, "handoff ok\n") == 0,
              "Normal mode should acknowledge each repeated input line");

    ar_data__destroy(own_first_message);
    ar_data__destroy(own_second_message);
    fclose(own_input_stream);
    fclose(own_output_stream);
    ar_shell_delegate__destroy(own_delegate);
    ar_shell_session__destroy(own_session);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);
    printf("    PASS\n");
}

static void test_shell_delegate__process_input_stream_renders_labeled_transcript_output(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_shell_session_t *own_session;
    ar_shell_delegate_t *own_delegate;
    FILE *own_input_stream;
    FILE *own_output_stream;
    char ref_output[128];
    int64_t agent_id;
    size_t processed_count;
    size_t output_bytes_read;

    printf("  test_shell_delegate__process_input_stream_renders_labeled_transcript_output...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    ar_system__init(own_system, NULL, NULL);

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should expose its agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should expose its methodology");
    AR_ASSERT(ar_methodology__register_shell_method(mut_methodology),
              "Shell method should register for shell delegate tests");

    agent_id = ar_agency__create_agent(mut_agency, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION, NULL);
    AR_ASSERT(agent_id > 0, "Receiving agent should be created");

    own_session = ar_shell_session__create(13, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Session creation should succeed");
    AR_ASSERT(ar_shell_session__activate(own_session, agent_id),
              "Session should activate with receiving agent");
    ar_shell_session__set_transcript_labels_enabled(own_session, true);

    own_delegate = ar_shell_delegate__create(ar_system__get_log(own_system), own_session, agent_id);
    AR_ASSERT(own_delegate != NULL, "Shell delegate creation should succeed");

    own_input_stream = _create_stream_with_text("send(1, \"hello\")\n");
    AR_ASSERT(own_input_stream != NULL, "Input stream creation should succeed");
    if (errno != 0) {
        errno = 0;
    }
    own_output_stream = tmpfile();
    AR_ASSERT(own_output_stream != NULL, "Output stream creation should succeed");
    AR_ASSERT(errno == 0, "Output stream creation should not leave errno set on success");

    processed_count = ar_shell_delegate__process_input_stream(
        own_delegate,
        own_system,
        own_input_stream,
        own_output_stream);
    AR_ASSERT(processed_count == 1,
              "Labeled transcript mode should still process the accepted input line");

    rewind(own_output_stream);
    if (errno != 0) {
        errno = 0;
    }
    output_bytes_read = fread(ref_output, 1, sizeof(ref_output) - 1, own_output_stream);
    AR_ASSERT(errno == 0, "Labeled transcript output read should not leave errno set on success");
    AR_ASSERT(output_bytes_read > 0, "Labeled transcript output should be readable");
    ref_output[output_bytes_read] = '\0';
    AR_ASSERT(strstr(ref_output, "IN: ") != NULL,
              "Labeled transcript mode should render the IN prompt prefix");
    AR_ASSERT(strstr(ref_output, "OUT: handoff ok\n") != NULL,
              "Labeled transcript mode should prefix handoff acknowledgements with OUT");

    fclose(own_input_stream);
    fclose(own_output_stream);
    ar_shell_delegate__destroy(own_delegate);
    ar_shell_session__destroy(own_session);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);
    printf("    PASS\n");
}

static void test_shell_delegate__process_input_stream_reports_verbose_handoff_details(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_shell_session_t *own_session;
    ar_shell_delegate_t *own_delegate;
    FILE *own_input_stream;
    FILE *own_output_stream;
    char ref_output_line[128];
    char ref_expected_line[128];
    char *ref_read_result;
    int output_stream_errno;
    int line_read_errno;
    int64_t agent_id;

    printf("  test_shell_delegate__process_input_stream_reports_verbose_handoff_details...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    ar_system__init(own_system, NULL, NULL);

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should expose its agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should expose its methodology");
    AR_ASSERT(ar_methodology__register_shell_method(mut_methodology),
              "Shell method should register for shell delegate tests");

    agent_id = ar_agency__create_agent(mut_agency, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION, NULL);
    AR_ASSERT(agent_id > 0, "Receiving agent should be created");

    own_session = ar_shell_session__create(3, AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(own_session != NULL, "Session creation should succeed");
    AR_ASSERT(ar_shell_session__activate(own_session, agent_id),
              "Session should activate with receiving agent");

    own_delegate = ar_shell_delegate__create(ar_system__get_log(own_system), own_session, agent_id);
    AR_ASSERT(own_delegate != NULL, "Shell delegate creation should succeed");

    own_input_stream = _create_stream_with_text("send(1, \"hello\")\n");
    AR_ASSERT(own_input_stream != NULL, "Input stream creation should succeed");
    if (errno != 0) {
        errno = 0;
    }
    own_output_stream = tmpfile();
    output_stream_errno = errno;
    AR_ASSERT(own_output_stream != NULL, "Output stream creation should succeed");
    AR_ASSERT(output_stream_errno == 0, "Output stream creation should not leave errno set on success");

    AR_ASSERT(ar_shell_delegate__process_input_stream(
        own_delegate,
        own_system,
        own_input_stream,
        own_output_stream) == 1,
        "Verbose mode should still process the accepted input line");

    rewind(own_output_stream);
    if (errno != 0) {
        errno = 0;
    }
    ref_read_result = fgets(ref_output_line, sizeof(ref_output_line), own_output_stream);
    line_read_errno = errno;
    AR_ASSERT(ref_read_result != NULL,
              "Verbose mode should emit a handoff acknowledgement");
    AR_ASSERT(line_read_errno == 0, "Verbose acknowledgement read should not leave errno set on success");
    snprintf(ref_expected_line, sizeof(ref_expected_line), "handoff ok agent_id=%lld\n", (long long)agent_id);
    AR_ASSERT(strcmp(ref_output_line, ref_expected_line) == 0,
              "Verbose mode should include receiving-agent detail in the handoff acknowledgement");

    fclose(own_input_stream);
    fclose(own_output_stream);
    ar_shell_delegate__destroy(own_delegate);
    ar_shell_session__destroy(own_session);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);
    printf("    PASS\n");
}

static void test_shell_delegate__process_input_stream_closes_shell_session_on_eof(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_shell_delegate_t *own_delegate;
    ar_system_t *mut_system;
    ar_agency_t *mut_agency;
    FILE *own_input_stream;
    FILE *own_output_stream;
    int64_t agent_id;

    printf("  test_shell_delegate__process_input_stream_closes_shell_session_on_eof...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");

    ref_session = ar_shell__start_session(own_shell, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(ref_session != NULL, "Shell should create a session");

    mut_system = ar_shell__get_system(own_shell);
    AR_ASSERT(mut_system != NULL, "Shell should expose its wrapped system");
    mut_agency = ar_system__get_agency(mut_system);
    AR_ASSERT(mut_agency != NULL, "Shell should expose its wrapped agency");

    agent_id = ar_shell_session__get_agent_id(ref_session);
    AR_ASSERT(agent_id > 0, "Shell session should expose the receiving agent ID");

    own_delegate = ar_shell_delegate__create(ar_system__get_log(mut_system), ref_session, agent_id);
    AR_ASSERT(own_delegate != NULL, "Shell delegate creation should succeed");

    own_input_stream = tmpfile();
    own_output_stream = tmpfile();
    AR_ASSERT(own_input_stream != NULL, "Input stream creation should succeed");
    AR_ASSERT(own_output_stream != NULL, "Output stream creation should succeed");
    rewind(own_input_stream);

    AR_ASSERT(ar_shell_delegate__process_input_stream(
        own_delegate,
        mut_system,
        own_input_stream,
        own_output_stream) == 0,
        "EOF without accepted input should still return zero processed lines");
    AR_ASSERT(!ar_shell_session__is_active(ref_session),
              "EOF should close the shell session through the shell delegate path");
    AR_ASSERT(!ar_agency__agent_exists(mut_agency, agent_id),
              "EOF should destroy the receiving agent immediately");

    fclose(own_input_stream);
    fclose(own_output_stream);
    ar_shell_delegate__destroy(own_delegate);
    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}

static FILE* _create_stream_with_text(const char *ref_text) {
    FILE *own_stream;

    own_stream = tmpfile();
    if (!own_stream) {
        return NULL;
    }

    if (fputs(ref_text, own_stream) == EOF) {
        fclose(own_stream);
        return NULL;
    }

    rewind(own_stream);
    return own_stream;
}
