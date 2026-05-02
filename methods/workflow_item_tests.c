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

static ar_data_t *create_initialize_message_with_definition(
    int64_t reporter_agent_id,
    int64_t definition_agent_id
) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Initialize message should be created");
    ar_data__set_map_string(own_message, "action", "initialize");
    ar_data__set_map_string(own_message, "workflow_name", "default_workflow");
    ar_data__set_map_string(own_message, "item_id", "demo-item-1");
    ar_data__set_map_string(own_message, "title", "demo_work_item");
    ar_data__set_map_string(own_message, "priority", "high");
    ar_data__set_map_string(own_message, "owner", "workflow_owner");
    ar_data__set_map_string(own_message, "review_status", "approved");
    ar_data__set_map_string(own_message, "initial_stage", "intake");
    ar_data__set_map_integer(own_message, "definition_agent_id", (int)definition_agent_id);
    ar_data__set_map_integer(own_message, "reporter_agent_id", (int)reporter_agent_id);
    return own_message;
}

static ar_data_t *create_initialize_message(int64_t reporter_agent_id) {
    return create_initialize_message_with_definition(reporter_agent_id, 0);
}

static ar_data_t *create_transition_decision_from_stage(
    const char *ref_from_stage,
    const char *ref_outcome,
    const char *ref_next_stage,
    const char *ref_status,
    const char *ref_terminal_outcome,
    const char *ref_reason,
    int64_t retryable
) {
    char complete_trace[256];
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Transition decision should be created");
    AR_ASSERT(snprintf(complete_trace, sizeof(complete_trace),
                       "COMPLETE_TRACE[phase=transition|outcome=%s|reason=%s]",
                       ref_outcome, ref_reason) > 0,
              "Transition complete trace should format");
    ar_data__set_map_string(own_message, "action", "transition_decision");
    ar_data__set_map_string(own_message, "workflow_name", "default_workflow");
    ar_data__set_map_string(own_message, "from_stage", ref_from_stage);
    ar_data__set_map_string(own_message, "outcome", ref_outcome);
    ar_data__set_map_string(own_message, "next_stage", ref_next_stage);
    ar_data__set_map_string(own_message, "status", ref_status);
    ar_data__set_map_string(own_message, "validation_clause", "review_gate");
    ar_data__set_map_string(own_message, "reason", ref_reason);
    ar_data__set_map_integer(own_message, "retryable", (int)retryable);
    ar_data__set_map_string(own_message, "terminal_outcome", ref_terminal_outcome);
    ar_data__set_map_string(own_message, "note", ref_reason);
    ar_data__set_map_string(own_message, "complete_trace", complete_trace);
    return own_message;
}

static ar_data_t *create_transition_decision(const char *ref_outcome, const char *ref_next_stage,
                                             const char *ref_status, const char *ref_terminal_outcome,
                                             const char *ref_reason, int64_t retryable) {
    return create_transition_decision_from_stage(
        "review",
        ref_outcome,
        ref_next_stage,
        ref_status,
        ref_terminal_outcome,
        ref_reason,
        retryable
    );
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

    ar_data_t *own_initialize = create_initialize_message(reporter_agent_id);
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
    AR_ASSERT(log_file_contains("COMPLETE_TRACE[phase=transition|outcome=advance|reason=approved]"),
              "Reporter should highlight transition complete() trace markers");
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

    ar_data_t *own_initialize = create_initialize_message(reporter_agent_id);
    AR_ASSERT(ar_data__set_map_string(own_initialize, "initial_stage", "review"),
              "Retryable stay fixture should start at review");
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

static ar_data_t *take_definition_evaluate(
    ar_agency_t *mut_agency,
    int64_t definition_agent_id,
    const char *ref_expected_stage
) {
    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Definition agent should receive evaluate_transition");
    ar_data_t *own_message = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(own_message != NULL, "Evaluate transition should be retrievable");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_message, "action"), "evaluate_transition") == 0,
              "Message should ask definition to evaluate transition");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_message, "stage"), ref_expected_stage) == 0,
              "Evaluate transition should use expected current stage");
    return own_message;
}

