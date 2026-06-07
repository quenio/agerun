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

static void append_agent_id(ar_data_t *mut_list, int64_t agent_id) {
    ar_data_t *own_id = ar_data__create_integer(checked_agent_id(agent_id));
    AR_ASSERT(own_id != NULL, "Agent id value should be created");
    AR_ASSERT(ar_data__list_add_last_data(mut_list, own_id), "Agent id should be appended");
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_text := message.text\n"
        "memory.last_correlation_id := message.correlation_id\n"
        "memory.last_status := message.status\n"
        "memory.last_work_id := message.work_id\n"
        "memory.last_assignment_count := message.assignment_count\n"
        "memory.last_sent_count := message.sent_count\n"
        "memory.last_route_status := message.route_status\n"
        "memory.last_route_sent := message.route_sent\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_distribution__assigns_unbounded_workers_through_routing(void) {
    printf("Testing distribution assigns unbounded workers through routing...\n");

    // Given distribution delegates assignments through a routing agent
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("distribution_routing");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "routing");
    load_method(own_fixture, "distribution");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_router_context = create_context();
    ar_data_t *own_distribution_context = create_context();
    ar_data_t *own_worker_a_context = create_context();
    ar_data_t *own_worker_b_context = create_context();
    ar_data_t *own_worker_c_context = create_context();
    ar_data_t *own_worker_d_context = create_context();
    ar_data_t *own_observer_context = create_context();
    int64_t router_agent = ar_agency__create_agent(
        mut_agency, "routing", "1.0.0", own_router_context);
    int64_t distribution_agent = ar_agency__create_agent(
        mut_agency, "distribution", "1.0.0", own_distribution_context);
    int64_t worker_a = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_worker_a_context);
    int64_t worker_b = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_worker_b_context);
    int64_t worker_c = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_worker_c_context);
    int64_t worker_d = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_worker_d_context);
    int64_t observer = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_observer_context);

    // When work is assigned to more workers than the old fixed slots allowed
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Distribution message should be created");
    ar_data_t *own_workers = ar_data__create_list();
    AR_ASSERT(own_workers != NULL, "Worker list should be created");
    append_agent_id(own_workers, worker_a);
    append_agent_id(own_workers, worker_b);
    append_agent_id(own_workers, worker_c);
    append_agent_id(own_workers, worker_d);

    ar_data__set_map_string(own_message, "action", "distribute");
    ar_data__set_map_integer(
        own_message, "routing_agent", checked_agent_id(router_agent));
    AR_ASSERT(ar_data__set_map_data(own_message, "workers", own_workers),
              "Worker list should be stored");
    own_workers = NULL;
    ar_data__set_map_string(own_message, "work_text", "shared-work");
    ar_data__set_map_string(own_message, "work_id", "job-2");
    ar_data__set_map_integer(own_message, "reply_to", checked_agent_id(observer));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, distribution_agent, own_message),
              "Distribution message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then all workers receive the assignment through routing
    const ar_data_t *ref_worker_a_memory = ar_agency__get_agent_memory(mut_agency, worker_a);
    const ar_data_t *ref_worker_b_memory = ar_agency__get_agent_memory(mut_agency, worker_b);
    const ar_data_t *ref_worker_c_memory = ar_agency__get_agent_memory(mut_agency, worker_c);
    const ar_data_t *ref_worker_d_memory = ar_agency__get_agent_memory(mut_agency, worker_d);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_a_memory, "last_text"), "shared-work") == 0,
              "Worker A should receive work");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_b_memory, "last_text"), "shared-work") == 0,
              "Worker B should receive work");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_c_memory, "last_text"), "shared-work") == 0,
              "Worker C should receive work");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_d_memory, "last_text"), "shared-work") == 0,
              "Worker D should receive work");

    const ar_data_t *ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_action"),
                     "distribution_result") == 0,
              "Observer should receive distribution result");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_work_id"),
                     "job-2") == 0,
              "Distribution result should preserve work id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_assignment_count") == 4,
              "Distribution result should report all routed workers");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_sent_count") == 4,
              "Distribution result should report all sent workers");

    ar_data_t *own_route_result = ar_data__create_map();
    AR_ASSERT(own_route_result != NULL, "Mismatched route result should be created");
    ar_data__set_map_string(own_route_result, "action", "route_result");
    ar_data__set_map_string(own_route_result, "status", "routed");
    ar_data__set_map_string(own_route_result, "correlation_id", "other-job");
    ar_data__set_map_integer(own_route_result, "routed_count", 99);
    ar_data__set_map_integer(own_route_result, "sent_count", 99);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, distribution_agent, own_route_result),
              "Mismatched route result should queue");
    own_route_result = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_work_id"),
                     "job-2") == 0,
              "Mismatched route result should not overwrite active work id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_assignment_count") == 4,
              "Mismatched route result should not overwrite assignment count");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_sent_count") == 4,
              "Mismatched route result should not overwrite sent count");

    // And a failed route handoff should immediately report terminal failure
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Failed distribution message should be created");
    own_workers = ar_data__create_list();
    AR_ASSERT(own_workers != NULL, "Failed worker list should be created");
    append_agent_id(own_workers, worker_a);
    ar_data__set_map_string(own_message, "action", "distribute");
    ar_data__set_map_integer(own_message, "routing_agent", 98765);
    AR_ASSERT(ar_data__set_map_data(own_message, "workers", own_workers),
              "Failed worker list should be stored");
    own_workers = NULL;
    ar_data__set_map_string(own_message, "work_text", "lost-work");
    ar_data__set_map_string(own_message, "work_id", "job-failed-route");
    ar_data__set_map_integer(own_message, "reply_to", checked_agent_id(observer));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, distribution_agent, own_message),
              "Failed distribution message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_action"),
                     "distribution_result") == 0,
              "Observer should receive failed handoff distribution result");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_work_id"),
                     "job-failed-route") == 0,
              "Failed handoff result should use the distribution work id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "route_failed") == 0,
              "Failed handoff should report route failure status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_assignment_count") == 0,
              "Failed handoff should not report assignments");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_sent_count") == 0,
              "Failed handoff should not report sent assignments");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_route_status"),
                     "route_failed") == 0,
              "Failed handoff should report route failure route status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_route_sent") == 0,
              "Failed handoff result should preserve the failed route send");

    // And a stale route_result after a failed handoff should not overwrite the terminal result
    own_route_result = ar_data__create_map();
    AR_ASSERT(own_route_result != NULL, "Manual route result should be created");
    ar_data__set_map_string(own_route_result, "action", "route_result");
    ar_data__set_map_string(own_route_result, "status", "ignored");
    ar_data__set_map_integer(own_route_result, "routed_count", 0);
    ar_data__set_map_integer(own_route_result, "sent_count", 0);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, distribution_agent, own_route_result),
              "Manual route result should queue");
    own_route_result = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_work_id"),
                     "job-failed-route") == 0,
              "Stale route result should not overwrite the failed handoff work id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "route_failed") == 0,
              "Stale route result should not overwrite failed handoff status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_route_sent") == 0,
              "Stale route result should preserve the failed route handoff");

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_router_context);
    ar_data__destroy(own_distribution_context);
    ar_data__destroy(own_worker_a_context);
    ar_data__destroy(own_worker_b_context);
    ar_data__destroy(own_worker_c_context);
    ar_data__destroy(own_worker_d_context);
    ar_data__destroy(own_observer_context);
}

int main(void) {
    printf("Running distribution method tests...\n\n");
    test_distribution__assigns_unbounded_workers_through_routing();
    printf("\nAll distribution method tests passed!\n");
    return 0;
}
