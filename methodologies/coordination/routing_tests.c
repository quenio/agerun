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

static ar_data_t *create_routes(
        const char **ref_keys,
        size_t key_count,
        const int *ref_recipients,
        size_t recipient_count) {
    ar_data_t *own_routes = ar_data__create_map();
    AR_ASSERT(own_routes != NULL, "Routes map should be created");

    ar_data_t *own_keys = ar_data__create_list();
    AR_ASSERT(own_keys != NULL, "Route keys list should be created");
    for (size_t i = 0; i < key_count; i++) {
        AR_ASSERT(ar_data__list_add_last_string(own_keys, ref_keys[i]),
                  "Route key should be stored");
    }
    AR_ASSERT(ar_data__set_map_data(own_routes, "keys", own_keys),
              "Routes map should own keys list");
    own_keys = NULL;

    ar_data_t *own_recipients = ar_data__create_list();
    AR_ASSERT(own_recipients != NULL, "Route recipients list should be created");
    for (size_t i = 0; i < recipient_count; i++) {
        AR_ASSERT(ar_data__list_add_last_integer(own_recipients, ref_recipients[i]),
                  "Route recipient should be stored");
    }
    AR_ASSERT(ar_data__set_map_data(own_routes, "recipients", own_recipients),
              "Routes map should own recipients list");
    own_recipients = NULL;

    return own_routes;
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_text := message.text\n"
        "memory.last_kind := message.kind\n"
        "memory.last_sender := message.sender\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_status := message.status\n"
        "memory.last_state := message.state\n"
        "memory.last_routed_count := message.routed_count\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_sent_count := message.sent_count\n"
        "memory.last_failed_count := message.failed_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static ar_data_t *create_payload(const char *ref_action,
                                 const char *ref_text,
                                 const char *ref_kind,
                                 int sender) {
    ar_data_t *own_payload = ar_data__create_map();
    AR_ASSERT(own_payload != NULL, "Route payload should be created");
    AR_ASSERT(ar_data__set_map_string(own_payload, "action", ref_action),
              "Route payload should set action");
    AR_ASSERT(ar_data__set_map_string(own_payload, "text", ref_text),
              "Route payload should set text");
    AR_ASSERT(ar_data__set_map_string(own_payload, "kind", ref_kind),
              "Route payload should set caller-owned field");
    AR_ASSERT(ar_data__set_map_integer(own_payload, "sender", sender),
              "Route payload should set standard sender");
    return own_payload;
}

