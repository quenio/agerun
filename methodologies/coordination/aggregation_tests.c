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

static void send_collect(ar_agency_t *mut_agency,
                         int64_t aggregation_agent,
                         const char *ref_trace_id,
                         const char *ref_payload) {
    ar_data_t *own_collect = ar_data__create_map();
    AR_ASSERT(own_collect != NULL, "Collect message should be created");
    ar_data__set_map_string(own_collect, "request", "aggregation_collect");
    ar_data__set_map_string(own_collect, "trace_id", ref_trace_id);
    ar_data__set_map_string(own_collect, "payload", ref_payload);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_collect),
              "Collect message should queue");
    own_collect = NULL;
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_source := message.source\n"
        "memory.last_status := message.status\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_payloads := message.payloads\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_aggregation__combines_required_payloads(void) {
    printf("Testing aggregation combines required payloads...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("aggregation_payloads");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "aggregation");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_aggregation_context = create_context();
    ar_data_t *own_receiver_context = create_context();
    int64_t aggregation_agent = ar_agency__create_agent(
        mut_agency, "aggregation", "1.0.0", own_aggregation_context);
    int64_t receiver_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_context);

    ar_data_t *own_reset = ar_data__create_map();
    AR_ASSERT(own_reset != NULL, "Aggregate reset message should be created");
    ar_data__set_map_string(own_reset, "request", "aggregation_start");
    ar_data__set_map_string(own_reset, "trace_id", "agg-trace-1");
    ar_data__set_map_integer(own_reset, "expected_count", 4);
    ar_data__set_map_integer(own_reset, "source", checked_agent_id(receiver_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_reset),
              "Aggregate reset message should queue");
    own_reset = NULL;

    send_collect(mut_agency, aggregation_agent, "agg-trace-1", "alpha");
    send_collect(mut_agency, aggregation_agent, "agg-trace-1", "beta");
    send_collect(mut_agency, aggregation_agent, "agg-trace-1", "gamma");
    send_collect(mut_agency, aggregation_agent, "agg-trace-1", "delta");

    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    const char *ref_action = ar_data__get_map_string(ref_receiver_memory, "last_action");
    AR_ASSERT(ref_action == NULL, "Aggregate completion response should not include action");
    const char *ref_response = ar_data__get_map_string(ref_receiver_memory, "last_response");
    AR_ASSERT(ref_response != NULL && strcmp(ref_response, "aggregation_result") == 0,
              "Aggregate completion should be a response");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_source") ==
                  checked_agent_id(aggregation_agent),
              "Aggregate completion should identify the aggregation source");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_status"), "success") == 0,
              "Aggregate completion should report standard success status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_trace_id"),
                     "agg-trace-1") == 0,
              "Aggregate completion should preserve trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_success_count") == 4,
              "Aggregate completion should report successful payload count");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_failure_count") == 0,
              "Aggregate completion should report no failed payloads");
    const ar_data_t *ref_payloads = ar_data__get_map_data(ref_receiver_memory, "last_payloads");
    AR_ASSERT(ref_payloads != NULL, "Aggregate should include payload list");
    AR_ASSERT(ar_data__get_type(ref_payloads) == AR_DATA_TYPE__LIST,
              "Aggregate payloads should be a list");
    AR_ASSERT(ar_data__list_count(ref_payloads) == 4,
              "Aggregate payloads should include all required values");
    ar_data_t **own_items = ar_data__list_items(ref_payloads);
    AR_ASSERT(own_items != NULL, "Aggregate payload items should be readable");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[0]), "alpha") == 0,
              "Aggregate payloads should include first value");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[1]), "beta") == 0,
              "Aggregate payloads should include second value");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[2]), "gamma") == 0,
              "Aggregate payloads should include third value");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[3]), "delta") == 0,
              "Aggregate payloads should include fourth value");
    AR__HEAP__FREE(own_items);
    send_collect(mut_agency, aggregation_agent, "agg-trace-1", "epsilon");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_aggregation_memory =
        ar_agency__get_agent_memory(mut_agency, aggregation_agent);
    const ar_data_t *ref_stored_payloads = ar_data__get_map_data(ref_aggregation_memory, "payloads");
    AR_ASSERT(ref_stored_payloads != NULL, "Aggregation memory should retain payload list");
    AR_ASSERT(ar_data__list_count(ref_stored_payloads) == 4,
              "Late payloads should not mutate completed aggregation state");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "count") == 4,
              "Late payloads should not increment completed aggregation count");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_success_count") == 4,
              "Late payloads should not emit another completion");

    own_reset = ar_data__create_map();
    AR_ASSERT(own_reset != NULL, "Failed completion reset message should be created");
    ar_data__set_map_string(own_reset, "request", "aggregation_start");
    ar_data__set_map_string(own_reset, "trace_id", "agg-failed-trace");
    ar_data__set_map_integer(own_reset, "expected_count", 2);
    ar_data__set_map_integer(own_reset, "source", 98765);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_reset),
              "Failed completion reset message should queue");
    own_reset = NULL;

    send_collect(mut_agency, aggregation_agent, "agg-failed-trace", "one");
    send_collect(mut_agency, aggregation_agent, "agg-failed-trace", "two");
    ar_method_fixture__process_all_messages(own_fixture);

    ref_aggregation_memory = ar_agency__get_agent_memory(mut_agency, aggregation_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "completed") == 0,
              "Failed completion send should not mark aggregate complete");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "count") == 2,
              "Failed completion send should retain collected count");

    send_collect(mut_agency, aggregation_agent, "agg-failed-trace", "three");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "completed") == 0,
              "Aggregate should stay open after repeated completion send failure");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "count") == 2,
              "Aggregate should not collect beyond the requested outcome count");

    own_reset = ar_data__create_map();
    AR_ASSERT(own_reset != NULL, "Reset message should be created");
    ar_data__set_map_string(own_reset, "request", "aggregation_start");
    ar_data__set_map_string(own_reset, "trace_id", "agg-reset-trace");
    ar_data__set_map_integer(own_reset, "expected_count", 2);
    ar_data__set_map_integer(own_reset, "source", checked_agent_id(receiver_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_reset),
              "Reset message should queue");
    own_reset = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    send_collect(mut_agency, aggregation_agent, "agg-reset-trace", "fresh-one");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "count") == 1,
              "Reset aggregate should collect only fresh payloads");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_trace_id"),
                     "agg-trace-1") == 0,
              "Single fresh payload should not complete the reset aggregate early");

    send_collect(mut_agency, aggregation_agent, "agg-reset-trace", "fresh-two");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_trace_id"),
                     "agg-reset-trace") == 0,
              "Reset aggregate should complete with the new trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_success_count") == 2,
              "Reset aggregate should count only fresh payloads toward completion");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_aggregation_context);
    ar_data__destroy(own_receiver_context);
}

