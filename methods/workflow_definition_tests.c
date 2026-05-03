#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "ar_method_fixture.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_assert.h"
#include "ar_method.h"
#include "ar_log.h"

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
    snprintf(g_runner_path, sizeof(g_runner_path), "./fake-workflow-definition-runner-%ld.sh", (long)getpid());
    snprintf(g_model_path, sizeof(g_model_path), "./fake-workflow-definition-model-%ld.gguf", (long)getpid());

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

static void setup_prompt_sensitive_runner(void) {
    snprintf(
        g_runner_path,
        sizeof(g_runner_path),
        "./fake-workflow-definition-runner-%ld.sh",
        (long)getpid()
    );
    snprintf(
        g_model_path,
        sizeof(g_model_path),
        "./fake-workflow-definition-model-%ld.gguf",
        (long)getpid()
    );

    FILE *own_model = fopen(g_model_path, "w");
    AR_ASSERT(own_model != NULL, "Fake model should be created");
    fputs("fake-model", own_model);
    fclose(own_model);

    FILE *own_runner = fopen(g_runner_path, "w");
    AR_ASSERT(own_runner != NULL, "Prompt-sensitive fake runner should be created");
    fputs("#!/bin/sh\n", own_runner);
    fputs("prompt=\"$*\"\n", own_runner);
    fputs("case \"$prompt\" in\n", own_runner);
    fputs(
        "  *\"workflow_name=caller_typo\"*) "
        "printf 'outcome=reject\\nreason=caller_workflow_name_leaked\\n' ;;\n",
        own_runner
    );
    fputs(
        "  *\"review_status=pending\"*|*\"review_status is pending\"*) "
        "printf 'outcome=stay\\nreason=review_not_approved\\n' ;;\n",
        own_runner
    );
    fputs(
        "  *\"review_status=blocked\"*|*\"review_status is blocked\"*) "
        "printf 'outcome=reject\\nreason=review_blocked\\n' ;;\n",
        own_runner
    );
    fputs(
        "  *\"review_status=approved\"*|*\"review_status is approved\"*) "
        "printf 'outcome=advance\\nreason=approved_by_review\\n' ;;\n",
        own_runner
    );
    fputs(
        "  *) printf 'outcome=reject\\nreason=missing_workflow_item_context\\n' ;;\n",
        own_runner
    );
    fputs("esac\n", own_runner);
    fclose(own_runner);
    AR_ASSERT(chmod(g_runner_path, 0700) == 0, "Prompt-sensitive runner should be executable");

    AR_ASSERT(setenv("AGERUN_COMPLETE_RUNNER", g_runner_path, 1) == 0,
              "Fake runner env should be set");
    AR_ASSERT(setenv("AGERUN_COMPLETE_MODEL", g_model_path, 1) == 0,
              "Fake model env should be set");
}

static void setup_transition_prompt_runner(void) {
    snprintf(
        g_runner_path,
        sizeof(g_runner_path),
        "./fake-workflow-transition-runner-%ld.sh",
        (long)getpid()
    );
    snprintf(
        g_model_path,
        sizeof(g_model_path),
        "./fake-workflow-transition-model-%ld.gguf",
        (long)getpid()
    );

    FILE *own_model = fopen(g_model_path, "w");
    AR_ASSERT(own_model != NULL, "Fake model should be created");
    fputs("fake-model", own_model);
    fclose(own_model);

    FILE *own_runner = fopen(g_runner_path, "w");
    AR_ASSERT(own_runner != NULL, "Transition prompt fake runner should be created");
    fputs("#!/bin/sh\n", own_runner);
    fputs("prompt=\"$*\"\n", own_runner);
    fputs("case \"$prompt\" in\n", own_runner);
    fputs("  *\"intake_to_triage_prompt\"*) printf 'outcome=advance\\nreason=intake_prompt_used\\n' ;;\n", own_runner);
    fputs("  *\"triage_to_active_prompt\"*) printf 'outcome=stay\\nreason=triage_prompt_used\\n' ;;\n", own_runner);
    fputs("  *\"active_to_review_prompt\"*) printf 'outcome=reject\\nreason=active_prompt_used\\n' ;;\n", own_runner);
    fputs("  *\"review_to_completion_prompt\"*) printf 'outcome=advance\\nreason=review_prompt_used\\n' ;;\n", own_runner);
    fputs("  *) printf 'outcome=reject\\nreason=missing_transition_prompt\\n' ;;\n", own_runner);
    fputs("esac\n", own_runner);
    fclose(own_runner);
    AR_ASSERT(chmod(g_runner_path, 0700) == 0, "Transition prompt runner should be executable");

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
             "./missing-workflow-definition-model-%ld.gguf", (long)getpid());
    AR_ASSERT(setenv("AGERUN_COMPLETE_MODEL", g_model_path, 1) == 0,
              "Missing model env should be set");
    unsetenv("AGERUN_COMPLETE_RUNNER");
}

