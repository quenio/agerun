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

static char g_runner_path[256] = {0};
static char g_model_path[256] = {0};
static const char *REF_REAL_MODEL_PATH = "../../models/phi-3-mini-q4.gguf";

#if defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#define AR_WORKFLOW_REAL_COMPLETION_SANITIZER_BUILD 1
#endif
#endif
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
#define AR_WORKFLOW_REAL_COMPLETION_SANITIZER_BUILD 1
#endif
#ifndef AR_WORKFLOW_REAL_COMPLETION_SANITIZER_BUILD
#define AR_WORKFLOW_REAL_COMPLETION_SANITIZER_BUILD 0
#endif

static bool should_skip_real_completion_tests(void) {
    const char *ref_skip = getenv("AGERUN_SKIP_REAL_COMPLETION_TESTS");
    return AR_WORKFLOW_REAL_COMPLETION_SANITIZER_BUILD || (
        ref_skip != NULL && ref_skip[0] != '\0' && strcmp(ref_skip, "0") != 0
    );
}

static void setup_fake_runner(const char *ref_output) {
    snprintf(g_runner_path, sizeof(g_runner_path), "./fake-workflow-coordinator-runner-%ld.sh", (long)getpid());
    snprintf(g_model_path, sizeof(g_model_path), "./fake-workflow-coordinator-model-%ld.gguf", (long)getpid());

    FILE *own_model = fopen(g_model_path, "w");
    AR_ASSERT(own_model != NULL, "Fake model should be created");
    fputs("fake-model", own_model);
    fclose(own_model);

    FILE *own_runner = fopen(g_runner_path, "w");
    AR_ASSERT(own_runner != NULL, "Fake runner should be created");
    fputs("#!/bin/sh\n", own_runner);
    fprintf(own_runner, "printf '%s'\n", ref_output);
    fclose(own_runner);
    AR_ASSERT(chmod(g_runner_path, 0700) == 0, "Fake runner should be executable");

    AR_ASSERT(setenv("AGERUN_COMPLETE_RUNNER", g_runner_path, 1) == 0,
              "Fake runner env should be set");
    AR_ASSERT(setenv("AGERUN_COMPLETE_MODEL", g_model_path, 1) == 0,
              "Fake model env should be set");
}

static void cleanup_fake_runner(void) {
    unsetenv("AGERUN_COMPLETE_RUNNER");
    unsetenv("AGERUN_COMPLETE_MODEL");
    if (g_runner_path[0] != '\0') {
        remove(g_runner_path);
        g_runner_path[0] = '\0';
    }
    if (g_model_path[0] != '\0') {
        remove(g_model_path);
        g_model_path[0] = '\0';
    }
}

static void setup_missing_model(void) {
    cleanup_fake_runner();
    snprintf(g_model_path, sizeof(g_model_path),
             "./missing-workflow-coordinator-model-%ld.gguf", (long)getpid());
    AR_ASSERT(setenv("AGERUN_COMPLETE_MODEL", g_model_path, 1) == 0,
              "Missing model env should be set");
    unsetenv("AGERUN_COMPLETE_RUNNER");
}

static void setup_real_completion_runtime(void) {
    cleanup_fake_runner();
    AR_ASSERT(access(REF_REAL_MODEL_PATH, F_OK) == 0,
              "Real completion model should exist before workflow-coordinator real tests");
    unsetenv("AGERUN_COMPLETE_RUNNER");
    AR_ASSERT(setenv("AGERUN_COMPLETE_MODEL", REF_REAL_MODEL_PATH, 1) == 0,
              "Real completion model env should be set");
}

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

static ar_data_t *create_start_message(void) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Start message should be created");
    ar_data__set_map_string(own_message, "action", "start");
    ar_data__set_map_string(own_message, "definition_method_name", "workflow-definition");
    ar_data__set_map_string(own_message, "definition_method_version", "1.0.0");
    ar_data__set_map_string(own_message, "definition_path", "workflows/default.workflow");
    ar_data__set_map_string(own_message, "reporter_method_name", "workflow-reporter");
    ar_data__set_map_string(own_message, "reporter_method_version", "1.0.0");
    ar_data__set_map_string(own_message, "item_id", "demo-item-1");
    ar_data__set_map_string(own_message, "title", "demo_work_item");
    ar_data__set_map_string(own_message, "priority", "high");
    ar_data__set_map_string(own_message, "owner", "workflow_owner");
    ar_data__set_map_string(own_message, "review_status", "approved");
    return own_message;
}