static void test_aggregation__reports_collection_failures_on_completion(void) {
    printf("Testing aggregation reports collection failures on completion...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("aggregation_trace_mismatch");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "aggregation");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_aggregation_context = create_context();
    ar_data_t *own_receiver_context = create_context();
    int64_t aggregation_agent = ar_agency__create_agent(
        mut_agency, "aggregation", "1.0.0", own_aggregation_context);
    int64_t receiver_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_context);

    ar_data_t *own_reset = ar_data__create_map();
    AR_ASSERT(own_reset != NULL, "Aggregate reset message should be created");
    ar_data__set_map_string(own_reset, "request", "aggregation_start");
    ar_data__set_map_string(own_reset, "trace_id", "agg-trace-1");
    ar_data__set_map_integer(own_reset, "expected_count", 2);
    ar_data__set_map_integer(own_reset, "source", checked_agent_id(receiver_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_reset),
              "Aggregate reset message should queue");
    own_reset = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    send_collect(mut_agency, aggregation_agent, "wrong-trace", "intruder");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    const char *ref_action = ar_data__get_map_string(ref_receiver_memory, "last_action");
    AR_ASSERT(ref_action == NULL, "Mismatched collect should not emit an immediate response");

    const ar_data_t *ref_aggregation_memory =
        ar_agency__get_agent_memory(mut_agency, aggregation_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "count") == 0,
              "Mismatched collect should not increment aggregation count");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "failure_count") == 1,
              "Mismatched collect should increment collection failure count");

    send_collect(mut_agency, aggregation_agent, "agg-trace-1", "first");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "count") == 1,
              "Matching collect should increment aggregation count");
    const char *ref_status = ar_data__get_map_string(ref_receiver_memory, "last_status");
    AR_ASSERT(ref_status != NULL && strcmp(ref_status, "failure") == 0,
              "Completion should report standard failure status when collection failures occurred");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_trace_id"),
                     "agg-trace-1") == 0,
              "Completion should preserve the original aggregate trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_success_count") == 1,
              "Completion should count only matching collect requests");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_failure_count") == 1,
              "Completion should report collection failures");

    const ar_data_t *ref_payloads = ar_data__get_map_data(ref_receiver_memory, "last_payloads");
    AR_ASSERT(ref_payloads != NULL, "Completion should include payload list");
    AR_ASSERT(ar_data__list_count(ref_payloads) == 1,
              "Rejected mismatched payload should be excluded from payload list");
    ar_data_t **own_items = ar_data__list_items(ref_payloads);
    AR_ASSERT(own_items != NULL, "Aggregate payload items should be readable");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[0]), "first") == 0,
              "First accepted payload should be retained");
    AR__HEAP__FREE(own_items);

    send_collect(mut_agency, aggregation_agent, "agg-trace-1", "second");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "count") == 1,
              "Late matching collect should not increment completed aggregate count");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_aggregation_context);
    ar_data__destroy(own_receiver_context);
}

int main(void) {
    printf("Running aggregation method tests...\n\n");
    test_aggregation__combines_required_payloads();
    test_aggregation__reports_collection_failures_on_completion();
    printf("\nAll aggregation method tests passed!\n");
    return 0;
}
