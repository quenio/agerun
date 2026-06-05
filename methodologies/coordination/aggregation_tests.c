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
        "memory.last_status := message.status\n"
        "memory.last_aggregate_id := message.aggregate_id\n"
        "memory.last_result_a := message.result_a\n"
        "memory.last_result_b := message.result_b\n"
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
    ar_data__set_map_integer(own_start, "required_count", 2);
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(receiver_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_start),
              "Start message should queue");
    own_start = NULL;

    ar_data_t *own_result_a = ar_data__create_map();
    AR_ASSERT(own_result_a != NULL, "First result should be created");
    ar_data__set_map_string(own_result_a, "action", "result");
    ar_data__set_map_string(own_result_a, "slot", "a");
    ar_data__set_map_string(own_result_a, "value", "alpha");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_result_a),
              "First result should queue");
    own_result_a = NULL;

    ar_data_t *own_result_b = ar_data__create_map();
    AR_ASSERT(own_result_b != NULL, "Second result should be created");
    ar_data__set_map_string(own_result_b, "action", "result");
    ar_data__set_map_string(own_result_b, "slot", "b");
    ar_data__set_map_string(own_result_b, "value", "beta");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, aggregation_agent, own_result_b),
              "Second result should queue");
    own_result_b = NULL;

    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    const char *ref_action = ar_data__get_map_string(ref_receiver_memory, "last_action");
    AR_ASSERT(ref_action != NULL && strcmp(ref_action, "aggregate_complete") == 0,
              "Receiver should observe aggregate completion");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_status"), "complete") == 0,
              "Aggregate status should be complete");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_result_a"), "alpha") == 0,
              "Aggregate should include slot a");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_result_b"), "beta") == 0,
              "Aggregate should include slot b");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_received_count") == 2,
              "Aggregate should report two received results");

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
