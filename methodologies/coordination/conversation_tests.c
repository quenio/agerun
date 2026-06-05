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
        "memory.last_conversation_id := message.conversation_id\n"
        "memory.last_state := message.state\n"
        "memory.last_text := message.text\n"
        "memory.last_turn_count := message.turn_count\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_conversation__maintains_context_across_messages(void) {
    printf("Testing conversation maintains context across messages...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("conversation_context");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "conversation");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_conversation_context = create_context();
    ar_data_t *own_receiver_context = create_context();
    int64_t conversation_agent = ar_agency__create_agent(
        mut_agency, "conversation", "1.0.0", own_conversation_context);
    int64_t receiver_agent = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_receiver_context);

    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Conversation start should be created");
    ar_data__set_map_string(own_start, "action", "start");
    ar_data__set_map_string(own_start, "conversation_id", "chat-1");
    ar_data__set_map_string(own_start, "user_id", "user-1");
    ar_data__set_map_integer(own_start, "reply_to", checked_agent_id(receiver_agent));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_start),
              "Conversation start should queue");
    own_start = NULL;

    ar_data_t *own_first = ar_data__create_map();
    AR_ASSERT(own_first != NULL, "First conversation message should be created");
    ar_data__set_map_string(own_first, "action", "message");
    ar_data__set_map_string(own_first, "text", "first");
    ar_data__set_map_string(own_first, "intent", "ask");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_first),
              "First conversation message should queue");
    own_first = NULL;

    ar_data_t *own_second = ar_data__create_map();
    AR_ASSERT(own_second != NULL, "Second conversation message should be created");
    ar_data__set_map_string(own_second, "action", "message");
    ar_data__set_map_string(own_second, "text", "second");
    ar_data__set_map_string(own_second, "intent", "followup");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_second),
              "Second conversation message should queue");
    own_second = NULL;

    ar_data_t *own_summary = ar_data__create_map();
    AR_ASSERT(own_summary != NULL, "Summary request should be created");
    ar_data__set_map_string(own_summary, "action", "summary");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_summary),
              "Summary request should queue");
    own_summary = NULL;

    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_receiver_memory = ar_agency__get_agent_memory(mut_agency, receiver_agent);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_action"),
                     "conversation_summary") == 0,
              "Receiver should observe conversation summary");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_conversation_id"),
                     "chat-1") == 0,
              "Summary should preserve conversation id");
    AR_ASSERT(ar_data__get_map_integer(ref_receiver_memory, "last_turn_count") == 2,
              "Summary should report two turns");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_receiver_memory, "last_text"),
                     "conversation=chat-1|user=user-1|state=active|turns=2|last=second|previous=first") == 0,
              "Summary should include recent conversational context");

    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_conversation_context);
    ar_data__destroy(own_receiver_context);
}

int main(void) {
    printf("Running conversation method tests...\n\n");
    test_conversation__maintains_context_across_messages();
    printf("\nAll conversation method tests passed!\n");
    return 0;
}
