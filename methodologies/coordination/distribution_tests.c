#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_heap.h"
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

static ar_data_t *create_worker_context(void) {
    return create_context();
}

static void initialize_worker_memory(ar_agency_t *mut_agency, int64_t worker_agent) {
    ar_data_t *mut_memory = ar_agency__get_agent_mutable_memory(mut_agency, worker_agent);
    AR_ASSERT(mut_memory != NULL, "Worker memory should be available");
    ar_data_t *own_received_texts = ar_data__create_list();
    AR_ASSERT(own_received_texts != NULL, "Worker history should be created");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "received_texts", own_received_texts),
              "Worker memory should own history list");
    own_received_texts = NULL;
    AR_ASSERT(ar_data__set_map_integer(mut_memory, "received_count", 0),
              "Worker memory should initialize received count");
}

static int checked_agent_id(int64_t agent_id) {
    AR_ASSERT(agent_id > 0, "Agent id should be positive");
    AR_ASSERT(agent_id <= INT_MAX, "Agent id should fit test message integer");
    return (int)agent_id;
}

static ar_data_t *create_payload(const char *ref_action,
                                 const char *ref_text,
                                 const char *ref_kind,
                                 int sender) {
    ar_data_t *own_payload = ar_data__create_map();
    AR_ASSERT(own_payload != NULL, "Distribution payload should be created");
    AR_ASSERT(ar_data__set_map_string(own_payload, "action", ref_action),
              "Distribution payload should set action");
    AR_ASSERT(ar_data__set_map_string(own_payload, "text", ref_text),
              "Distribution payload should set text");
    AR_ASSERT(ar_data__set_map_string(own_payload, "kind", ref_kind),
              "Distribution payload should set caller-owned field");
    AR_ASSERT(ar_data__set_map_integer(own_payload, "sender", sender),
              "Distribution payload should set standard sender");
    return own_payload;
}

static ar_data_t *create_payloads(const char **ref_payloads, size_t payload_count, int sender) {
    ar_data_t *own_payloads = ar_data__create_list();
    AR_ASSERT(own_payloads != NULL, "Payload list should be created");
    for (size_t i = 0; i < payload_count; i++) {
        ar_data_t *own_payload = create_payload("work",
                                                ref_payloads[i],
                                                "caller-shaped",
                                                sender);
        AR_ASSERT(ar_data__list_add_last_data(own_payloads, own_payload),
                  "Payload should be appended");
        own_payload = NULL;
    }
    return own_payloads;
}

static ar_data_t *create_workers(const int *ref_workers, size_t worker_count) {
    ar_data_t *own_workers = ar_data__create_list();
    AR_ASSERT(own_workers != NULL, "Worker list should be created");
    for (size_t i = 0; i < worker_count; i++) {
        AR_ASSERT(ar_data__list_add_last_integer(own_workers, ref_workers[i]),
                  "Worker should be appended");
    }
    return own_workers;
}

static void register_worker_recorder(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.received_texts := if(memory.received_count = 0, [], memory.received_texts)\n"
        "memory.append_buffer := memory.received_texts\n"
        "memory.append_ok := append(memory.append_buffer, message.text)\n"
        "memory.received_texts := if(memory.append_ok = 1, memory.append_buffer, memory.received_texts)\n"
        "memory.received_count := memory.received_count + memory.append_ok\n"
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_kind := message.kind\n"
        "memory.last_sender := message.sender\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "worker-recorder",
                                            ref_instructions,
                                            "1.0.0"),
              "worker-recorder should be registered");
    verify_method_parses(mut_methodology, "worker-recorder");
}

static void register_report_recorder(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_message := message\n"
        "memory.last_sender := message.sender\n"
        "memory.last_status := message.status\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_work_id := message.work_id\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "report-recorder",
                                            ref_instructions,
                                            "1.0.0"),
              "report-recorder should be registered");
    verify_method_parses(mut_methodology, "report-recorder");
}

