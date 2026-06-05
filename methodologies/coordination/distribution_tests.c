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
        "memory.last_correlation_id := message.correlation_id\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_distribution__assigns_portions_through_routing(void) {
    printf("Testing distribution assigns portions through routing...\n");

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
    int64_t router_agent = ar_agency__create_agent(
        mut_agency, "routing", "1.0.0", own_router_context);
    int64_t distribution_agent = ar_agency__create_agent(
        mut_agency, "distribution", "1.0.0", own_distribution_context);
    int64_t worker_a = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_worker_a_context);
    int64_t worker_b = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_worker_b_context);

    // When work is split into two portions
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Distribution message should be created");
    ar_data__set_map_string(own_message, "action", "distribute");
    ar_data__set_map_integer(
        own_message, "routing_agent", checked_agent_id(router_agent));
    ar_data__set_map_integer(
        own_message, "worker_a", checked_agent_id(worker_a));
    ar_data__set_map_integer(
        own_message, "worker_b", checked_agent_id(worker_b));
    ar_data__set_map_integer(own_message, "worker_c", 0);
    ar_data__set_map_string(own_message, "portion_a", "left");
    ar_data__set_map_string(own_message, "portion_b", "right");
    ar_data__set_map_string(own_message, "portion_c", "");
    ar_data__set_map_string(own_message, "work_id", "job-2");
    ar_data__set_map_integer(own_message, "reply_to", 0);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, distribution_agent, own_message),
              "Distribution message should queue");
    own_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then workers receive assigned portions through routing
    const ar_data_t *ref_worker_a_memory = ar_agency__get_agent_memory(mut_agency, worker_a);
    const ar_data_t *ref_worker_b_memory = ar_agency__get_agent_memory(mut_agency, worker_b);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_a_memory, "last_text"), "left") == 0,
              "Worker A should receive portion A");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_b_memory, "last_text"), "right") == 0,
              "Worker B should receive portion B");

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_router_context);
    ar_data__destroy(own_distribution_context);
    ar_data__destroy(own_worker_a_context);
    ar_data__destroy(own_worker_b_context);
}

int main(void) {
    printf("Running distribution method tests...\n\n");
    test_distribution__assigns_portions_through_routing();
    printf("\nAll distribution method tests passed!\n");
    return 0;
}
