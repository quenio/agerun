#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_agent_registry.h"
#include "ar_delegate.h"
#include "ar_delegate_registry.h"
#include "ar_delegation.h"
#include "ar_shell.h"
#include "ar_shell_delegate.h"
#include "ar_assert.h"
#include "ar_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

/* Message strings */
static const char *g_test_message = "test_message";

typedef struct test_system_delegate_state_s {
    bool was_called;
    int64_t sender_id;
    char message[32];
} test_system_delegate_state_t;

static bool _test_system_delegate_handler(void *mut_context, ar_data_t *ref_message, int64_t sender_id) {
    test_system_delegate_state_t *mut_state = mut_context;

    if (!mut_state || !ref_message || ar_data__get_type(ref_message) != AR_DATA_TYPE__STRING) {
        return false;
    }

    mut_state->was_called = true;
    mut_state->sender_id = sender_id;
    snprintf(mut_state->message, sizeof(mut_state->message), "%s", ar_data__get_string(ref_message));
    return true;
}

static void _destroy_test_system_delegate_state(void *own_context) {
    AR__HEAP__FREE(own_context);
}

/* Test function prototypes */
static void test_no_auto_loading_on_init(void);
static void test_method_creation(ar_system_t *mut_system);
static void test_agent_creation(ar_system_t *mut_system);
static void test_message_passing(ar_system_t *mut_system);
static void test_system__has_delegation(void);
static void test_system__processes_delegate_messages(void);
static void test_system__processes_delegate_messages_with_agent_sender_id(void);
static void test_system__preserves_agency_assigned_memory_self(void);
static void test_system__closed_shell_session_delegate_discards_late_reply(void);
static void test_message_forwarding__whole_message_reuses_pointer(void);
static void test_message_forwarding__message_field_still_copies(void);
static void test_system__init_can_create_shell_agent_after_registration(void);

static void test_no_auto_loading_on_init(void) {
    printf("Testing that system does NOT auto-load files on init...\n");
    
    // Given we want to verify no file loading happens during init
    // We'll redirect stdout to capture any warning messages
    
    // Save original stdout
    fflush(stdout);
    int original_stdout = dup(1);
    AR_ASSERT(original_stdout != -1, "Could not duplicate stdout for capture");
    
    // Create a pipe to capture output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        close(original_stdout);
        AR_ASSERT(false, "Could not create pipe for stdout capture");
    }
    
    // Redirect stdout to pipe
    if (dup2(pipefd[1], 1) == -1) {
        close(original_stdout);
        close(pipefd[0]);
        close(pipefd[1]);
        AR_ASSERT(false, "Could not redirect stdout to pipe");
    }
    close(pipefd[1]);
    
    // When we create and init a system
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");
    
    ar_system__init(mut_system, NULL, NULL);
    
    // Restore stdout
    fflush(stdout);
    if (dup2(original_stdout, 1) == -1) {
        // Can't restore stdout, but test should still fail appropriately
    }
    close(original_stdout);
    
    // Read captured output
    char buffer[1024] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    
    // Then verify NO loading warnings were printed
    bool loading_attempted = (strstr(buffer, "Warning: Could not load") != NULL);
    if (loading_attempted) {
        printf("Captured output: %s\n", buffer);
    }
    AR_ASSERT(!loading_attempted, "System should NOT attempt to load files during init");
    
    // Clean up
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);
    
    // Remove temp files
    remove("methodology.agerun");
    remove("agerun.agency");
    
    printf("No auto-loading test passed!\n");
}

