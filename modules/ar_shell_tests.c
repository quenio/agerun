#include "ar_shell.h"
#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_delegation.h"
#include "ar_delegate_registry.h"
#include "ar_methodology.h"
#include "ar_shell_delegate.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void test_shell__create_stores_default_mode_and_system(void);
static void test_shell__start_session_creates_receiving_agent(void);
static void test_shell__start_session_registers_runtime_delegate_and_stores_delegate_id(void);
static void test_shell__runtime_delegate_routes_store_and_load_requests(void);
static void test_shell__receiving_agent_executes_shell_method_after_input_delivery(void);

int main(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            return 1;
        }
    }

    printf("Running shell module tests...\n");
    test_shell__create_stores_default_mode_and_system();
    test_shell__start_session_creates_receiving_agent();
    test_shell__start_session_registers_runtime_delegate_and_stores_delegate_id();
    test_shell__runtime_delegate_routes_store_and_load_requests();
    test_shell__receiving_agent_executes_shell_method_after_input_delivery();
    printf("All shell module tests passed!\n");
    return 0;
}

static void test_shell__create_stores_default_mode_and_system(void) {
    ar_shell_t *own_shell;

    printf("  test_shell__create_stores_default_mode_and_system...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");
    AR_ASSERT(ar_shell__get_default_mode(own_shell) == AR_SHELL_MODE__VERBOSE,
              "Shell should retain the requested default mode");
    AR_ASSERT(ar_shell__get_system(own_shell) != NULL,
              "Shell should wrap an owned system instance");

    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}

static void test_shell__start_session_creates_receiving_agent(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_agency_t *mut_agency;
    int64_t agent_id;

    printf("  test_shell__start_session_creates_receiving_agent...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");

    ref_session = ar_shell__start_session(own_shell, AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(ref_session != NULL, "Shell should create a session");
    AR_ASSERT(ar_shell__get_session_count(own_shell) == 1,
              "Shell should track the created session");
    AR_ASSERT(ar_shell_session__get_mode(ref_session) == AR_SHELL_MODE__VERBOSE,
              "Session should retain the requested mode");
    AR_ASSERT(ar_shell_session__is_active(ref_session),
              "Session should become active after startup");

    agent_id = ar_shell_session__get_agent_id(ref_session);
    AR_ASSERT(agent_id > 0, "Shell startup should create a receiving agent");

    mut_agency = ar_system__get_agency(ar_shell__get_system(own_shell));
    AR_ASSERT(mut_agency != NULL, "Shell should expose its wrapped agency");
    AR_ASSERT(ar_agency__agent_exists(mut_agency, agent_id),
              "Created receiving agent should exist in the wrapped system");

    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}

static void test_shell__start_session_registers_runtime_delegate_and_stores_delegate_id(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_system_t *mut_system;
    ar_agency_t *mut_agency;
    ar_delegate_registry_t *mut_registry;
    ar_data_t *mut_agent_memory;
    int64_t agent_id;
    int delegate_id;

    printf("  test_shell__start_session_registers_runtime_delegate_and_stores_delegate_id...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");

    ref_session = ar_shell__start_session(own_shell, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(ref_session != NULL, "Shell should create a session");

    mut_system = ar_shell__get_system(own_shell);
    AR_ASSERT(mut_system != NULL, "Shell should expose its wrapped system");

    mut_agency = ar_system__get_agency(mut_system);
    AR_ASSERT(mut_agency != NULL, "Shell should expose its wrapped agency");

    agent_id = ar_shell_session__get_agent_id(ref_session);
    AR_ASSERT(agent_id > 0, "Shell session should expose its receiving agent ID");

    mut_agent_memory = ar_agency__get_agent_mutable_memory(mut_agency, agent_id);
    AR_ASSERT(mut_agent_memory != NULL, "Receiving agent should expose mutable memory");
    delegate_id = ar_data__get_map_integer(mut_agent_memory, "shell_session_delegate_id");
    AR_ASSERT(delegate_id < 0, "Shell startup should store a negative runtime delegate ID on the receiving agent");

    mut_registry = ar_system__get_delegate_registry(mut_system);
    AR_ASSERT(mut_registry != NULL, "Shell system should expose the delegate registry");
    AR_ASSERT(ar_delegate_registry__is_registered(mut_registry, delegate_id),
              "Shell startup should register the shell-session runtime delegate");

    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}

static void test_shell__runtime_delegate_routes_store_and_load_requests(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_system_t *mut_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_delegation_t *mut_delegation;
    ar_data_t *mut_shell_agent_memory;
    ar_data_t *own_requester_context;
    ar_data_t *own_store_request;
    ar_data_t *own_load_request;
    ar_data_t *own_reply_message;
    ar_data_t *own_stored_value;
    ar_data_t *ref_loaded_value;
    int64_t shell_agent_id;
    int64_t requester_agent_id;
    int delegate_id;

    printf("  test_shell__runtime_delegate_routes_store_and_load_requests...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");

    ref_session = ar_shell__start_session(own_shell, AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(ref_session != NULL, "Shell should create a session");

    mut_system = ar_shell__get_system(own_shell);
    AR_ASSERT(mut_system != NULL, "Shell should expose its wrapped system");

    mut_agency = ar_system__get_agency(mut_system);
    AR_ASSERT(mut_agency != NULL, "Shell should expose its wrapped agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Shell agency should expose its methodology");
    AR_ASSERT(ar_methodology__create_method(mut_methodology, "reply-sink", "memory.seen := 1", "1.0.0"),
              "Reply-sink method should register for runtime-delegate tests");

    shell_agent_id = ar_shell_session__get_agent_id(ref_session);
    mut_shell_agent_memory = ar_agency__get_agent_mutable_memory(mut_agency, shell_agent_id);
    AR_ASSERT(mut_shell_agent_memory != NULL, "Receiving agent should expose mutable memory");
    delegate_id = ar_data__get_map_integer(mut_shell_agent_memory, "shell_session_delegate_id");
    AR_ASSERT(delegate_id < 0, "Receiving agent should retain the runtime delegate ID");

    own_requester_context = ar_data__create_map();
    AR_ASSERT(own_requester_context != NULL, "Requester context creation should succeed");
    requester_agent_id = ar_agency__create_agent(mut_agency, "reply-sink", "1.0.0", own_requester_context);
    AR_ASSERT(requester_agent_id > 0, "Requester agent should be created");
    ar_data__destroy(own_requester_context);

    mut_delegation = ar_system__get_delegation(mut_system);
    AR_ASSERT(mut_delegation != NULL, "Shell system should expose its delegation");

    own_store_request = ar_data__create_map();
    AR_ASSERT(own_store_request != NULL, "Store request should be created");
    own_stored_value = ar_data__create_string("Ready");
    AR_ASSERT(own_stored_value != NULL, "Store value should be created");
    AR_ASSERT(ar_data__set_map_string(own_store_request, "action", "ar_shell_session__store_value"),
              "Store request should include the action");
    AR_ASSERT(ar_data__set_map_string(own_store_request, "path", "memory.prompt"),
              "Store request should include the session path");
    AR_ASSERT(ar_data__set_map_data(own_store_request, "value", own_stored_value),
              "Store request should include the owned value");
    AR_ASSERT(ar_delegation__send_to_delegate_with_sender(mut_delegation, delegate_id, own_store_request, requester_agent_id),
              "Store request should queue to the shell-session runtime delegate");
    AR_ASSERT(ar_system__process_next_message(mut_system),
              "System should process the queued store request through the runtime delegate");
    AR_ASSERT(strcmp(ar_data__get_map_string(ar_shell_session__get_memory(ref_session), "prompt"), "Ready") == 0,
              "Runtime store requests should update the shell-session memory map");

    own_load_request = ar_data__create_map();
    AR_ASSERT(own_load_request != NULL, "Load request should be created");
    AR_ASSERT(ar_data__set_map_string(own_load_request, "action", "ar_shell_session__load_value"),
              "Load request should include the action");
    AR_ASSERT(ar_data__set_map_string(own_load_request, "path", "memory.prompt"),
              "Load request should include the session path");
    AR_ASSERT(ar_data__set_map_integer(own_load_request, "request_id", 77),
              "Load request should include the request ID");
    AR_ASSERT(ar_delegation__send_to_delegate_with_sender(mut_delegation, delegate_id, own_load_request, requester_agent_id),
              "Load request should queue to the shell-session runtime delegate");
    AR_ASSERT(ar_system__process_next_message(mut_system),
              "System should process the queued load request through the runtime delegate");

    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, requester_agent_id),
              "Load requests should send a reply back to the requesting agent");
    own_reply_message = ar_agency__get_agent_message(mut_agency, requester_agent_id);
    AR_ASSERT(own_reply_message != NULL, "Requester agent should receive a load reply message");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply_message, "action"), "ar_shell_session__return_loaded_value") == 0,
              "Load replies should use the loaded-value action");
    AR_ASSERT(ar_data__get_map_integer(own_reply_message, "request_id") == 77,
              "Load replies should preserve the request correlation ID");
    ref_loaded_value = ar_data__get_map_data(own_reply_message, "value");
    AR_ASSERT(ref_loaded_value != NULL, "Load replies should include the requested value");
    AR_ASSERT(strcmp(ar_data__get_string(ref_loaded_value), "Ready") == 0,
              "Load replies should preserve the stored shell-session value");

    ar_data__destroy(own_reply_message);
    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}

