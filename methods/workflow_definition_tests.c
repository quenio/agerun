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
        "  *\"review_status=pending\"*) "
        "printf 'outcome=stay\\nreason=review_not_approved\\n' ;;\n",
        own_runner
    );
    fputs(
        "  *\"review_status=approved\"*) "
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
    AR_ASSERT(ar_data__set_map_integer(own_message, "transition_count", 3),
              "Evaluate message should include transition count");
    AR_ASSERT(ar_data__set_map_integer(own_message, "reply_to", (int)reply_to),
              "Evaluate message should include reply target");
    return own_message;
}

static ar_data_t *create_evaluate_transition_message(const char *ref_stage, int64_t reply_to) {
    return create_evaluate_transition_message_with_review(ref_stage, "approved", reply_to);
}

static void debug_parse(void) {
    FILE *own_file = fopen("../../methods/workflow-definition-1.0.0.method", "r");
    AR_ASSERT(own_file != NULL, "Method file should open");
    fseek(own_file, 0, SEEK_END);
    long file_size = ftell(own_file);
    AR_ASSERT(file_size >= 0, "Definition method file size should be readable");
    fseek(own_file, 0, SEEK_SET);
    char *own_content = malloc((size_t)file_size + 1U);
    AR_ASSERT(own_content != NULL, "Debug content should allocate");
    size_t read_size = fread(own_content, 1, (size_t)file_size, own_file);
    own_content[read_size] = '\0';
    fclose(own_file);
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Debug log should create");
    ar_method_t *own_method = ar_method__create_with_log("workflow-definition-debug", own_content, "1.0.0", own_log);
    AR_ASSERT(own_method != NULL, "Debug method should create");
    if (ar_method__get_ast(own_method) == NULL) {
        const char *ref_error = ar_log__get_last_error_message(own_log);
        printf("parse-error=%s\n", ref_error ? ref_error : "(none)");
        fflush(stdout);
    }
    ar_method__destroy(own_method);
    ar_log__destroy(own_log);
    free(own_content);
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

static void test_workflow_definition__prepare_definition_reads_known_file_and_queues_ready_message(void) {
    printf("Testing workflow-definition prepare_definition queues definition_ready...\n");

    setup_fake_runner("outcome=advance\\nreason=probe_ok\\n");

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(mut_agency, "workflow-definition", "1.0.0", own_context);
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_message = create_prepare_definition_message("workflows/default.workflow", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Prepare definition should queue");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Definition should process prepare_definition");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, definition_agent_id);
    AR_ASSERT(ref_memory != NULL, "Definition memory should exist");
    const char *ref_workflow_name = ar_data__get_map_string(ref_memory, "workflow_name");
    const char *ref_file_status = ar_data__get_map_string(ref_memory, "file_status");
    const char *ref_dependency_status = ar_data__get_map_string(ref_memory, "dependency_status");
    const ar_data_t *ref_ready_payload = ar_data__get_map_data(ref_memory, "ready_payload");
    const char *ref_ready_action = ref_ready_payload ? ar_data__get_map_string(ref_ready_payload, "action") : NULL;
    printf("workflow_name=%s file_status=%s dependency_status=%s probe_ok=%d transition_ok=%d ready_flag=%d error_flag=%d ready_sent=%d reply_to_id=%d ready_action=%s\n",
           ref_workflow_name ? ref_workflow_name : "(null)",
           ref_file_status ? ref_file_status : "(null)",
           ref_dependency_status ? ref_dependency_status : "(null)",
           ar_data__get_map_integer(ref_memory, "probe_ok"),
           ar_data__get_map_integer(ref_memory, "transition_ok"),
           ar_data__get_map_integer(ref_memory, "ready_flag"),
           ar_data__get_map_integer(ref_memory, "error_flag"),
           ar_data__get_map_integer(ref_memory, "ready_sent"),
           ar_data__get_map_integer(ref_memory, "reply_to_id"),
           ref_ready_action ? ref_ready_action : "(null)");
    fflush(stdout);
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

    ar_method_fixture_t *own_fixture = create_fixture();
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Definition context should be created");
    int64_t definition_agent_id = ar_agency__create_agent(mut_agency, "workflow-definition", "1.0.0", own_context);
    AR_ASSERT(definition_agent_id == 1, "Definition agent should be created");

    ar_data_t *own_message = create_prepare_definition_message("invalid.workflow", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_message),
              "Prepare definition should queue");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Definition should process invalid definition");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, definition_agent_id);
    const ar_data_t *ref_error_payload = ar_data__get_map_data(ref_memory, "error_payload");
    const char *ref_error_action = ref_error_payload ? ar_data__get_map_string(ref_error_payload, "action") : NULL;
    printf("invalid last_reason=%s error_flag=%d error_sent=%d reply_to_id=%d error_action=%s category=%s\n",
           ar_data__get_map_string(ref_memory, "last_reason") ? ar_data__get_map_string(ref_memory, "last_reason") : "(null)",
           ar_data__get_map_integer(ref_memory, "error_flag"),
           ar_data__get_map_integer(ref_memory, "error_sent"),
           ar_data__get_map_integer(ref_memory, "reply_to_id"),
           ref_error_action ? ref_error_action : "(null)",
           ar_data__get_map_string(ref_memory, "error_category") ? ar_data__get_map_string(ref_memory, "error_category") : "(null)" );
    fflush(stdout);
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

    ar_data_t *own_prepare = create_prepare_definition_message("workflows/default.workflow", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_prepare),
              "Prepare definition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Prepare should process");
    ar_data_t *own_prepare_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
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

    ar_data_t *own_prepare = create_prepare_definition_message("workflows/default.workflow", definition_agent_id);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, definition_agent_id, own_prepare),
              "Prepare definition should queue");
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Prepare should process");
    ar_data_t *own_prepare_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
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
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Prepare should process");
    ar_data_t *own_prepare_reply = ar_agency__get_agent_message(mut_agency, definition_agent_id);
    ar_data__destroy(own_prepare_reply);

    // When evaluating a review transition whose item review is still pending
    ar_data_t *own_message = create_evaluate_transition_message_with_review(
        "review",
        "pending",
        definition_agent_id
    );
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

int main(void) {
    printf("Workflow Definition Method Tests\n");
    printf("===============================\n\n");

    debug_parse();
    test_workflow_definition__prepare_definition_reads_known_file_and_queues_ready_message();
    test_workflow_definition__invalid_schema_returns_definition_error();
    test_workflow_definition__complete_failure_maps_to_retryable_stay();
    test_workflow_definition__complete_success_uses_outcome_and_reason();
    test_workflow_definition__complete_prompt_uses_workflow_item_context();

    printf("\nAll workflow-definition tests passed!\n");
    return 0;
}
