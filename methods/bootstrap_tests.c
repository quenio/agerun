#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ar_method_fixture.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_assert.h"
#include "ar_io.h"

static char g_runner_path[256] = {0};
static char g_model_path[256] = {0};

static void setup_fake_runner(const char *ref_output) {
    snprintf(g_runner_path, sizeof(g_runner_path), "./fake-bootstrap-workflow-runner-%ld.sh", (long)getpid());
    snprintf(g_model_path, sizeof(g_model_path), "./fake-bootstrap-workflow-model-%ld.gguf", (long)getpid());

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

static void load_workflow_methods(ar_method_fixture_t *own_fixture) {
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "bootstrap",
        "../../methods/bootstrap-1.0.0.method", "1.0.0"), "Load bootstrap");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-coordinator",
        "../../methods/workflow-coordinator-1.0.0.method", "1.0.0"), "Load workflow-coordinator");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-definition",
        "../../methods/workflow-definition-1.0.0.method", "1.0.0"), "Load workflow-definition");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-item",
        "../../methods/workflow-item-1.0.0.method", "1.0.0"), "Load workflow-item");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "workflow-reporter",
        "../../methods/workflow-reporter-1.0.0.method", "1.0.0"), "Load workflow-reporter");
}

static void test_bootstrap_runs_workflow_demo_on_boot(void) {
    printf("Testing bootstrap runs workflow demo on boot message...\n");

    remove("agerun.log");
    setup_fake_runner("outcome=advance\\nreason=approved\\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("bootstrap_workflow_boot");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Directory");
    load_workflow_methods(own_fixture);

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Bootstrap context should be created");
    int64_t bootstrap_id = ar_agency__create_agent(mut_agency, "bootstrap", "1.0.0", own_context);
    AR_ASSERT(bootstrap_id == 1, "Bootstrap should be agent 1");

    ar_data_t *own_boot = ar_data__create_string("__boot__");
    ar_agency__send_to_agent(mut_agency, bootstrap_id, own_boot);
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Boot message should be processed");

    const ar_data_t *bootstrap_memory = ar_agency__get_agent_memory(mut_agency, bootstrap_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(bootstrap_memory, "method_name"), "workflow-coordinator") == 0,
              "Bootstrap should target workflow-coordinator");
    AR_ASSERT(ar_data__get_map_integer(bootstrap_memory, "coordinator_id") == 2,
              "Bootstrap should spawn workflow-coordinator as agent 2");
    const ar_data_t *start_message = ar_data__get_map_data(bootstrap_memory, "start_message");
    AR_ASSERT(start_message != NULL, "Start message should be stored");
    AR_ASSERT(strcmp(ar_data__get_map_string(start_message, "action"), "start") == 0,
              "Bootstrap should queue workflow start action");
    AR_ASSERT(strcmp(ar_data__get_map_string(start_message, "definition_path"), "workflows/default-workflow.yaml") == 0,
              "Bootstrap should pass bundled workflow path");
    AR_ASSERT(strcmp(ar_data__get_map_string(bootstrap_memory, "demo_status"), "Workflow demo queued") == 0,
              "Bootstrap should record queued workflow demo");

    while (ar_method_fixture__process_next_message(own_fixture)) {
    }

    ar_method_fixture__destroy(own_fixture);
    AR_ASSERT(log_file_contains("stage=intake"), "Workflow demo should log intake progress");
    AR_ASSERT(log_file_contains("terminal=completed reason=approved"),
              "Workflow demo should log terminal summary");
    cleanup_fake_runner();
    ar_data__destroy(own_context);
    remove("agerun.log");
    printf("PASS\n");
}

int main(void) {
    printf("Bootstrap Method Tests\n");
    printf("======================\n\n");

    test_bootstrap_runs_workflow_demo_on_boot();

    printf("\nAll bootstrap tests passed!\n");
    return 0;
}