static void test_method_creation(ar_system_t *mut_system) {
    printf("Testing method creation...\n");
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Given we want to create a new method
    const char *method_name = "test_method";
    const char *method_body = "send(0, \"Hello, World!\")";
    
    // When we create the method
    ar_method_t *own_method = ar_method__create(method_name, method_body, "1.0.0");
    
    // Then the method should be created successfully
    assert(own_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create a new version of the same method
    const char *updated_body = "send(0, \"Hello, Updated World!\")";
    ar_method_t *own_method2 = ar_method__create(method_name, updated_body, "2.0.0");
    
    // Then the method should be created successfully
    assert(own_method2 != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_method2);
    own_method2 = NULL; // Mark as transferred
    
    // For test purposes, we use version "2.0.0"
    const char *version2 = "2.0.0";
    
    // And the new version should be different from the previous version
    assert(strcmp(version2, version) != 0);
    
    printf("Method creation test passed.\n");
}

static void test_agent_creation(ar_system_t *mut_system) {
    printf("Testing agent creation...\n");
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Given we have a method for an agent
    const char *method_name = "agent_test";
    const char *method_body = "send(0, \"Agent created\")";
    
    // Create method and register it with the system's methodology 
    ar_method_t *own_method = ar_method__create(method_name, method_body, "1.0.0");
    assert(own_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an agent with this method using the system's agency
    int64_t agent_id = ar_agency__create_agent(mut_agency, method_name, version, NULL);
    
    // Then the agent should be created successfully
    assert(agent_id > 0);
    
    // And the agent should exist in the system
    ar_agent_registry_t *ref_registry = ar_agency__get_registry(mut_agency);
    assert(ar_agent_registry__is_registered(ref_registry, agent_id));
    
    // When we send a message to the agent
    ar_data_t *test_message = ar_data__create_string(g_test_message);
    assert(test_message != NULL);
    bool send_result = ar_agency__send_to_agent(mut_agency, agent_id, test_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the test message
    ar_system__process_next_message(mut_system);
    
    // When we destroy the agent
    ar_agency__destroy_agent(mut_agency, agent_id);
    
    // Then the destruction should succeed
    // Agency destroy returns void
    
    // And the agent should no longer exist in the system
    assert(!ar_agent_registry__is_registered(ref_registry, agent_id));
    
    printf("Agent creation test passed.\n");
}

static void test_system__init_can_create_shell_agent_after_registration(void) {
    printf("Testing system init can create shell agent after built-in registration...\n");

    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);

    ar_agency_t *mut_agency = ar_system__get_agency(own_system);
    assert(mut_agency != NULL);

    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    assert(mut_methodology != NULL);
    assert(ar_methodology__register_shell_method(mut_methodology));

    int64_t shell_agent_id = ar_system__init(own_system, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION);
    assert(shell_agent_id > 0);
    assert(ar_agency__agent_exists(mut_agency, shell_agent_id));

    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);

    printf("System shell-agent init test passed.\n");
}

static void test_system__closed_shell_session_delegate_discards_late_reply(void) {
    printf("Testing closed shell-session delegate discards late replies...\n");

    ar_shell_t *own_shell = ar_shell__create(AR_SHELL_MODE__NORMAL);
    assert(own_shell != NULL);

    ar_shell_session_t *ref_session = ar_shell__start_session(own_shell, AR_SHELL_MODE__NORMAL);
    assert(ref_session != NULL);

    ar_system_t *mut_system = ar_shell__get_system(own_shell);
    assert(mut_system != NULL);

    ar_delegation_t *mut_delegation = ar_system__get_delegation(mut_system);
    assert(mut_delegation != NULL);

    ar_shell_session__close(ref_session);
    assert(!ar_shell_session__is_active(ref_session));

    ar_data_t *own_late_reply = ar_data__create_string("late");
    assert(own_late_reply != NULL);
    assert(ar_delegation__send_to_delegate_with_sender(
        mut_delegation,
        ar_shell_session__get_runtime_delegate_id(ref_session),
        own_late_reply,
        4242));

    assert(ar_system__process_next_message(mut_system));
    assert(!ar_shell_session__is_active(ref_session));

    ar_shell__destroy(own_shell);
    printf("Closed shell-session delegate discard test passed.\n");
}

static void test_message_passing(ar_system_t *mut_system) {
    printf("Testing message passing between agents...\n");
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Given methods for sender and receiver agents
    // Create and register sender method
    ar_method_t *own_sender_method = ar_method__create("sender", "send(target_id, \"Hello from sender!\")", "1.0.0");
    assert(own_sender_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_sender_method);
    own_sender_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *sender_version = "1.0.0";
    
    // Create and register receiver method
    ar_method_t *own_receiver_method = ar_method__create("receiver", "memory[\"received\"] := \"true\"", "1.0.0");
    assert(own_receiver_method != NULL);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_receiver_method);
    own_receiver_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *receiver_version = "1.0.0";
    
    // And a receiver agent created with the receiver method
    int64_t receiver_id = ar_agency__create_agent(mut_agency, "receiver", receiver_version, NULL);
    assert(receiver_id > 0);
    
    // And a sender agent created with the sender method
    // Note: In the full implementation, a context with receiver ID would be passed
    int64_t sender_id = ar_agency__create_agent(mut_agency, "sender", sender_version, NULL);
    assert(sender_id > 0);
    
    // When we process all pending messages
    // With opaque ar_map_t, we can't rely on the exact count
    ar_system__process_all_messages(mut_system);
    
    // When we clean up the agents
    ar_agency__destroy_agent(mut_agency, sender_id);
    ar_agency__destroy_agent(mut_agency, receiver_id);
    
    // Then the destruction should succeed
    // Agency destroy returns void
    
    printf("Message passing test passed.\n");
}