static void setup_real_completion_runtime(void) {
    cleanup_fake_runner();
    AR_ASSERT(access(REF_REAL_MODEL_PATH, F_OK) == 0,
              "Real completion model should exist before workflow-definition real tests");
    unsetenv("AGERUN_COMPLETE_RUNNER");
    AR_ASSERT(setenv("AGERUN_COMPLETE_MODEL", REF_REAL_MODEL_PATH, 1) == 0,
              "Real completion model env should be set");
}

static ar_data_t *create_prepare_definition_message(const char *ref_path, int64_t reply_to) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Prepare message should be created");
    ar_data__set_map_string(own_message, "action", "prepare_definition");
    ar_data__set_map_string(own_message, "definition_path", ref_path);
    ar_data__set_map_string(own_message, "stage", "");
    ar_data__set_map_string(own_message, "review_status", "approved");
    ar_data__set_map_integer(own_message, "reply_to", (int)reply_to);
    return own_message;
}

static ar_data_t *create_evaluate_transition_message_with_review(
    const char *ref_stage,
    const char *ref_review_status,
    int64_t reply_to
) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Evaluate message should be created");
    AR_ASSERT(ar_data__set_map_string(own_message, "action", "evaluate_transition"),
              "Evaluate message should include action");
    AR_ASSERT(ar_data__set_map_string(own_message, "workflow_name", "default_workflow"),
              "Evaluate message should include workflow name");
    AR_ASSERT(ar_data__set_map_string(own_message, "stage", ref_stage),
              "Evaluate message should include stage");
    AR_ASSERT(ar_data__set_map_string(own_message, "item_id", "demo-item-1"),
              "Evaluate message should include item id");
    AR_ASSERT(ar_data__set_map_string(own_message, "title", "demo_work_item"),
              "Evaluate message should include title");
    AR_ASSERT(ar_data__set_map_string(own_message, "priority", "high"),
              "Evaluate message should include priority");
    AR_ASSERT(ar_data__set_map_string(own_message, "owner", "workflow_owner"),
              "Evaluate message should include owner");
    AR_ASSERT(ar_data__set_map_string(own_message, "review_status", ref_review_status),
              "Evaluate message should include review status");
    AR_ASSERT(ar_data__set_map_integer(own_message, "transition_count", 4),
              "Evaluate message should include transition count");
    AR_ASSERT(ar_data__set_map_integer(own_message, "reply_to", (int)reply_to),
              "Evaluate message should include reply target");
    return own_message;
}

static ar_data_t *create_evaluate_transition_message(const char *ref_stage, int64_t reply_to) {
    return create_evaluate_transition_message_with_review(ref_stage, "approved", reply_to);
}

static ar_method_fixture_t *create_fixture(void) {
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_definition");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "workflow-definition",
        "../../methods/workflow-definition-1.0.0.method",
        "1.0.0"
    ), "workflow-definition method should load");
    return own_fixture;
}

