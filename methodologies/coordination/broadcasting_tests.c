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
    AR_ASSERT(ar_data__list_add_last_integer(mut_list, checked_agent_id(agent_id)),
              "Agent id should be appended");
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_text := message.text\n"
        "memory.last_kind := message.kind\n"
        "memory.last_source := message.source\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_status := message.status\n"
        "memory.last_state := message.state\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_recipient_count := message.recipient_count\n"
        "memory.last_sent_count := message.sent_count\n"
        "memory.last_failed_count := message.failed_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static ar_data_t *create_targets(int64_t first,
                                 int64_t second,
                                 int64_t third,
                                 int64_t fourth) {
    ar_data_t *own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Targets list should be created");
    append_agent_id(own_targets, first);
    append_agent_id(own_targets, second);
    append_agent_id(own_targets, third);
    append_agent_id(own_targets, fourth);
    return own_targets;
}

static ar_data_t *create_payload(const char *ref_action,
                                 const char *ref_text,
                                 const char *ref_kind,
                                 int source) {
    ar_data_t *own_payload = ar_data__create_map();
    AR_ASSERT(own_payload != NULL, "Broadcast payload should be created");
    AR_ASSERT(ar_data__set_map_string(own_payload, "action", ref_action),
              "Broadcast payload should set action");
    AR_ASSERT(ar_data__set_map_string(own_payload, "text", ref_text),
              "Broadcast payload should set text");
    AR_ASSERT(ar_data__set_map_string(own_payload, "kind", ref_kind),
              "Broadcast payload should set caller-owned field");
    AR_ASSERT(ar_data__set_map_integer(own_payload, "source", source),
              "Broadcast payload should set standard source");
    return own_payload;
}