static void test_shell__receiving_agent_executes_shell_method_after_input_delivery(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_system_t *mut_system;
    ar_agency_t *mut_agency;
    ar_data_t *own_input_envelope;
    ar_data_t *mut_agent_memory;
    int64_t agent_id;

    printf("  test_shell__receiving_agent_executes_shell_method_after_input_delivery...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");

    ref_session = ar_shell__start_session(own_shell, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(ref_session != NULL, "Shell should create a session");

    mut_system = ar_shell__get_system(own_shell);
    AR_ASSERT(mut_system != NULL, "Shell should expose its wrapped system");
    mut_agency = ar_system__get_agency(mut_system);
    AR_ASSERT(mut_agency != NULL, "Shell should expose its wrapped agency");

    agent_id = ar_shell_session__get_agent_id(ref_session);
    AR_ASSERT(agent_id > 0, "Shell session should expose its receiving agent ID");

    own_input_envelope = ar_shell_delegate__create_input_envelope("memory.prompt := \"Ready\"");
    AR_ASSERT(own_input_envelope != NULL, "Shell input envelope creation should succeed");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, agent_id, own_input_envelope),
              "Shell input envelope should queue to the receiving agent");
    AR_ASSERT(ar_system__process_next_message(mut_system),
              "System should process the queued shell input message");

    mut_agent_memory = ar_agency__get_agent_mutable_memory(mut_agency, agent_id);
    AR_ASSERT(mut_agent_memory != NULL, "Receiving agent should expose mutable memory");
    AR_ASSERT(ar_data__get_map_string(mut_agent_memory, "last_input") != NULL,
              "Receiving agent should execute the shell method and record the delivered input text");
    AR_ASSERT(strcmp(ar_data__get_map_string(mut_agent_memory, "last_input"), "memory.prompt := \"Ready\"") == 0,
              "Receiving agent should preserve the delivered input text exactly");

    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}
