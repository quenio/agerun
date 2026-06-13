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
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_text := message.text\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_session_id := message.session_id\n"
        "memory.last_kind := message.kind\n"
        "memory.last_sender := message.sender\n"
        "memory.last_message := message\n"
        "memory.last_status := message.status\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void assert_workflow_result_omits_redundant_fields(const ar_data_t *ref_report_memory) {
    const ar_data_t *ref_message = ar_data__get_map_data(ref_report_memory, "last_message");
    AR_ASSERT(ref_message != NULL, "Workflow result message should be recorded");
    AR_ASSERT(ar_data__get_map_data(ref_message, "current_step") == NULL,
              "Workflow result should omit redundant current step");
    AR_ASSERT(ar_data__get_map_data(ref_message, "completed_step_count") == NULL,
              "Workflow result should omit redundant completed step count");
    AR_ASSERT(ar_data__get_map_data(ref_message, "state") == NULL,
              "Workflow result should omit redundant state");
}

static ar_data_t *create_step_payload(const char *ref_action,
                                      const char *ref_text,
                                      const char *ref_kind,
                                      int sender) {
    ar_data_t *own_payload = ar_data__create_map();
    AR_ASSERT(own_payload != NULL, "Workflow step payload should be created");
    AR_ASSERT(ar_data__set_map_string(own_payload, "action", ref_action),
              "Workflow step payload should set action");
    AR_ASSERT(ar_data__set_map_string(own_payload, "text", ref_text),
              "Workflow step payload should set text");
    AR_ASSERT(ar_data__set_map_string(own_payload, "kind", ref_kind),
              "Workflow step payload should set caller-owned field");
    AR_ASSERT(ar_data__set_map_integer(own_payload, "sender", sender),
              "Workflow step payload should set standard sender");
    return own_payload;
}

static void append_workflow_step(ar_data_t *mut_step_recipients,
                                 ar_data_t *mut_step_payloads,
                                 int recipient,
                                 const char *ref_action,
                                 const char *ref_text) {
    AR_ASSERT(ar_data__list_add_last_integer(mut_step_recipients, recipient),
              "Workflow step recipient should append");
    ar_data_t *own_payload = create_step_payload(ref_action,
                                                 ref_text,
                                                 "caller-shaped",
                                                 31415);
    AR_ASSERT(ar_data__list_add_last_data(mut_step_payloads, own_payload),
              "Workflow step payload should append");
    own_payload = NULL;
}

static void send_step_done(ar_agency_t *mut_agency,
                           int64_t workflow_agent,
                           const char *ref_session_id,
                           int step,
                           const char *ref_outcome) {
    ar_data_t *own_done = ar_data__create_map();
    AR_ASSERT(own_done != NULL, "Step completion should be created");
    ar_data__set_map_string(own_done, "request", "workflow_step_done");
    ar_data__set_map_string(own_done, "trace_id", ref_session_id);
    ar_data__set_map_string(own_done, "session_id", ref_session_id);
    ar_data__set_map_integer(own_done, "step", step);
    ar_data__set_map_string(own_done, "outcome", ref_outcome);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_done),
              "Step completion should queue");
    own_done = NULL;
}

