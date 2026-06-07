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
        "memory.last_attempt := message.attempt\n"
        "memory.last_correlation_id := message.correlation_id\n"
        "memory.last_schedule_id := message.schedule_id\n"
        "memory.last_status := message.status\n"
        "memory.last_pending := message.pending\n"
        "memory.last_current_tick := message.current_tick\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_scheduling__triggers_future_work_on_tick(void) {
    printf("Testing scheduling triggers future work on tick...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("scheduling_trigger");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "scheduling");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_scheduling_context = create_context();
    ar_data_t *own_receiver_context = create_context();
    ar_data_t *own_observer_context = create_context();
    int64_t scheduling_agent = ar_agency__create_agent(
        mut_agency, "scheduling", "1.0.0", own_scheduling_context);
    int64_t receiver_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_context);
    int64_t observer_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_observer_context);

    ar_data_t *own_schedule = ar_data__create_map();
    AR_ASSERT(own_schedule != NULL, "Schedule message should be created");
    ar_data__set_map_string(own_schedule, "action", "schedule");
    ar_data__set_map_string(own_schedule, "schedule_id", "sched-1");
    ar_data__set_map_integer(own_schedule, "due_tick", 5);
    ar_data__set_map_integer(own_schedule, "target", checked_agent_id(receiver_agent));
    ar_data__set_map_string(own_schedule, "payload_action", "execute");
    ar_data__set_map_string(own_schedule, "payload_text", "delayed");
    ar_data__set_map_integer(own_schedule, "payload_attempt", 7);
    ar_data__set_map_string(own_schedule, "correlation_id", "job-1");
    ar_data__set_map_integer(own_schedule, "reply_to", checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduling_agent, own_schedule),
              "Schedule message should queue");
    own_schedule = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "scheduled") == 0,
              "Observer should receive scheduled status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_current_tick") == 0,
              "Scheduled status should report initial tick");

    ar_data_t *own_early_tick = ar_data__create_map();
    AR_ASSERT(own_early_tick != NULL, "Early tick should be created");
    ar_data__set_map_string(own_early_tick, "action", "tick");
    ar_data__set_map_integer(own_early_tick, "tick", 4);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduling_agent, own_early_tick),
              "Early tick should queue");
    own_early_tick = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    AR_ASSERT(ar_data__get_map_data(ref_receiver_memory, "last_action") == NULL,
              "Receiver should not run before the due tick");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_current_tick") == 0,
              "Observer should not receive status for a pending early tick");

    ar_data_t *own_due_tick = ar_data__create_map();
    AR_ASSERT(own_due_tick != NULL, "Due tick should be created");
    ar_data__set_map_string(own_due_tick, "action", "tick");
    ar_data__set_map_integer(own_due_tick, "tick", 5);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduling_agent, own_due_tick),
              "Due tick should queue");
    own_due_tick = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const char *ref_action = ar_data__get_map_string(ref_receiver_memory, "last_action");
    AR_ASSERT(ref_action != NULL && strcmp(ref_action, "execute") == 0,
              "Receiver should observe scheduled execution");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_text"), "delayed") == 0,
              "Receiver should observe scheduled text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_schedule_id"), "sched-1") == 0,
              "Receiver should observe schedule id");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_attempt") == 7,
              "Receiver should observe scheduled attempt");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "triggered") == 0,
              "Observer should receive triggered status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_current_tick") == 5,
              "Triggered status should report due tick");

    ar_data_t *own_late_cancel = ar_data__create_map();
    AR_ASSERT(own_late_cancel != NULL, "Late cancel should be created");
    ar_data__set_map_string(own_late_cancel, "action", "cancel");
    ar_data__set_map_string(own_late_cancel, "schedule_id", "sched-1");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduling_agent, own_late_cancel),
              "Late cancel should queue");
    own_late_cancel = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_scheduling_memory =
        ar_agency__get_agent_memory(mut_agency, scheduling_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "triggered") == 0,
              "Late cancel should not overwrite triggered observer status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_scheduling_memory, "status"),
                     "triggered") == 0,
              "Late cancel should not overwrite triggered scheduler status");
    AR_ASSERT(ar_data__get_map_integer(ref_scheduling_memory, "pending") == 0,
              "Late cancel should leave triggered schedule non-pending");

    ar_data_t *own_failed_schedule = ar_data__create_map();
    AR_ASSERT(own_failed_schedule != NULL, "Failed schedule message should be created");
    ar_data__set_map_string(own_failed_schedule, "action", "schedule");
    ar_data__set_map_string(own_failed_schedule, "schedule_id", "sched-failed");
    ar_data__set_map_integer(own_failed_schedule, "due_tick", 8);
    ar_data__set_map_integer(own_failed_schedule, "target", 98765);
    ar_data__set_map_string(own_failed_schedule, "payload_action", "execute");
    ar_data__set_map_string(own_failed_schedule, "payload_text", "lost");
    ar_data__set_map_string(own_failed_schedule, "correlation_id", "job-failed");
    ar_data__set_map_integer(own_failed_schedule, "reply_to", checked_agent_id(observer_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduling_agent, own_failed_schedule),
              "Failed schedule message should queue");
    own_failed_schedule = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *own_failed_tick = ar_data__create_map();
    AR_ASSERT(own_failed_tick != NULL, "Failed trigger tick should be created");
    ar_data__set_map_string(own_failed_tick, "action", "tick");
    ar_data__set_map_integer(own_failed_tick, "tick", 8);
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, scheduling_agent, own_failed_tick),
              "Failed trigger tick should queue");
    own_failed_tick = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "trigger_failed") == 0,
              "Observer should receive failed trigger status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_pending") == 1,
              "Failed trigger should remain pending");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_current_tick") == 8,
              "Failed trigger status should report due tick");
    AR_ASSERT(ar_data__get_map_integer(ref_scheduling_memory, "pending") == 1,
              "Scheduler memory should keep failed trigger pending");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_scheduling_context);
    ar_data__destroy(own_receiver_context);
    ar_data__destroy(own_observer_context);
}

int main(void) {
    printf("Running scheduling method tests...\n\n");
    test_scheduling__triggers_future_work_on_tick();
    printf("\nAll scheduling method tests passed!\n");
    return 0;
}
