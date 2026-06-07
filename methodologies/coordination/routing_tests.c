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
        "memory.last_text := message.text\n"
        "memory.last_correlation_id := message.correlation_id\n"
        "memory.last_status := message.status\n"
        "memory.last_routed_count := message.routed_count\n"
        "memory.last_sent_count := message.sent_count\n"
        "memory.last_failed_count := message.failed_count\n"
        "memory.last_continuation_sent := message.continuation_sent\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_routing__forwards_one_and_many_messages(void) {
    printf("Testing routing forwards one and many messages...\n");

    // Given a routing agent and several receiver agents
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("routing_forward_many");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "routing");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_routing_context = create_context();
    ar_data_t *own_receiver_one_context = create_context();
    ar_data_t *own_receiver_a_context = create_context();
    ar_data_t *own_receiver_b_context = create_context();
    ar_data_t *own_receiver_c_context = create_context();
    ar_data_t *own_receiver_d_context = create_context();
    ar_data_t *own_report_context = create_context();
    int64_t routing_agent = ar_agency__create_agent(
        mut_agency, "routing", "1.0.0", own_routing_context);
    int64_t receiver_one = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_one_context);
    int64_t receiver_a = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_a_context);
    int64_t receiver_b = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_b_context);
    int64_t receiver_c = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_c_context);
    int64_t receiver_d = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_d_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_report_context);

    // When a one-to-one route request is processed
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Route message should be created");
    ar_data__set_map_string(own_message, "action", "route");
    ar_data__set_map_string(own_message, "mode", "one");
    ar_data__set_map_integer(
        own_message, "target", checked_agent_id(receiver_one));
    ar_data__set_map_string(own_message, "payload_action", "work");
    ar_data__set_map_string(own_message, "payload_text", "alpha");
    ar_data__set_map_string(own_message, "correlation_id", "job-1");
    ar_data__set_map_integer(own_message, "reply_to", 0);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Route message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then the receiver observes the forwarded payload contract
    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_one);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_action"), "work") == 0,
              "Receiver should observe forwarded action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_text"), "alpha") == 0,
              "Receiver should observe forwarded text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_correlation_id"),
                     "job-1") == 0,
              "Receiver should observe forwarded correlation id");

    // When a one-to-many route request carries more than three targets
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Many route message should be created");
    ar_data__set_map_string(own_message, "action", "route");
    ar_data__set_map_string(own_message, "mode", "many");
    ar_data_t *own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Targets list should be created");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, checked_agent_id(receiver_a)),
              "First target should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, checked_agent_id(receiver_b)),
              "Second target should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, checked_agent_id(receiver_c)),
              "Third target should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, checked_agent_id(receiver_d)),
              "Fourth target should be stored");
    AR_ASSERT(ar_data__set_map_data(own_message, "targets", own_targets),
              "Route message should own targets list");
    own_targets = NULL;
    ar_data__set_map_string(own_message, "payload_action", "work");
    ar_data__set_map_string(own_message, "payload_text", "fanout");
    ar_data__set_map_string(own_message, "correlation_id", "job-2");
    ar_data__set_map_integer(own_message, "reply_to", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Many route message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then every target receives the forwarded payload through recursive head/tail routing
    const ar_data_t *ref_receiver_a_memory = ar_agency__get_agent_memory(mut_agency, receiver_a);
    const ar_data_t *ref_receiver_b_memory = ar_agency__get_agent_memory(mut_agency, receiver_b);
    const ar_data_t *ref_receiver_c_memory = ar_agency__get_agent_memory(mut_agency, receiver_c);
    const ar_data_t *ref_receiver_d_memory = ar_agency__get_agent_memory(mut_agency, receiver_d);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_text"), "fanout") == 0,
              "First many target should observe forwarded text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_b_memory, "last_text"), "fanout") == 0,
              "Second many target should observe forwarded text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_c_memory, "last_text"), "fanout") == 0,
              "Third many target should observe forwarded text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_d_memory, "last_text"), "fanout") == 0,
              "Fourth many target should observe forwarded text");

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_action"), "route_result") == 0,
              "Report receiver should observe route result action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "routed") == 0,
              "Report receiver should observe routed status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_correlation_id"),
                     "job-2") == 0,
              "Route result should preserve correlation id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_routed_count") == 4,
              "Route result should count every successful target route");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sent_count") == 4,
              "Route result should count every successful target send");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failed_count") == 0,
              "Route result should report no failed target sends");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_continuation_sent") == 0,
              "Final route result should report no pending continuation");

    // When a many route includes a positive target id that cannot receive messages
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Partial failure route message should be created");
    ar_data__set_map_string(own_message, "action", "route");
    ar_data__set_map_string(own_message, "mode", "many");
    own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Partial failure targets list should be created");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, checked_agent_id(receiver_a)),
              "First partial target should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 98765),
              "Invalid positive target should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, checked_agent_id(receiver_b)),
              "Final partial target should be stored");
    AR_ASSERT(ar_data__set_map_data(own_message, "targets", own_targets),
              "Partial route message should own targets list");
    own_targets = NULL;
    ar_data__set_map_string(own_message, "payload_action", "work");
    ar_data__set_map_string(own_message, "payload_text", "partial-fanout");
    ar_data__set_map_string(own_message, "correlation_id", "job-failed-send");
    ar_data__set_map_integer(own_message, "reply_to", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Partial failure route message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then the route result reports successful and failed deliveries separately
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_text"),
                     "partial-fanout") == 0,
              "First valid partial target should observe forwarded text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_b_memory, "last_text"),
                     "partial-fanout") == 0,
              "Final valid partial target should observe forwarded text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_action"),
                     "route_result") == 0,
              "Partial failure should still emit route result action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "route_failed") == 0,
              "Partial failure should report route_failed status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_correlation_id"),
                     "job-failed-send") == 0,
              "Partial failure result should preserve correlation id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_routed_count") == 2,
              "Partial failure result should count successful target routes only");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sent_count") == 2,
              "Partial failure result should count successful target sends only");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failed_count") == 1,
              "Partial failure result should count failed target sends");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_continuation_sent") == 0,
              "Partial failure final result should report no pending continuation");

    // When a one-to-one keyed route does not select a target
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Missed keyed route message should be created");
    ar_data__set_map_string(own_message, "action", "route");
    ar_data__set_map_string(own_message, "mode", "one");
    ar_data__set_map_string(own_message, "route_key", "missing");
    ar_data__set_map_string(own_message, "route_a_key", "known");
    ar_data__set_map_integer(own_message, "target_a", checked_agent_id(receiver_c));
    ar_data__set_map_string(own_message, "payload_action", "work");
    ar_data__set_map_string(own_message, "payload_text", "missed");
    ar_data__set_map_string(own_message, "correlation_id", "job-missing-key");
    ar_data__set_map_integer(own_message, "reply_to", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Missed keyed route message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then the route result reports failure instead of successful zero-delivery routing
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_c_memory, "last_text"), "fanout") == 0,
              "Missed keyed route should not forward to unmatched target");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "route_failed") == 0,
              "Missed keyed route should report route_failed status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_correlation_id"),
                     "job-missing-key") == 0,
              "Missed keyed route result should preserve correlation id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_routed_count") == 0,
              "Missed keyed route should report zero routed targets");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sent_count") == 0,
              "Missed keyed route should report zero sent targets");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failed_count") == 0,
              "Missed keyed route should not count a failed positive target send");

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_routing_context);
    ar_data__destroy(own_receiver_one_context);
    ar_data__destroy(own_receiver_a_context);
    ar_data__destroy(own_receiver_b_context);
    ar_data__destroy(own_receiver_c_context);
    ar_data__destroy(own_receiver_d_context);
    ar_data__destroy(own_report_context);
}

int main(void) {
    printf("Running routing method tests...\n\n");
    test_routing__forwards_one_and_many_messages();
    printf("\nAll routing method tests passed!\n");
    return 0;
}