static void test_no_auto_saving_on_shutdown(void) {
    printf("Testing that system does NOT auto-save on shutdown...\n");

    // Given a clean environment with no existing files
    remove("methodology.agerun");
    remove("agerun.agency");

    // And a system with methodology and agents
    ar_system_t *mut_system = ar_system__create();
    assert(mut_system != NULL);
    ar_system__init(mut_system, NULL, NULL);

    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    assert(mut_agency != NULL);

    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    assert(mut_methodology != NULL);

    // And the methodology has methods
    ar_method_t *own_method = ar_method__create("test_method", "send(0, \"test\")", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method(mut_methodology, own_method);

    // And the agency has active agents
    int64_t agent_id = ar_agency__create_agent(mut_agency, "test_method", "1.0.0", NULL);
    assert(agent_id > 0);

    // When the system is shut down
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);

    // Then no files should be saved
    struct stat st;
    bool methodology_exists = (stat("methodology.agerun", &st) == 0);
    bool agency_exists = (stat("agerun.agency", &st) == 0);

    AR_ASSERT(!methodology_exists, "methodology.agerun should NOT have been saved on shutdown");
    AR_ASSERT(!agency_exists, "agerun.agency should NOT have been saved on shutdown");

    printf("No auto-saving test passed.\n");
}

static void test_system__has_delegate_registry(void) {
    printf("Testing that system has proxy registry...\n");

    // Given a system instance
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");

    // When we get the proxy registry
    ar_delegate_registry_t *ref_registry = ar_system__get_delegate_registry(mut_system);

    // Then the registry should exist
    AR_ASSERT(ref_registry != NULL, "System should have a proxy registry");

    // Clean up
    ar_system__destroy(mut_system);

    printf("Proxy registry test passed.\n");
}

static void test_system__register_proxy(void) {
    printf("Testing proxy registration in system...\n");

    // Given a system instance
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");

    // And a log instance for the proxy
    ar_log_t *ref_log = ar_system__get_log(mut_system);
    AR_ASSERT(ref_log != NULL, "System should have a log");

    // And a test proxy
    ar_delegate_t *own_proxy = ar_delegate__create(ref_log, "test");
    AR_ASSERT(own_proxy != NULL, "Proxy creation should succeed");

    // When we register the proxy with ID -100
    bool result = ar_system__register_delegate(mut_system, -100, own_proxy);

    // Then the registration should succeed
    AR_ASSERT(result, "Proxy registration should succeed");

    // And we should be able to find it in the registry
    ar_delegate_registry_t *ref_registry = ar_system__get_delegate_registry(mut_system);
    ar_delegate_t *ref_found = ar_delegate_registry__find(ref_registry, -100);
    AR_ASSERT(ref_found != NULL, "Registered proxy should be findable");
    AR_ASSERT(strcmp(ar_delegate__get_type(ref_found), "test") == 0, "Found proxy should have correct type");

    // Clean up (system owns proxy now, will destroy it)
    ar_system__destroy(mut_system);

    printf("Proxy registration test passed.\n");
}