static ar_data_t *prepare_definition_and_take_reply(
    ar_method_fixture_t *mut_fixture,
    ar_agency_t *mut_agency,
    int64_t definition_agent_id,
    const char *ref_definition_path
) {
    ar_data_t *own_prepare = create_prepare_definition_message(ref_definition_path, definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_prepare),
              "Prepare definition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(mut_fixture),
              "Definition should queue file delegate read");
    AR_ASSERT(ar_method_fixture__process_next_message(mut_fixture),
              "File delegate should process definition read");
    AR_ASSERT(ar_method_fixture__process_next_message(mut_fixture),
              "Definition should process file delegate response");
    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Prepared definition should reply");
    return ar_agency__get_agent_message(mut_agency, definition_agent_id);
}

static void assert_review_transition_decision(
    ar_method_fixture_t *mut_fixture,
    ar_agency_t *mut_agency,
    int64_t definition_agent_id,
    const char *ref_review_status,
    const char *ref_expected_workflow_name,
    const char *ref_expected_outcome,
    const char *ref_expected_next_stage,
    const char *ref_expected_status,
    const char *ref_expected_reason,
    const char *ref_expected_terminal_outcome,
    int64_t expected_retryable
) {
    char mut_expected_trace[256];
    AR_ASSERT(snprintf(
        mut_expected_trace,
        sizeof(mut_expected_trace),
        "COMPLETE_TRACE[phase=transition|outcome=%s|reason=%s]",
        ref_expected_outcome,
        ref_expected_reason
    ) > 0, "Expected complete trace should format");

    ar_data_t *own_message = create_evaluate_transition_message_with_review(
        "review",
        ref_review_status,
        definition_agent_id
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Review transition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(mut_fixture),
              "Review transition should process");
    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Review transition decision should be queued");

    ar_data_t *own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "action"), "transition_decision") == 0,
              "Review reply should be transition_decision");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "workflow_name"),
                     ref_expected_workflow_name) == 0,
              "Review reply should use canonical workflow name");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "outcome"),
                     ref_expected_outcome) == 0,
              "Review status should drive expected outcome");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "next_stage"),
                     ref_expected_next_stage) == 0,
              "Review status should drive expected next stage");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "status"),
                     ref_expected_status) == 0,
              "Review status should drive expected item status");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"),
                     ref_expected_reason) == 0,
              "Review status should drive expected reason");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "terminal_outcome"),
                     ref_expected_terminal_outcome) == 0,
              "Review status should drive expected terminal outcome");
    AR_ASSERT(ar_data__get_map_integer(own_reply, "retryable") == expected_retryable,
              "Review status should drive retryable flag");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "complete_trace"),
                     mut_expected_trace) == 0,
              "Review transition should expose exact complete trace");
    ar_data__destroy(own_reply);
}

static void assert_real_review_matrix_for_definition(
    const char *ref_definition_path,
    const char *ref_expected_workflow_name
) {
    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(
        mut_agency,
        "workflow-definition",
        "1.0.0",
        own_context
    );
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_prepare_reply = prepare_definition_and_take_reply(
        own_fixture,
        mut_agency,
        definition_agent_id,
        ref_definition_path
    );
    AR_ASSERT(strcmp(ar_data__get_map_string(own_prepare_reply, "action"), "definition_ready") == 0,
              "Real completion definition should prepare");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_prepare_reply, "workflow_name"),
                     ref_expected_workflow_name) == 0,
              "Prepared definition should use expected workflow name");
    ar_data__destroy(own_prepare_reply);

    assert_review_transition_decision(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "approved",
        ref_expected_workflow_name,
        "advance",
        "completion",
        "completion",
        "approved_by_review",
        "completed",
        0
    );
    assert_review_transition_decision(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "pending",
        ref_expected_workflow_name,
        "stay",
        "review",
        "review_waiting",
        "review_not_approved",
        "",
        1
    );
    assert_review_transition_decision(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "blocked",
        ref_expected_workflow_name,
        "reject",
        "review",
        "rejected",
        "review_blocked",
        "rejected",
        0
    );

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
}