static ar_data_t *create_test_workflow_start_message(const char *ref_review_status) {
    ar_data_t *own_message = create_start_message();
    AR_ASSERT(ar_data__set_map_string(own_message, "definition_path", "workflows/test.workflow"),
              "Start message should use test workflow definition");
    AR_ASSERT(ar_data__set_map_string(own_message, "review_status", ref_review_status),
              "Start message should use requested review status");
    return own_message;
}

static void load_workflow_methods(ar_method_fixture_t *own_fixture) {
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-coordinator",
        "../../methods/workflow-coordinator-1.0.0.method", "1.0.0"),
        "workflow-coordinator should load");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-definition",
        "../../methods/workflow-definition-1.0.0.method", "1.0.0"),
        "workflow-definition should load");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-item",
        "../../methods/workflow-item-1.0.0.method", "1.0.0"),
        "workflow-item should load");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-reporter",
        "../../methods/workflow-reporter-1.0.0.method", "1.0.0"),
        "workflow-reporter should load");
}

static void test_workflow_coordinator__waits_for_definition_ready_before_spawning_item(void) {
    printf("Testing workflow-coordinator waits for definition_ready before spawning item...\n");

    remove("agerun.log");
    setup_fake_runner("outcome=advance\\nreason=approved\\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_coordinator_success");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_workflow_methods(own_fixture);

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Coordinator context should be created");
    int64_t coordinator_agent_id = ar_agency__create_agent(mut_agency, "workflow-coordinator", "1.0.0", own_context);
    AR_ASSERT(coordinator_agent_id == 1, "Coordinator should be created first");

    ar_data_t *own_start = create_start_message();
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, coordinator_agent_id, own_start),
              "Start message should queue");
    own_start = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Coordinator should process start");
    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, coordinator_agent_id);
    const char *ref_run_status = ar_data__get_map_string(ref_memory, "run_status");
    AR_ASSERT(ref_run_status != NULL, "Coordinator run_status should be stored");
    AR_ASSERT(strcmp(ref_run_status, "waiting_for_definition") == 0,
              "Coordinator should wait for definition before item creation");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "item_agent_id") == 0,
              "Item should not exist before definition readiness");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process prepare_definition");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "File delegate should process workflow definition read");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process workflow definition file");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Coordinator should process definition_ready");

    ref_memory = ar_agency__get_agent_memory(mut_agency, coordinator_agent_id);
    int64_t item_agent_id = ar_data__get_map_integer(ref_memory, "item_agent_id");
    AR_ASSERT(item_agent_id > 0, "Coordinator should spawn workflow-item after readiness");
    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, item_agent_id),
              "Coordinator should queue initialize message to workflow-item");

    while (ar_method_fixture__process_next_message(own_fixture)) {
    }

    ref_memory = ar_agency__get_agent_memory(mut_agency, coordinator_agent_id);
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "definition_agent_id") > 0,
              "Definition agent should be stored");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "reporter_agent_id") > 0,
              "Reporter agent should be stored");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "item_agent_id") > 0,
              "Item agent should be created after readiness");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "run_status"), "active") == 0,
              "Coordinator should become active after readiness");

    ar_method_fixture__destroy(own_fixture);
    AR_ASSERT(log_file_contains("terminal=completed reason=approved"),
              "Successful run should produce terminal summary");
    AR_ASSERT(log_file_contains("COMPLETE_TRACE[phase=transition|outcome=advance|reason=approved]"),
              "Successful run should include highlighted transition complete() trace markers");
    cleanup_fake_runner();
    ar_data__destroy(own_context);
    remove("agerun.log");
}

static void test_workflow_coordinator__startup_failure_skips_item_creation(void) {
    printf("Testing workflow-coordinator reports startup failure and skips item creation...\n");

    remove("agerun.log");
    setup_missing_model();

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_coordinator_startup_failure");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_workflow_methods(own_fixture);

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Coordinator context should be created");
    int64_t coordinator_agent_id = ar_agency__create_agent(mut_agency, "workflow-coordinator", "1.0.0", own_context);
    AR_ASSERT(coordinator_agent_id == 1, "Coordinator should be created first");

    ar_data_t *own_start = create_start_message();
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, coordinator_agent_id, own_start),
              "Start message should queue");
    own_start = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Coordinator should process start");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process prepare_definition");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "File delegate should process workflow definition read");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process workflow definition file");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Coordinator should process definition_error");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Reporter should process startup failure");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Log delegate should process startup failure");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, coordinator_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "run_status"), "startup_failed") == 0,
              "Coordinator should record startup_failed");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "item_agent_id") == 0,
              "Startup failure should skip item creation");

    ar_method_fixture__destroy(own_fixture);
    AR_ASSERT(log_file_contains("startup_dependency_unavailable"),
              "Startup failure should be logged");
    ar_data__destroy(own_context);
    remove("agerun.log");
}

