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

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_supervision__restarts_child_after_failure_event(void) {
    printf("Testing supervision restarts child after failure event...\n");

    // Given a supervision agent with a restart policy
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("supervision_restart");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "supervision");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_supervision_context = create_context();
    int64_t supervision_agent = ar_agency__create_agent(
        mut_agency, "supervision", "1.0.0", own_supervision_context);

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Supervision start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "child_method_name", "record-receiver");
    ar_data__set_map_string(own_start, "child_method_version", "1.0.0");
    ar_data__set_map_string(own_start, "policy", "restart");
    ar_data__set_map_integer(own_start, "reply_to", 0);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_start),
              "Supervision start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    int64_t first_child = ar_data__get_map_integer(ref_memory, "child_agent_id");
    AR_ASSERT(first_child > 0, "Supervision should create a child agent");

    // When the child is reported failed
    ar_data_t *own_failure = ar_data__create_map();
    AR_ASSERT(own_failure != NULL, "Child failure should be created");
    ar_data__set_map_string(own_failure, "action", "child_failed");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, supervision_agent, own_failure),
              "Child failure should queue");
    own_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then supervision records a restarted child
    ref_memory = ar_agency__get_agent_memory(mut_agency, supervision_agent);
    int64_t restarted_child = ar_data__get_map_integer(ref_memory, "child_agent_id");
    AR_ASSERT(restarted_child > first_child, "Supervision should spawn a replacement child");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "status"), "restarted") == 0,
              "Supervision should record restarted status");

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_supervision_context);
}

int main(void) {
    printf("Running supervision method tests...\n\n");
    test_supervision__restarts_child_after_failure_event();
    printf("\nAll supervision method tests passed!\n");
    return 0;
}
