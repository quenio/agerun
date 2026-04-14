#include <stdio.h>
#include <string.h>
#include "ar_method_fixture.h"
#include "ar_agency.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_assert.h"
#include "ar_shell.h"
#include "ar_shell_delegate.h"

static void test_shell__method_asset_loads_and_can_create_agent(void);
static void test_shell__method_redirects_string_assignment_to_shell_session(void);
static void test_shell__method_redirects_assigned_spawn_result_to_shell_session(void);
static void test_shell__method_reports_invalid_syntax_without_closing_session(void);

int main(void) {
    printf("Shell Method Tests\n");
    printf("==================\n\n");

    test_shell__method_asset_loads_and_can_create_agent();
    test_shell__method_redirects_string_assignment_to_shell_session();
    test_shell__method_redirects_assigned_spawn_result_to_shell_session();
    test_shell__method_reports_invalid_syntax_without_closing_session();

    printf("All shell method tests passed!\n");
    return 0;
}

static void test_shell__method_asset_loads_and_can_create_agent(void) {
    ar_method_fixture_t *own_fixture;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_method_t *ref_method;
    int64_t agent_id;

    printf("Testing shell method asset loads and can create an agent...\n");

    own_fixture = ar_method_fixture__create("shell_method_load");
    AR_ASSERT(own_fixture != NULL, "Method fixture should be created");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Method fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Method tests should run from bin");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        AR_SHELL_METHOD_NAME,
        "../../methods/shell-1.0.0.method",
        AR_SHELL_METHOD_VERSION
    ), "Shell method asset should load into the methodology");

    mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Method fixture should expose its agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should expose its methodology");

    ref_method = ar_methodology__get_method(mut_methodology, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION);
    AR_ASSERT(ref_method != NULL, "Shell method should be registered");
    AR_ASSERT(ar_method__get_ast(ref_method) != NULL, "Shell method asset should parse to an AST");

    agent_id = ar_agency__create_agent(mut_agency, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION, NULL);
    AR_ASSERT(agent_id > 0, "Shell method should support creating a receiving agent");
    AR_ASSERT(ar_agency__destroy_agent(mut_agency, agent_id), "Shell agent should be destroyable");

    while (ar_method_fixture__process_next_message(own_fixture)) {
    }
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "Method fixture should report no leaks");
    ar_method_fixture__destroy(own_fixture);
}

static void test_shell__method_redirects_string_assignment_to_shell_session(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_system_t *mut_system;
    ar_agency_t *mut_agency;
    ar_data_t *own_input_envelope;
    int64_t agent_id;

    printf("Testing shell method redirects string assignment to shell session...\n");

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

    own_input_envelope = ar_shell_delegate__create_input_envelope("memory.prompt := \"Ready\"");
    AR_ASSERT(own_input_envelope != NULL, "Input envelope creation should succeed");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, agent_id, own_input_envelope),
              "Shell method input should queue to the receiving agent");

    AR_ASSERT(ar_system__process_all_messages(mut_system) >= 1,
              "System should process the queued shell input interaction");
    AR_ASSERT(ar_data__get_map_string(ar_shell_session__get_memory(ref_session), "prompt") != NULL,
              "String assignment lines should store a prompt value in shell-session memory");
    AR_ASSERT(strcmp(ar_data__get_map_string(ar_shell_session__get_memory(ref_session), "prompt"), "Ready") == 0,
              "String assignment lines should store the assigned value in shell-session memory");

    ar_shell__destroy(own_shell);
}

static void test_shell__method_redirects_assigned_spawn_result_to_shell_session(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_system_t *mut_system;
    ar_agency_t *mut_agency;
    ar_data_t *own_input_envelope;
    int64_t agent_id;
    int64_t spawned_agent_id;

    printf("Testing shell method redirects assigned spawn result to shell session...\n");

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

    own_input_envelope = ar_shell_delegate__create_input_envelope(
        "memory.echo_id := spawn(\"echo\", \"1.0.0\", context)");
    AR_ASSERT(own_input_envelope != NULL, "Input envelope creation should succeed");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, agent_id, own_input_envelope),
              "Assigned spawn input should queue to the receiving agent");

    AR_ASSERT(ar_system__process_all_messages(mut_system) >= 1,
              "System should process the queued assigned-spawn interaction");

    AR_ASSERT(ar_data__get_map_data(ar_shell_session__get_memory(ref_session), "echo_id") != NULL,
              "Assigned spawn lines should store a spawned agent ID in shell-session memory");

    spawned_agent_id = ar_data__get_map_integer(ar_shell_session__get_memory(ref_session), "echo_id");
    AR_ASSERT(spawned_agent_id > 0,
              "Assigned spawn lines should store a positive spawned agent ID in shell-session memory");
    AR_ASSERT(ar_agency__agent_exists(mut_agency, spawned_agent_id),
              "Assigned spawn lines should create the requested runtime agent");

    ar_shell__destroy(own_shell);
}

static void test_shell__method_reports_invalid_syntax_without_closing_session(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_system_t *mut_system;
    ar_agency_t *mut_agency;
    ar_data_t *own_input_envelope;
    const ar_data_t *ref_agent_memory;
    int64_t agent_id;

    printf("Testing shell method reports invalid syntax without closing session...\n");

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

    own_input_envelope = ar_shell_delegate__create_input_envelope("totally invalid shell syntax");
    AR_ASSERT(own_input_envelope != NULL, "Input envelope creation should succeed");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, agent_id, own_input_envelope),
              "Invalid shell input should still queue to the receiving agent");

    AR_ASSERT(ar_system__process_all_messages(mut_system) >= 1,
              "System should process the queued invalid shell input interaction");
    AR_ASSERT(ar_shell_session__is_active(ref_session),
              "Invalid shell syntax should not close the shell session");
    AR_ASSERT(ar_agency__agent_exists(mut_agency, agent_id),
              "Invalid shell syntax should not destroy the receiving agent");

    ref_agent_memory = ar_agency__get_agent_memory(mut_agency, agent_id);
    AR_ASSERT(ref_agent_memory != NULL, "Receiving agent should expose agent memory after invalid syntax");
    AR_ASSERT(ar_data__get_map_string(ref_agent_memory, "last_error") != NULL,
              "Invalid shell syntax should record a recoverable error for later reporting");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_agent_memory, "last_error"), "Invalid shell syntax") == 0,
              "Invalid shell syntax should preserve the expected recoverable error message");

    ar_shell__destroy(own_shell);
}