static void test_workflow_definition__prepare_definition_reads_known_file_and_queues_ready_message(void) {
    printf("Testing workflow-definition prepare_definition queues definition_ready...\n");

    setup_fake_runner("outcome=advance\\nreason=probe_ok\\n");

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(
        mut_agency,
        "workflow-definition",
        "1.0.0",
        own_context
    );
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_message = create_prepare_definition_message("workflows/default.workflow", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Prepare definition should queue");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should queue file delegate read");
    AR_ASSERT(!ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Prepare should be asynchronous until file delegate responds");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "File delegate should process workflow definition read");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process workflow definition content");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, definition_agent_id);
    AR_ASSERT(ref_memory != NULL, "Definition memory should exist");
    const char *ref_workflow_name = ar_data__get_map_string(ref_memory, "workflow_name");
    const char *ref_file_status = ar_data__get_map_string(ref_memory, "file_status");
    const char *ref_dependency_status = ar_data__get_map_string(ref_memory, "dependency_status");
    AR_ASSERT(ref_workflow_name != NULL, "Workflow name should be stored");
    AR_ASSERT(strcmp(ref_workflow_name, "default_workflow") == 0,
              "Workflow name should be stored");
    AR_ASSERT(ref_file_status != NULL, "File status should be stored");
    AR_ASSERT(strcmp(ref_file_status, "loaded") == 0,
              "File status should be loaded");
    AR_ASSERT(ref_dependency_status != NULL, "Dependency status should be stored");
    AR_ASSERT(strcmp(ref_dependency_status, "ready") == 0,
              "Dependency status should be ready");
    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Definition should queue reply to reply_to");
    ar_data_t *own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(own_reply != NULL, "Definition reply should be retrievable");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "action"), "definition_ready") == 0,
              "Reply should be definition_ready");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "initial_stage"), "intake") == 0,
              "Reply should include initial stage");
    AR_ASSERT(ar_data__get_map_integer(own_reply, "transition_count") == 4,
              "Reply should include parsed transition count");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "complete_trace"),
                     "COMPLETE_TRACE[phase=startup|outcome=advance|reason=probe_ok]") == 0,
              "Definition ready should carry highlighted startup complete() trace markers");
    ar_data__destroy(own_reply);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    cleanup_fake_runner();
}

static void test_workflow_definition__invalid_schema_returns_definition_error(void) {
    printf("Testing workflow-definition rejects invalid schema...\n");

    setup_fake_runner("outcome=ready\nreason=ok\n");

    char mut_definition_path[256];
    char mut_actual_path[320];
    snprintf(mut_definition_path, sizeof(mut_definition_path),
             "workflows/invalid-schema-%ld.workflow", (long)getpid());
    snprintf(mut_actual_path, sizeof(mut_actual_path),
             "../../%s", mut_definition_path);
    FILE *own_invalid_workflow = fopen(mut_actual_path, "w");
    AR_ASSERT(own_invalid_workflow != NULL, "Invalid workflow fixture should be created");
    fputs(
        "workflow_name=invalid_workflow "
        "workflow_version=1.0.0 "
        "initial_stage=intake "
        "terminal_completed=completed "
        "terminal_rejected=rejected "
        "requires_local_completion=true "
        "item_fields=item_id,title,priority,owner,review_status "
        "stages=intake,triage,active,review,completion "
        "transition_count=3 "
        "transition_1_from=intake transition_1_to=triage transition_1_prompt=intake_to_triage_prompt "
        "transition_2_from=triage transition_2_to=active transition_2_prompt=triage_to_active_prompt "
        "transition_3_from=active transition_3_to=review transition_3_prompt=active_to_review_prompt "
        "transition_4_from=review transition_4_to=completion transition_4_prompt=review_to_completion_prompt\n",
        own_invalid_workflow
    );
    fclose(own_invalid_workflow);

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(
        mut_agency,
        "workflow-definition",
        "1.0.0",
        own_context
    );
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_message = create_prepare_definition_message(mut_definition_path, definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Prepare definition should queue");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should queue invalid definition file read");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "File delegate should process invalid definition read");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "Definition should process invalid file response");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "last_reason"), "invalid_definition_schema") == 0,
              "Invalid schema should record schema reason");

    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Definition should queue definition_error to reply_to");
    ar_data_t *own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "action"), "definition_error") == 0,
              "Reply should be definition_error");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "invalid_definition_schema") == 0,
              "Reply should include schema reason");
    ar_data__destroy(own_reply);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    remove(mut_actual_path);
    cleanup_fake_runner();
}