static void test_routing__selects_one_recipient_by_key_only(void) {
    printf("Testing routing selects one recipient by key only...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("routing_keyed_only");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "routing");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_routing_context = create_context();
    ar_data_t *own_receiver_a_context = create_context();
    ar_data_t *own_receiver_b_context = create_context();
    ar_data_t *own_receiver_c_context = create_context();
    ar_data_t *own_receiver_d_context = create_context();
    ar_data_t *own_report_context = create_context();
    int64_t routing_agent = ar_agency__create_agent(
        mut_agency, "routing", "1.0.0", own_routing_context);
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

    // When a direct-recipient request is sent without a route key
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Direct route message should be created");
    ar_data__set_map_string(own_message, "request", "routing_start");
    ar_data__set_map_integer(own_message, "recipient", checked_agent_id(receiver_a));
    ar_data_t *own_payload = create_payload("domain_event", "direct", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Direct route message should own opaque payload");
    own_payload = NULL;
    ar_data__set_map_string(own_message, "trace_id", "job-direct");
    ar_data__set_map_integer(own_message, "sender", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Direct route message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then routing does not treat a direct recipient as a supported route mechanism
    const ar_data_t *ref_receiver_a_memory = ar_agency__get_agent_memory(mut_agency, receiver_a);
    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(ar_data__get_map_data(ref_receiver_a_memory, "last_text") == NULL,
              "Routing should not forward direct recipient requests");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_response"),
                     "routing_result") == 0,
              "Direct recipient request should emit a route response");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sender") ==
                  checked_agent_id(routing_agent),
              "Route response should identify the routing sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Direct recipient request should report standard failure status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"),
                     "route_failed") == 0,
              "Direct recipient request should fail because routing is key-based only");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_routed_count") == 0,
              "Direct recipient request should report zero routed recipients");

    // When an unrelated message carries a matching route table
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Ignored route-shaped message should be created");
    ar_data__set_map_string(own_message, "request", "routing_ignored");
    ar_data__set_map_string(own_message, "route_key", "ignored-key");
    const char *ref_ignored_route_keys[] = {"ignored-key"};
    const int ref_ignored_route_recipients[] = {checked_agent_id(receiver_b)};
    ar_data_t *own_routes = create_routes(ref_ignored_route_keys, 1, ref_ignored_route_recipients, 1);
    AR_ASSERT(ar_data__set_map_data(own_message, "routes", own_routes),
              "Ignored route-shaped message should own routes map");
    own_routes = NULL;
    own_payload = create_payload("domain_event", "ignored", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Ignored route-shaped message should own opaque payload");
    own_payload = NULL;
    ar_data__set_map_string(own_message, "trace_id", "job-ignored");
    ar_data__set_map_integer(own_message, "sender", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Ignored route-shaped message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_routing_memory = ar_agency__get_agent_memory(mut_agency, routing_agent);
    const ar_data_t *ref_receiver_b_memory = ar_agency__get_agent_memory(mut_agency, receiver_b);
    AR_ASSERT(ar_data__get_map_data(ref_receiver_b_memory, "last_text") == NULL,
              "Ignored route-shaped message should not deliver payload");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_routing_memory, "status"),
                     "failure") == 0,
              "Ignored route-shaped message should not overwrite standard route status");
    AR_ASSERT(ar_data__get_map_integer(ref_routing_memory, "routed_count") == 0,
              "Ignored route-shaped message should not change routed count");
    AR_ASSERT(ar_data__get_map_integer(ref_routing_memory, "sent_count") == 0,
              "Ignored route-shaped message should not change sent count");

    // When a keyed route request carries more than three candidate routes
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Keyed route message should be created");
    ar_data__set_map_string(own_message, "request", "routing_start");
    ar_data__set_map_string(own_message, "route_key", "delta");
    const char *ref_route_keys[] = {"alpha", "beta", "gamma", "delta"};
    const int ref_route_recipients[] = {
        checked_agent_id(receiver_a),
        checked_agent_id(receiver_b),
        checked_agent_id(receiver_c),
        checked_agent_id(receiver_d)
    };
    own_routes = create_routes(ref_route_keys, 4, ref_route_recipients, 4);
    AR_ASSERT(ar_data__set_map_data(own_message, "routes", own_routes),
              "Keyed route message should own routes map");
    own_routes = NULL;
    own_payload = create_payload("domain_event",
                                 "keyed",
                                 "caller-shaped",
                                 checked_agent_id(receiver_a));
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Keyed route message should own opaque payload");
    own_payload = NULL;
    ar_data__set_map_string(own_message, "trace_id", "job-keyed");
    ar_data__set_map_integer(own_message, "sender", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Keyed route message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then keyed routing scans the unbounded route list until it finds the matching route
    const ar_data_t *ref_receiver_d_memory = ar_agency__get_agent_memory(mut_agency, receiver_d);
    const char *ref_receiver_d_action =
        ar_data__get_map_string(ref_receiver_d_memory, "last_action");
    AR_ASSERT(ref_receiver_d_action != NULL, "Fourth keyed route recipient should receive payload");
    AR_ASSERT(strcmp(ref_receiver_d_action, "domain_event") == 0,
              "Fourth keyed route recipient should observe caller payload action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_d_memory, "last_text"), "keyed") == 0,
              "Fourth keyed route recipient should observe delivered text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_d_memory, "last_kind"),
                     "caller-shaped") == 0,
              "Fourth keyed route recipient should observe caller-owned field");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_d_memory, "last_sender") ==
                  checked_agent_id(receiver_a),
              "Routing should preserve caller payload sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"), "routed") == 0,
              "Keyed route should report routed status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "success") == 0,
              "Keyed route should report standard success status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "job-keyed") == 0,
              "Keyed route result should preserve trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_routed_count") == 1,
              "Keyed route should report one routed recipient");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 1,
              "Keyed route should report one successful recipient");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Keyed route should report no failed recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sent_count") == 1,
              "Keyed route should report one sent recipient");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failed_count") == 0,
              "Keyed route should report no failed recipient sends");

    // When a keyed route does not select a recipient
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Missed keyed route message should be created");
    ar_data__set_map_string(own_message, "request", "routing_start");
    ar_data__set_map_string(own_message, "route_key", "missing");
    const char *ref_missed_route_keys[] = {"known"};
    const int ref_missed_route_recipients[] = {checked_agent_id(receiver_c)};
    own_routes = create_routes(ref_missed_route_keys, 1, ref_missed_route_recipients, 1);
    AR_ASSERT(ar_data__set_map_data(own_message, "routes", own_routes),
              "Missed keyed route message should own routes map");
    own_routes = NULL;
    own_payload = create_payload("domain_event", "missed", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Missed keyed route message should own opaque payload");
    own_payload = NULL;
    ar_data__set_map_integer(own_message, "sender", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, routing_agent, own_message),
              "Missed keyed route message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_c_memory = ar_agency__get_agent_memory(mut_agency, receiver_c);
    AR_ASSERT(ar_data__get_map_data(ref_receiver_c_memory, "last_text") == NULL,
              "Missed keyed route should not forward to unmatched recipient");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"),
                     "route_failed") == 0,
              "Missed keyed route should report route_failed status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Missed keyed route should report standard failure status");
    int expected_generated_trace_id =
        checked_agent_id(report_agent) * 1000 + checked_agent_id(routing_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_trace_id") ==
                  expected_generated_trace_id,
              "Missed keyed route result should generate trace id when omitted");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_routed_count") == 0,
              "Missed keyed route should report zero routed recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 0,
              "Missed keyed route should report zero successful recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Missed keyed route should report zero failed recipient sends");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sent_count") == 0,
              "Missed keyed route should report zero sent recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failed_count") == 0,
              "Missed keyed route should not count a failed positive recipient send");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_routing_context);
    ar_data__destroy(own_receiver_a_context);
    ar_data__destroy(own_receiver_b_context);
    ar_data__destroy(own_receiver_c_context);
    ar_data__destroy(own_receiver_d_context);
    ar_data__destroy(own_report_context);
}

int main(void) {
    printf("Running routing method tests...\n\n");
    test_routing__selects_one_recipient_by_key_only();
    printf("\nAll routing method tests passed!\n");
    return 0;
}