static void test_system__has_delegation(void) {
    printf("Testing that system has delegation...\n");

    // Given a system instance
    ar_system_t *mut_system = ar_system__create();
    AR_ASSERT(mut_system != NULL, "System creation should succeed");

    // When we get the delegation
    ar_delegation_t *ref_delegation = ar_system__get_delegation(mut_system);

    // Then the delegation should exist
    AR_ASSERT(ref_delegation != NULL, "System should have a delegation");

    // Clean up
    ar_system__destroy(mut_system);

    printf("Delegation test passed.\n");
}

static void test_system__processes_delegate_messages(void) {
    ar_system_t *own_system;
    ar_log_t *ref_log;
    ar_delegation_t *mut_delegation;
    test_system_delegate_state_t *own_state;
    ar_delegate_t *own_delegate;

    printf("Testing that system processes registered delegate messages...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    AR_ASSERT(ar_system__init(own_system, NULL, NULL) == 0, "System init should succeed");

    ref_log = ar_system__get_log(own_system);
    AR_ASSERT(ref_log != NULL, "System should provide a log");

    mut_delegation = ar_system__get_delegation(own_system);
    AR_ASSERT(mut_delegation != NULL, "System should provide a delegation");

    own_state = AR__HEAP__MALLOC(sizeof(test_system_delegate_state_t), "test system delegate state");
    AR_ASSERT(own_state != NULL, "Delegate state allocation should succeed");
    memset(own_state, 0, sizeof(test_system_delegate_state_t));

    own_delegate = ar_delegate__create_with_handler(
        ref_log,
        "test",
        _test_system_delegate_handler,
        own_state,
        _destroy_test_system_delegate_state
    );
    AR_ASSERT(own_delegate != NULL, "Delegate creation with handler should succeed");
    AR_ASSERT(ar_system__register_delegate(own_system, -120, own_delegate),
              "Delegate registration should succeed");
    AR_ASSERT(ar_delegation__send_to_delegate(mut_delegation, -120, ar_data__create_string("system delegate")),
              "Delegate message send should succeed");

    AR_ASSERT(ar_system__process_next_message(own_system),
              "System should process delegate messages when no agent messages are queued");
    AR_ASSERT(own_state->was_called == true, "Delegate handler should be called by system processing");
    AR_ASSERT(own_state->sender_id == 0, "System delegate processing currently supplies sender ID 0");
    AR_ASSERT(strcmp(own_state->message, "system delegate") == 0,
              "Delegate handler should receive queued message contents");

    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);

    printf("Delegate processing test passed.\n");
}

static void test_system__processes_delegate_messages_with_agent_sender_id(void) {
    ar_system_t *own_system;
    ar_log_t *ref_log;
    ar_delegation_t *mut_delegation;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    test_system_delegate_state_t *own_state;
    ar_delegate_t *own_delegate;
    ar_data_t *own_context;
    ar_data_t *own_trigger_message;
    ar_data_t *mut_agent_memory;
    int64_t agent_id;

    printf("Testing that delegate processing preserves the originating agent ID...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    AR_ASSERT(ar_system__init(own_system, NULL, NULL) == 0,
              "System init without initial agent should succeed");

    ref_log = ar_system__get_log(own_system);
    AR_ASSERT(ref_log != NULL, "System should provide a log");

    mut_delegation = ar_system__get_delegation(own_system);
    AR_ASSERT(mut_delegation != NULL, "System should provide a delegation");

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should provide an agency");

    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should provide a methodology");
    AR_ASSERT(ar_methodology__create_method(mut_methodology, "send-to-delegate", "send(memory.target_id, \"from agent\")", "1.0.0"),
              "Delegate-sender test method should register successfully");

    own_state = AR__HEAP__MALLOC(sizeof(test_system_delegate_state_t), "test system delegate state");
    AR_ASSERT(own_state != NULL, "Delegate state allocation should succeed");
    memset(own_state, 0, sizeof(test_system_delegate_state_t));

    own_delegate = ar_delegate__create_with_handler(
        ref_log,
        "test",
        _test_system_delegate_handler,
        own_state,
        _destroy_test_system_delegate_state
    );
    AR_ASSERT(own_delegate != NULL, "Delegate creation with handler should succeed");
    AR_ASSERT(ar_system__register_delegate(own_system, -121, own_delegate),
              "Delegate registration should succeed");

    own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Delegate-sender context should be created");

    agent_id = ar_agency__create_agent(mut_agency, "send-to-delegate", "1.0.0", own_context);
    AR_ASSERT(agent_id > 0, "Delegate-sender agent should be created");

    mut_agent_memory = ar_agency__get_agent_mutable_memory(mut_agency, agent_id);
    AR_ASSERT(mut_agent_memory != NULL, "Delegate-sender agent should expose mutable memory");
    AR_ASSERT(ar_data__set_map_integer(mut_agent_memory, "target_id", -121),
              "Delegate-sender agent should store the delegate target ID");

    own_trigger_message = ar_data__create_string("go");
    AR_ASSERT(own_trigger_message != NULL, "Trigger message creation should succeed");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, agent_id, own_trigger_message),
              "Trigger message send should succeed");

    AR_ASSERT(ar_system__process_next_message(own_system),
              "System should process the agent message that sends to the delegate");
    AR_ASSERT(ar_system__process_next_message(own_system),
              "System should process the queued delegate message");
    AR_ASSERT(own_state->was_called == true, "Delegate handler should be called by system processing");
    AR_ASSERT(own_state->sender_id == agent_id,
              "Delegate handler should receive the originating agent ID");
    AR_ASSERT(strcmp(own_state->message, "from agent") == 0,
              "Delegate handler should receive the delegated message contents");

    ar_data__destroy(own_context);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);

    printf("Delegate sender-ID preservation test passed.\n");
}