static void assert_text_history(const ar_data_t *ref_memory,
                                const char **ref_expected,
                                size_t expected_count) {
    const ar_data_t *ref_history = ar_data__get_map_data(ref_memory, "received_texts");
    AR_ASSERT(ref_history != NULL, "Worker should record received text history");
    AR_ASSERT(ar_data__list_count(ref_history) == expected_count,
              "Worker should record expected text count");

    ar_data_t **own_items = ar_data__list_items(ref_history);
    AR_ASSERT(own_items != NULL, "Text history items should be available");
    for (size_t i = 0; i < expected_count; i++) {
        const char *ref_text = ar_data__get_string(own_items[i]);
        AR_ASSERT(ref_text != NULL, "Text history item should be a string");
        AR_ASSERT(strcmp(ref_text, ref_expected[i]) == 0,
                  "Text history item should match expected round-robin assignment");
    }
    AR__HEAP__FREE(own_items);
}

static void assert_result_omits_redundant_fields(const ar_data_t *ref_report_memory) {
    const ar_data_t *ref_message = ar_data__get_map_data(ref_report_memory, "last_message");
    AR_ASSERT(ref_message != NULL, "Distribution result message should be recorded");
    AR_ASSERT(ar_data__get_map_data(ref_message, "state") == NULL,
              "Distribution result should omit redundant state");
    AR_ASSERT(ar_data__get_map_data(ref_message, "assignment_count") == NULL,
              "Distribution result should omit redundant assignment count");
    AR_ASSERT(ar_data__get_map_data(ref_message, "sent_count") == NULL,
              "Distribution result should omit redundant sent count");
    AR_ASSERT(ar_data__get_map_data(ref_message, "failed_count") == NULL,
              "Distribution result should omit redundant failed count");
}

static void send_distribution(ar_agency_t *mut_agency,
                              int64_t distribution_agent,
                              const char *ref_work_id,
                              ar_data_t *own_payloads,
                              ar_data_t *own_workers,
                              int sender) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Distribution message should be created");
    ar_data__set_map_string(own_message, "request", "distribution_start");
    ar_data__set_map_string(own_message, "work_id", ref_work_id);
    AR_ASSERT(ar_data__set_map_data(own_message, "payloads", own_payloads),
              "Distribution message should own payloads");
    own_payloads = NULL;
    AR_ASSERT(ar_data__set_map_data(own_message, "workers", own_workers),
              "Distribution message should own workers");
    own_workers = NULL;
    ar_data__set_map_string(own_message, "trace_id", ref_work_id);
    ar_data__set_map_integer(own_message, "sender", sender);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, distribution_agent, own_message),
              "Distribution message should queue");
    own_message = NULL;
}

static void test_distribution__round_robins_payloads_across_workers(void) {
    printf("Testing distribution round-robins payloads across workers...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("distribution_round_robin");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "distribution");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_worker_recorder(mut_agency);
    register_report_recorder(mut_agency);

    ar_data_t *own_distribution_context = create_context();
    ar_data_t *own_worker_a_context = create_worker_context();
    ar_data_t *own_worker_b_context = create_worker_context();
    ar_data_t *own_report_context = create_context();
    int64_t distribution_agent = ar_agency__create_agent(
        mut_agency, "distribution", "1.0.0", own_distribution_context);
    int64_t worker_a = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_a_context);
    int64_t worker_b = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_b_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "report-recorder", "1.0.0", own_report_context);
    initialize_worker_memory(mut_agency, worker_a);
    initialize_worker_memory(mut_agency, worker_b);

    const char *ref_payload_values[] = {"p1", "p2", "p3", "p4", "p5"};
    const int ref_workers[] = {
        checked_agent_id(worker_a),
        checked_agent_id(worker_b)
    };
    ar_data_t *own_payloads = create_payloads(ref_payload_values, 5, checked_agent_id(worker_b));
    ar_data_t *own_workers = create_workers(ref_workers, 2);

    send_distribution(mut_agency,
                      distribution_agent,
                      "job-round-robin",
                      own_payloads,
                      own_workers,
                      checked_agent_id(report_agent));
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_worker_a_memory = ar_agency__get_agent_memory(mut_agency, worker_a);
    const ar_data_t *ref_worker_b_memory = ar_agency__get_agent_memory(mut_agency, worker_b);
    const char *ref_worker_a_expected[] = {"p1", "p3", "p5"};
    const char *ref_worker_b_expected[] = {"p2", "p4"};
    assert_text_history(ref_worker_a_memory, ref_worker_a_expected, 3);
    assert_text_history(ref_worker_b_memory, ref_worker_b_expected, 2);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_a_memory, "last_action"), "work") == 0,
              "Distribution should preserve caller payload action");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_worker_a_memory, "last_kind"),
                     "caller-shaped") == 0,
              "Distribution should preserve caller-owned field");
    AR_ASSERT(ar_data__get_map_integer(ref_worker_a_memory, "last_sender") ==
                  checked_agent_id(worker_b),
              "Distribution should preserve caller payload sender");

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_response"),
                     "distribution_result") == 0,
              "Distribution result should be a response");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_sender") ==
                  checked_agent_id(distribution_agent),
              "Distribution result should identify the distribution sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "success") == 0,
              "Distribution result should report standard success status");
    assert_result_omits_redundant_fields(ref_report_memory);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_work_id"),
                     "job-round-robin") == 0,
              "Distribution result should preserve work id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "job-round-robin") == 0,
              "Distribution result should preserve trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 5,
              "Distribution result should report successful assignment sends");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Distribution result should report no failed assignment sends");

    ar_data_t *own_ignored_message = ar_data__create_map();
    AR_ASSERT(own_ignored_message != NULL, "Ignored message should be created");
    AR_ASSERT(ar_data__set_map_string(own_ignored_message, "request", "distribution_ignored"),
              "Ignored message should set action");
    AR_ASSERT(ar_data__set_map_integer(own_ignored_message, "sender", checked_agent_id(report_agent)),
              "Ignored message should set sender");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, distribution_agent, own_ignored_message),
              "Ignored message should queue");
    own_ignored_message = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_distribution_memory =
        ar_agency__get_agent_memory(mut_agency, distribution_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_distribution_memory, "status"),
                     "success") == 0,
              "Ignored messages should not overwrite distribution status");
    AR_ASSERT(ar_data__get_map_integer(ref_distribution_memory, "assignment_count") == 5,
              "Ignored messages should not reset assignment count");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_work_id"),
                     "job-round-robin") == 0,
              "Ignored messages should not emit a new distribution result");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_distribution_context);
    ar_data__destroy(own_worker_a_context);
    ar_data__destroy(own_worker_b_context);
    ar_data__destroy(own_report_context);
}

