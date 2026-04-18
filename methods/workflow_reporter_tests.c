#include <stdio.h>
#include <string.h>
#include <unistd.h>
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

static ar_data_t *create_startup_failure_message(void) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Startup failure message should be created");
    ar_data__set_map_string(own_message, "action", "startup_failure");
    ar_data__set_map_string(own_message, "reason", "startup_dependency_unavailable");
    ar_data__set_map_string(own_message, "failure_category", "runtime_unavailable");
    return own_message;
}

static ar_data_t *create_empty_summary_message(void) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Summary message should be created");
    ar_data__set_map_string(own_message, "action", "summary");
    ar_data__set_map_string(own_message, "workflow_name", "default_workflow");
    ar_data__set_map_string(own_message, "item_id", "demo-item-1");
    ar_data__set_map_string(own_message, "stage", "review");
    ar_data__set_map_string(own_message, "status", "rejected");
    ar_data__set_map_string(own_message, "owner", "workflow_owner");
    ar_data__set_map_integer(own_message, "transition_count", 3);
    ar_data__set_map_string(own_message, "terminal_outcome", "rejected");
    ar_data__set_map_string(own_message, "reason", "policy_rejected");
    ar_data__set_map_string(own_message, "text", "");
    return own_message;
}

static void test_workflow_reporter__logs_startup_failure_without_item_fields(void) {
    printf("Testing workflow-reporter logs startup failure without item fields...\n");

    remove("agerun.log");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_reporter_startup_failure");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "workflow-reporter",
        "../../methods/workflow-reporter-1.0.0.method",
        "1.0.0"
    ), "workflow-reporter method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Reporter context should be created");
    int64_t reporter_agent_id = ar_agency__create_agent(mut_agency, "workflow-reporter", "1.0.0", own_context);
    AR_ASSERT(reporter_agent_id > 0, "Reporter agent should be created");

    ar_data_t *own_message = create_startup_failure_message();
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, reporter_agent_id, own_message),
              "Startup failure should queue");
    own_message = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Reporter should process startup failure");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Log delegate should process startup failure");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, reporter_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "last_event_type"), "startup_failure") == 0,
              "Reporter should record startup_failure event type");

    ar_method_fixture__destroy(own_fixture);
    AR_ASSERT(log_file_contains("startup_dependency_unavailable"),
              "Log should include startup failure reason");
    AR_ASSERT(!log_file_contains("item=demo-item-1"),
              "Startup failure log should not invent work-item fields");
    ar_data__destroy(own_context);
    remove("agerun.log");
}

static void test_workflow_reporter__builds_fallback_summary_when_text_missing(void) {
    printf("Testing workflow-reporter builds fallback summary when text is missing...\n");

    remove("agerun.log");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_reporter_fallback_summary");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "workflow-reporter",
        "../../methods/workflow-reporter-1.0.0.method",
        "1.0.0"
    ), "workflow-reporter method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Reporter context should be created");
    int64_t reporter_agent_id = ar_agency__create_agent(mut_agency, "workflow-reporter", "1.0.0", own_context);
    AR_ASSERT(reporter_agent_id > 0, "Reporter agent should be created");

    ar_data_t *own_message = create_empty_summary_message();
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, reporter_agent_id, own_message),
              "Summary should queue");
    own_message = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Reporter should process summary");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Log delegate should process summary");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, reporter_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "last_event_type"), "summary") == 0,
              "Reporter should record summary event type");

    ar_method_fixture__destroy(own_fixture);
    AR_ASSERT(log_file_contains("workflow=default_workflow item=demo-item-1 stage=review terminal=rejected"),
              "Reporter should emit fallback summary details");
    ar_data__destroy(own_context);
    remove("agerun.log");
}

int main(void) {
    printf("Workflow Reporter Method Tests\n");
    printf("=============================\n\n");

    test_workflow_reporter__logs_startup_failure_without_item_fields();
    test_workflow_reporter__builds_fallback_summary_when_text_missing();

    printf("\nAll workflow-reporter tests passed!\n");
    return 0;
}
