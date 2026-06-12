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
        "memory.last_source := message.source\n"
        "memory.last_text := message.text\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_session_id := message.session_id\n"
        "memory.last_attempt := message.attempt\n"
        "memory.last_operation_id := message.operation_id\n"
        "memory.last_status := message.status\n"
        "memory.last_state := message.state\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_attempts := message.attempts\n"
        "memory.last_schedule_id := message.schedule_id\n"
        "memory.last_due_tick := message.due_tick\n"
        "memory.last_target := message.target\n"
        "memory.last_payload_attempt := message.payload_attempt\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_retry__reexecutes_and_reports_success(void) {
    printf("Testing retry reexecutes and reports success...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("retry_success");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "retry");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_retry_context = create_context();
    ar_data_t *own_operation_context = create_context();
    ar_data_t *own_report_context = create_context();
    ar_data_t *own_scheduled_retry_context = create_context();
    ar_data_t *own_scheduled_operation_context = create_context();
    ar_data_t *own_scheduler_context = create_context();
    int64_t retry_agent = ar_agency__create_agent(
        mut_agency, "retry", "1.0.0", own_retry_context);
    int64_t operation_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_operation_context);
    int64_t report_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_report_context);
    int64_t scheduled_retry_agent = ar_agency__create_agent(
        mut_agency, "retry", "1.0.0", own_scheduled_retry_context);
    int64_t scheduled_operation_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_scheduled_operation_context);
    int64_t scheduler_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_scheduler_context);

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Retry start should be created");
    ar_data__set_map_string(own_start, "request", "retry_start");
    ar_data__set_map_string(own_start, "operation_id", "op-1");
    ar_data__set_map_string(own_start, "trace_id", "retry-trace-1");
    ar_data__set_map_string(own_start, "session_id", "op-1");
    ar_data__set_map_integer(own_start, "operation_target", checked_agent_id(operation_agent));
    ar_data__set_map_string(own_start, "operation_request", "attempt");
    ar_data__set_map_string(own_start, "operation_text", "work");
    ar_data__set_map_integer(own_start, "max_attempts", 2);
    ar_data__set_map_string(own_start, "strategy", "immediate");
    ar_data__set_map_integer(own_start, "scheduler_agent", 0);
    ar_data__set_map_integer(own_start, "delay_ticks", 0);
    ar_data__set_map_integer(own_start, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_start),
              "Retry start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_operation_memory = ar_agency__get_agent_memory(mut_agency, operation_agent);
    const ar_data_t *ref_retry_memory = ar_agency__get_agent_memory(mut_agency, retry_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_operation_memory, "last_attempt") == 1,
              "Retry should execute first attempt");
    AR_ASSERT(ar_data__get_map_integer(ref_operation_memory, "last_source") ==
                  checked_agent_id(retry_agent),
              "Operation attempt should identify the retry source");

    ar_data_t *own_failure = ar_data__create_map();
    AR_ASSERT(own_failure != NULL, "Failure message should be created");
    ar_data__set_map_string(own_failure, "request", "retry_failure");
    ar_data__set_map_string(own_failure, "trace_id", "retry-trace-1");
    ar_data__set_map_string(own_failure, "session_id", "op-1");
    ar_data__set_map_integer(own_failure, "attempt", 1);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_failure),
              "Failure message should queue");
    own_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(ar_data__get_map_integer(ref_operation_memory, "last_attempt") == 2,
              "Retry should execute second attempt after failure");

    ar_data_t *own_duplicate_failure = ar_data__create_map();
    AR_ASSERT(own_duplicate_failure != NULL, "Duplicate stale failure should be created");
    ar_data__set_map_string(own_duplicate_failure, "request", "retry_failure");
    ar_data__set_map_string(own_duplicate_failure, "trace_id", "retry-trace-1");
    ar_data__set_map_string(own_duplicate_failure, "session_id", "op-1");
    ar_data__set_map_integer(own_duplicate_failure, "attempt", 1);
    ar_data__set_map_integer(own_duplicate_failure, "current_tick", 49);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_duplicate_failure),
              "Duplicate stale failure should queue");
    own_duplicate_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Duplicate stale failure should leave retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 2,
              "Duplicate stale failure should not consume another attempt");
    AR_ASSERT(ar_data__get_map_integer(ref_operation_memory, "last_attempt") == 2,
              "Duplicate stale failure should not send a third operation attempt");

    ar_data_t *own_success = ar_data__create_map();
    AR_ASSERT(own_success != NULL, "Success message should be created");
    ar_data__set_map_string(own_success, "request", "retry_success");
    ar_data__set_map_string(own_success, "trace_id", "retry-trace-1");
    ar_data__set_map_string(own_success, "session_id", "op-1");
    ar_data__set_map_integer(own_success, "attempt", 2);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_success),
              "Success message should queue");
    own_success = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_report_memory = ar_agency__get_agent_memory(mut_agency, report_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_response"),
                     "retry_result") == 0,
              "Retry terminal report should be a response");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_source") ==
                  checked_agent_id(retry_agent),
              "Retry terminal report should identify the retry source");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"), "success") == 0,
              "Retry terminal success should report standard success status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"), "succeeded") == 0,
              "Retry should report success");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_trace_id"),
                     "retry-trace-1") == 0,
              "Retry result should preserve trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_session_id"),
                     "op-1") == 0,
              "Retry result should preserve session id");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 1,
              "Retry result should report one successful operation");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 0,
              "Retry result should report no terminal failures");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_attempts") == 2,
              "Retry should report final attempt count");

    ar_data_t *own_stale_failure = ar_data__create_map();
    AR_ASSERT(own_stale_failure != NULL, "Stale failure message should be created");
    ar_data__set_map_string(own_stale_failure, "request", "retry_failure");
    ar_data__set_map_string(own_stale_failure, "trace_id", "retry-trace-1");
    ar_data__set_map_string(own_stale_failure, "session_id", "op-1");
    ar_data__set_map_integer(own_stale_failure, "attempt", 1);
    ar_data__set_map_integer(own_stale_failure, "current_tick", 50);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_stale_failure),
              "Stale failure message should queue");
    own_stale_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "success") == 0,
              "Stale failure should not change succeeded retry status");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 2,
              "Stale failure should not increment succeeded retry attempts");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"), "succeeded") == 0,
              "Stale failure should not overwrite success report");
    AR_ASSERT(ar_data__get_map_integer(ref_operation_memory, "last_attempt") == 2,
              "Stale failure should not send another operation attempt after success");

    ar_data_t *own_final_start = ar_data__create_map();
    AR_ASSERT(own_final_start != NULL, "Final failure retry start should be created");
    ar_data__set_map_string(own_final_start, "request", "retry_start");
    ar_data__set_map_string(own_final_start, "operation_id", "op-final");
    ar_data__set_map_string(own_final_start, "trace_id", "op-final-start");
    ar_data__set_map_string(own_final_start, "session_id", "op-final");
    ar_data__set_map_integer(own_final_start,
                             "operation_target",
                             checked_agent_id(operation_agent));
    ar_data__set_map_string(own_final_start, "operation_request", "attempt");
    ar_data__set_map_string(own_final_start, "operation_text", "final-work");
    ar_data__set_map_integer(own_final_start, "max_attempts", 1);
    ar_data__set_map_string(own_final_start, "strategy", "immediate");
    ar_data__set_map_integer(own_final_start, "scheduler_agent", 0);
    ar_data__set_map_integer(own_final_start, "delay_ticks", 0);
    ar_data__set_map_integer(own_final_start, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_final_start),
              "Final failure retry start should queue");
    own_final_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *own_final_failure = ar_data__create_map();
    AR_ASSERT(own_final_failure != NULL, "Final failure message should be created");
    ar_data__set_map_string(own_final_failure, "request", "retry_failure");
    ar_data__set_map_string(own_final_failure, "trace_id", "op-final");
    ar_data__set_map_string(own_final_failure, "session_id", "op-final");
    ar_data__set_map_integer(own_final_failure, "attempt", 1);
    ar_data__set_map_integer(own_final_failure, "current_tick", 60);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_final_failure),
              "Final failure message should queue");
    own_final_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"), "failed") == 0,
              "Retry should report final failure");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Retry final failure should report standard failure status");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_attempts") == 1,
              "Final failure should report one attempt");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_success_count") == 0,
              "Final failure should report no successful operation");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_failure_count") == 1,
              "Final failure should report one failed operation");

    own_stale_failure = ar_data__create_map();
    AR_ASSERT(own_stale_failure != NULL, "Stale final failure message should be created");
    ar_data__set_map_string(own_stale_failure, "request", "retry_failure");
    ar_data__set_map_string(own_stale_failure, "trace_id", "op-final");
    ar_data__set_map_string(own_stale_failure, "session_id", "op-final");
    ar_data__set_map_integer(own_stale_failure, "attempt", 1);
    ar_data__set_map_integer(own_stale_failure, "current_tick", 61);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_stale_failure),
              "Stale final failure message should queue");
    own_stale_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "failure") == 0,
              "Stale failure should not change failed retry status");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 1,
              "Stale failure should not increment failed retry attempts");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"), "failed") == 0,
              "Stale failure should not overwrite failure report");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_attempts") == 1,
              "Stale failure should preserve failure attempt count");

    ar_data_t *own_scheduled_start = ar_data__create_map();
    AR_ASSERT(own_scheduled_start != NULL, "Scheduled retry start should be created");
    ar_data__set_map_string(own_scheduled_start, "request", "retry_start");
    ar_data__set_map_string(own_scheduled_start, "operation_id", "op-scheduled");
    ar_data__set_map_string(own_scheduled_start, "trace_id", "op-scheduled-start");
    ar_data__set_map_string(own_scheduled_start, "session_id", "op-scheduled");
    ar_data__set_map_integer(own_scheduled_start,
                             "operation_target",
                             checked_agent_id(scheduled_operation_agent));
    ar_data__set_map_string(own_scheduled_start, "operation_request", "attempt");
    ar_data__set_map_string(own_scheduled_start, "operation_text", "scheduled-work");
    ar_data__set_map_integer(own_scheduled_start, "max_attempts", 2);
    ar_data__set_map_string(own_scheduled_start, "strategy", "scheduled");
    ar_data__set_map_integer(own_scheduled_start, "scheduler_agent", checked_agent_id(scheduler_agent));
    ar_data__set_map_integer(own_scheduled_start, "delay_ticks", 3);
    ar_data__set_map_integer(own_scheduled_start, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduled_retry_agent, own_scheduled_start),
              "Scheduled retry start should queue");
    own_scheduled_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_scheduled_operation_memory =
        ar_agency__get_agent_memory(mut_agency, scheduled_operation_agent);
    AR_ASSERT(ar_data__get_map_integer(ref_scheduled_operation_memory, "last_attempt") == 1,
              "Scheduled retry should execute first attempt");

    ar_data_t *own_scheduled_failure = ar_data__create_map();
    AR_ASSERT(own_scheduled_failure != NULL, "Scheduled failure message should be created");
    ar_data__set_map_string(own_scheduled_failure, "request", "retry_failure");
    ar_data__set_map_string(own_scheduled_failure, "trace_id", "op-scheduled");
    ar_data__set_map_string(own_scheduled_failure, "session_id", "op-scheduled");
    ar_data__set_map_integer(own_scheduled_failure, "attempt", 1);
    ar_data__set_map_integer(own_scheduled_failure, "current_tick", 10);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduled_retry_agent, own_scheduled_failure),
              "Scheduled failure message should queue");
    own_scheduled_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_scheduler_memory = ar_agency__get_agent_memory(mut_agency, scheduler_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_scheduler_memory, "last_request"),
                     "scheduling_schedule") == 0,
              "Scheduled retry should hand off to scheduling");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_scheduler_memory, "last_schedule_id"),
                     "op-scheduled") == 0,
              "Scheduled retry should preserve operation id as schedule id");
    AR_ASSERT(ar_data__get_map_integer(ref_scheduler_memory, "last_due_tick") == 13,
              "Scheduled retry should add delay to the failure tick");
    AR_ASSERT(ar_data__get_map_integer(ref_scheduler_memory, "last_target") ==
                  checked_agent_id(scheduled_operation_agent),
              "Scheduled retry should target the operation agent");
    AR_ASSERT(ar_data__get_map_integer(ref_scheduler_memory, "last_source") ==
                  checked_agent_id(scheduled_retry_agent),
              "Scheduled retry request should identify the retry source");
    AR_ASSERT(ar_data__get_map_integer(ref_scheduler_memory, "last_payload_attempt") == 2,
              "Scheduled retry should preserve the next attempt number");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_scheduler_memory, "last_session_id"),
                     "op-scheduled") == 0,
              "Scheduled retry should preserve operation session id");

    ar_data_t *own_failed_start = ar_data__create_map();
    AR_ASSERT(own_failed_start != NULL, "Failed start dispatch should be created");
    ar_data__set_map_string(own_failed_start, "request", "retry_start");
    ar_data__set_map_string(own_failed_start, "operation_id", "op-start-dispatch-failed");
    ar_data__set_map_string(own_failed_start, "trace_id", "op-start-dispatch-failed-start");
    ar_data__set_map_string(own_failed_start, "session_id", "op-start-dispatch-failed");
    ar_data__set_map_integer(own_failed_start, "operation_target", 98765);
    ar_data__set_map_string(own_failed_start, "operation_request", "attempt");
    ar_data__set_map_string(own_failed_start, "operation_text", "missing-worker");
    ar_data__set_map_integer(own_failed_start, "max_attempts", 2);
    ar_data__set_map_string(own_failed_start, "strategy", "immediate");
    ar_data__set_map_integer(own_failed_start, "scheduler_agent", 0);
    ar_data__set_map_integer(own_failed_start, "delay_ticks", 0);
    ar_data__set_map_integer(own_failed_start, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_failed_start),
              "Failed start dispatch should queue");
    own_failed_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"),
                     "failure") == 0,
              "Failed initial dispatch should record dispatch_failed after reporting");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 0,
              "Failed initial dispatch should not record an attempt");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_state"),
                     "dispatch_failed") == 0,
              "Failed initial dispatch should report dispatch_failed");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_status"),
                     "failure") == 0,
              "Failed initial dispatch should report standard failure status");
    AR_ASSERT(ar_data__get_map_integer(ref_report_memory, "last_attempts") == 0,
              "Failed initial dispatch should report zero attempts");

    ar_data_t *own_dispatch_failure = ar_data__create_map();
    AR_ASSERT(own_dispatch_failure != NULL, "Failed start synthetic failure should be created");
    ar_data__set_map_string(own_dispatch_failure, "request", "retry_failure");
    ar_data__set_map_string(own_dispatch_failure,
                            "trace_id",
                            "op-start-dispatch-failed");
    ar_data__set_map_string(own_dispatch_failure,
                            "session_id",
                            "op-start-dispatch-failed");
    ar_data__set_map_integer(own_dispatch_failure, "attempt", 1);
    ar_data__set_map_integer(own_dispatch_failure, "current_tick", 70);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_dispatch_failure),
              "Failed start synthetic failure should queue");
    own_dispatch_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"),
                     "failure") == 0,
              "Synthetic failure after failed start should not reactivate retry");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 0,
              "Synthetic failure after failed start should not consume an attempt");

    ar_data_t *own_correlated_start = ar_data__create_map();
    AR_ASSERT(own_correlated_start != NULL, "Correlated retry start should be created");
    ar_data__set_map_string(own_correlated_start, "request", "retry_start");
    ar_data__set_map_string(own_correlated_start, "operation_id", "op-correlated");
    ar_data__set_map_string(own_correlated_start, "trace_id", "op-correlated");
    ar_data__set_map_string(own_correlated_start, "session_id", "op-correlated");
    ar_data__set_map_integer(own_correlated_start,
                             "operation_target",
                             checked_agent_id(operation_agent));
    ar_data__set_map_string(own_correlated_start, "operation_request", "attempt");
    ar_data__set_map_string(own_correlated_start, "operation_text", "correlated-work");
    ar_data__set_map_integer(own_correlated_start, "max_attempts", 2);
    ar_data__set_map_string(own_correlated_start, "strategy", "immediate");
    ar_data__set_map_integer(own_correlated_start, "scheduler_agent", 0);
    ar_data__set_map_integer(own_correlated_start, "delay_ticks", 0);
    ar_data__set_map_integer(own_correlated_start, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_correlated_start),
              "Correlated retry start should queue");
    own_correlated_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_operation_memory, "last_trace_id"),
                     "op-correlated") == 0,
              "Correlated retry should execute first active operation attempt");

    ar_data_t *own_stale_active_failure = ar_data__create_map();
    AR_ASSERT(own_stale_active_failure != NULL, "Stale active failure should be created");
    ar_data__set_map_string(own_stale_active_failure, "request", "retry_failure");
    ar_data__set_map_string(own_stale_active_failure,
                            "trace_id",
                            "op-start-dispatch-failed");
    ar_data__set_map_string(own_stale_active_failure,
                            "session_id",
                            "op-start-dispatch-failed");
    ar_data__set_map_integer(own_stale_active_failure, "attempt", 1);
    ar_data__set_map_integer(own_stale_active_failure, "current_tick", 90);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_stale_active_failure),
              "Stale active failure should queue");
    own_stale_active_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Stale failure for previous operation should leave new retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 1,
              "Stale failure for previous operation should not increment new retry attempts");
    AR_ASSERT(ar_data__get_map_integer(ref_operation_memory, "last_attempt") == 1,
              "Stale failure for previous operation should not dispatch a new attempt");

    ar_data_t *own_stale_active_success = ar_data__create_map();
    AR_ASSERT(own_stale_active_success != NULL, "Stale active success should be created");
    ar_data__set_map_string(own_stale_active_success, "request", "retry_success");
    ar_data__set_map_string(own_stale_active_success,
                            "trace_id",
                            "op-start-dispatch-failed");
    ar_data__set_map_string(own_stale_active_success,
                            "session_id",
                            "op-start-dispatch-failed");
    ar_data__set_map_integer(own_stale_active_success, "attempt", 1);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_stale_active_success),
              "Stale active success should queue");
    own_stale_active_success = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Stale success for previous operation should leave new retry active");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_report_memory, "last_operation_id"),
                     "op-start-dispatch-failed") == 0,
              "Stale success for previous operation should not report the new retry");

    ar_data_t *own_failed_success_report_start = ar_data__create_map();
    AR_ASSERT(own_failed_success_report_start != NULL,
              "Failed success report start should be created");
    ar_data__set_map_string(own_failed_success_report_start, "request", "retry_start");
    ar_data__set_map_string(own_failed_success_report_start,
                            "operation_id",
                            "op-success-report-failed");
    ar_data__set_map_string(own_failed_success_report_start,
                            "trace_id",
                            "op-success-report-failed-start");
    ar_data__set_map_string(own_failed_success_report_start,
                            "session_id",
                            "op-success-report-failed");
    ar_data__set_map_integer(own_failed_success_report_start,
                             "operation_target",
                             checked_agent_id(operation_agent));
    ar_data__set_map_string(own_failed_success_report_start, "operation_request", "attempt");
    ar_data__set_map_string(own_failed_success_report_start, "operation_text", "success-report");
    ar_data__set_map_integer(own_failed_success_report_start, "max_attempts", 2);
    ar_data__set_map_string(own_failed_success_report_start, "strategy", "immediate");
    ar_data__set_map_integer(own_failed_success_report_start, "scheduler_agent", 0);
    ar_data__set_map_integer(own_failed_success_report_start, "delay_ticks", 0);
    ar_data__set_map_integer(own_failed_success_report_start, "source", 98765);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       retry_agent,
                                       own_failed_success_report_start),
              "Failed success report start should queue");
    own_failed_success_report_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *own_failed_success_report = ar_data__create_map();
    AR_ASSERT(own_failed_success_report != NULL, "Failed success report should be created");
    ar_data__set_map_string(own_failed_success_report, "request", "retry_success");
    ar_data__set_map_string(own_failed_success_report,
                            "trace_id",
                            "op-success-report-failed");
    ar_data__set_map_string(own_failed_success_report,
                            "session_id",
                            "op-success-report-failed");
    ar_data__set_map_integer(own_failed_success_report, "attempt", 1);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_failed_success_report),
              "Failed success report should queue");
    own_failed_success_report = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Failed success report delivery should leave retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 1,
              "Failed success report delivery should preserve attempt count");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "pending_terminal_status"),
                     "succeeded") == 0,
              "Failed success report should preserve pending terminal success");

    ar_data_t *own_opposite_failure = ar_data__create_map();
    AR_ASSERT(own_opposite_failure != NULL, "Opposite failure after success should be created");
    ar_data__set_map_string(own_opposite_failure, "request", "retry_failure");
    ar_data__set_map_string(own_opposite_failure,
                            "trace_id",
                            "op-success-report-failed");
    ar_data__set_map_string(own_opposite_failure,
                            "session_id",
                            "op-success-report-failed");
    ar_data__set_map_integer(own_opposite_failure, "attempt", 1);
    ar_data__set_map_integer(own_opposite_failure, "current_tick", 95);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_opposite_failure),
              "Opposite failure after success should queue");
    own_opposite_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Opposite failure during pending success report should leave retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 1,
              "Opposite failure during pending success report should preserve attempt count");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "pending_terminal_status"),
                     "succeeded") == 0,
              "Opposite failure should not replace pending terminal success");
    AR_ASSERT(ar_data__get_map_integer(ref_operation_memory, "last_attempt") == 1,
              "Opposite failure during pending success report should not dispatch a retry");

    own_failed_success_report = ar_data__create_map();
    AR_ASSERT(own_failed_success_report != NULL, "Repeated failed success report should be created");
    ar_data__set_map_string(own_failed_success_report, "request", "retry_success");
    ar_data__set_map_string(own_failed_success_report,
                            "trace_id",
                            "op-success-report-failed");
    ar_data__set_map_string(own_failed_success_report,
                            "session_id",
                            "op-success-report-failed");
    ar_data__set_map_integer(own_failed_success_report, "attempt", 1);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_failed_success_report),
              "Repeated failed success report should queue");
    own_failed_success_report = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Repeated failed success report delivery should leave retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 1,
              "Repeated failed success report delivery should preserve attempt count");

    ar_data_t *own_failed_failure_report_start = ar_data__create_map();
    AR_ASSERT(own_failed_failure_report_start != NULL,
              "Failed final failure report start should be created");
    ar_data__set_map_string(own_failed_failure_report_start, "request", "retry_start");
    ar_data__set_map_string(own_failed_failure_report_start,
                            "operation_id",
                            "op-failure-report-failed");
    ar_data__set_map_string(own_failed_failure_report_start,
                            "trace_id",
                            "op-failure-report-failed-start");
    ar_data__set_map_string(own_failed_failure_report_start,
                            "session_id",
                            "op-failure-report-failed");
    ar_data__set_map_integer(own_failed_failure_report_start,
                             "operation_target",
                             checked_agent_id(operation_agent));
    ar_data__set_map_string(own_failed_failure_report_start, "operation_request", "attempt");
    ar_data__set_map_string(own_failed_failure_report_start, "operation_text", "failure-report");
    ar_data__set_map_integer(own_failed_failure_report_start, "max_attempts", 1);
    ar_data__set_map_string(own_failed_failure_report_start, "strategy", "immediate");
    ar_data__set_map_integer(own_failed_failure_report_start, "scheduler_agent", 0);
    ar_data__set_map_integer(own_failed_failure_report_start, "delay_ticks", 0);
    ar_data__set_map_integer(own_failed_failure_report_start, "source", 98765);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency,
                                       retry_agent,
                                       own_failed_failure_report_start),
              "Failed final failure report start should queue");
    own_failed_failure_report_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *own_failed_failure_report = ar_data__create_map();
    AR_ASSERT(own_failed_failure_report != NULL, "Failed final failure report should be created");
    ar_data__set_map_string(own_failed_failure_report, "request", "retry_failure");
    ar_data__set_map_string(own_failed_failure_report,
                            "trace_id",
                            "op-failure-report-failed");
    ar_data__set_map_string(own_failed_failure_report,
                            "session_id",
                            "op-failure-report-failed");
    ar_data__set_map_integer(own_failed_failure_report, "attempt", 1);
    ar_data__set_map_integer(own_failed_failure_report, "current_tick", 100);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_failed_failure_report),
              "Failed final failure report should queue");
    own_failed_failure_report = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Failed final failure report delivery should leave retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 1,
              "Failed final failure report delivery should preserve attempt count");

    own_failed_failure_report = ar_data__create_map();
    AR_ASSERT(own_failed_failure_report != NULL,
              "Repeated failed final failure report should be created");
    ar_data__set_map_string(own_failed_failure_report, "request", "retry_failure");
    ar_data__set_map_string(own_failed_failure_report,
                            "trace_id",
                            "op-failure-report-failed");
    ar_data__set_map_string(own_failed_failure_report,
                            "session_id",
                            "op-failure-report-failed");
    ar_data__set_map_integer(own_failed_failure_report, "attempt", 1);
    ar_data__set_map_integer(own_failed_failure_report, "current_tick", 101);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, retry_agent, own_failed_failure_report),
              "Repeated failed final failure report should queue");
    own_failed_failure_report = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_retry_memory, "status"), "active") == 0,
              "Repeated failed final failure report delivery should leave retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_retry_memory, "attempts") == 1,
              "Repeated failed final failure report delivery should preserve attempt count");

    ar_data_t *own_failed_schedule_start = ar_data__create_map();
    AR_ASSERT(own_failed_schedule_start != NULL, "Failed schedule start should be created");
    ar_data__set_map_string(own_failed_schedule_start, "request", "retry_start");
    ar_data__set_map_string(own_failed_schedule_start, "operation_id", "op-schedule-failed");
    ar_data__set_map_string(own_failed_schedule_start,
                            "trace_id",
                            "op-schedule-failed-start");
    ar_data__set_map_string(own_failed_schedule_start,
                            "session_id",
                            "op-schedule-failed");
    ar_data__set_map_integer(own_failed_schedule_start,
                             "operation_target",
                             checked_agent_id(scheduled_operation_agent));
    ar_data__set_map_string(own_failed_schedule_start, "operation_request", "attempt");
    ar_data__set_map_string(own_failed_schedule_start, "operation_text", "schedule-missing");
    ar_data__set_map_integer(own_failed_schedule_start, "max_attempts", 2);
    ar_data__set_map_string(own_failed_schedule_start, "strategy", "scheduled");
    ar_data__set_map_integer(own_failed_schedule_start, "scheduler_agent", 98765);
    ar_data__set_map_integer(own_failed_schedule_start, "delay_ticks", 3);
    ar_data__set_map_integer(own_failed_schedule_start, "source", checked_agent_id(report_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduled_retry_agent, own_failed_schedule_start),
              "Failed schedule start should queue");
    own_failed_schedule_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *own_schedule_failure = ar_data__create_map();
    AR_ASSERT(own_schedule_failure != NULL, "Failed schedule failure should be created");
    ar_data__set_map_string(own_schedule_failure, "request", "retry_failure");
    ar_data__set_map_string(own_schedule_failure, "trace_id", "op-schedule-failed");
    ar_data__set_map_string(own_schedule_failure, "session_id", "op-schedule-failed");
    ar_data__set_map_integer(own_schedule_failure, "attempt", 1);
    ar_data__set_map_integer(own_schedule_failure, "current_tick", 80);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduled_retry_agent, own_schedule_failure),
              "Failed schedule failure should queue");
    own_schedule_failure = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_scheduled_retry_memory =
        ar_agency__get_agent_memory(mut_agency, scheduled_retry_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_scheduled_retry_memory, "status"), "active") == 0,
              "Failed scheduled retry dispatch should leave retry active");
    AR_ASSERT(ar_data__get_map_integer(ref_scheduled_retry_memory, "attempts") == 1,
              "Failed scheduled retry dispatch should not consume an attempt");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_retry_context);
    ar_data__destroy(own_operation_context);
    ar_data__destroy(own_report_context);
    ar_data__destroy(own_scheduled_retry_context);
    ar_data__destroy(own_scheduled_operation_context);
    ar_data__destroy(own_scheduler_context);
}

int main(void) {
    printf("Running retry method tests...\n\n");
    test_retry__reexecutes_and_reports_success();
    printf("\nAll retry method tests passed!\n");
    return 0;
}
