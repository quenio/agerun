#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "ar_method_fixture.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_assert.h"
#include "ar_io.h"

static bool log_file_contains(const char *ref_expected_text) {
    FILE *fp = NULL;
    char line[512];

    if (ar_io__open_file("agerun.log", "r", &fp) != AR_FILE_RESULT__SUCCESS) {
        return false;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, ref_expected_text) != NULL) {
            ar_io__close_file(fp, "agerun.log");
            return true;
        }
    }

    ar_io__close_file(fp, "agerun.log");
    return false;
}

static ar_data_t *create_initialize_message(int64_t sender, int64_t reporter_agent_id) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Initialize message should be created");
    ar_data__set_map_string(own_message, "action", "initialize");
    ar_data__set_map_integer(own_message, "sender", (int)sender);
    ar_data__set_map_string(own_message, "workflow_name", "default_workflow");
    ar_data__set_map_string(own_message, "item_id", "demo-item-1");
    ar_data__set_map_string(own_message, "title", "demo_work_item");
    ar_data__set_map_string(own_message, "priority", "high");
    ar_data__set_map_string(own_message, "owner", "workflow_owner");
    ar_data__set_map_string(own_message, "review_status", "approved");
    ar_data__set_map_string(own_message, "initial_stage", "intake");
    ar_data__set_map_integer(own_message, "definition_agent_id", 0);
    ar_data__set_map_integer(own_message, "reporter_agent_id", (int)reporter_agent_id);
    return own_message;
}

static ar_data_t *create_transition_decision(const char *ref_outcome, const char *ref_next_stage,
                                             const char *ref_status, const char *ref_terminal_outcome,
                                             const char *ref_reason, int64_t retryable) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Transition decision should be created");
    ar_data__set_map_string(own_message, "action", "transition_decision");
    ar_data__set_map_string(own_message, "workflow_name", "default_workflow");
    ar_data__set_map_string(own_message, "from_stage", "review");
    ar_data__set_map_string(own_message, "outcome", ref_outcome);
    ar_data__set_map_string(own_message, "next_stage", ref_next_stage);
    ar_data__set_map_string(own_message, "status", ref_status);
    ar_data__set_map_string(own_message, "validation_clause", "review_gate");
    ar_data__set_map_string(own_message, "reason", ref_reason);
    ar_data__set_map_integer(own_message, "retryable", (int)retryable);
    ar_data__set_map_string(own_message, "terminal_outcome", ref_terminal_outcome);
    ar_data__set_map_string(own_message, "note", ref_reason);
    return own_message;
}

static void test_workflow_item__advance_decision_produces_terminal_summary(void) {
    printf("Testing workflow-item advance decision produces terminal summary...\n");

    remove("agerun.log");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_item_terminal_summary");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-item",
        "../../methods/workflow-item-1.0.0.method", "1.0.0"),
        "workflow-item method should load");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-reporter",
        "../../methods/workflow-reporter-1.0.0.method", "1.0.0"),
        "workflow-reporter method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Workflow item context should be created");
    int64_t reporter_agent_id = ar_agency__create_agent(mut_agency, "workflow-reporter", "1.0.0", own_context);
    int64_t item_agent_id = ar_agency__create_agent(mut_agency, "workflow-item", "1.0.0", own_context);
    AR_ASSERT(reporter_agent_id > 0 && item_agent_id > 0, "Reporter and item agents should be created");

    ar_data_t *own_initialize = create_initialize_message(item_agent_id, reporter_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_initialize),
              "Initialize should queue");
    own_initialize = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should process initialize");
    while (ar_method_fixture__process_next_message(own_fixture)) {
    }

    ar_data_t *own_decision = create_transition_decision(
        "advance", "completion", "completed", "completed", "approved", 0
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_decision),
              "Transition decision should queue");
    own_decision = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should process transition decision");
    while (ar_method_fixture__process_next_message(own_fixture)) {
    }

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, item_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "terminal_outcome"), "completed") == 0,
              "Advance decision should complete the item");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "summary"), "workflow=default_workflow item=demo-item-1 stage=completion terminal=completed reason=approved") == 0,
              "Summary should capture terminal state");

    ar_method_fixture__destroy(own_fixture);
    AR_ASSERT(log_file_contains("terminal=completed reason=approved"),
              "Reporter should log terminal summary");
    ar_data__destroy(own_context);
    remove("agerun.log");
}

static void test_workflow_item__retryable_stay_preserves_review_stage(void) {
    printf("Testing workflow-item retryable stay preserves review stage...\n");

    remove("agerun.log");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_item_retryable_stay");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-item",
        "../../methods/workflow-item-1.0.0.method", "1.0.0"),
        "workflow-item method should load");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-reporter",
        "../../methods/workflow-reporter-1.0.0.method", "1.0.0"),
        "workflow-reporter method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Workflow item context should be created");
    int64_t reporter_agent_id = ar_agency__create_agent(mut_agency, "workflow-reporter", "1.0.0", own_context);
    int64_t item_agent_id = ar_agency__create_agent(mut_agency, "workflow-item", "1.0.0", own_context);
    AR_ASSERT(reporter_agent_id > 0 && item_agent_id > 0, "Reporter and item agents should be created");

    ar_data_t *own_initialize = create_initialize_message(item_agent_id, reporter_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_initialize),
              "Initialize should queue");
    own_initialize = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should process initialize");
    while (ar_method_fixture__process_next_message(own_fixture)) {
    }

    ar_data_t *own_decision = create_transition_decision(
        "stay", "review", "review_waiting", "", "complete_transition_failed", 1
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_decision),
              "Retryable stay should queue");
    own_decision = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should process retryable stay");
    while (ar_method_fixture__process_next_message(own_fixture)) {
    }

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, item_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "current_stage"), "review") == 0,
              "Retryable stay should preserve review stage");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "last_reason"), "complete_transition_failed") == 0,
              "Retryable stay should retain failure reason");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "terminal_outcome"), "") == 0,
              "Retryable stay should not set terminal outcome");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    remove("agerun.log");
}

int main(void) {
    printf("Workflow Item Method Tests\n");
    printf("==========================\n\n");

    test_workflow_item__advance_decision_produces_terminal_summary();
    test_workflow_item__retryable_stay_preserves_review_stage();

    printf("\nAll workflow-item tests passed!\n");
    return 0;
}
