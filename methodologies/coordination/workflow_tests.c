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
        "memory.last_workflow_id := message.workflow_id\n"
        "memory.last_status := message.status\n"
        "memory.last_current_step := message.current_step\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_workflow__routes_branching_steps_to_completion(void) {
    printf("Testing workflow routes branching steps to completion...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_branching");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "routing");
    load_method(own_fixture, "workflow");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_routing_context = create_context();
    ar_data_t *own_workflow_context = create_context();
    ar_data_t *own_step1_context = create_context();
    ar_data_t *own_step2_context = create_context();
    ar_data_t *own_step3_context = create_context();
    ar_data_t *own_report_context = create_context();
    int64_t routing_agent = ar_agency__create_agent(
        mut_agency, "routing", "1.0.0", own_routing_context);
    int64_t workflow_agent = ar_agency__create_agent(
        mut_agency, "workflow", "1.0.0", own_workflow_context);
    int64_t step1_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step1_context);
    int64_t step2_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step2_context);
    int64_t step3_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step3_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_report_context);

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Workflow start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-1");
    ar_data__set_map_integer(own_start, "routing_agent", checked_agent_id(routing_agent));
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(report_agent));
    ar_data__set_map_integer(own_start, "step1_target", checked_agent_id(step1_agent));
    ar_data__set_map_string(own_start, "step1_action", "step1");
    ar_data__set_map_string(own_start, "step1_text", "first");
    ar_data__set_map_integer(own_start, "step2_target", checked_agent_id(step2_agent));
    ar_data__set_map_string(own_start, "step2_action", "step2");
    ar_data__set_map_string(own_start, "step2_text", "second");
    ar_data__set_map_integer(own_start, "step3_target", checked_agent_id(step3_agent));
    ar_data__set_map_string(own_start, "step3_action", "step3");
    ar_data__set_map_string(own_start, "step3_text", "third");
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step1_memory = ar_agency__get_agent_memory(mut_agency, step1_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_text"), "first") == 0,
              "Workflow should route first step");

    ar_data_t *own_branch = ar_data__create_map();
    AR_ASSERT(own_branch != NULL, "Step completion should be created");
    ar_data__set_map_string(own_branch, "action", "step_done");
    ar_data__set_map_integer(own_branch, "step", 1);
    ar_data__set_map_string(own_branch, "outcome", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_branch),
              "Branch completion should queue");
    own_branch = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step2_memory = ar_agency__get_agent_memory(mut_agency, step2_agent);
    const ar_data_t *ref_step3_memory = ar_agency__get_agent_memory(mut_agency, step3_agent);
    AR_ASSERT(ar_data__get_map_data(ref_step2_memory, "last_text") == NULL,
              "Workflow should skip step two on matching branch outcome");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step3_memory, "last_text"), "third") == 0,
              "Workflow should route branched step three");

    ar_data_t *own_complete = ar_data__create_map();
    AR_ASSERT(own_complete != NULL, "Final completion should be created");
    ar_data__set_map_string(own_complete, "action", "step_done");
    ar_data__set_map_integer(own_complete, "step", 3);
    ar_data__set_map_string(own_complete, "outcome", "done");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_complete),
              "Final completion should queue");
    own_complete = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_action"),
                     "workflow_complete") == 0,
              "Workflow should emit completion report");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "complete") == 0,
              "Workflow completion status should be complete");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_current_step") == 3,
              "Workflow completion should report final step");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_routing_context);
    ar_data__destroy(own_workflow_context);
    ar_data__destroy(own_step1_context);
    ar_data__destroy(own_step2_context);
    ar_data__destroy(own_step3_context);
    ar_data__destroy(own_report_context);
}

int main(void) {
    printf("Running workflow method tests...\n\n");
    test_workflow__routes_branching_steps_to_completion();
    printf("\nAll workflow method tests passed!\n");
    return 0;
}