static void test_broadcasting__sends_same_payload_to_all_recipients(void) {
    printf("Testing broadcasting sends same payload to all recipients...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("broadcasting_fanout");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "broadcasting");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_broadcasting_context = create_context();
    ar_data_t *own_receiver_a_context = create_context();
    ar_data_t *own_receiver_b_context = create_context();
    ar_data_t *own_receiver_c_context = create_context();
    ar_data_t *own_receiver_d_context = create_context();
    ar_data_t *own_report_context = create_context();
    int64_t broadcasting_agent = ar_agency__create_agent(
        mut_agency, "broadcasting", "1.0.0", own_broadcasting_context);
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

    // When one broadcast carries more than three recipients
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Broadcast message should be created");
    ar_data__set_map_string(own_message, "request", "broadcasting_start");
    ar_data_t *own_targets = create_targets(receiver_a, receiver_b, receiver_c, receiver_d);
    AR_ASSERT(ar_data__set_map_data(own_message, "targets", own_targets),
              "Broadcast message should own targets list");
    own_targets = NULL;
    ar_data_t *own_payload = create_payload("domain_event",
                                            "fanout",
                                            "caller-shaped",
                                            checked_agent_id(receiver_d));
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Broadcast message should own opaque payload");
    own_payload = NULL;
    ar_data__set_map_string(own_message, "trace_id", "broadcast-fanout");
    ar_data__set_map_integer(own_message, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, broadcasting_agent, own_message),
              "Broadcast message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then every recipient receives the same delivery payload
    const ar_data_t *ref_receiver_a_memory = ar_agency__get_agent_memory(mut_agency, receiver_a);
    const ar_data_t *ref_receiver_b_memory = ar_agency__get_agent_memory(mut_agency, receiver_b);
    const ar_data_t *ref_receiver_c_memory = ar_agency__get_agent_memory(mut_agency, receiver_c);
    const ar_data_t *ref_receiver_d_memory = ar_agency__get_agent_memory(mut_agency, receiver_d);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_action"),
                     "domain_event") == 0,
              "First recipient should receive caller payload action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_b_memory, "last_action"),
                     "domain_event") == 0,
              "Second recipient should receive caller payload action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_c_memory, "last_action"),
                     "domain_event") == 0,
              "Third recipient should receive caller payload action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_d_memory, "last_action"),
                     "domain_event") == 0,
              "Fourth recipient should receive caller payload action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_text"), "fanout") == 0,
              "First recipient should receive broadcast text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_b_memory, "last_text"), "fanout") == 0,
              "Second recipient should receive broadcast text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_c_memory, "last_text"), "fanout") == 0,
              "Third recipient should receive broadcast text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_d_memory, "last_text"), "fanout") == 0,
              "Fourth recipient should receive broadcast text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_kind"),
                     "caller-shaped") == 0,
              "First recipient should receive caller-owned payload field");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_a_memory, "last_source") ==
                  checked_agent_id(receiver_d),
              "First recipient should receive only the payload source");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_kind"),
                     "caller-shaped") == 0,
              "Broadcasting should preserve caller-owned fields");

    const ar_data_t *ref_broadcasting_memory =
        ar_agency__get_agent_memory(mut_agency, broadcasting_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_broadcasting_memory, "status"),
                     "success") == 0,
              "Broadcasting agent should record broadcasted status");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 4,
              "Broadcasting agent should count every successful recipient");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 4,
              "Broadcasting agent should count every successful send");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "failed_count") == 0,
              "Broadcasting agent should report no failed sends");

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_response"),
                     "broadcasting_result") == 0,
              "Broadcasting should emit a response");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_source") ==
                  checked_agent_id(broadcasting_agent),
              "Broadcasting response should identify the broadcasting source");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "success") == 0,
              "Broadcasting should report standard success status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "broadcast-fanout") == 0,
              "Broadcast result should preserve trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 4,
              "Broadcast result should report successful recipient count");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Broadcast result should report no failed recipients");

    // When an unrelated message carries positive targets
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Ignored message should be created");
    ar_data__set_map_string(own_message, "request", "broadcasting_ignored");
    own_targets = create_targets(receiver_a, receiver_b, receiver_c, receiver_d);
    AR_ASSERT(ar_data__set_map_data(own_message, "targets", own_targets),
              "Ignored message should own targets list");
    own_targets = NULL;
    own_payload = create_payload("domain_event", "ignored", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Ignored message should own opaque payload");
    own_payload = NULL;
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, broadcasting_agent, own_message),
              "Ignored message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_text"), "fanout") == 0,
              "Ignored message should not deliver to first recipient");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_broadcasting_memory, "status"),
                     "success") == 0,
              "Ignored message should not overwrite broadcast status");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 4,
              "Ignored message should not change recipient count");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 4,
              "Ignored message should not change sent count");

    // When an unrelated message omits broadcast-only fields
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Ignored message without targets should be created");
    ar_data__set_map_string(own_message, "request", "broadcasting_ignored");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, broadcasting_agent, own_message),
              "Ignored message without targets should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_broadcasting_memory, "status"),
                     "success") == 0,
              "Ignored message without targets should not overwrite broadcast status");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 4,
              "Ignored message without targets should not change recipient count");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 4,
              "Ignored message without targets should not change sent count");

    // When one recipient cannot receive messages
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Partial broadcast message should be created");
    ar_data__set_map_string(own_message, "request", "broadcasting_start");
    own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Partial broadcast targets list should be created");
    append_agent_id(own_targets, receiver_a);
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 98765),
              "Invalid positive recipient should be stored");
    append_agent_id(own_targets, receiver_b);
    AR_ASSERT(ar_data__set_map_data(own_message, "targets", own_targets),
              "Partial broadcast message should own targets list");
    own_targets = NULL;
    own_payload = create_payload("domain_event", "partial", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Partial broadcast message should own opaque payload");
    own_payload = NULL;
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, broadcasting_agent, own_message),
              "Partial broadcast message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_text"), "partial") == 0,
              "First valid recipient should receive partial broadcast text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_b_memory, "last_text"), "partial") == 0,
              "Final valid recipient should receive partial broadcast text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_broadcasting_memory, "status"),
                     "failure") == 0,
              "Partial failure should record broadcast_failed status");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 2,
              "Partial failure should count successful recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 2,
              "Partial failure should count successful sends");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "failed_count") == 1,
              "Partial failure should count failed sends");

    // When the target list has no positive recipients
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Zero-recipient broadcast message should be created");
    ar_data__set_map_string(own_message, "request", "broadcasting_start");
    own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Zero-recipient targets list should be created");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 0),
              "Zero recipient placeholder should be stored");
    AR_ASSERT(ar_data__set_map_data(own_message, "targets", own_targets),
              "Zero-recipient broadcast message should own targets list");
    own_targets = NULL;
    own_payload = create_payload("domain_event", "zero", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Zero-recipient broadcast message should own opaque payload");
    own_payload = NULL;
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, broadcasting_agent, own_message),
              "Zero-recipient broadcast message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_broadcasting_memory, "status"),
                     "failure") == 0,
              "Zero-recipient broadcast should record broadcast_failed");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 0,
              "Zero-recipient broadcast should record zero recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 0,
              "Zero-recipient broadcast should record zero sent messages");

    // When consecutive placeholder targets precede a valid recipient
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Placeholder broadcast message should be created");
    ar_data__set_map_string(own_message, "request", "broadcasting_start");
    own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Placeholder targets list should be created");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 0),
              "First placeholder should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 0),
              "Second placeholder should be stored");
    append_agent_id(own_targets, receiver_c);
    AR_ASSERT(ar_data__set_map_data(own_message, "targets", own_targets),
              "Placeholder broadcast message should own targets list");
    own_targets = NULL;
    own_payload = create_payload("domain_event", "skip-placeholders", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Placeholder broadcast message should own opaque payload");
    own_payload = NULL;
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, broadcasting_agent, own_message),
              "Placeholder broadcast message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_c_memory, "last_text"),
                     "skip-placeholders") == 0,
              "Later valid recipient should receive placeholder broadcast text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_broadcasting_memory, "status"),
                     "success") == 0,
              "Placeholder broadcast should record broadcasted status");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 1,
              "Placeholder broadcast should count one successful recipient");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 1,
              "Placeholder broadcast should count one sent message");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "failed_count") == 0,
              "Placeholder broadcast should report no failed sends");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_broadcasting_context);
    ar_data__destroy(own_receiver_a_context);
    ar_data__destroy(own_receiver_b_context);
    ar_data__destroy(own_receiver_c_context);
    ar_data__destroy(own_receiver_d_context);
    ar_data__destroy(own_report_context);
}