static void test_distribution__reports_failed_assignments_and_empty_inputs(void) {
    printf("Testing distribution reports failed assignments and empty inputs...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("distribution_failures");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "distribution");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_worker_recorder(mut_agency);
    register_report_recorder(mut_agency);

    ar_data_t *own_distribution_context = create_context();
    ar_data_t *own_worker_context = create_worker_context();
    ar_data_t *own_report_context = create_context();
    int64_t distribution_agent = ar_agency__create_agent(
        mut_agency, "distribution", "1.0.0", own_distribution_context);
    int64_t worker_agent = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "report-recorder", "1.0.0", own_report_context);
    initialize_worker_memory(mut_agency, worker_agent);

    const char *ref_payload_values[] = {"x1", "x2", "x3"};
    const int ref_workers[] = {
        checked_agent_id(worker_agent),
        98765
    };
    ar_data_t *own_payloads = create_payloads(ref_payload_values, 3, 0);
    ar_data_t *own_workers = create_workers(ref_workers, 2);
    send_distribution(mut_agency,
                      distribution_agent,
                      "job-partial",
                      own_payloads,
                      own_workers,
                      checked_agent_id(report_agent));
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_worker_memory = ar_agency__get_agent_memory(mut_agency, worker_agent);
    const char *ref_worker_expected[] = {"x1", "x3"};
    assert_text_history(ref_worker_memory, ref_worker_expected, 2);

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Partial worker failure should report standard failure status");
    assert_result_omits_redundant_fields(ref_report_memory);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_work_id"),
                     "job-partial") == 0,
              "Partial worker failure should preserve work id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 2,
              "Partial worker failure should report successful assignment sends");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 1,
              "Partial worker failure should report failed assignment sends");

    ar_data_t *own_empty_payloads = ar_data__create_list();
    AR_ASSERT(own_empty_payloads != NULL, "Empty payload list should be created");
    const int ref_single_worker[] = {checked_agent_id(worker_agent)};
    ar_data_t *own_single_worker = create_workers(ref_single_worker, 1);
    send_distribution(mut_agency,
                      distribution_agent,
                      "job-empty-payloads",
                      own_empty_payloads,
                      own_single_worker,
                      checked_agent_id(report_agent));
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Empty payload list should report standard failure status");
    assert_result_omits_redundant_fields(ref_report_memory);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_work_id"),
                     "job-empty-payloads") == 0,
              "Empty payload list should preserve work id");

    const char *ref_one_payload[] = {"orphan"};
    own_payloads = create_payloads(ref_one_payload, 1, 0);
    ar_data_t *own_empty_workers = ar_data__create_list();
    AR_ASSERT(own_empty_workers != NULL, "Empty worker list should be created");
    send_distribution(mut_agency,
                      distribution_agent,
                      "job-empty-workers",
                      own_payloads,
                      own_empty_workers,
                      checked_agent_id(report_agent));
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Empty worker list should report standard failure status");
    assert_result_omits_redundant_fields(ref_report_memory);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_work_id"),
                     "job-empty-workers") == 0,
              "Empty worker list should preserve work id");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_distribution_context);
    ar_data__destroy(own_worker_context);
    ar_data__destroy(own_report_context);
}

