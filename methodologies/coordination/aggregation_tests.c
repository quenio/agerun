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

static void send_result(ar_agency_t *mut_agency, int64_t aggregation_agent, const char *ref_value) {
    ar_data_t *own_result = ar_data__create_map();
    AR_ASSERT(own_result != NULL, "Result message should be created");
    ar_data__set_map_string(own_result, "action", "result");
    ar_data__set_map_string(own_result, "value", ref_value);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_result),
              "Result message should queue");
    own_result = NULL;
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_status := message.status\n"
        "memory.last_aggregate_id := message.aggregate_id\n"
        "memory.last_result := message.result\n"
        "memory.last_received_count := message.received_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_aggregation__combines_required_results(void) {
    printf("Testing aggregation combines required results...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("aggregation_results");
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

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Start message should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "aggregate_id", "agg-1");
    ar_data__set_map_integer(own_start, "required_count", 4);
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(receiver_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_start),
              "Start message should queue");
    own_start = NULL;

    send_result(mut_agency, aggregation_agent, "alpha");
    send_result(mut_agency, aggregation_agent, "beta");
    send_result(mut_agency, aggregation_agent, "gamma");
    send_result(mut_agency, aggregation_agent, "delta");

    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    const char *ref_action = ar_data__get_map_string(ref_receiver_memory, "last_action");
    AR_ASSERT(ref_action != NULL && strcmp(ref_action, "aggregate_complete") == 0,
              "Receiver should observe aggregate completion");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_status"), "complete") == 0,
              "Aggregate status should be complete");
    const ar_data_t *ref_result = ar_data__get_map_data(ref_receiver_memory, "last_result");
    AR_ASSERT(ref_result != NULL, "Aggregate should include result list");
    AR_ASSERT(ar_data__get_type(ref_result) == AR_DATA_TYPE__LIST,
              "Aggregate result should be a list");
    AR_ASSERT(ar_data__list_count(ref_result) == 4,
              "Aggregate result should include all required values");
    ar_data_t **own_items = ar_data__list_items(ref_result);
    AR_ASSERT(own_items != NULL, "Aggregate result items should be readable");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[0]), "alpha") == 0,
              "Aggregate result should include first value");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[1]), "beta") == 0,
              "Aggregate result should include second value");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[2]), "gamma") == 0,
              "Aggregate result should include third value");
    AR_ASSERT(strcmp(ar_data__get_string(own_items[3]), "delta") == 0,
              "Aggregate result should include fourth value");
    AR__HEAP__FREE(own_items);
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_received_count") == 4,
              "Aggregate should report four received results");

    send_result(mut_agency, aggregation_agent, "epsilon");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_aggregation_memory =
        ar_agency__get_agent_memory(mut_agency, aggregation_agent);
    const ar_data_t *ref_stored_results = ar_data__get_map_data(ref_aggregation_memory, "results");
    AR_ASSERT(ref_stored_results != NULL, "Aggregation memory should retain result list");
    AR_ASSERT(ar_data__list_count(ref_stored_results) == 4,
              "Late results should not mutate completed aggregation state");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "received_count") == 4,
              "Late results should not increment completed aggregation count");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_received_count") == 4,
              "Late results should not emit another completion");

    ar_data_t *own_failed_start = ar_data__create_map();
    AR_ASSERT(own_failed_start != NULL, "Failed completion start message should be created");
    ar_data__set_map_string(own_failed_start, "action", "start");
    ar_data__set_map_string(own_failed_start, "aggregate_id", "agg-failed-send");
    ar_data__set_map_integer(own_failed_start, "required_count", 2);
    ar_data__set_map_integer(own_failed_start, "reply_to", 98765);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_failed_start),
              "Failed completion start message should queue");
    own_failed_start = NULL;

    send_result(mut_agency, aggregation_agent, "one");
    send_result(mut_agency, aggregation_agent, "two");
    ar_method_fixture__process_all_messages(own_fixture);

    ref_aggregation_memory = ar_agency__get_agent_memory(mut_agency, aggregation_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "completed") == 0,
              "Failed completion send should not mark aggregate complete");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "received_count") == 2,
              "Failed completion send should retain collected count");

    send_result(mut_agency, aggregation_agent, "three");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "completed") == 0,
              "Aggregate should stay open after repeated completion send failure");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "received_count") == 3,
              "Aggregate should keep collecting after completion send failure");

    ar_data_t *own_zero_start = ar_data__create_map();
    AR_ASSERT(own_zero_start != NULL, "Zero-count start message should be created");
    ar_data__set_map_string(own_zero_start, "action", "start");
    ar_data__set_map_string(own_zero_start, "aggregate_id", "agg-zero");
    ar_data__set_map_integer(own_zero_start, "required_count", 0);
    ar_data__set_map_integer(own_zero_start, "reply_to", checked_agent_id(receiver_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_zero_start),
              "Zero-count start message should queue");
    own_zero_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ref_aggregation_memory = ar_agency__get_agent_memory(mut_agency, aggregation_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "completed") == 0,
              "Zero required count should not complete on start");
    AR_ASSERT(ar_data__get_map_integer(ref_aggregation_memory, "received_count") == 0,
              "Zero required count start should not collect results");

    send_result(mut_agency, aggregation_agent, "only");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_aggregate_id"),
                     "agg-zero") == 0,
              "Zero required count should complete after first result");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_received_count") == 1,
              "Zero required count should behave as one required result");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_aggregation_context);
    ar_data__destroy(own_receiver_context);
}

int main(void) {
    printf("Running aggregation method tests...\n\n");
    test_aggregation__combines_required_results();
    printf("\nAll aggregation method tests passed!\n");
    return 0;
}
