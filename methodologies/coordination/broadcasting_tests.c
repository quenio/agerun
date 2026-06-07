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
        "memory.last_text := message.text\n"
        "memory.last_kind := message.kind\n"
        "memory.last_reply_to := message.reply_to\n"
        "memory.last_source := message.source\n"
        "memory.last_correlation_id := message.correlation_id\n"
        "memory.last_status := message.status\n"
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
                                 int reply_to) {
    ar_data_t *own_payload = ar_data__create_map();
    AR_ASSERT(own_payload != NULL, "Broadcast payload should be created");
    AR_ASSERT(ar_data__set_map_string(own_payload, "action", ref_action),
              "Broadcast payload should set action");
    AR_ASSERT(ar_data__set_map_string(own_payload, "text", ref_text),
              "Broadcast payload should set text");
    AR_ASSERT(ar_data__set_map_string(own_payload, "kind", ref_kind),
              "Broadcast payload should set caller-owned field");
    AR_ASSERT(ar_data__set_map_string(own_payload, "source", "caller-owned-source"),
              "Broadcast payload should set caller-owned source field");
    if (reply_to > 0) {
        AR_ASSERT(ar_data__set_map_integer(own_payload, "reply_to", reply_to),
                  "Broadcast payload should set caller-owned reply target");
    }
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

    // When one broadcast carries more than three recipients
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Broadcast message should be created");
    ar_data__set_map_string(own_message, "action", "broadcast");
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
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_a_memory, "last_reply_to") ==
                  checked_agent_id(receiver_d),
              "First recipient should receive only the payload reply_to");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_a_memory, "last_source"),
                     "caller-owned-source") == 0,
              "Broadcasting should preserve caller-owned source field");

    const ar_data_t *ref_broadcasting_memory =
        ar_agency__get_agent_memory(mut_agency, broadcasting_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_broadcasting_memory, "status"),
                     "broadcasted") == 0,
              "Broadcasting agent should record broadcasted status");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 4,
              "Broadcasting agent should count every successful recipient");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 4,
              "Broadcasting agent should count every successful send");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "failed_count") == 0,
              "Broadcasting agent should report no failed sends");

    // When one recipient cannot receive messages
    own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Partial broadcast message should be created");
    ar_data__set_map_string(own_message, "action", "broadcast");
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
                     "broadcast_failed") == 0,
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
    ar_data__set_map_string(own_message, "action", "broadcast");
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
                     "broadcast_failed") == 0,
              "Zero-recipient broadcast should record broadcast_failed");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "recipient_count") == 0,
              "Zero-recipient broadcast should record zero recipients");
    AR_ASSERT(ar_data__get_map_integer(ref_broadcasting_memory, "sent_count") == 0,
              "Zero-recipient broadcast should record zero sent messages");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_broadcasting_context);
    ar_data__destroy(own_receiver_a_context);
    ar_data__destroy(own_receiver_b_context);
    ar_data__destroy(own_receiver_c_context);
    ar_data__destroy(own_receiver_d_context);
}

int main(void) {
    printf("Running broadcasting method tests...\n\n");
    test_broadcasting__sends_same_payload_to_all_recipients();
    printf("\nAll broadcasting method tests passed!\n");
    return 0;
}