static void process_until_definition_has_message(
    ar_method_fixture_t *mut_fixture,
    ar_agency_t *mut_agency,
    int64_t definition_agent_id,
    const char *ref_context
) {
    int attempts = 0;
    while (!ar_agency__agent_has_messages(mut_agency, definition_agent_id) && attempts < 8) {
        AR_ASSERT(ar_method_fixture__process_next_message(mut_fixture), ref_context);
        attempts++;
    }
}

static void test_workflow_item__requests_transition_decision_at_each_non_terminal_stage(void) {
    printf("Testing workflow-item requests definition decision at each non-terminal stage...\n");

    remove("agerun.log");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_item_generic_transitions");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-item",
        "../../methods/workflow-item-1.0.0.method", "1.0.0"),
        "workflow-item method should load");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-reporter",
        "../../methods/workflow-reporter-1.0.0.method", "1.0.0"),
        "workflow-reporter method should load");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-definition",
        "../../methods/workflow-definition-1.0.0.method", "1.0.0"),
        "workflow-definition method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Workflow item context should be created");
    int64_t reporter_agent_id = ar_agency__create_agent(mut_agency, "workflow-reporter", "1.0.0", own_context);
    int64_t definition_agent_id = ar_agency__create_agent(
        mut_agency,
        "workflow-definition",
        "1.0.0",
        own_context
    );
    int64_t item_agent_id = ar_agency__create_agent(mut_agency, "workflow-item", "1.0.0", own_context);
    AR_ASSERT(reporter_agent_id > 0 && definition_agent_id > 0 && item_agent_id > 0,
              "Reporter, definition, and item agents should be created");

    ar_data_t *own_initialize = create_initialize_message_with_definition(
        reporter_agent_id,
        definition_agent_id
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_initialize),
              "Initialize should queue");
    own_initialize = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should process initialize");
    process_until_definition_has_message(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "Item should request intake decision"
    );
    ar_data_t *own_evaluate = take_definition_evaluate(mut_agency, definition_agent_id, "intake");
    ar_data__destroy(own_evaluate);

    ar_data_t *own_decision = create_transition_decision_from_stage(
        "intake", "advance", "triage", "triage", "", "entered_triage", 0
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_decision),
              "Intake advance decision should queue");
    own_decision = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should apply intake decision");
    process_until_definition_has_message(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "Item should request triage decision"
    );
    own_evaluate = take_definition_evaluate(mut_agency, definition_agent_id, "triage");
    ar_data__destroy(own_evaluate);

    own_decision = create_transition_decision_from_stage(
        "triage", "advance", "active", "active", "", "entered_active", 0
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_decision),
              "Triage advance decision should queue");
    own_decision = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should apply triage decision");
    process_until_definition_has_message(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "Item should request active decision"
    );
    own_evaluate = take_definition_evaluate(mut_agency, definition_agent_id, "active");
    ar_data__destroy(own_evaluate);

    own_decision = create_transition_decision_from_stage(
        "active", "advance", "review", "review", "", "entered_review", 0
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, item_agent_id, own_decision),
              "Active advance decision should queue");
    own_decision = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Item should apply active decision");
    process_until_definition_has_message(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "Item should request review decision"
    );
    own_evaluate = take_definition_evaluate(mut_agency, definition_agent_id, "review");
    ar_data__destroy(own_evaluate);

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, item_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "current_stage"), "review") == 0,
              "Item should apply returned next_stage generically");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "transition_count") == 3,
              "Item should count generic advances");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    remove("agerun.log");
}

int main(void) {
    printf("Workflow Item Method Tests\n");
    printf("==========================\n\n");

    test_workflow_item__advance_decision_produces_terminal_summary();
    test_workflow_item__retryable_stay_preserves_review_stage();
    test_workflow_item__requests_transition_decision_at_each_non_terminal_stage();

    printf("\nAll workflow-item tests passed!\n");
    return 0;
}