static void test_system__preserves_agency_assigned_memory_self(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_data_t *own_context;
    ar_data_t *own_message;
    ar_data_t *mut_agent_memory;
    int64_t agent_id;

    printf("Testing system processing preserves agency-assigned memory.self...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    AR_ASSERT(ar_system__init(own_system, NULL, NULL) == 0, "System init without initial agent should succeed");

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should provide an agency");

    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should provide a methodology");
    AR_ASSERT(
        ar_methodology__create_method(mut_methodology, "record-self", "memory.observed_self := memory.self", "1.0.0"),
        "Record-self method should be created"
    );

    own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Record-self context should be created");
    agent_id = ar_agency__create_agent(mut_agency, "record-self", "1.0.0", own_context);
    AR_ASSERT(agent_id > 0, "Record-self agent should be created");

    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Trigger message should be created");
    AR_ASSERT(ar_data__set_map_string(own_message, "kind", "no-self-provided"),
              "Trigger message kind should be set");
    AR_ASSERT(ar_data__get_map_data(own_message, "self") == NULL,
              "Test message should not provide self explicitly");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, agent_id, own_message),
              "Trigger message send should succeed");

    AR_ASSERT(ar_system__process_next_message(own_system), "System should process record-self message");

    mut_agent_memory = ar_agency__get_agent_mutable_memory(mut_agency, agent_id);
    AR_ASSERT(mut_agent_memory != NULL, "Record-self memory should be available");
    AR_ASSERT(ar_data__get_map_integer(mut_agent_memory, "self") == (int)agent_id,
              "Agency-assigned memory.self should remain the agent ID");
    AR_ASSERT(ar_data__get_map_integer(mut_agent_memory, "observed_self") == (int)agent_id,
              "Methods should read self from memory.self");

    ar_data__destroy(own_context);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);

    printf("Memory self preservation test passed.\n");
}