static void test_distribution__records_completion_when_response_fails(void) {
    printf("Testing distribution records completion when response fails...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("distribution_response_fails");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "distribution");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_worker_recorder(mut_agency);

    ar_data_t *own_distribution_context = create_context();
    ar_data_t *own_worker_context = create_worker_context();
    int64_t distribution_agent = ar_agency__create_agent(
        mut_agency, "distribution", "1.0.0", own_distribution_context);
    int64_t worker_agent = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_context);
    initialize_worker_memory(mut_agency, worker_agent);

    const char *ref_payload_values[] = {"solo"};
    const int ref_workers[] = {checked_agent_id(worker_agent)};
    ar_data_t *own_payloads = create_payloads(ref_payload_values, 1, 0);
    ar_data_t *own_workers = create_workers(ref_workers, 1);
    send_distribution(mut_agency,
                      distribution_agent,
                      "job-response-fails",
                      own_payloads,
                      own_workers,
                      98765);
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_worker_memory = ar_agency__get_agent_memory(mut_agency, worker_agent);
    const char *ref_worker_expected[] = {"solo"};
    assert_text_history(ref_worker_memory, ref_worker_expected, 1);

    const ar_data_t *ref_distribution_memory =
        ar_agency__get_agent_memory(mut_agency, distribution_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_distribution_memory, "status"),
                     "success") == 0,
              "Distribution should record terminal status when response delivery fails");
    AR_ASSERT(ar_data__get_map_integer(ref_distribution_memory, "assignment_count") == 1,
              "Distribution should record attempted assignment count when response delivery fails");
    AR_ASSERT(ar_data__get_map_integer(ref_distribution_memory, "sent_count") == 1,
              "Distribution should record successful send count when response delivery fails");
    AR_ASSERT(ar_data__get_map_integer(ref_distribution_memory, "failed_count") == 0,
              "Distribution should record failed send count when response delivery fails");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_distribution_context);
    ar_data__destroy(own_worker_context);
}

static void test_distribution__preserves_trace_for_interleaved_jobs(void) {
    printf("Testing distribution preserves trace for interleaved jobs...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("distribution_interleaved");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "distribution");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_worker_recorder(mut_agency);
    register_report_recorder(mut_agency);

    ar_data_t *own_distribution_context = create_context();
    ar_data_t *own_worker_a_context = create_worker_context();
    ar_data_t *own_worker_b_context = create_worker_context();
    ar_data_t *own_report_one_context = create_context();
    ar_data_t *own_report_two_context = create_context();
    int64_t distribution_agent = ar_agency__create_agent(
        mut_agency, "distribution", "1.0.0", own_distribution_context);
    int64_t worker_a = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_a_context);
    int64_t worker_b = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_b_context);
    int64_t report_one = ar_agency__create_agent(
        mut_agency, "report-recorder", "1.0.0", own_report_one_context);
    int64_t report_two = ar_agency__create_agent(
        mut_agency, "report-recorder", "1.0.0", own_report_two_context);
    initialize_worker_memory(mut_agency, worker_a);
    initialize_worker_memory(mut_agency, worker_b);

    const char *ref_job_one_payloads[] = {"one-a", "one-b"};
    const int ref_job_one_workers[] = {
        checked_agent_id(worker_a),
        checked_agent_id(worker_b)
    };
    ar_data_t *own_job_one_payloads = create_payloads(ref_job_one_payloads, 2, 0);
    ar_data_t *own_job_one_workers = create_workers(ref_job_one_workers, 2);
    send_distribution(mut_agency,
                      distribution_agent,
                      "job-one",
                      own_job_one_payloads,
                      own_job_one_workers,
                      checked_agent_id(report_one));

    const char *ref_job_two_payloads[] = {"two-a"};
    const int ref_job_two_workers[] = {checked_agent_id(worker_a)};
    ar_data_t *own_job_two_payloads = create_payloads(ref_job_two_payloads, 1, 0);
    ar_data_t *own_job_two_workers = create_workers(ref_job_two_workers, 1);
    send_distribution(mut_agency,
                      distribution_agent,
                      "job-two",
                      own_job_two_payloads,
                      own_job_two_workers,
                      checked_agent_id(report_two));

    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_report_one_memory = ar_agency__get_agent_memory(mut_agency, report_one);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_one_memory, "last_work_id"),
                     "job-one") == 0,
              "First interleaved result should preserve work id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_one_memory, "last_trace_id"),
                     "job-one") == 0,
              "First interleaved result should preserve its own trace id");
    assert_result_omits_redundant_fields(ref_report_one_memory);

    const ar_data_t *ref_report_two_memory = ar_agency__get_agent_memory(mut_agency, report_two);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_two_memory, "last_work_id"),
                     "job-two") == 0,
              "Second interleaved result should preserve work id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_two_memory, "last_trace_id"),
                     "job-two") == 0,
              "Second interleaved result should preserve its own trace id");
    assert_result_omits_redundant_fields(ref_report_two_memory);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_distribution_context);
    ar_data__destroy(own_worker_a_context);
    ar_data__destroy(own_worker_b_context);
    ar_data__destroy(own_report_one_context);
    ar_data__destroy(own_report_two_context);
}