static void test_broadcasting__reports_failed_when_targets_missing(void) {
    printf("Testing broadcasting reports failed when targets are missing...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("broadcasting_missing_targets");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "broadcasting");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_broadcasting_context = create_context();
    ar_data_t *own_report_context = create_context();
    int64_t broadcasting_agent = ar_agency__create_agent(
        mut_agency, "broadcasting", "1.0.0", own_broadcasting_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_report_context);

    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Broadcast message without targets should be created");
    ar_data__set_map_string(own_message, "request", "broadcasting_start");
    ar_data_t *own_payload = create_payload("domain_event", "missing", "caller-shaped", 0);
    AR_ASSERT(ar_data__set_map_data(own_message, "payload", own_payload),
              "Broadcast message without targets should own opaque payload");
    own_payload = NULL;
    ar_data__set_map_string(own_message, "trace_id", "broadcast-missing-targets");
    ar_data__set_map_integer(own_message, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, broadcasting_agent, own_message),
              "Broadcast message without targets should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_broadcasting_memory =
        ar_agency__get_agent_memory(mut_agency, broadcasting_agent);
    const char *ref_status = ar_data__get_map_string(ref_broadcasting_memory, "status");
    AR_ASSERT(ref_status != NULL, "Broadcasting should record status without targets");
    AR_ASSERT(strcmp(ref_status, "failure") == 0,
              "Broadcast without targets should record broadcast_failed");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 0,
              "Broadcast without targets should record zero recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 0,
              "Broadcast without targets should record zero sent messages");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "failed_count") == 0,
              "Broadcast without targets should record zero failed sends");

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_response"),
                     "broadcasting_result") == 0,
              "Broadcast without targets should emit a response");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Broadcast without targets should report standard failure status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"),
                     "broadcast_failed") == 0,
              "Broadcast without targets should report failed status");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 0,
              "Broadcast without targets should report zero successes");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Broadcast without targets should report zero failures");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_broadcasting_context);
    ar_data__destroy(own_report_context);
}

int main(void) {
    printf("Running broadcasting method tests...\n\n");
    test_broadcasting__sends_same_payload_to_all_recipients();
    test_broadcasting__reports_failed_when_targets_missing();
    printf("\nAll broadcasting method tests passed!\n");
    return 0;
}