static void test_message_forwarding__whole_message_reuses_pointer(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_data_t *own_shared_context;
    ar_data_t *own_initial_message;
    ar_data_t *own_forwarded_message;
    ar_data_t *ref_payload;
    ar_data_t *ref_forwarded_payload;
    ar_data_t *mut_sender_memory;
    int64_t sender_id;
    int64_t receiver_id;

    printf("Testing zero-copy forwarding for send(..., message)...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    AR_ASSERT(ar_system__init(own_system, NULL, NULL) == 0, "System init without initial agent should succeed");

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should provide an agency");

    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should provide a methodology");

    own_shared_context = ar_data__create_map();
    AR_ASSERT(own_shared_context != NULL, "Shared context should be created");

    AR_ASSERT(
        ar_methodology__create_method(mut_methodology, "forward-whole", "send(memory.next_id, message)", "1.0.0"),
        "Forward-whole method should be created"
    );
    AR_ASSERT(
        ar_methodology__create_method(mut_methodology, "receiver-idle", "send(0, \"idle\")", "1.0.0"),
        "Receiver method should be created"
    );

    sender_id = ar_agency__create_agent(mut_agency, "forward-whole", "1.0.0", own_shared_context);
    receiver_id = ar_agency__create_agent(mut_agency, "receiver-idle", "1.0.0", own_shared_context);
    AR_ASSERT(sender_id > 0, "Sender agent should be created");
    AR_ASSERT(receiver_id > 0, "Receiver agent should be created");

    mut_sender_memory = ar_agency__get_agent_mutable_memory(mut_agency, sender_id);
    AR_ASSERT(mut_sender_memory != NULL, "Sender memory should be available");
    AR_ASSERT(
        ar_data__set_map_integer(mut_sender_memory, "next_id", (int)receiver_id),
        "Sender next_id should be set"
    );

    own_initial_message = ar_data__create_map();
    AR_ASSERT(own_initial_message != NULL, "Initial message should be created");
    AR_ASSERT(
        ar_data__set_map_string(own_initial_message, "kind", "forward-whole"),
        "Initial message field should be set"
    );
    ref_payload = ar_data__get_map_data(own_initial_message, "kind");
    AR_ASSERT(ref_payload != NULL, "Initial message payload should exist");

    AR_ASSERT(
        ar_agency__send_to_agent(mut_agency, sender_id, own_initial_message),
        "Initial message send should succeed"
    );

    AR_ASSERT(ar_system__process_next_message(own_system), "Processing sender message should succeed");

    own_forwarded_message = ar_agency__get_agent_message(mut_agency, receiver_id);
    AR_ASSERT(own_forwarded_message != NULL, "Receiver should have forwarded message queued");
    AR_ASSERT(
        own_forwarded_message == own_initial_message,
        "Whole-message forwarding should reuse the original message pointer"
    );

    ref_forwarded_payload = ar_data__get_map_data(own_forwarded_message, "kind");
    AR_ASSERT(ref_forwarded_payload != NULL, "Forwarded message payload should exist");
    AR_ASSERT(
        strcmp(ar_data__get_string(ref_forwarded_payload), "forward-whole") == 0,
        "Forwarded payload content should be preserved"
    );

    ar_data__destroy(own_forwarded_message);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);
    ar_data__destroy(own_shared_context);

    printf("Whole-message forwarding pointer reuse test passed.\n");
}

