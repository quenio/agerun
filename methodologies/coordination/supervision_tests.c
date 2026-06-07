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

static void append_child_method_name(ar_data_t *mut_child_method_names,
                                     const char *ref_method_name) {
    AR_ASSERT(ar_data__list_add_last_string(mut_child_method_names, ref_method_name),
              "Child method name should be appended");
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_status := message.status\n"
        "memory.last_child_agent_id := message.child_agent_id\n"
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
    ar_data__set_map_string(own_start, "action", "start");
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
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(observer_agent));
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
    const ar_data_t *ref_first_child = ar_data__list_first(ref_child_ids);
    int64_t first_child = ar_data__get_integer(ref_first_child);
    AR_ASSERT(first_child > 0, "Supervision should create a child agent");

    const ar_data_t *ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "running") == 0,
              "Observer should receive running supervision status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_child_count") == 4,
              "Observer should receive child count");

    // When a lifecycle event names an untracked agent
    ar_data_t *own_untracked_failure = ar_data__create_map();
    AR_ASSERT(own_untracked_failure != NULL, "Untracked failure should be created");
    ar_data__set_map_string(own_untracked_failure, "action", "child_failed");
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
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "ignored") == 0,
              "Observer should receive ignored status for untracked lifecycle event");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_child_agent_id") ==
                  untracked_agent,
              "Observer should receive the ignored lifecycle child id");

    // When the child is reported failed
    ar_data_t *own_failure = ar_data__create_map();
    AR_ASSERT(own_failure != NULL, "Child failure should be created");
    ar_data__set_map_string(own_failure, "action", "child_failed");
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

    // When a stop request names an untracked agent
    ar_data_t *own_untracked_stop = ar_data__create_map();
    AR_ASSERT(own_untracked_stop != NULL, "Untracked stop should be created");
    ar_data__set_map_string(own_untracked_stop, "action", "stop");
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
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "ignored") == 0,
              "Observer should receive ignored status for untracked stop");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_child_agent_id") ==
                  untracked_agent,
              "Observer should receive the ignored child id");

    // When a stop request names a tracked child
    ar_data_t *own_tracked_stop = ar_data__create_map();
    AR_ASSERT(own_tracked_stop != NULL, "Tracked stop should be created");
    ar_data__set_map_string(own_tracked_stop, "action", "stop");
    ar_data__set_map_integer(own_tracked_stop, "child_agent_id", checked_agent_id(first_child));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_tracked_stop),
              "Tracked stop should queue");
    own_tracked_stop = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then the tracked child exits and the supervisor reports stopped
    AR_ASSERT(!ar_agency__agent_exists(mut_agency, first_child),
              "Tracked stop should exit the named child");
    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "status"), "stopped") == 0,
              "Tracked stop should record stopped status");
    ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "stopped") == 0,
              "Observer should receive stopped status for tracked stop");

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
    ar_data__set_map_string(own_failed_handoff_start, "action", "start");
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
    ar_data__set_map_integer(own_failed_handoff_start,
                             "reply_to",
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
                     "handoff_failed") == 0,
              "Observer should receive failed start handoff status");

    ar_data_t *own_failed_continue_context = create_context();
    int64_t failed_continue_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_failed_continue_context);
    ar_data_t *own_failed_continue_start = ar_data__create_map();
    AR_ASSERT(own_failed_continue_start != NULL, "Failed continuation start should be created");
    ar_data__set_map_string(own_failed_continue_start, "action", "start");
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
    ar_data__set_map_integer(own_failed_continue_start,
                             "reply_to",
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
                     "handoff_failed") == 0,
              "Observer should receive failed spawn continuation status");

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_supervision_context);
    ar_data__destroy(own_observer_context);
    ar_data__destroy(own_untracked_context);
    ar_data__destroy(own_failed_handoff_context);
    ar_data__destroy(own_failed_continue_context);
}

int main(void) {
    printf("Running supervision method tests...\n\n");
    test_supervision__tracks_unbounded_children_and_restarts_failed_child();
    printf("\nAll supervision method tests passed!\n");
    return 0;
}
