#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_heap.h"
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

static void append_child_method_name(ar_data_t *mut_child_method_names,
                                     const char *ref_method_name) {
    AR_ASSERT(ar_data__list_add_last_string(mut_child_method_names, ref_method_name),
              "Child method name should be appended");
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_sender := message.sender\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_session_id := message.session_id\n"
        "memory.last_status := message.status\n"
        "memory.last_child_agent_id := message.child_agent_id\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_child_count := message.child_count\n"
        "memory.last_restart_count := message.restart_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_supervision__tracks_unbounded_children_and_restarts_failed_child(void) {
    printf("Testing supervision tracks unbounded children and restarts failed child...\n");

    // Given a supervision agent with more children than the old fixed shape allowed
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("supervision_unbounded");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "supervision");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_supervision_context = create_context();
    ar_data_t *own_observer_context = create_context();
    ar_data_t *own_untracked_context = create_context();
    int64_t supervision_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_supervision_context);
    int64_t observer_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_observer_context);
    int64_t untracked_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_untracked_context);

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Supervision start should be created");
    ar_data__set_map_string(own_start, "request", "supervision_start");
    ar_data_t *own_child_method_names = ar_data__create_list();
    AR_ASSERT(own_child_method_names != NULL, "Child method name list should be created");
    append_child_method_name(own_child_method_names, "record-receiver");
    append_child_method_name(own_child_method_names, "record-receiver");
    append_child_method_name(own_child_method_names, "record-receiver");
    append_child_method_name(own_child_method_names, "record-receiver");
    AR_ASSERT(ar_data__set_map_data(own_start, "child_method_names", own_child_method_names),
              "Start message should own child method name list");
    own_child_method_names = NULL;
    ar_data__set_map_string(own_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_start, "policy", "restart");
    ar_data__set_map_string(own_start, "trace_id", "supervision-trace-1");
    ar_data__set_map_string(own_start, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_start),
              "Supervision start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then all children are spawned and tracked in lists
    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    const ar_data_t *ref_child_ids = ar_data__get_map_data(ref_memory, "child_agent_ids");
    const ar_data_t *ref_child_records = ar_data__get_map_data(ref_memory, "child_records");
    AR_ASSERT(ref_child_ids != NULL && ar_data__list_count(ref_child_ids) == 4,
              "Supervision should track all spawned child ids");
    AR_ASSERT(ref_child_records != NULL && ar_data__list_count(ref_child_records) == 4,
              "Supervision should track all spawned child records");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "child_count") == 4,
              "Supervision should record four running children");
    ar_data_t **own_child_id_items = ar_data__list_items(ref_child_ids);
    AR_ASSERT(own_child_id_items != NULL, "Supervision child ids should be readable");
    int64_t first_child = ar_data__get_integer(own_child_id_items[0]);
    int64_t second_child = ar_data__get_integer(own_child_id_items[1]);
    int64_t third_child = ar_data__get_integer(own_child_id_items[2]);
    AR__HEAP__FREE(own_child_id_items);
    AR_ASSERT(first_child > 0, "Supervision should create a child agent");
    AR_ASSERT(second_child > first_child, "Supervision should create a second child agent");
    AR_ASSERT(third_child > second_child, "Supervision should create a third child agent");

    const ar_data_t *ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_response"),
                     "supervision_result") == 0,
              "Observer should receive a supervision response");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_sender") ==
                  checked_agent_id(supervision_agent),
              "Supervision response should identify the supervision sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "success") == 0,
              "Observer should receive standard success status for running supervision");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_child_count") == 4,
              "Observer should receive child count");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "supervision-trace-1") == 0,
              "Supervision status should preserve trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_session_id"),
                     "supervision-session-1") == 0,
              "Supervision status should preserve session id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_success_count") == 4,
              "Supervision status should report spawned child count");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_failure_count") == 0,
              "Supervision status should report no failed child coordination");

    // When a lifecycle event names an untracked agent
    ar_data_t *own_untracked_failure = ar_data__create_map();
    AR_ASSERT(own_untracked_failure != NULL, "Untracked failure should be created");
    ar_data__set_map_string(own_untracked_failure, "request", "supervision_child_failed");
    ar_data__set_map_string(own_untracked_failure, "trace_id", "supervision-untracked-failure");
    ar_data__set_map_string(own_untracked_failure, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_untracked_failure,
                             "child_agent_id",
                             checked_agent_id(untracked_agent));
    ar_data__set_map_string(own_untracked_failure, "child_method_name", "record-receiver");
    ar_data__set_map_string(own_untracked_failure, "child_method_version", "1.0.0");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_untracked_failure),
              "Untracked failure should queue");
    own_untracked_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then supervision ignores it without adding bogus child records
    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    ref_child_ids = ar_data__get_map_data(ref_memory, "child_agent_ids");
    AR_ASSERT(ref_child_ids != NULL && ar_data__list_count(ref_child_ids) == 4,
              "Untracked lifecycle event should not append a replacement child id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "status"), "running") == 0,
              "Untracked lifecycle event should not change supervisor status");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "restart_count") == 0,
              "Untracked lifecycle event should not increment restart count");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "success") == 0,
              "Observer should receive standard success status for ignored lifecycle event");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "supervision-untracked-failure") == 0,
              "Untracked lifecycle response should preserve request trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_child_agent_id") ==
                  untracked_agent,
              "Observer should receive the ignored lifecycle child id");

    // When the child is reported failed
    ar_data_t *own_failure = ar_data__create_map();
    AR_ASSERT(own_failure != NULL, "Child failure should be created");
    ar_data__set_map_string(own_failure, "request", "supervision_child_failed");
    ar_data__set_map_string(own_failure, "trace_id", "supervision-child-failure");
    ar_data__set_map_string(own_failure, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_failure, "child_agent_id", checked_agent_id(first_child));
    ar_data__set_map_string(own_failure, "child_method_name", "record-receiver");
    ar_data__set_map_string(own_failure, "child_method_version", "1.0.0");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_failure),
              "Child failure should queue");
    own_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then supervision records a replacement without losing the existing tracked set
    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    ref_child_ids = ar_data__get_map_data(ref_memory, "child_agent_ids");
    int64_t restarted_child = ar_data__get_map_integer(ref_memory, "child_agent_id");
    AR_ASSERT(restarted_child > first_child, "Supervision should spawn a replacement child");
    AR_ASSERT(ref_child_ids != NULL && ar_data__list_count(ref_child_ids) == 5,
              "Supervision should append the replacement child id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "status"), "restarted") == 0,
              "Supervision should record restarted status");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "restart_count") == 1,
              "Supervision should count the restart");

    // And a duplicate lifecycle event for the replaced child is ignored
    ar_data_t *own_duplicate_failure = ar_data__create_map();
    AR_ASSERT(own_duplicate_failure != NULL, "Duplicate child failure should be created");
    ar_data__set_map_string(own_duplicate_failure, "request", "supervision_child_failed");
    ar_data__set_map_string(own_duplicate_failure, "trace_id", "supervision-duplicate-failure");
    ar_data__set_map_string(own_duplicate_failure, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_duplicate_failure,
                             "child_agent_id",
                             checked_agent_id(first_child));
    ar_data__set_map_string(own_duplicate_failure, "child_method_name", "record-receiver");
    ar_data__set_map_string(own_duplicate_failure, "child_method_version", "1.0.0");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_duplicate_failure),
              "Duplicate child failure should queue");
    own_duplicate_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    ref_child_ids = ar_data__get_map_data(ref_memory, "child_agent_ids");
    AR_ASSERT(ref_child_ids != NULL && ar_data__list_count(ref_child_ids) == 5,
              "Duplicate child failure should not append another replacement");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "restart_count") == 1,
              "Duplicate child failure should not increment restart count");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "supervision-duplicate-failure") == 0,
              "Duplicate lifecycle response should preserve request trace id");

    // And a delayed duplicate for an older child is ignored after another child is handled
    ar_data_t *own_second_failure = ar_data__create_map();
    AR_ASSERT(own_second_failure != NULL, "Second child failure should be created");
    ar_data__set_map_string(own_second_failure, "request", "supervision_child_failed");
    ar_data__set_map_string(own_second_failure, "trace_id", "supervision-second-failure");
    ar_data__set_map_string(own_second_failure, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_second_failure,
                             "child_agent_id",
                             checked_agent_id(second_child));
    ar_data__set_map_string(own_second_failure, "child_method_name", "record-receiver");
    ar_data__set_map_string(own_second_failure, "child_method_version", "1.0.0");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_second_failure),
              "Second child failure should queue");
    own_second_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    ref_child_ids = ar_data__get_map_data(ref_memory, "child_agent_ids");
    AR_ASSERT(ref_child_ids != NULL && ar_data__list_count(ref_child_ids) == 6,
              "Second child failure should append one replacement");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "restart_count") == 2,
              "Second child failure should increment restart count");

    own_duplicate_failure = ar_data__create_map();
    AR_ASSERT(own_duplicate_failure != NULL, "Delayed duplicate failure should be created");
    ar_data__set_map_string(own_duplicate_failure, "request", "supervision_child_failed");
    ar_data__set_map_string(own_duplicate_failure, "trace_id", "supervision-delayed-duplicate");
    ar_data__set_map_string(own_duplicate_failure, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_duplicate_failure,
                             "child_agent_id",
                             checked_agent_id(first_child));
    ar_data__set_map_string(own_duplicate_failure, "child_method_name", "record-receiver");
    ar_data__set_map_string(own_duplicate_failure, "child_method_version", "1.0.0");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_duplicate_failure),
              "Delayed duplicate failure should queue");
    own_duplicate_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    ref_child_ids = ar_data__get_map_data(ref_memory, "child_agent_ids");
    AR_ASSERT(ref_child_ids != NULL && ar_data__list_count(ref_child_ids) == 6,
              "Delayed duplicate failure should not append another replacement");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "restart_count") == 2,
              "Delayed duplicate failure should not increment restart count");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "supervision-delayed-duplicate") == 0,
              "Delayed duplicate lifecycle response should preserve request trace id");

    // When a stop request names an untracked agent
    ar_data_t *own_untracked_stop = ar_data__create_map();
    AR_ASSERT(own_untracked_stop != NULL, "Untracked stop should be created");
    ar_data__set_map_string(own_untracked_stop, "request", "supervision_stop");
    ar_data__set_map_string(own_untracked_stop, "trace_id", "supervision-untracked-stop");
    ar_data__set_map_string(own_untracked_stop, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_untracked_stop,
                             "child_agent_id",
                             checked_agent_id(untracked_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_untracked_stop),
              "Untracked stop should queue");
    own_untracked_stop = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then the unrelated agent remains alive and the supervisor reports an ignored stop
    AR_ASSERT(ar_agency__agent_exists(mut_agency, untracked_agent),
              "Untracked stop should not exit an unrelated agent");
    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "status"), "restarted") == 0,
              "Untracked stop should not change the stored supervisor status");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "supervision-untracked-stop") == 0,
              "Untracked stop response should preserve request trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_child_agent_id") ==
                  untracked_agent,
              "Observer should receive the ignored child id");

    // When a stop request names a tracked child
    ar_data_t *own_tracked_stop = ar_data__create_map();
    AR_ASSERT(own_tracked_stop != NULL, "Tracked stop should be created");
    ar_data__set_map_string(own_tracked_stop, "request", "supervision_stop");
    ar_data__set_map_string(own_tracked_stop, "trace_id", "supervision-stop-trace");
    ar_data__set_map_string(own_tracked_stop, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_tracked_stop, "child_agent_id", checked_agent_id(third_child));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_tracked_stop),
              "Tracked stop should queue");
    own_tracked_stop = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then the tracked child exits and the supervisor reports stopped
    AR_ASSERT(!ar_agency__agent_exists(mut_agency, third_child),
              "Tracked stop should exit the named child");
    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "status"), "stopped") == 0,
              "Tracked stop should record stopped status");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "success") == 0,
              "Observer should receive standard success status for tracked stop");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "supervision-stop-trace") == 0,
              "Tracked stop response should preserve request trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_session_id"),
                     "supervision-session-1") == 0,
              "Tracked stop response should preserve session id");

    // And a delayed lifecycle event for the stopped child is ignored
    ar_data_t *own_stopped_lifecycle = ar_data__create_map();
    AR_ASSERT(own_stopped_lifecycle != NULL, "Stopped child lifecycle should be created");
    ar_data__set_map_string(own_stopped_lifecycle, "request", "supervision_child_exited");
    ar_data__set_map_string(own_stopped_lifecycle, "trace_id", "supervision-stopped-lifecycle");
    ar_data__set_map_string(own_stopped_lifecycle, "session_id", "supervision-session-1");
    ar_data__set_map_integer(own_stopped_lifecycle,
                             "child_agent_id",
                             checked_agent_id(third_child));
    ar_data__set_map_string(own_stopped_lifecycle, "child_method_name", "record-receiver");
    ar_data__set_map_string(own_stopped_lifecycle, "child_method_version", "1.0.0");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_stopped_lifecycle),
              "Stopped child lifecycle should queue");
    own_stopped_lifecycle = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    ref_child_ids = ar_data__get_map_data(ref_memory, "child_agent_ids");
    AR_ASSERT(ref_child_ids != NULL && ar_data__list_count(ref_child_ids) == 6,
              "Stopped child lifecycle should not append another replacement");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "restart_count") == 2,
              "Stopped child lifecycle should not increment restart count");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "supervision-stopped-lifecycle") == 0,
              "Stopped child lifecycle response should preserve request trace id");

    ar_data_t *own_empty_start_context = create_context();
    int64_t empty_start_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_empty_start_context);
    ar_data_t *own_empty_start = ar_data__create_map();
    AR_ASSERT(own_empty_start != NULL, "Empty child list start should be created");
    ar_data__set_map_string(own_empty_start, "request", "supervision_start");
    ar_data_t *own_empty_child_methods = ar_data__create_list();
    AR_ASSERT(own_empty_child_methods != NULL, "Empty child method list should be created");
    AR_ASSERT(ar_data__set_map_data(own_empty_start,
                                    "child_method_names",
                                    own_empty_child_methods),
              "Empty start should own child method list");
    own_empty_child_methods = NULL;
    ar_data__set_map_string(own_empty_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_empty_start, "policy", "restart");
    ar_data__set_map_string(own_empty_start, "trace_id", "supervision-empty-start");
    ar_data__set_map_string(own_empty_start, "session_id", "supervision-empty-session");
    ar_data__set_map_integer(own_empty_start, "sender", checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, empty_start_agent, own_empty_start),
              "Empty child list start should queue");
    own_empty_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_empty_start_memory =
        ar_agency__get_agent_memory(mut_agency, empty_start_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_empty_start_memory, "status"), "running") == 0,
              "Empty child list start should report running");
    AR_ASSERT(ar_data__get_map_integer(ref_empty_start_memory, "child_count") == 0,
              "Empty child list start should keep zero children");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "success") == 0,
              "Observer should receive standard success status for empty child list");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_child_count") == 0,
              "Observer should receive zero child count for empty child list");

    ar_data_t *own_failed_handoff_context = create_context();
    int64_t failed_handoff_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_failed_handoff_context);
    ar_data_t *mut_failed_handoff_memory =
        ar_agency__get_agent_mutable_memory(mut_agency, failed_handoff_agent);
    AR_ASSERT(mut_failed_handoff_memory != NULL, "Failed handoff supervisor memory should exist");
    AR_ASSERT(ar_data__set_map_integer(mut_failed_handoff_memory, "self", 98765),
              "Failed start handoff should corrupt supervisor self");

    ar_data_t *own_failed_handoff_start = ar_data__create_map();
    AR_ASSERT(own_failed_handoff_start != NULL, "Failed handoff start should be created");
    ar_data__set_map_string(own_failed_handoff_start, "request", "supervision_start");
    ar_data_t *own_failed_handoff_methods = ar_data__create_list();
    AR_ASSERT(own_failed_handoff_methods != NULL,
              "Failed handoff child methods should be created");
    append_child_method_name(own_failed_handoff_methods, "record-receiver");
    AR_ASSERT(ar_data__set_map_data(own_failed_handoff_start,
                                    "child_method_names",
                                    own_failed_handoff_methods),
              "Failed handoff start should own child methods");
    own_failed_handoff_methods = NULL;
    ar_data__set_map_string(own_failed_handoff_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_failed_handoff_start, "policy", "restart");
    ar_data__set_map_string(own_failed_handoff_start,
                            "trace_id",
                            "supervision-failed-handoff-start");
    ar_data__set_map_string(own_failed_handoff_start,
                            "session_id",
                            "supervision-failed-handoff-session");
    ar_data__set_map_integer(own_failed_handoff_start,
                             "sender",
                             checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       failed_handoff_agent,
                                       own_failed_handoff_start),
              "Failed handoff start should queue");
    own_failed_handoff_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_failed_handoff_memory =
        ar_agency__get_agent_memory(mut_agency, failed_handoff_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_handoff_memory, "status"),
                     "handoff_failed") == 0,
              "Failed start handoff should store handoff_failed status");
    AR_ASSERT(ar_data__get_map_integer(ref_failed_handoff_memory, "child_count") == 0,
              "Failed start handoff should not spawn children");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "failure") == 0,
              "Observer should receive standard failure status for failed start handoff");

    ar_data_t *own_failed_continue_context = create_context();
    int64_t failed_continue_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_failed_continue_context);
    ar_data_t *own_failed_continue_start = ar_data__create_map();
    AR_ASSERT(own_failed_continue_start != NULL, "Failed continuation start should be created");
    ar_data__set_map_string(own_failed_continue_start, "request", "supervision_start");
    ar_data_t *own_failed_continue_methods = ar_data__create_list();
    AR_ASSERT(own_failed_continue_methods != NULL,
              "Failed continuation child methods should be created");
    append_child_method_name(own_failed_continue_methods, "record-receiver");
    append_child_method_name(own_failed_continue_methods, "record-receiver");
    AR_ASSERT(ar_data__set_map_data(own_failed_continue_start,
                                    "child_method_names",
                                    own_failed_continue_methods),
              "Failed continuation start should own child methods");
    own_failed_continue_methods = NULL;
    ar_data__set_map_string(own_failed_continue_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_failed_continue_start, "policy", "restart");
    ar_data__set_map_string(own_failed_continue_start,
                            "trace_id",
                            "supervision-failed-continue-start");
    ar_data__set_map_string(own_failed_continue_start,
                            "session_id",
                            "supervision-failed-continue-session");
    ar_data__set_map_integer(own_failed_continue_start,
                             "sender",
                             checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       failed_continue_agent,
                                       own_failed_continue_start),
              "Failed continuation start should queue");
    own_failed_continue_start = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Failed continuation start should process");
    ar_data_t *mut_failed_continue_memory =
        ar_agency__get_agent_mutable_memory(mut_agency, failed_continue_agent);
    AR_ASSERT(mut_failed_continue_memory != NULL, "Failed continuation memory should exist");
    AR_ASSERT(ar_data__set_map_integer(mut_failed_continue_memory, "self", 98765),
              "Failed continuation should corrupt supervisor self");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Failed continuation spawn should process");

    const ar_data_t *ref_failed_continue_memory =
        ar_agency__get_agent_memory(mut_agency, failed_continue_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_continue_memory, "status"),
                     "handoff_failed") == 0,
              "Failed spawn continuation should store handoff_failed status");
    AR_ASSERT(ar_data__get_map_integer(ref_failed_continue_memory, "child_count") == 1,
              "Failed spawn continuation should preserve partial child count");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "failure") == 0,
              "Observer should receive standard failure status for failed spawn continuation");

    ar_data_t *own_failed_lifecycle_context = create_context();
    int64_t failed_lifecycle_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_failed_lifecycle_context);
    ar_data_t *own_failed_lifecycle_start = ar_data__create_map();
    AR_ASSERT(own_failed_lifecycle_start != NULL, "Failed lifecycle start should be created");
    ar_data__set_map_string(own_failed_lifecycle_start, "request", "supervision_start");
    ar_data_t *own_failed_lifecycle_methods = ar_data__create_list();
    AR_ASSERT(own_failed_lifecycle_methods != NULL,
              "Failed lifecycle child methods should be created");
    append_child_method_name(own_failed_lifecycle_methods, "record-receiver");
    AR_ASSERT(ar_data__set_map_data(own_failed_lifecycle_start,
                                    "child_method_names",
                                    own_failed_lifecycle_methods),
              "Failed lifecycle start should own child methods");
    own_failed_lifecycle_methods = NULL;
    ar_data__set_map_string(own_failed_lifecycle_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_failed_lifecycle_start, "policy", "restart");
    ar_data__set_map_string(own_failed_lifecycle_start,
                            "trace_id",
                            "supervision-failed-lifecycle-start");
    ar_data__set_map_string(own_failed_lifecycle_start,
                            "session_id",
                            "supervision-failed-lifecycle-session");
    ar_data__set_map_integer(own_failed_lifecycle_start,
                             "sender",
                             checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       failed_lifecycle_agent,
                                       own_failed_lifecycle_start),
              "Failed lifecycle start should queue");
    own_failed_lifecycle_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *mut_failed_lifecycle_memory =
        ar_agency__get_agent_mutable_memory(mut_agency, failed_lifecycle_agent);
    AR_ASSERT(mut_failed_lifecycle_memory != NULL, "Failed lifecycle memory should exist");
    int64_t failed_lifecycle_child =
        ar_data__get_map_integer(mut_failed_lifecycle_memory, "spawn_result");
    AR_ASSERT(failed_lifecycle_child > 0, "Failed lifecycle setup should spawn a child");
    AR_ASSERT(ar_data__set_map_integer(mut_failed_lifecycle_memory, "self", 98765),
              "Failed lifecycle should corrupt supervisor self");

    ar_data_t *own_failed_lifecycle_event = ar_data__create_map();
    AR_ASSERT(own_failed_lifecycle_event != NULL,
              "Failed lifecycle event should be created");
    ar_data__set_map_string(own_failed_lifecycle_event, "request", "supervision_child_failed");
    ar_data__set_map_string(own_failed_lifecycle_event,
                            "trace_id",
                            "supervision-failed-lifecycle-event");
    ar_data__set_map_string(own_failed_lifecycle_event,
                            "session_id",
                            "supervision-failed-lifecycle-session");
    ar_data__set_map_integer(own_failed_lifecycle_event,
                             "child_agent_id",
                             checked_agent_id(failed_lifecycle_child));
    ar_data__set_map_string(own_failed_lifecycle_event,
                            "child_method_name",
                            "record-receiver");
    ar_data__set_map_string(own_failed_lifecycle_event,
                            "child_method_version",
                            "1.0.0");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       failed_lifecycle_agent,
                                       own_failed_lifecycle_event),
              "Failed lifecycle event should queue");
    own_failed_lifecycle_event = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_failed_lifecycle_memory =
        ar_agency__get_agent_memory(mut_agency, failed_lifecycle_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_lifecycle_memory, "status"),
                     "handoff_failed") == 0,
              "Failed lifecycle validation should store handoff_failed status");
    AR_ASSERT(ar_data__get_map_integer(ref_failed_lifecycle_memory, "restart_count") == 0,
              "Failed lifecycle validation should not restart without validation");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "failure") == 0,
              "Observer should receive standard failure status for failed lifecycle validation");

    ar_data_t *own_failed_stop_context = create_context();
    int64_t failed_stop_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_failed_stop_context);
    ar_data_t *own_failed_stop_start = ar_data__create_map();
    AR_ASSERT(own_failed_stop_start != NULL, "Failed stop start should be created");
    ar_data__set_map_string(own_failed_stop_start, "request", "supervision_start");
    ar_data_t *own_failed_stop_methods = ar_data__create_list();
    AR_ASSERT(own_failed_stop_methods != NULL, "Failed stop child methods should be created");
    append_child_method_name(own_failed_stop_methods, "record-receiver");
    AR_ASSERT(ar_data__set_map_data(own_failed_stop_start,
                                    "child_method_names",
                                    own_failed_stop_methods),
              "Failed stop start should own child methods");
    own_failed_stop_methods = NULL;
    ar_data__set_map_string(own_failed_stop_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_failed_stop_start, "policy", "restart");
    ar_data__set_map_string(own_failed_stop_start,
                            "trace_id",
                            "supervision-failed-stop-start");
    ar_data__set_map_string(own_failed_stop_start,
                            "session_id",
                            "supervision-failed-stop-session");
    ar_data__set_map_integer(own_failed_stop_start,
                             "sender",
                             checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       failed_stop_agent,
                                       own_failed_stop_start),
              "Failed stop start should queue");
    own_failed_stop_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *mut_failed_stop_memory =
        ar_agency__get_agent_mutable_memory(mut_agency, failed_stop_agent);
    AR_ASSERT(mut_failed_stop_memory != NULL, "Failed stop memory should exist");
    int64_t failed_stop_child = ar_data__get_map_integer(mut_failed_stop_memory, "spawn_result");
    AR_ASSERT(failed_stop_child > 0, "Failed stop setup should spawn a child");
    AR_ASSERT(ar_data__set_map_integer(mut_failed_stop_memory, "self", 98765),
              "Failed stop should corrupt supervisor self");

    ar_data_t *own_failed_stop = ar_data__create_map();
    AR_ASSERT(own_failed_stop != NULL, "Failed stop message should be created");
    ar_data__set_map_string(own_failed_stop, "request", "supervision_stop");
    ar_data__set_map_string(own_failed_stop, "trace_id", "supervision-failed-stop");
    ar_data__set_map_string(own_failed_stop, "session_id", "supervision-failed-stop-session");
    ar_data__set_map_integer(own_failed_stop,
                             "child_agent_id",
                             checked_agent_id(failed_stop_child));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, failed_stop_agent, own_failed_stop),
              "Failed stop message should queue");
    own_failed_stop = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_failed_stop_memory =
        ar_agency__get_agent_memory(mut_agency, failed_stop_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_stop_memory, "status"),
                     "handoff_failed") == 0,
              "Failed stop validation should store handoff_failed status");
    AR_ASSERT(ar_agency__get_agent_memory(mut_agency, failed_stop_child) != NULL,
              "Failed stop validation should not exit child without validation");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "failure") == 0,
              "Observer should receive standard failure status for failed stop validation");

    ar_data_t *own_failed_spawn_context = create_context();
    ar_data_t *own_failed_spawn_observer_context = create_context();
    int64_t failed_spawn_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_failed_spawn_context);
    int64_t failed_spawn_observer_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_failed_spawn_observer_context);
    ar_data_t *own_failed_spawn_start = ar_data__create_map();
    AR_ASSERT(own_failed_spawn_start != NULL, "Failed spawn start should be created");
    ar_data__set_map_string(own_failed_spawn_start, "request", "supervision_start");
    ar_data_t *own_failed_spawn_methods = ar_data__create_list();
    AR_ASSERT(own_failed_spawn_methods != NULL, "Failed spawn methods should be created");
    append_child_method_name(own_failed_spawn_methods, "missing-child-method");
    AR_ASSERT(ar_data__set_map_data(own_failed_spawn_start,
                                    "child_method_names",
                                    own_failed_spawn_methods),
              "Failed spawn start should own child methods");
    own_failed_spawn_methods = NULL;
    ar_data__set_map_string(own_failed_spawn_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_failed_spawn_start, "policy", "restart");
    ar_data__set_map_string(own_failed_spawn_start,
                            "trace_id",
                            "supervision-failed-spawn-start");
    ar_data__set_map_string(own_failed_spawn_start,
                            "session_id",
                            "supervision-failed-spawn-session");
    ar_data__set_map_integer(own_failed_spawn_start,
                             "sender",
                             checked_agent_id(failed_spawn_observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       failed_spawn_agent,
                                       own_failed_spawn_start),
              "Failed spawn start should queue");
    own_failed_spawn_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_failed_spawn_memory =
        ar_agency__get_agent_memory(mut_agency, failed_spawn_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_spawn_memory, "status"),
                     "starting") == 0,
              "Failed child spawn should leave supervisor starting instead of running");
    AR_ASSERT(ar_data__get_map_integer(ref_failed_spawn_memory, "child_count") == 0,
              "Failed child spawn should not increment child count");
    const ar_data_t *ref_failed_spawn_observer_memory =
        ar_agency__get_agent_memory(mut_agency, failed_spawn_observer_agent);
    AR_ASSERT(ar_data__get_map_data(ref_failed_spawn_observer_memory, "last_status") == NULL,
              "Failed child spawn cannot emit status after spawn aborts evaluation");

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_supervision_context);
    ar_data__destroy(own_observer_context);
    ar_data__destroy(own_untracked_context);
    ar_data__destroy(own_failed_handoff_context);
    ar_data__destroy(own_failed_continue_context);
    ar_data__destroy(own_failed_lifecycle_context);
    ar_data__destroy(own_failed_stop_context);
    ar_data__destroy(own_empty_start_context);
    ar_data__destroy(own_failed_spawn_context);
    ar_data__destroy(own_failed_spawn_observer_context);
}

int main(void) {
    printf("Running supervision method tests...\n\n");
    test_supervision__tracks_unbounded_children_and_restarts_failed_child();
    printf("\nAll supervision method tests passed!\n");
    return 0;
}