static void assert_real_workflow_run_for_review_status(
    const char *ref_review_status,
    const char *ref_expected_stage,
    const char *ref_expected_status,
    const char *ref_expected_terminal_outcome,
    const char *ref_expected_reason,
    int64_t expected_transition_count,
    const char *ref_expected_log_text,
    const char *ref_expected_trace
) {
    remove("agerun.log");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_coordinator_real_review");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_workflow_methods(own_fixture);

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Coordinator context should be created");
    int64_t coordinator_agent_id = ar_agency__create_agent(
        mut_agency,
        "workflow-coordinator",
        "1.0.0",
        own_context
    );
    AR_ASSERT(coordinator_agent_id == 1, "Coordinator should be created first");

    ar_data_t *own_start = create_test_workflow_start_message(ref_review_status);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, coordinator_agent_id, own_start),
              "Start message should queue");
    own_start = NULL;

    while (ar_method_fixture__process_next_message(own_fixture)) {
    }

    const ar_data_t *ref_coordinator_memory =
        ar_agency__get_agent_memory(mut_agency, coordinator_agent_id);
    int64_t item_agent_id = ar_data__get_map_integer(ref_coordinator_memory, "item_agent_id");
    AR_ASSERT(item_agent_id > 0, "Coordinator should create item for real workflow run");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_coordinator_memory, "run_status"), "active") == 0,
              "Coordinator should remain active after real workflow run");

    const ar_data_t *ref_item_memory = ar_agency__get_agent_memory(mut_agency, item_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_item_memory, "workflow_name"), "test_workflow") == 0,
              "Item should use test workflow name");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_item_memory, "current_stage"),
                     ref_expected_stage) == 0,
              "Real review status should drive final item stage");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_item_memory, "current_status"),
                     ref_expected_status) == 0,
              "Real review status should drive final item status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_item_memory, "terminal_outcome"),
                     ref_expected_terminal_outcome) == 0,
              "Real review status should drive terminal outcome");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_item_memory, "last_reason"),
                     ref_expected_reason) == 0,
              "Real review status should drive final reason");
    AR_ASSERT(ar_data__get_map_integer(ref_item_memory, "transition_count") ==
              expected_transition_count,
              "Real workflow run should record expected transition count");

    ar_method_fixture__destroy(own_fixture);
    AR_ASSERT(log_file_contains(ref_expected_log_text),
              "Reporter should log expected real workflow result");
    AR_ASSERT(log_file_contains(ref_expected_trace),
              "Reporter should log expected real complete trace");
    ar_data__destroy(own_context);
    remove("agerun.log");
}

static void test_workflow_coordinator__real_completion_review_status_drives_outcomes(void) {
    printf("Testing workflow-coordinator real completion review status matrix...\n");

    if (should_skip_real_completion_tests()) {
        printf("Skipping workflow-coordinator real completion matrix in sanitizer/opt-out run.\n");
        return;
    }

    setup_real_completion_runtime();

    assert_real_workflow_run_for_review_status(
        "approved",
        "completion",
        "completion",
        "completed",
        "approved_by_review",
        4,
        "workflow=test_workflow item=demo-item-1 stage=completion terminal=completed reason=approved_by_review",
        "COMPLETE_TRACE[phase=transition|outcome=advance|reason=approved_by_review]"
    );
    assert_real_workflow_run_for_review_status(
        "pending",
        "review",
        "review_waiting",
        "",
        "review_not_approved",
        3,
        "workflow=test_workflow item=demo-item-1 stage=review status=review_waiting reason=review_not_approved",
        "COMPLETE_TRACE[phase=transition|outcome=stay|reason=review_not_approved]"
    );
    assert_real_workflow_run_for_review_status(
        "blocked",
        "review",
        "rejected",
        "rejected",
        "review_blocked",
        3,
        "workflow=test_workflow item=demo-item-1 stage=review terminal=rejected reason=review_blocked",
        "COMPLETE_TRACE[phase=transition|outcome=reject|reason=review_blocked]"
    );

    unsetenv("AGERUN_COMPLETE_MODEL");
    unsetenv("AGERUN_COMPLETE_RUNNER");
}

int main(void) {
    printf("Workflow Coordinator Method Tests\n");
    printf("=================================\n\n");

    test_workflow_coordinator__waits_for_definition_ready_before_spawning_item();
    test_workflow_coordinator__startup_failure_skips_item_creation();
    test_workflow_coordinator__real_completion_review_status_drives_outcomes();

    printf("\nAll workflow-coordinator tests passed!\n");
    return 0;
}
