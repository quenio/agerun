#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_method.h"
#include "ar_method_fixture.h"
#include "ar_methodology.h"

static void verify_method_parses(ar_methodology_t *mut_methodology, const char *ref_name) {
    ar_method_t *ref_method = ar_methodology__get_method(mut_methodology, ref_name, "1.0.0");
    AR_ASSERT(ref_method != NULL, "Method should be registered");
    AR_ASSERT(ar_method__get_ast(ref_method) != NULL, "Method should have a valid AST");
}

static void load_method(ar_method_fixture_t *mut_fixture, const char *ref_name) {
    char ref_path[160];
    int written = snprintf(ref_path,
                           sizeof(ref_path),
                           "../../methodologies/coordination/%s-1.0.0.method",
                           ref_name);
    AR_ASSERT(written > 0 && written < (int)sizeof(ref_path), "Method path should fit buffer");
    AR_ASSERT(ar_method_fixture__load_method(mut_fixture, ref_name, ref_path, "1.0.0"),
              "Method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(mut_fixture);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    verify_method_parses(mut_methodology, ref_name);
}

static ar_data_t *create_context(void) {
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Agent context should be created");
    return own_context;
}

static int checked_agent_id(int64_t agent_id) {
    AR_ASSERT(agent_id > 0, "Agent id should be positive");
    AR_ASSERT(agent_id <= INT_MAX, "Agent id should fit test message integer");
    return (int)agent_id;
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_source := message.source\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_session_id := message.session_id\n"
        "memory.last_status := message.status\n"
        "memory.last_state := message.state\n"
        "memory.last_text := message.text\n"
        "memory.last_sync_id := message.sync_id\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_done_count := message.done_count\n"
        "memory.last_dependencies := message.dependencies\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void send_dependency(ar_agency_t *mut_agency, int64_t sync_agent, const char *ref_sync_id,
                            const char *ref_session_id, const char *ref_trace_id,
                            const char *ref_dependency) {
    ar_data_t *own_dependency = ar_data__create_map();
    AR_ASSERT(own_dependency != NULL, "Dependency should be created");
    ar_data__set_map_string(own_dependency, "request", "synchronization_dependency");
    ar_data__set_map_string(own_dependency, "sync_id", ref_sync_id);
    ar_data__set_map_string(own_dependency, "session_id", ref_session_id);
    ar_data__set_map_string(own_dependency, "trace_id", ref_trace_id);
    ar_data__set_map_string(own_dependency, "dependency", ref_dependency);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_dependency),
              "Dependency should queue");
    own_dependency = NULL;
}

static void send_noise(ar_agency_t *mut_agency,
                       int64_t sync_agent,
                       const char *ref_sync_id,
                       const char *ref_session_id) {
    ar_data_t *own_noise = ar_data__create_map();
    AR_ASSERT(own_noise != NULL, "Noise message should be created");
    ar_data__set_map_string(own_noise, "request", "synchronization_noise");
    ar_data__set_map_string(own_noise, "sync_id", ref_sync_id);
    ar_data__set_map_string(own_noise, "session_id", ref_session_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_noise),
              "Noise message should queue");
    own_noise = NULL;
}