static void test_workflow_definition__invalid_schema_reason_wins_over_probe_failure(void) {
    printf("Testing workflow-definition preserves schema error when startup probe also fails...\n");

    setup_missing_model();

    char mut_definition_path[256];
    char mut_actual_path[320];
    snprintf(mut_definition_path, sizeof(mut_definition_path),
             "workflows/invalid-schema-and-probe-%ld.workflow", (long)getpid());
    snprintf(mut_actual_path, sizeof(mut_actual_path),
             "../../%s", mut_definition_path);
    FILE *own_invalid_workflow = fopen(mut_actual_path, "w");
    AR_ASSERT(own_invalid_workflow != NULL, "Invalid workflow fixture should be created");
    fputs(
        "workflow_name=invalid_workflow "
        "workflow_version=1.0.0 "
        "initial_stage=intake "
        "terminal_completed=completed "
        "terminal_rejected=rejected "
        "requires_local_completion=true "
        "item_fields=item_id,title,priority,owner,review_status "
        "stages=intake,triage,active,review,completion "
        "transition_count=3 "
        "transition_1_from=intake transition_1_to=triage transition_1_prompt=intake_to_triage_prompt "
        "transition_2_from=triage transition_2_to=active transition_2_prompt=triage_to_active_prompt "
        "transition_3_from=active transition_3_to=review transition_3_prompt=active_to_review_prompt "
        "transition_4_from=review transition_4_to=completion transition_4_prompt=review_to_completion_prompt\n",
        own_invalid_workflow
    );
    fclose(own_invalid_workflow);

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(
        mut_agency,
        "workflow-definition",
        "1.0.0",
        own_context
    );
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_reply = prepare_definition_and_take_reply(
        own_fixture,
        mut_agency,
        definition_agent_id,
        mut_definition_path
    );
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "action"), "definition_error") == 0,
              "Invalid schema with missing model should return definition_error");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "invalid_definition_schema") == 0,
              "Schema error should not be masked by startup dependency failure");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "failure_category"), "") == 0,
              "Schema error should not be categorized as runtime unavailable");
    ar_data__destroy(own_reply);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    remove(mut_actual_path);
    cleanup_fake_runner();
}

static void test_workflow_definition__complete_failure_maps_to_retryable_stay(void) {
    printf("Testing workflow-definition maps completion failure to retryable stay...\n");

    setup_missing_model();

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(mut_agency, "workflow-definition", "1.0.0", own_context);
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_prepare_reply = prepare_definition_and_take_reply(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "workflows/default.workflow"
    );
    ar_data__destroy(own_prepare_reply);

    ar_data_t *own_message = create_evaluate_transition_message("review", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Evaluate transition should queue");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Transition should process");

    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Transition decision should be queued");
    ar_data_t *own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "action"), "transition_decision") == 0,
              "Reply should be transition_decision");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "outcome"), "stay") == 0,
              "Complete failure should map to stay");
    AR_ASSERT(ar_data__get_map_integer(own_reply, "retryable") == 1,
              "Complete failure stay should be retryable");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "complete_transition_failed") == 0,
              "Retryable stay should surface failure reason");
    ar_data__destroy(own_reply);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
}

static void test_workflow_definition__complete_success_uses_outcome_and_reason(void) {
    printf("Testing workflow-definition uses fake-runner outcome and reason...\n");

    setup_fake_runner("outcome=reject\\nreason=policy_rejected\\n");

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(mut_agency, "workflow-definition", "1.0.0", own_context);
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_prepare_reply = prepare_definition_and_take_reply(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "workflows/default.workflow"
    );
    ar_data__destroy(own_prepare_reply);

    ar_data_t *own_message = create_evaluate_transition_message("review", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Evaluate transition should queue");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Transition should process");

    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Transition decision should be queued");
    ar_data_t *own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "outcome"), "reject") == 0,
              "Fake runner outcome should be preserved");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "policy_rejected") == 0,
              "Fake runner reason should be preserved");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "terminal_outcome"), "rejected") == 0,
              "Reject should produce terminal rejected outcome");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "complete_trace"),
                     "COMPLETE_TRACE[phase=transition|outcome=reject|reason=policy_rejected]") == 0,
              "Transition decision should carry highlighted complete() trace markers");
    ar_data__destroy(own_reply);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    cleanup_fake_runner();
}