static void test_distribution__skips_zero_worker_placeholders(void) {
    printf("Testing distribution skips zero worker placeholders...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("distribution_zero_workers");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "distribution");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_worker_recorder(mut_agency);
    register_report_recorder(mut_agency);

    ar_data_t *own_distribution_context = create_context();
    ar_data_t *own_worker_a_context = create_worker_context();
    ar_data_t *own_worker_b_context = create_worker_context();
    ar_data_t *own_report_context = create_context();
    int64_t distribution_agent = ar_agency__create_agent(
        mut_agency, "distribution", "1.0.0", own_distribution_context);
    int64_t worker_a = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_a_context);
    int64_t worker_b = ar_agency__create_agent(
        mut_agency, "worker-recorder", "1.0.0", own_worker_b_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "report-recorder", "1.0.0", own_report_context);
    initialize_worker_memory(mut_agency, worker_a);
    initialize_worker_memory(mut_agency, worker_b);

    const char *ref_payload_values[] = {"z1", "z2", "z3", "z4"};
    const int ref_workers[] = {
        0,
        checked_agent_id(worker_a),
        0,
        checked_agent_id(worker_b),
        0
    };
    ar_data_t *own_payloads = create_payloads(ref_payload_values, 4, 0);
    ar_data_t *own_workers = create_workers(ref_workers, 5);
    send_distribution(mut_agency,
                      distribution_agent,
                      "job-zero-worker",
                      own_payloads,
                      own_workers,
                      checked_agent_id(report_agent));
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_worker_a_memory = ar_agency__get_agent_memory(mut_agency, worker_a);
    const ar_data_t *ref_worker_b_memory = ar_agency__get_agent_memory(mut_agency, worker_b);
    const char *ref_worker_a_expected[] = {"z1", "z3"};
    const char *ref_worker_b_expected[] = {"z2", "z4"};
    assert_text_history(ref_worker_a_memory, ref_worker_a_expected, 2);
    assert_text_history(ref_worker_b_memory, ref_worker_b_expected, 2);

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "success") == 0,
              "Zero worker placeholders should still distribute work");
    assert_result_omits_redundant_fields(ref_report_memory);

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_distribution_context);
    ar_data__destroy(own_worker_a_context);
    ar_data__destroy(own_worker_b_context);
    ar_data__destroy(own_report_context);
}

int main(void) {
    printf("Running distribution method tests...\n\n");
    test_distribution__round_robins_payloads_across_workers();
    test_distribution__reports_failed_assignments_and_empty_inputs();
    test_distribution__records_completion_when_response_fails();
    test_distribution__preserves_trace_for_interleaved_jobs();
    test_distribution__skips_zero_worker_placeholders();
    printf("\nAll distribution method tests passed!\n");
    return 0;
}