static void test_synchronization__emits_continuation_after_unbounded_dependencies(void) {
    printf("Testing synchronization emits continuation after unbounded dependencies...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("synchronization_wait");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "synchronization");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_sync_context = create_context();
    ar_data_t *own_receiver_context = create_context();
    ar_data_t *own_observer_context = create_context();
    int64_t sync_agent = ar_agency__create_agent(
        mut_agency, "synchronization", "1.0.0", own_sync_context);
    int64_t receiver_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_context);
    int64_t observer_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_observer_context);

    ar_data_t *own_wait = ar_data__create_map();
    AR_ASSERT(own_wait != NULL, "Wait message should be created");
    ar_data__set_map_string(own_wait, "request", "synchronization_wait");
    ar_data__set_map_string(own_wait, "sync_id", "sync-1");
    ar_data__set_map_string(own_wait, "trace_id", "sync-start-1");
    ar_data__set_map_string(own_wait, "session_id", "sync-session-1");
    ar_data__set_map_integer(own_wait, "required_count", 4);
    ar_data__set_map_integer(own_wait, "source", checked_agent_id(observer_agent));
    ar_data__set_map_integer(own_wait, "continuation_target", checked_agent_id(receiver_agent));
    ar_data__set_map_string(own_wait, "continuation_request", "continue");
    ar_data__set_map_string(own_wait, "continuation_text", "go");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_wait),
              "Wait message should queue");
    own_wait = NULL;

    send_dependency(mut_agency, sync_agent, "sync-1", "sync-session-1", "sync-dep-a", "ready-a");
    send_dependency(mut_agency, sync_agent, "sync-1", "sync-session-1", "sync-dep-b", "ready-b");
    send_dependency(mut_agency, sync_agent, "sync-1", "sync-session-1", "sync-dep-c", "ready-c");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    AR_ASSERT(ar_data__get_map_data(ref_receiver_memory, "last_request") == NULL,
              "Receiver should not continue before all dependencies arrive");

    send_dependency(mut_agency, sync_agent, "sync-1", "sync-session-1", "sync-dep-d", "ready-d");
    ar_method_fixture__process_all_messages(own_fixture);

    const char *ref_request = ar_data__get_map_string(ref_receiver_memory, "last_request");
    AR_ASSERT(ref_request != NULL && strcmp(ref_request, "continue") == 0,
              "Receiver should observe continuation");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_text"), "go") == 0,
              "Continuation should include text");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_done_count") == 4,
              "Continuation should report satisfied dependency count");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_trace_id"),
                     "sync-dep-d") == 0,
              "Continuation should preserve completing dependency trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_session_id"),
                     "sync-session-1") == 0,
              "Continuation should preserve synchronization session id");
    const ar_data_t *ref_dependencies = ar_data__get_map_data(ref_receiver_memory,
                                                              "last_dependencies");
    AR_ASSERT(ref_dependencies != NULL && ar_data__list_count(ref_dependencies) == 4,
              "Continuation should include every received dependency");
    const ar_data_t *ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_response"),
                     "synchronization_result") == 0,
              "Synchronization should emit a response to the request source");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_source") ==
                  checked_agent_id(sync_agent),
              "Synchronization response should identify the synchronization source");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "sync-dep-d") == 0,
              "Synchronization response should preserve completing dependency trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_session_id"),
                     "sync-session-1") == 0,
              "Synchronization response should preserve session id");

    const ar_data_t *ref_sync_memory = ar_agency__get_agent_memory(mut_agency, sync_agent);
    const ar_data_t *ref_received = ar_data__get_map_data(ref_sync_memory, "received");
    AR_ASSERT(ref_received != NULL && ar_data__list_count(ref_received) == 4,
              "Synchronization memory should retain every dependency");

    own_wait = ar_data__create_map();
    AR_ASSERT(own_wait != NULL, "Zero-count wait message should be created");
    ar_data__set_map_string(own_wait, "request", "synchronization_wait");
    ar_data__set_map_string(own_wait, "sync_id", "sync-zero");
    ar_data__set_map_string(own_wait, "trace_id", "sync-zero-start");
    ar_data__set_map_string(own_wait, "session_id", "sync-zero-session");
    ar_data__set_map_integer(own_wait, "required_count", 0);
    ar_data__set_map_integer(own_wait, "source", checked_agent_id(observer_agent));
    ar_data__set_map_integer(own_wait, "continuation_target", checked_agent_id(receiver_agent));
    ar_data__set_map_string(own_wait, "continuation_request", "continue");
    ar_data__set_map_string(own_wait, "continuation_text", "zero-go");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_wait),
              "Zero-count wait message should queue");
    own_wait = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ref_sync_memory = ar_agency__get_agent_memory(mut_agency, sync_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "completed") == 0,
              "Zero required count should not complete on wait");
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "done_count") == 0,
              "Zero required count wait should not collect dependencies");

    send_dependency(mut_agency,
                    sync_agent,
                    "sync-zero",
                    "sync-zero-session",
                    "sync-zero-dep",
                    "ready-zero");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_sync_id"),
                     "sync-zero") == 0,
              "Zero required count should continue after first dependency");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_done_count") == 1,
              "Zero required count should behave as one required dependency");

    own_wait = ar_data__create_map();
    AR_ASSERT(own_wait != NULL, "Failed-continuation wait message should be created");
    ar_data__set_map_string(own_wait, "request", "synchronization_wait");
    ar_data__set_map_string(own_wait, "sync_id", "sync-failed-continuation");
    ar_data__set_map_string(own_wait, "trace_id", "sync-failed-continuation-start");
    ar_data__set_map_string(own_wait, "session_id", "sync-failed-continuation-session");
    ar_data__set_map_integer(own_wait, "required_count", 2);
    ar_data__set_map_integer(own_wait, "source", checked_agent_id(observer_agent));
    ar_data__set_map_integer(own_wait, "continuation_target", 98765);
    ar_data__set_map_string(own_wait, "continuation_request", "continue");
    ar_data__set_map_string(own_wait, "continuation_text", "failed-go");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_wait),
              "Failed-continuation wait message should queue");
    own_wait = NULL;

    send_dependency(mut_agency,
                    sync_agent,
                    "sync-failed-continuation",
                    "sync-failed-continuation-session",
                    "sync-failed-continuation-x",
                    "ready-x");
    send_dependency(mut_agency,
                    sync_agent,
                    "sync-failed-continuation",
                    "sync-failed-continuation-session",
                    "sync-failed-continuation-y",
                    "ready-y");
    ar_method_fixture__process_all_messages(own_fixture);

    ref_sync_memory = ar_agency__get_agent_memory(mut_agency, sync_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "completed") == 0,
              "Synchronization should stay open after failed continuation send");
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "done_count") == 2,
              "Synchronization should retain dependencies after failed continuation send");

    ar_data_t *mut_sync_memory = ar_agency__get_agent_mutable_memory(mut_agency, sync_agent);
    AR_ASSERT(mut_sync_memory != NULL, "Synchronization memory should be mutable");
    AR_ASSERT(ar_data__set_map_integer(mut_sync_memory,
                                       "continuation_target",
                                       checked_agent_id(receiver_agent)),
              "Failed continuation target should be repairable for retry");
    send_noise(mut_agency,
               sync_agent,
               "sync-failed-continuation",
               "sync-failed-continuation-session");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_sync_id"),
                     "sync-zero") == 0,
              "Unrelated messages should not retry failed continuation");

    send_dependency(mut_agency,
                    sync_agent,
                    "sync-failed-continuation",
                    "sync-failed-continuation-session",
                    "sync-failed-continuation-z",
                    "ready-z");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "completed") == 1,
              "Synchronization should complete after continuation retry succeeds");
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "done_count") == 2,
              "Synchronization should freeze dependencies after failed continuation send");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_sync_id"),
                     "sync-failed-continuation") == 0,
              "Continuation retry should preserve sync id");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_done_count") == 2,
              "Continuation retry should report the frozen dependency count");
    ref_dependencies = ar_data__get_map_data(ref_receiver_memory, "last_dependencies");
    AR_ASSERT(ref_dependencies != NULL && ar_data__list_count(ref_dependencies) == 2,
              "Continuation retry should use the frozen dependency list");

    own_wait = ar_data__create_map();
    AR_ASSERT(own_wait != NULL, "Failed-status wait message should be created");
    ar_data__set_map_string(own_wait, "request", "synchronization_wait");
    ar_data__set_map_string(own_wait, "sync_id", "sync-failed-status");
    ar_data__set_map_string(own_wait, "trace_id", "sync-trace-status");
    ar_data__set_map_string(own_wait, "session_id", "sync-failed-status-session");
    ar_data__set_map_integer(own_wait, "required_count", 2);
    ar_data__set_map_integer(own_wait, "source", 98765);
    ar_data__set_map_integer(own_wait, "continuation_target", checked_agent_id(receiver_agent));
    ar_data__set_map_string(own_wait, "continuation_request", "continue");
    ar_data__set_map_string(own_wait, "continuation_text", "status-go");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_wait),
              "Failed-status wait message should queue");
    own_wait = NULL;

    send_dependency(mut_agency,
                    sync_agent,
                    "sync-failed-status",
                    "sync-failed-status-session",
                    "sync-failed-status-m",
                    "ready-m");
    send_dependency(mut_agency,
                    sync_agent,
                    "sync-failed-status",
                    "sync-failed-status-session",
                    "sync-failed-status-n",
                    "ready-n");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_sync_id"),
                     "sync-failed-status") == 0,
              "Continuation should still be sent when status response fails");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_source") ==
                  checked_agent_id(sync_agent),
              "Continuation should identify the synchronization source");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_done_count") == 2,
              "Continuation should include dependencies even when status response fails");
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "completed") == 0,
              "Synchronization should stay open after failed status send");
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "continuation_done") == 1,
              "Synchronization should remember delivered continuation after failed status send");

    AR_ASSERT(ar_data__set_map_integer(mut_sync_memory,
                                       "source",
                                       checked_agent_id(receiver_agent)),
              "Failed status target should be repairable for retry");
    send_noise(mut_agency, sync_agent, "sync-failed-status", "sync-failed-status-session");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_request"),
                     "continue") == 0,
              "Unrelated messages should not retry failed status report");

    send_dependency(mut_agency,
                    sync_agent,
                    "sync-failed-status",
                    "sync-failed-status-session",
                    "sync-failed-status-o",
                    "ready-o");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "completed") == 1,
              "Synchronization should complete after matching dependency retries status send");
    AR_ASSERT(ar_data__get_map_integer(ref_sync_memory, "done_count") == 2,
              "Synchronization should freeze dependencies after status retry");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_response"),
                     "synchronization_result") == 0,
              "Synchronization status retry should emit a response");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_source") ==
                  checked_agent_id(sync_agent),
              "Synchronization response should identify the synchronization source");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_status"), "success") == 0,
              "Synchronization status retry should report standard success");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_state"), "complete") == 0,
              "Synchronization status retry should preserve completion state");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_done_count") == 2,
              "Status retry should preserve frozen dependency count");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_trace_id"),
                     "sync-failed-status-o") == 0,
              "Synchronization status should preserve retry dependency trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_session_id"),
                     "sync-failed-status-session") == 0,
              "Synchronization status should preserve session id");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_success_count") == 2,
              "Synchronization status should report satisfied dependencies");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_failure_count") == 0,
              "Synchronization status should report no failed dependencies");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_sync_context);
    ar_data__destroy(own_receiver_context);
    ar_data__destroy(own_observer_context);
}

int main(void) {
    printf("Running synchronization method tests...\n\n");
    test_synchronization__emits_continuation_after_unbounded_dependencies();
    printf("\nAll synchronization method tests passed!\n");
    return 0;
}
