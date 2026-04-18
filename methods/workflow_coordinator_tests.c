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
#include "ar_method.h"
#include "ar_log.h"

static char g_runner_path[256] = {0};
static char g_model_path[256] = {0};

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

static ar_data_t *create_start_message(int64_t sender) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Start message should be created");
    ar_data__set_map_string(own_message, "action", "start");
    ar_data__set_map_integer(own_message, "sender", (int)sender);
    ar_data__set_map_string(own_message, "definition_method_name", "workflow-definition");
    ar_data__set_map_string(own_message, "definition_method_version", "1.0.0");
    ar_data__set_map_string(own_message, "definition_path", "workflows/default-workflow.yaml");
    ar_data__set_map_string(own_message, "reporter_method_name", "workflow-reporter");
    ar_data__set_map_string(own_message, "reporter_method_version", "1.0.0");
    ar_data__set_map_string(own_message, "item_id", "demo-item-1");
    ar_data__set_map_string(own_message, "title", "demo_work_item");
    ar_data__set_map_string(own_message, "priority", "high");
    ar_data__set_map_string(own_message, "owner", "workflow_owner");
    ar_data__set_map_string(own_message, "review_status", "approved");
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

static void debug_workflow_coordinator_parse(void) {
    FILE *own_file = fopen("../../methods/workflow-coordinator-1.0.0.method", "r");
    AR_ASSERT(own_file != NULL, "Coordinator method file should open");
    fseek(own_file, 0, SEEK_END);
    long file_size = ftell(own_file);
    AR_ASSERT(file_size >= 0, "Coordinator method file size should be readable");
    fseek(own_file, 0, SEEK_SET);
    char *own_content = malloc((size_t)file_size + 1U);
    AR_ASSERT(own_content != NULL, "Coordinator debug buffer should allocate");
    size_t read_size = fread(own_content, 1, (size_t)file_size, own_file);
    own_content[read_size] = '\0';
    fclose(own_file);
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Coordinator debug log should create");
    ar_method_t *own_method = ar_method__create_with_log("workflow-coordinator-debug", own_content, "1.0.0", own_log);
    AR_ASSERT(own_method != NULL, "Coordinator debug method should create");
    if (ar_method__get_ast(own_method) == NULL) {
        const char *ref_error = ar_log__get_last_error_message(own_log);
        printf("coordinator-parse-error=%s\n", ref_error ? ref_error : "(none)");
        fflush(stdout);
    }
    ar_method__destroy(own_method);
    ar_log__destroy(own_log);
    free(own_content);
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

    ar_data_t *own_start = create_start_message(coordinator_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, coordinator_agent_id, own_start),
              "Start message should queue");
    own_start = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Coordinator should process start");
    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, coordinator_agent_id);
    const char *ref_run_status = ar_data__get_map_string(ref_memory, "run_status");
    printf("run_status=%s definition_agent_id=%d reporter_agent_id=%d prepare_sent=%d\n",
           ref_run_status ? ref_run_status : "(null)",
           (int)ar_data__get_map_integer(ref_memory, "definition_agent_id"),
           (int)ar_data__get_map_integer(ref_memory, "reporter_agent_id"),
           (int)ar_data__get_map_integer(ref_memory, "prepare_sent"));
    fflush(stdout);
    AR_ASSERT(ref_run_status != NULL, "Coordinator run_status should be stored");
    AR_ASSERT(strcmp(ref_run_status, "waiting_for_definition") == 0,
              "Coordinator should wait for definition before item creation");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "item_agent_id") == 0,
              "Item should not exist before definition readiness");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process prepare_definition");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Coordinator should process definition_ready");
    printf("queued after coordinator ready: c=%d d=%d r=%d i=%d\n",
           ar_agency__agent_has_messages(mut_agency, 1),
           ar_agency__agent_has_messages(mut_agency, 2),
           ar_agency__agent_has_messages(mut_agency, 3),
           ar_agency__agent_has_messages(mut_agency, 4));
    fflush(stdout);
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Reporter should process summary");
    const ar_data_t *ref_reporter_memory = ar_agency__get_agent_memory(mut_agency, 3);
    printf("reporter last_event_type=%s last_message=%s delivery_status=%s\n",
           ar_data__get_map_string(ref_reporter_memory, "last_event_type") ? ar_data__get_map_string(ref_reporter_memory, "last_event_type") : "(null)",
           ar_data__get_map_string(ref_reporter_memory, "last_message") ? ar_data__get_map_string(ref_reporter_memory, "last_message") : "(null)",
           ar_data__get_map_string(ref_reporter_memory, "delivery_status") ? ar_data__get_map_string(ref_reporter_memory, "delivery_status") : "(null)");
    fflush(stdout);
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Log delegate should process summary");

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
    AR_ASSERT(log_file_contains("COMPLETE_TRACE[phase=startup|outcome=advance|reason=approved]"),
              "Successful run should include highlighted startup complete() trace markers");
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

    ar_data_t *own_start = create_start_message(coordinator_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, coordinator_agent_id, own_start),
              "Start message should queue");
    own_start = NULL;

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Coordinator should process start");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process prepare_definition");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Coordinator should process definition_error");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Reporter should process startup failure");
    const ar_data_t *ref_reporter_memory = ar_agency__get_agent_memory(mut_agency, 3);
    printf("startup reporter last_event_type=%s last_message=%s delivery_status=%s\n",
           ar_data__get_map_string(ref_reporter_memory, "last_event_type") ? ar_data__get_map_string(ref_reporter_memory, "last_event_type") : "(null)",
           ar_data__get_map_string(ref_reporter_memory, "last_message") ? ar_data__get_map_string(ref_reporter_memory, "last_message") : "(null)",
           ar_data__get_map_string(ref_reporter_memory, "delivery_status") ? ar_data__get_map_string(ref_reporter_memory, "delivery_status") : "(null)");
    fflush(stdout);
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

int main(void) {
    printf("Workflow Coordinator Method Tests\n");
    printf("=================================\n\n");

    debug_workflow_coordinator_parse();
    test_workflow_coordinator__waits_for_definition_ready_before_spawning_item();
    test_workflow_coordinator__startup_failure_skips_item_creation();

    printf("\nAll workflow-coordinator tests passed!\n");
    return 0;
}
