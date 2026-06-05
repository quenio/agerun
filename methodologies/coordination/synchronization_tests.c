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
        "memory.last_sync_id := message.sync_id\n"
        "memory.last_done_count := message.done_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_synchronization__emits_continuation_after_dependencies(void) {
    printf("Testing synchronization emits continuation after dependencies...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("synchronization_wait");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "synchronization");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_sync_context = create_context();
    ar_data_t *own_receiver_context = create_context();
    int64_t sync_agent = ar_agency__create_agent(
        mut_agency, "synchronization", "1.0.0", own_sync_context);
    int64_t receiver_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_context);

    ar_data_t *own_wait = ar_data__create_map();
    AR_ASSERT(own_wait != NULL, "Wait message should be created");
    ar_data__set_map_string(own_wait, "action", "wait");
    ar_data__set_map_string(own_wait, "sync_id", "sync-1");
    ar_data__set_map_integer(own_wait, "required_count", 2);
    ar_data__set_map_string(own_wait, "required_a", "ready-a");
    ar_data__set_map_string(own_wait, "required_b", "ready-b");
    ar_data__set_map_string(own_wait, "required_c", "unused");
    ar_data__set_map_integer(own_wait, "reply_to", 0);
    ar_data__set_map_integer(own_wait, "continuation_target", checked_agent_id(receiver_agent));
    ar_data__set_map_string(own_wait, "continuation_action", "continue");
    ar_data__set_map_string(own_wait, "continuation_text", "go");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_wait),
              "Wait message should queue");
    own_wait = NULL;

    ar_data_t *own_first_dependency = ar_data__create_map();
    AR_ASSERT(own_first_dependency != NULL, "First dependency should be created");
    ar_data__set_map_string(own_first_dependency, "action", "dependency");
    ar_data__set_map_string(own_first_dependency, "dependency", "ready-a");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_first_dependency),
              "First dependency should queue");
    own_first_dependency = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    AR_ASSERT(ar_data__get_map_data(ref_receiver_memory, "last_action") == NULL,
              "Receiver should not continue before all dependencies arrive");

    ar_data_t *own_second_dependency = ar_data__create_map();
    AR_ASSERT(own_second_dependency != NULL, "Second dependency should be created");
    ar_data__set_map_string(own_second_dependency, "action", "dependency");
    ar_data__set_map_string(own_second_dependency, "dependency", "ready-b");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, sync_agent, own_second_dependency),
              "Second dependency should queue");
    own_second_dependency = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const char *ref_action = ar_data__get_map_string(ref_receiver_memory, "last_action");
    AR_ASSERT(ref_action != NULL && strcmp(ref_action, "continue") == 0,
              "Receiver should observe continuation");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_text"), "go") == 0,
              "Continuation should include text");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_done_count") == 2,
              "Continuation should report satisfied dependency count");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_sync_context);
    ar_data__destroy(own_receiver_context);
}

int main(void) {
    printf("Running synchronization method tests...\n\n");
    test_synchronization__emits_continuation_after_dependencies();
    printf("\nAll synchronization method tests passed!\n");
    return 0;
}