static void test_workflow_definition__complete_prompt_uses_workflow_item_context(void) {
    printf("Testing workflow-definition complete prompt uses workflow item context...\n");

    // Given a runner that branches on the complete() prompt
    setup_prompt_sensitive_runner();

    // Given a workflow-definition agent
    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(
        mut_agency,
        "workflow-definition",
        "1.0.0",
        own_context
    );
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    // Given a prepared known workflow definition
    ar_data_t *own_prepare = create_prepare_definition_message(
        "workflows/default.workflow",
        definition_agent_id
    );
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_prepare),
              "Prepare definition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Prepare should queue file read");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "File delegate should read definition");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Prepare response should process");
    ar_data_t *own_prepare_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    ar_data__destroy(own_prepare_reply);

    // When evaluating a review transition whose item review is still pending
    ar_data_t *own_message = create_evaluate_transition_message_with_review(
        "review",
        "pending",
        definition_agent_id
    );

    // When caller-supplied workflow_name conflicts with the prepared definition
    AR_ASSERT(ar_data__set_map_string(own_message, "workflow_name", "caller_typo"),
              "Evaluate message should allow caller workflow name override");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Evaluate transition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Transition should process");

    // Then complete() should see the workflow item context and request a stay decision
    AR_ASSERT(ar_agency__agent_has_messages(mut_agency, definition_agent_id),
              "Transition decision should be queued");
    ar_data_t *own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "outcome"), "stay") == 0,
              "Pending review should stay when complete() sees workflow item context");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "review_not_approved") == 0,
              "Pending review should preserve context-sensitive complete() reason");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "next_stage"), "review") == 0,
              "Stay decision should keep the current stage");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "workflow_name"), "default_workflow") == 0,
              "Transition reply should keep canonical definition workflow name");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "complete_trace"),
                     "COMPLETE_TRACE[phase=transition|outcome=stay|"
                     "reason=review_not_approved]") == 0,
              "Context-sensitive decision should be reflected in complete trace");
    ar_data__destroy(own_reply);

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    cleanup_fake_runner();
}

static void test_workflow_definition__real_completion_review_status_drives_default_and_test_outcomes(void) {
    printf("Testing workflow-definition real completion review status matrix...\n");

    if (should_skip_real_completion_tests()) {
        printf("Skipping workflow-definition real completion matrix in sanitizer/opt-out run.\n");
        return;
    }

    setup_real_completion_runtime();

    assert_real_review_matrix_for_definition(
        "workflows/default.workflow",
        "default_workflow"
    );
    assert_real_review_matrix_for_definition(
        "workflows/test.workflow",
        "test_workflow"
    );

    unsetenv("AGERUN_COMPLETE_MODEL");
    unsetenv("AGERUN_COMPLETE_RUNNER");
}