static void test_workflow__sends_unbounded_steps_with_branching_to_completion(void) {
    printf("Testing workflow sends unbounded steps with branching to completion...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("workflow_branching");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "workflow");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_workflow_context = create_context();
    ar_data_t *own_step1_context = create_context();
    ar_data_t *own_step2_context = create_context();
    ar_data_t *own_step3_context = create_context();
    ar_data_t *own_step4_context = create_context();
    ar_data_t *own_step5_context = create_context();
    ar_data_t *own_missing_payload_context = create_context();
    ar_data_t *own_report_context = create_context();
    int64_t workflow_agent = ar_agency__create_agent(
        mut_agency, "workflow", "1.0.0", own_workflow_context);
    int64_t step1_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step1_context);
    int64_t step2_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step2_context);
    int64_t step3_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step3_context);
    int64_t step4_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step4_context);
    int64_t step5_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_step5_context);
    int64_t missing_payload_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_missing_payload_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_report_context);

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "workflow-trace-1");
    ar_data__set_map_string(own_start, "session_id", "wf-1");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    ar_data_t *own_step_recipients = ar_data__create_list();
    ar_data_t *own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Workflow step recipients list should be created");
    AR_ASSERT(own_step_payloads != NULL, "Workflow step payloads list should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "first");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step2_agent), "step2", "second");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step3_agent), "step3", "third");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step4_agent), "step4", "fourth");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step5_agent), "step5", "fifth");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Workflow start should own step recipients list");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Workflow start should own step payloads list");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step1_memory = ar_agency__get_agent_memory(mut_agency, step1_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_action"), "step1") == 0,
              "Workflow should preserve first step payload action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_text"), "first") == 0,
              "Workflow should send first step");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_kind"),
                     "caller-shaped") == 0,
              "Workflow should preserve caller-owned payload field");
    AR_ASSERT(ar_data__get_map_integer(ref_step1_memory, "last_sender") == 31415,
              "Workflow should preserve caller payload sender");

    const ar_data_t *ref_step2_memory = ar_agency__get_agent_memory(mut_agency, step2_agent);
    ar_data_t *own_forged_execute = ar_data__create_map();
    AR_ASSERT(own_forged_execute != NULL, "Forged execute-step message should be created");
    ar_data__set_map_string(own_forged_execute, "request", "workflow_execute_step");
    ar_data__set_map_string(own_forged_execute, "trace_id", "workflow-forged-execute");
    ar_data__set_map_string(own_forged_execute, "session_id", "wf-1");
    ar_data__set_map_integer(own_forged_execute, "sender", checked_agent_id(report_agent));
    ar_data__set_map_integer(own_forged_execute, "step_number", 2);
    ar_data__set_map_integer(own_forged_execute, "skip_positive", 0);
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Forged execute recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Forged execute payloads should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step2_agent), "step2", "forged-execute");
    AR_ASSERT(ar_data__set_map_data(own_forged_execute, "recipients", own_step_recipients),
              "Forged execute should own recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_forged_execute, "payloads", own_step_payloads),
              "Forged execute should own payloads");
    own_step_payloads = NULL;
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_forged_execute),
              "Forged execute-step message should queue");
    own_forged_execute = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_data(ref_step2_memory, "last_text") == NULL,
              "Workflow should ignore externally sent execute-step requests");

    send_step_done(mut_agency, workflow_agent, "wf-1", 3, "continue");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step3_memory = ar_agency__get_agent_memory(mut_agency, step3_agent);
    AR_ASSERT(ar_data__get_map_data(ref_step2_memory, "last_text") == NULL,
              "Workflow should ignore out-of-order completion before sending step two");
    AR_ASSERT(ar_data__get_map_data(ref_step3_memory, "last_text") == NULL,
              "Workflow should ignore out-of-order completion before sending step three");

    send_step_done(mut_agency, workflow_agent, "wf-1", 1, "skip");
    send_step_done(mut_agency, workflow_agent, "wf-1", 1, "skip");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_data(ref_step2_memory, "last_text") == NULL,
              "Workflow should skip step two on matching branch outcome");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step3_memory, "last_text"), "third") == 0,
              "Workflow should send branched step three");

    send_step_done(mut_agency, workflow_agent, "wf-1", 3, "continue");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step4_memory = ar_agency__get_agent_memory(mut_agency, step4_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step4_memory, "last_text"), "fourth") == 0,
              "Workflow should send fourth step from an unbounded list");

    send_step_done(mut_agency, workflow_agent, "wf-1", 4, "continue");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step5_memory = ar_agency__get_agent_memory(mut_agency, step5_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step5_memory, "last_text"), "fifth") == 0,
              "Workflow should send fifth step from an unbounded list");

    send_step_done(mut_agency, workflow_agent, "wf-1", 5, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_response"),
                     "workflow_result") == 0,
              "Workflow completion should be a response");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sender") ==
                  checked_agent_id(workflow_agent),
              "Workflow completion should identify the workflow sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "success") == 0,
              "Workflow completion should report standard success status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "wf-1") == 0,
              "Workflow completion should preserve completing request trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_session_id"),
                     "wf-1") == 0,
              "Workflow completion should preserve session id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 4,
              "Workflow completion should report completed step count");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Workflow completion should report no handoff failures");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Missing-recipient workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-missing-recipients-start");
    ar_data__set_map_string(own_start, "session_id", "wf-missing-recipients");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Missing-recipient workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "wf-missing-recipients-start") == 0,
              "Missing-recipient workflow should complete without looping");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "success") == 0,
              "Missing-recipient workflow should report standard success status");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 0,
              "Missing-recipient workflow should report zero completed steps");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Missing-recipient workflow should report no handoff failure");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Non-list workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-non-list-recipients-start");
    ar_data__set_map_string(own_start, "session_id", "wf-non-list-recipients");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    ar_data__set_map_integer(own_start, "recipients", 42);
    ar_data__set_map_integer(own_start, "payloads", 7);
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Non-list workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "wf-non-list-recipients-start") == 0,
              "Non-list workflow should complete without looping");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "success") == 0,
              "Non-list workflow should report standard success status");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 0,
              "Non-list workflow should report zero completed steps");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Non-list workflow should report no handoff failure");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Short-payload workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-short-payloads-start");
    ar_data__set_map_string(own_start, "session_id", "wf-short-payloads");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Short-payload recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Short-payload payloads should be created");
    AR_ASSERT(ar_data__list_add_last_integer(own_step_recipients,
                                             checked_agent_id(missing_payload_agent)),
              "Short-payload recipient should append");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Short-payload start should own recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Short-payload start should own empty payload list");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Short-payload workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_missing_payload_memory =
        ar_agency__get_agent_memory(mut_agency, missing_payload_agent);
    AR_ASSERT(ar_data__get_map_data(ref_missing_payload_memory, "last_sender") == NULL,
              "Short-payload workflow should not send a sentinel payload");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "wf-short-payloads-start") == 0,
              "Short-payload workflow should emit its own result");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "failure") == 0,
              "Short-payload workflow should report standard failure status");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 0,
              "Short-payload workflow should report zero completed steps");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 1,
              "Short-payload workflow should report one handoff failure");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Zero-head workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-zero-head-start");
    ar_data__set_map_string(own_start, "session_id", "wf-zero-head");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Zero-head recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Zero-head payloads should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         0, "noop", "placeholder");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "after-zero");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Zero-head start should own step recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Zero-head start should own step payloads");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Zero-head workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_text"),
                     "after-zero") == 0,
              "Workflow should skip a zero head and send the later positive step");

    send_step_done(mut_agency, workflow_agent, "wf-zero-head", 1, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 1,
              "Zero-head workflow should count the sent positive step");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Consecutive zero-head workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-zero-head-repeat-start");
    ar_data__set_map_string(own_start, "session_id", "wf-zero-head-repeat");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Consecutive zero-head recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Consecutive zero-head payloads should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         0, "noop", "first-placeholder");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         0, "noop", "second-placeholder");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "after-two-zeros");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Consecutive zero-head start should own step recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Consecutive zero-head start should own step payloads");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Consecutive zero-head workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_text"),
                     "after-two-zeros") == 0,
              "Workflow should skip consecutive zero heads and send the later positive step");

    send_step_done(mut_agency, workflow_agent, "wf-zero-head-repeat", 1, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 1,
              "Consecutive zero-head workflow should count the sent positive step");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Post-completion zero workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-post-completion-zero-start");
    ar_data__set_map_string(own_start, "session_id", "wf-post-completion-zero");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Post-completion zero recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Post-completion zero payloads should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "before-zero");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         0, "noop", "placeholder");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         0, "noop", "second-placeholder");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step2_agent), "step2", "after-completion-zero");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Post-completion zero start should own step recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Post-completion zero start should own step payloads");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Post-completion zero workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_text"),
                     "before-zero") == 0,
              "Post-completion zero workflow should send first step");

    send_step_done(mut_agency, workflow_agent, "wf-post-completion-zero", 1, "continue");
    ar_method_fixture__process_all_messages(own_fixture);

    const char *ref_step2_text = ar_data__get_map_string(ref_step2_memory, "last_text");
    AR_ASSERT(ref_step2_text != NULL && strcmp(ref_step2_text, "after-completion-zero") == 0,
              "Workflow should skip consecutive pending zero heads after step completion");

    send_step_done(mut_agency, workflow_agent, "wf-post-completion-zero", 2, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 2,
              "Post-completion zero workflow should count sent positive steps only");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Branch zero-skip workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-branch-zero-skip-start");
    ar_data__set_map_string(own_start, "session_id", "wf-branch-zero-skip");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Branch zero-skip recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Branch zero-skip payloads should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "branch-before-zero");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         0, "noop", "placeholder");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step2_agent), "step2", "branch-skipped");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step3_agent), "step3", "branch-after-zero-skip");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Branch zero-skip start should own step recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Branch zero-skip start should own step payloads");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Branch zero-skip workflow should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_text"),
                     "branch-before-zero") == 0,
              "Branch zero-skip workflow should send first step");

    send_step_done(mut_agency, workflow_agent, "wf-branch-zero-skip", 1, "skip");
    ar_method_fixture__process_all_messages(own_fixture);

    ref_step2_text = ar_data__get_map_string(ref_step2_memory, "last_text");
    AR_ASSERT(ref_step2_text == NULL || strcmp(ref_step2_text, "branch-skipped") != 0,
              "Branch skip should skip the next positive step after zero placeholders");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step3_memory, "last_text"),
                     "branch-after-zero-skip") == 0,
              "Branch skip should send the later positive step after the skipped step");

    send_step_done(mut_agency, workflow_agent, "wf-branch-zero-skip", 3, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 2,
              "Branch zero-skip workflow should count only sent positive steps");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);

    const ar_data_t *ref_workflow_memory = ar_agency__get_agent_memory(mut_agency, workflow_agent);
    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Failed worker send workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-failed-worker-send-start");
    ar_data__set_map_string(own_start, "session_id", "wf-failed-worker-send");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Failed worker send recipient list should be created");
    AR_ASSERT(own_step_payloads != NULL, "Failed worker send payload list should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         98765, "step1", "missing-worker");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Failed worker send start should own step recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Failed worker send start should own step payloads");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Failed worker send workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Failed worker send should report standard failure status");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 0,
              "Failed worker send should report no completed steps");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 1,
              "Failed worker send should report one handoff failure");
    assert_workflow_result_omits_redundant_fields(ref_report_memory);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_workflow_memory, "status"),
                     "failure") == 0,
              "Failed worker send should store standard failure status");

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Failed completion workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-failed-completion-start");
    ar_data__set_map_string(own_start, "session_id", "wf-failed-completion");
    ar_data__set_map_integer(own_start, "sender", 98765);
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Failed completion step recipients list should be created");
    AR_ASSERT(own_step_payloads != NULL, "Failed completion step payloads list should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "single");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Failed completion start should own step recipients list");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Failed completion start should own step payloads list");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Failed completion workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    send_step_done(mut_agency, workflow_agent, "wf-failed-completion", 1, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_workflow_memory, "status"), "active") == 0,
              "Failed completion send should keep workflow active");
    AR_ASSERT(ar_data__get_map_integer(ref_workflow_memory, "completion_pending") == 1,
              "Failed completion send should leave completion pending");
    AR_ASSERT(ar_data__get_map_integer(ref_workflow_memory, "completed_step_count") == 1,
              "Failed completion send should count the final step once");

    send_step_done(mut_agency, workflow_agent, "wf-failed-completion", 1, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_workflow_memory, "status"), "active") == 0,
              "Repeated failed completion send should keep workflow active");
    AR_ASSERT(ar_data__get_map_integer(ref_workflow_memory, "completion_pending") == 1,
              "Repeated failed completion send should leave completion pending");
    AR_ASSERT(ar_data__get_map_integer(ref_workflow_memory, "completed_step_count") == 1,
              "Repeated completion retry should not count the final step again");

    ar_data_t *own_failed_handoff_context = create_context();
    int64_t failed_handoff_agent = ar_agency__create_agent(
        mut_agency, "workflow", "1.0.0", own_failed_handoff_context);
    ar_data_t *mut_failed_handoff_memory =
        ar_agency__get_agent_mutable_memory(mut_agency, failed_handoff_agent);
    AR_ASSERT(mut_failed_handoff_memory != NULL, "Failed handoff workflow memory should exist");
    AR_ASSERT(ar_data__set_map_integer(mut_failed_handoff_memory, "self", 98765),
              "Failed start handoff should corrupt self for regression setup");

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Failed start handoff workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-failed-start-handoff-start");
    ar_data__set_map_string(own_start, "session_id", "wf-failed-start-handoff");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Failed start handoff recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Failed start handoff payloads should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "handoff");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Failed start handoff should own step recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Failed start handoff should own step payloads");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, failed_handoff_agent, own_start),
              "Failed start handoff workflow should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_failed_handoff_memory =
        ar_agency__get_agent_memory(mut_agency, failed_handoff_agent);
    assert_workflow_result_omits_redundant_fields(ref_report_memory);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_handoff_memory, "status"),
                     "failure") == 0,
              "Failed start handoff should store standard failure status");

    ar_data_t *own_failed_continue_context = create_context();
    int64_t failed_continue_agent = ar_agency__create_agent(
        mut_agency, "workflow", "1.0.0", own_failed_continue_context);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Failed continuation workflow start should be created");
    ar_data__set_map_string(own_start, "request", "workflow_start");
    ar_data__set_map_string(own_start, "trace_id", "wf-failed-continue-handoff-start");
    ar_data__set_map_string(own_start, "session_id", "wf-failed-continue-handoff");
    ar_data__set_map_integer(own_start, "sender", checked_agent_id(report_agent));
    own_step_recipients = ar_data__create_list();
    own_step_payloads = ar_data__create_list();
    AR_ASSERT(own_step_recipients != NULL, "Failed continuation recipients should be created");
    AR_ASSERT(own_step_payloads != NULL, "Failed continuation payloads should be created");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step1_agent), "step1", "continue-a");
    append_workflow_step(own_step_recipients, own_step_payloads,
                         checked_agent_id(step2_agent), "step2", "continue-b");
    AR_ASSERT(ar_data__set_map_data(own_start, "recipients", own_step_recipients),
              "Failed continuation should own step recipients");
    own_step_recipients = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "payloads", own_step_payloads),
              "Failed continuation should own step payloads");
    own_step_payloads = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, failed_continue_agent, own_start),
              "Failed continuation workflow should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *mut_failed_continue_memory =
        ar_agency__get_agent_mutable_memory(mut_agency, failed_continue_agent);
    AR_ASSERT(mut_failed_continue_memory != NULL, "Failed continuation memory should exist");
    AR_ASSERT(ar_data__set_map_integer(mut_failed_continue_memory, "self", 98765),
              "Failed continuation should corrupt self for regression setup");
    send_step_done(mut_agency, failed_continue_agent, "wf-failed-continue-handoff", 1, "continue");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_failed_continue_memory =
        ar_agency__get_agent_memory(mut_agency, failed_continue_agent);
    assert_workflow_result_omits_redundant_fields(ref_report_memory);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_continue_memory, "status"),
                     "failure") == 0,
              "Failed continuation handoff should store standard failure status");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_workflow_context);
    ar_data__destroy(own_step1_context);
    ar_data__destroy(own_step2_context);
    ar_data__destroy(own_step3_context);
    ar_data__destroy(own_step4_context);
    ar_data__destroy(own_step5_context);
    ar_data__destroy(own_missing_payload_context);
    ar_data__destroy(own_report_context);
    ar_data__destroy(own_failed_handoff_context);
    ar_data__destroy(own_failed_continue_context);
}

int main(void) {
    printf("Running workflow method tests...\n\n");
    test_workflow__sends_unbounded_steps_with_branching_to_completion();
    printf("\nAll workflow method tests passed!\n");
    return 0;
}