static void test_message_forwarding__message_field_still_copies(void) {
    ar_system_t *own_system;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_data_t *own_shared_context;
    ar_data_t *own_initial_message;
    ar_data_t *own_forwarded_message;
    ar_data_t *ref_original_payload;
    ar_data_t *mut_sender_memory;
    int64_t sender_id;
    int64_t receiver_id;

    printf("Testing that send(..., message.field) still copies...\n");

    own_system = ar_system__create();
    AR_ASSERT(own_system != NULL, "System creation should succeed");
    AR_ASSERT(ar_system__init(own_system, NULL, NULL) == 0, "System init without initial agent should succeed");

    mut_agency = ar_system__get_agency(own_system);
    AR_ASSERT(mut_agency != NULL, "System should provide an agency");

    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should provide a methodology");

    own_shared_context = ar_data__create_map();
    AR_ASSERT(own_shared_context != NULL, "Shared context should be created");

    AR_ASSERT(
        ar_methodology__create_method(
            mut_methodology,
            "forward-field",
            "send(memory.next_id, message.payload)",
            "1.0.0"
        ),
        "Forward-field method should be created"
    );
    AR_ASSERT(
        ar_methodology__create_method(mut_methodology, "receiver-idle-field", "send(0, \"idle\")", "1.0.0"),
        "Receiver method should be created"
    );

    sender_id = ar_agency__create_agent(mut_agency, "forward-field", "1.0.0", own_shared_context);
    receiver_id = ar_agency__create_agent(mut_agency, "receiver-idle-field", "1.0.0", own_shared_context);
    AR_ASSERT(sender_id > 0, "Sender agent should be created");
    AR_ASSERT(receiver_id > 0, "Receiver agent should be created");

    mut_sender_memory = ar_agency__get_agent_mutable_memory(mut_agency, sender_id);
    AR_ASSERT(mut_sender_memory != NULL, "Sender memory should be available");
    AR_ASSERT(
        ar_data__set_map_integer(mut_sender_memory, "next_id", (int)receiver_id),
        "Sender next_id should be set"
    );

    own_initial_message = ar_data__create_map();
    AR_ASSERT(own_initial_message != NULL, "Initial message should be created");
    AR_ASSERT(
        ar_data__set_map_string(own_initial_message, "payload", "field-copy"),
        "Initial message payload should be set"
    );
    ref_original_payload = ar_data__get_map_data(own_initial_message, "payload");
    AR_ASSERT(ref_original_payload != NULL, "Original payload reference should exist");

    AR_ASSERT(
        ar_agency__send_to_agent(mut_agency, sender_id, own_initial_message),
        "Initial message send should succeed"
    );

    AR_ASSERT(ar_system__process_next_message(own_system), "Processing sender message should succeed");

    own_forwarded_message = ar_agency__get_agent_message(mut_agency, receiver_id);
    AR_ASSERT(own_forwarded_message != NULL, "Receiver should have forwarded payload queued");
    AR_ASSERT(
        own_forwarded_message != ref_original_payload,
        "message.field forwarding should still use copy semantics"
    );
    AR_ASSERT(ar_data__get_type(own_forwarded_message) == AR_DATA_TYPE__STRING, "Forwarded payload should be a string");
    AR_ASSERT(
        strcmp(ar_data__get_string(own_forwarded_message), "field-copy") == 0,
        "Forwarded payload content should be preserved"
    );

    ar_data__destroy(own_forwarded_message);
    ar_system__shutdown(own_system);
    ar_system__destroy(own_system);
    ar_data__destroy(own_shared_context);

    printf("Message field copy behavior test passed.\n");
}

int main(void) {
    printf("Starting Agerun tests...\n");

    // Test that system does NOT auto-load files
    test_no_auto_loading_on_init();

    // Test that system does NOT auto-save files
    test_no_auto_saving_on_shutdown();

    // Test proxy registry integration
    test_system__has_delegate_registry();
    test_system__register_proxy();
    test_system__has_delegation();
    test_system__processes_delegate_messages();
    test_system__processes_delegate_messages_with_agent_sender_id();
    test_system__preserves_agency_assigned_memory_self();
    test_message_forwarding__whole_message_reuses_pointer();
    test_message_forwarding__message_field_still_copies();
    test_system__init_can_create_shell_agent_after_registration();
    test_system__closed_shell_session_delegate_discards_late_reply();

    // Create system instance
    ar_system_t *mut_system = ar_system__create();
    if (mut_system == NULL) {
        printf("Error: Failed to create system instance\n");
        return 1;
    }
    
    // Initialize the system with no initial agent
    ar_system__init(mut_system, NULL, NULL);
    
    // Given we create a test method
    ar_method_t *own_method = ar_method__create("test_init", "send(0, \"Runtime initialized\")", "1.0.0");
    
    // Then the method should be created successfully
    if (own_method == NULL) {
        printf("Error: Failed to create test_init method\n");
        ar_system__shutdown(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Get the system's agency and methodology
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // Register with the system's methodology
    ar_methodology__register_method(mut_methodology, own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0"
    const char *version = "1.0.0";
    
    // When we create an initial agent with this method using the system's agency
    int64_t initial_agent = ar_agency__create_agent(mut_agency, "test_init", version, NULL);
    
    // Then the agent should be created successfully
    if (initial_agent == 0) {
        printf("Error: Failed to create initial agent\n");
        ar_system__shutdown(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // When we run all system tests
    test_method_creation(mut_system);
    test_agent_creation(mut_system);
    test_message_passing(mut_system);
    
    // Clean up the initial agent using the system's agency
    ar_agency__destroy_agent(mut_agency, initial_agent);
    
    // Then clean up the system
    ar_system__shutdown(mut_system);
    ar_system__destroy(mut_system);
    
    // Global cleanup
    /* ar_methodology__cleanup() removed - system handles cleanup */
    // ar_agency__reset(); // Global API removed
    
    // And report success
    printf("All tests passed!\n");
    return 0;
}