static void test_workflow_definition__missing_transition_fields_returns_definition_error(void) {
    printf("Testing workflow-definition rejects missing transition fields...\n");

    setup_fake_runner("outcome=ready\\nreason=ok\\n");

    char mut_definition_path[256];
    char mut_actual_path[320];
    snprintf(mut_definition_path, sizeof(mut_definition_path),
             "workflows/missing-transition-%ld.workflow", (long)getpid());
    snprintf(mut_actual_path, sizeof(mut_actual_path),
             "../../%s", mut_definition_path);
    FILE *own_malformed_workflow = fopen(mut_actual_path, "w");
    AR_ASSERT(own_malformed_workflow != NULL, "Malformed workflow fixture should be created");
    fputs(
        "workflow_name=default_workflow "
        "workflow_version=1.0.0 "
        "initial_stage=intake "
        "terminal_completed=completed "
        "terminal_rejected=rejected "
        "requires_local_completion=true "
        "item_fields=item_id,title,priority,owner,review_status "
        "stages=intake,triage,active,review,completion "
        "transition_count=3 "
        "transition_1_from=intake transition_1_to=triage transition_1_prompt=intake_to_triage_prompt "
        "transition_2_from=triage transition_2_to=active transition_2_prompt=triage_to_active_prompt "
        "transition_3_from=active transition_3_to=review transition_3_prompt=active_to_review_prompt "
        "transition_4_from=review transition_4_to=completion transition_4_prompt=review_to_completion_prompt\n",
        own_malformed_workflow
    );
    fclose(own_malformed_workflow);

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(mut_agency, "workflow-definition", "1.0.0", own_context);
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_reply = prepare_definition_and_take_reply(
        own_fixture,
        mut_agency,
        definition_agent_id,
        mut_definition_path
    );
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "action"), "definition_error") == 0,
              "Missing transition prompt should return definition_error");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "invalid_definition_schema") == 0,
              "Missing transition prompt should be a schema error");
    ar_data__destroy(own_reply);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    remove(mut_actual_path);
    cleanup_fake_runner();
}

static void test_workflow_definition__uses_configured_prompt_for_each_transition(void) {
    printf("Testing workflow-definition uses configured prompt for each transition...\n");

    setup_transition_prompt_runner();

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(mut_agency, "workflow-definition", "1.0.0", own_context);
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_prepare_reply = prepare_definition_and_take_reply(
        own_fixture,
        mut_agency,
        definition_agent_id,
        "workflows/default.workflow"
    );
    AR_ASSERT(strcmp(ar_data__get_map_string(own_prepare_reply, "action"), "definition_ready") == 0,
              "Default definition should prepare");
    ar_data__destroy(own_prepare_reply);

    ar_data_t *own_message = create_evaluate_transition_message("intake", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Intake transition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Intake transition should process");
    ar_data_t *own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "intake_prompt_used") == 0,
              "Intake transition should use intake prompt");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "next_stage"), "triage") == 0,
              "Intake advance should use configured to-stage");
    ar_data__destroy(own_reply);

    own_message = create_evaluate_transition_message("triage", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Triage transition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Triage transition should process");
    own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "outcome"), "stay") == 0,
              "Triage prompt should produce stay");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "next_stage"), "triage") == 0,
              "Stay should keep triage");
    AR_ASSERT(ar_data__get_map_integer(own_reply, "retryable") == 1,
              "Successful stay should be retryable");
    ar_data__destroy(own_reply);

    own_message = create_evaluate_transition_message("active", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Active transition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Active transition should process");
    own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "outcome"), "reject") == 0,
              "Active prompt should produce reject");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "terminal_outcome"), "rejected") == 0,
              "Reject should use terminal rejected outcome");
    ar_data__destroy(own_reply);

    own_message = create_evaluate_transition_message("review", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Review transition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Review transition should process");
    own_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "reason"), "review_prompt_used") == 0,
              "Review transition should use review prompt");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_reply, "terminal_outcome"), "completed") == 0,
              "Review advance should use terminal completed outcome");
    ar_data__destroy(own_reply);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
    cleanup_fake_runner();
}

int main(void) {
    printf("Workflow Definition Method Tests\n");
    printf("===============================\n\n");

    test_workflow_definition__prepare_definition_reads_known_file_and_queues_ready_message();
    test_workflow_definition__invalid_schema_returns_definition_error();
    test_workflow_definition__invalid_schema_reason_wins_over_probe_failure();
    test_workflow_definition__complete_failure_maps_to_retryable_stay();
    test_workflow_definition__complete_success_uses_outcome_and_reason();
    test_workflow_definition__complete_prompt_uses_workflow_item_context();
    test_workflow_definition__real_completion_review_status_drives_default_and_test_outcomes();
    test_workflow_definition__missing_transition_fields_returns_definition_error();
    test_workflow_definition__uses_configured_prompt_for_each_transition();

    printf("\nAll workflow-definition tests passed!\n");
    return 0;
}
