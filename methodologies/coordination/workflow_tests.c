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
        "memory.last_current_step := message.current_step\n"
        "memory.last_completed_step_count := message.completed_step_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void append_workflow_step(ar_data_t *mut_step_targets,
                                 ar_data_t *mut_step_actions,
                                 ar_data_t *mut_step_texts,
                                 int target,
                                 const char *ref_action,
                                 const char *ref_text) {
    AR_ASSERT(ar_data__list_add_last_integer(mut_step_targets, target),
              "Workflow step target should append");
    AR_ASSERT(ar_data__list_add_last_string(mut_step_actions, ref_action),
              "Workflow step action should append");
    AR_ASSERT(ar_data__list_add_last_string(mut_step_texts, ref_text),
              "Workflow step text should append");
}

static void send_step_done(ar_agency_t *mut_agency,
                           int64_t workflow_agent,
                           const char *ref_workflow_id,
                           int step,
                           const char *ref_outcome) {
    ar_data_t *own_done = ar_data__create_map();
    AR_ASSERT(own_done != NULL, "Step completion should be created");
    ar_data__set_map_string(own_done, "action", "step_done");
    ar_data__set_map_string(own_done, "workflow_id", ref_workflow_id);
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
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_report_context);

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Workflow start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-1");
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(report_agent));
    ar_data_t *own_step_targets = ar_data__create_list();
    ar_data_t *own_step_actions = ar_data__create_list();
    ar_data_t *own_step_texts = ar_data__create_list();
    AR_ASSERT(own_step_targets != NULL, "Workflow step targets list should be created");
    AR_ASSERT(own_step_actions != NULL, "Workflow step actions list should be created");
    AR_ASSERT(own_step_texts != NULL, "Workflow step texts list should be created");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step1_agent), "step1", "first");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step2_agent), "step2", "second");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step3_agent), "step3", "third");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step4_agent), "step4", "fourth");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step5_agent), "step5", "fifth");
    AR_ASSERT(ar_data__set_map_data(own_start, "step_targets", own_step_targets),
              "Workflow start should own step targets list");
    own_step_targets = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_actions", own_step_actions),
              "Workflow start should own step actions list");
    own_step_actions = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_texts", own_step_texts),
              "Workflow start should own step texts list");
    own_step_texts = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step1_memory = ar_agency__get_agent_memory(mut_agency, step1_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_step1_memory, "last_text"), "first") == 0,
              "Workflow should send first step");

    send_step_done(mut_agency, workflow_agent, "wf-1", 3, "continue");
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_step2_memory = ar_agency__get_agent_memory(mut_agency, step2_agent);
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
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_action"),
                     "workflow_complete") == 0,
              "Workflow should emit completion report");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "complete") == 0,
              "Workflow completion status should be complete");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_current_step") == 5,
              "Workflow completion should report final step");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_completed_step_count") == 4,
              "Workflow completion should count executed steps");

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Zero-head workflow start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-zero-head");
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(report_agent));
    own_step_targets = ar_data__create_list();
    own_step_actions = ar_data__create_list();
    own_step_texts = ar_data__create_list();
    AR_ASSERT(own_step_targets != NULL, "Zero-head targets should be created");
    AR_ASSERT(own_step_actions != NULL, "Zero-head actions should be created");
    AR_ASSERT(own_step_texts != NULL, "Zero-head texts should be created");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         0, "noop", "placeholder");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step1_agent), "step1", "after-zero");
    AR_ASSERT(ar_data__set_map_data(own_start, "step_targets", own_step_targets),
              "Zero-head start should own step targets");
    own_step_targets = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_actions", own_step_actions),
              "Zero-head start should own step actions");
    own_step_actions = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_texts", own_step_texts),
              "Zero-head start should own step texts");
    own_step_texts = NULL;
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

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_workflow_id"),
                     "wf-zero-head") == 0,
              "Zero-head workflow should report the workflow id after sent step");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "complete") == 0,
              "Zero-head workflow should complete after the sent positive step");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_current_step") == 1,
              "Zero-head workflow should not count the placeholder as a step");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_completed_step_count") == 1,
              "Zero-head workflow should count the sent positive step");

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Post-completion zero workflow start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-post-completion-zero");
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(report_agent));
    own_step_targets = ar_data__create_list();
    own_step_actions = ar_data__create_list();
    own_step_texts = ar_data__create_list();
    AR_ASSERT(own_step_targets != NULL, "Post-completion zero targets should be created");
    AR_ASSERT(own_step_actions != NULL, "Post-completion zero actions should be created");
    AR_ASSERT(own_step_texts != NULL, "Post-completion zero texts should be created");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step1_agent), "step1", "before-zero");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         0, "noop", "placeholder");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step2_agent), "step2", "after-completion-zero");
    AR_ASSERT(ar_data__set_map_data(own_start, "step_targets", own_step_targets),
              "Post-completion zero start should own step targets");
    own_step_targets = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_actions", own_step_actions),
              "Post-completion zero start should own step actions");
    own_step_actions = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_texts", own_step_texts),
              "Post-completion zero start should own step texts");
    own_step_texts = NULL;
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
              "Workflow should skip a pending zero head after step completion");

    send_step_done(mut_agency, workflow_agent, "wf-post-completion-zero", 2, "done");
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_workflow_id"),
                     "wf-post-completion-zero") == 0,
              "Post-completion zero workflow should report the workflow id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "complete") == 0,
              "Post-completion zero workflow should complete after later positive step");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_current_step") == 2,
              "Post-completion zero workflow should number the later positive step");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_completed_step_count") == 2,
              "Post-completion zero workflow should count sent positive steps only");

    const ar_data_t *ref_workflow_memory = ar_agency__get_agent_memory(mut_agency, workflow_agent);
    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Failed worker send workflow start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-failed-worker-send");
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(report_agent));
    own_step_targets = ar_data__create_list();
    own_step_actions = ar_data__create_list();
    own_step_texts = ar_data__create_list();
    AR_ASSERT(own_step_targets != NULL, "Failed worker send target list should be created");
    AR_ASSERT(own_step_actions != NULL, "Failed worker send action list should be created");
    AR_ASSERT(own_step_texts != NULL, "Failed worker send text list should be created");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         98765, "step1", "missing-worker");
    AR_ASSERT(ar_data__set_map_data(own_start, "step_targets", own_step_targets),
              "Failed worker send start should own step targets");
    own_step_targets = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_actions", own_step_actions),
              "Failed worker send start should own step actions");
    own_step_actions = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_texts", own_step_texts),
              "Failed worker send start should own step texts");
    own_step_texts = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, workflow_agent, own_start),
              "Failed worker send workflow start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_workflow_id"),
                     "wf-failed-worker-send") == 0,
              "Failed worker send should report the workflow id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "handoff_failed") == 0,
              "Failed worker send should report handoff failure");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_current_step") == 1,
              "Failed worker send should report the attempted step");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_completed_step_count") == 0,
              "Failed worker send should not count the undelivered step");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_workflow_memory, "status"),
                     "handoff_failed") == 0,
              "Failed worker send should store handoff failure status");

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Failed completion workflow start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-failed-completion");
    ar_data__set_map_integer(own_start, "reply_to", 98765);
    own_step_targets = ar_data__create_list();
    own_step_actions = ar_data__create_list();
    own_step_texts = ar_data__create_list();
    AR_ASSERT(own_step_targets != NULL, "Failed completion step targets list should be created");
    AR_ASSERT(own_step_actions != NULL, "Failed completion step actions list should be created");
    AR_ASSERT(own_step_texts != NULL, "Failed completion step texts list should be created");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step1_agent), "step1", "single");
    AR_ASSERT(ar_data__set_map_data(own_start, "step_targets", own_step_targets),
              "Failed completion start should own step targets list");
    own_step_targets = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_actions", own_step_actions),
              "Failed completion start should own step actions list");
    own_step_actions = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_texts", own_step_texts),
              "Failed completion start should own step texts list");
    own_step_texts = NULL;
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
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-failed-start-handoff");
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(report_agent));
    own_step_targets = ar_data__create_list();
    own_step_actions = ar_data__create_list();
    own_step_texts = ar_data__create_list();
    AR_ASSERT(own_step_targets != NULL, "Failed start handoff targets should be created");
    AR_ASSERT(own_step_actions != NULL, "Failed start handoff actions should be created");
    AR_ASSERT(own_step_texts != NULL, "Failed start handoff texts should be created");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step1_agent), "step1", "handoff");
    AR_ASSERT(ar_data__set_map_data(own_start, "step_targets", own_step_targets),
              "Failed start handoff should own step targets");
    own_step_targets = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_actions", own_step_actions),
              "Failed start handoff should own step actions");
    own_step_actions = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_texts", own_step_texts),
              "Failed start handoff should own step texts");
    own_step_texts = NULL;
    ar_data__set_map_string(own_start, "branch_value", "skip");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, failed_handoff_agent, own_start),
              "Failed start handoff workflow should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_failed_handoff_memory =
        ar_agency__get_agent_memory(mut_agency, failed_handoff_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_workflow_id"),
                     "wf-failed-start-handoff") == 0,
              "Failed start handoff should report the workflow id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "handoff_failed") == 0,
              "Failed start handoff should report handoff failure");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_handoff_memory, "status"),
                     "handoff_failed") == 0,
              "Failed start handoff should store handoff failure status");

    ar_data_t *own_failed_continue_context = create_context();
    int64_t failed_continue_agent = ar_agency__create_agent(
        mut_agency, "workflow", "1.0.0", own_failed_continue_context);

    own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Failed continuation workflow start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "workflow_id", "wf-failed-continue-handoff");
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(report_agent));
    own_step_targets = ar_data__create_list();
    own_step_actions = ar_data__create_list();
    own_step_texts = ar_data__create_list();
    AR_ASSERT(own_step_targets != NULL, "Failed continuation targets should be created");
    AR_ASSERT(own_step_actions != NULL, "Failed continuation actions should be created");
    AR_ASSERT(own_step_texts != NULL, "Failed continuation texts should be created");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step1_agent), "step1", "continue-a");
    append_workflow_step(own_step_targets, own_step_actions, own_step_texts,
                         checked_agent_id(step2_agent), "step2", "continue-b");
    AR_ASSERT(ar_data__set_map_data(own_start, "step_targets", own_step_targets),
              "Failed continuation should own step targets");
    own_step_targets = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_actions", own_step_actions),
              "Failed continuation should own step actions");
    own_step_actions = NULL;
    AR_ASSERT(ar_data__set_map_data(own_start, "step_texts", own_step_texts),
              "Failed continuation should own step texts");
    own_step_texts = NULL;
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
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_workflow_id"),
                     "wf-failed-continue-handoff") == 0,
              "Failed continuation handoff should report the workflow id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "handoff_failed") == 0,
              "Failed continuation handoff should report handoff failure");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_failed_continue_memory, "status"),
                     "handoff_failed") == 0,
              "Failed continuation handoff should store handoff failure status");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_workflow_context);
    ar_data__destroy(own_step1_context);
    ar_data__destroy(own_step2_context);
    ar_data__destroy(own_step3_context);
    ar_data__destroy(own_step4_context);
    ar_data__destroy(own_step5_context);
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
