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
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_conversation_id := message.conversation_id\n"
        "memory.last_from := message.from\n"
        "memory.last_to := message.to\n"
        "memory.last_state := message.state\n"
        "memory.last_status := message.status\n"
        "memory.last_result := message.result\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_text := message.text\n"
        "memory.last_intent := message.intent\n"
        "memory.last_turn_count := message.turn_count\n"
        "memory.last_history := message.history\n"
        "memory.last_last_sender := message.last_sender\n"
        "memory.last_last_recipient := message.last_recipient\n"
        "memory.last_last_text := message.last_text\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_conversation__coordinates_two_participant_agents(void) {
    printf("Testing conversation coordinates two participant agents...\n");

    // Given a conversation coordinator, two participant agents, and a response observer
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("conversation_two_agents");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "conversation");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_conversation_context = create_context();
    ar_data_t *own_participant_a_context = create_context();
    ar_data_t *own_participant_b_context = create_context();
    ar_data_t *own_observer_context = create_context();
    int64_t conversation_agent = ar_agency__create_agent(
        mut_agency, "conversation", "1.0.0", own_conversation_context);
    int64_t participant_a = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_participant_a_context);
    int64_t participant_b = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_participant_b_context);
    int64_t observer = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_observer_context);

    // When the coordinator starts a conversation and relays one turn from each participant
    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Conversation start should be created");
    ar_data__set_map_string(own_start, "request", "conversation_start");
    ar_data__set_map_string(own_start, "conversation_id", "chat-1");
    ar_data__set_map_string(own_start, "trace_id", "chat-trace-1");
    ar_data__set_map_integer(own_start, "participant_a", checked_agent_id(participant_a));
    ar_data__set_map_integer(own_start, "participant_b", checked_agent_id(participant_b));
    ar_data__set_map_integer(own_start, "source_agent", checked_agent_id(observer));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_start),
              "Conversation start should queue");
    own_start = NULL;

    ar_data_t *own_first = ar_data__create_map();
    AR_ASSERT(own_first != NULL, "First conversation message should be created");
    ar_data__set_map_string(own_first, "request", "conversation_message");
    ar_data__set_map_string(own_first, "conversation_id", "chat-1");
    ar_data__set_map_string(own_first, "trace_id", "chat-turn-1");
    ar_data__set_map_integer(own_first, "sender", checked_agent_id(participant_a));
    ar_data__set_map_string(own_first, "text", "hello");
    ar_data__set_map_string(own_first, "intent", "ask");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_first),
              "First conversation message should queue");
    own_first = NULL;

    ar_data_t *own_second = ar_data__create_map();
    AR_ASSERT(own_second != NULL, "Second conversation message should be created");
    ar_data__set_map_string(own_second, "request", "conversation_message");
    ar_data__set_map_string(own_second, "conversation_id", "chat-1");
    ar_data__set_map_string(own_second, "trace_id", "chat-turn-2");
    ar_data__set_map_integer(own_second, "sender", checked_agent_id(participant_b));
    ar_data__set_map_string(own_second, "text", "reply");
    ar_data__set_map_string(own_second, "intent", "answer");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_second),
              "Second conversation message should queue");
    own_second = NULL;

    ar_data_t *own_summary = ar_data__create_map();
    AR_ASSERT(own_summary != NULL, "Summary request should be created");
    ar_data__set_map_string(own_summary, "request", "conversation_summary");
    ar_data__set_map_string(own_summary, "conversation_id", "chat-1");
    ar_data__set_map_string(own_summary, "trace_id", "chat-summary-1");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_summary),
              "Summary request should queue");
    own_summary = NULL;

    ar_method_fixture__process_all_messages(own_fixture);

    // Then participant B should receive participant A's first turn
    const ar_data_t *ref_participant_b_memory = ar_agency__get_agent_memory(mut_agency,
                                                                           participant_b);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_request"),
                     "conversation_turn") == 0,
              "Participant B should receive a conversation turn");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_b_memory, "last_from") ==
                  checked_agent_id(participant_a),
              "Participant B should see participant A as sender");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_b_memory, "last_to") ==
                  checked_agent_id(participant_b),
              "Participant B should be the first turn recipient");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_text"),
                     "hello") == 0,
              "Participant B should receive participant A's text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_trace_id"),
                     "chat-turn-1") == 0,
              "Participant B turn should preserve first message trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_b_memory, "last_turn_count") == 1,
              "Participant B should receive turn one");

    // Then participant A should receive participant B's reply turn
    const ar_data_t *ref_participant_a_memory = ar_agency__get_agent_memory(mut_agency,
                                                                           participant_a);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_a_memory, "last_request"),
                     "conversation_turn") == 0,
              "Participant A should receive a conversation turn");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_a_memory, "last_from") ==
                  checked_agent_id(participant_b),
              "Participant A should see participant B as sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_a_memory, "last_text"),
                     "reply") == 0,
              "Participant A should receive participant B's text");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_a_memory, "last_trace_id"),
                     "chat-turn-2") == 0,
              "Participant A turn should preserve second message trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_a_memory, "last_turn_count") == 2,
              "Participant A should receive turn two");

    // Then the observer should receive a structured summary with both turns
    const ar_data_t *ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_response"),
                     "conversation_result") == 0,
              "Conversation summary should be a response");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "success") == 0,
              "Conversation summary should report standard success status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_conversation_id"),
                     "chat-1") == 0,
              "Summary should preserve conversation id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_turn_count") == 2,
              "Summary should report two turns");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "chat-summary-1") == 0,
              "Conversation summary should preserve summary request trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_success_count") == 2,
              "Conversation summary should report successful turn count");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_failure_count") == 0,
              "Conversation summary should report no failed turns");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_last_sender") ==
                  checked_agent_id(participant_b),
              "Summary should report participant B as the last sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_last_text"),
                     "reply") == 0,
              "Summary should report the last text");
    ar_data_t *ref_history = ar_data__get_map_data(ref_observer_memory, "last_history");
    AR_ASSERT(ref_history != NULL, "Summary should include history");
    AR_ASSERT(ar_data__list_count(ref_history) == 2, "Summary history should include both turns");

    // When a participant message cannot be delivered
    ar_data_t *own_failed_start = ar_data__create_map();
    AR_ASSERT(own_failed_start != NULL, "Failed relay conversation start should be created");
    ar_data__set_map_string(own_failed_start, "request", "conversation_start");
    ar_data__set_map_string(own_failed_start, "conversation_id", "chat-2");
    ar_data__set_map_integer(own_failed_start, "participant_a", checked_agent_id(participant_a));
    ar_data__set_map_integer(own_failed_start, "participant_b", 98765);
    ar_data__set_map_integer(own_failed_start, "source_agent", checked_agent_id(observer));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_failed_start),
              "Failed relay start should queue");
    own_failed_start = NULL;

    ar_data_t *own_failed_turn = ar_data__create_map();
    AR_ASSERT(own_failed_turn != NULL, "Failed relay message should be created");
    ar_data__set_map_string(own_failed_turn, "request", "conversation_message");
    ar_data__set_map_string(own_failed_turn, "conversation_id", "chat-2");
    ar_data__set_map_string(own_failed_turn, "trace_id", "chat-failed-turn");
    ar_data__set_map_integer(own_failed_turn, "sender", checked_agent_id(participant_a));
    ar_data__set_map_string(own_failed_turn, "text", "undeliverable");
    ar_data__set_map_string(own_failed_turn, "intent", "notify");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_failed_turn),
              "Failed relay message should queue");
    own_failed_turn = NULL;

    ar_method_fixture__process_all_messages(own_fixture);

    // Then the coordinator should report the failed relay without recording the turn
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_response"),
                     "conversation_result") == 0,
              "Failed relay response should be a response");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"),
                     "failure") == 0,
              "Failed relay response should report standard failure status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_result"),
                     "relay_failed") == 0,
              "Observer should see relay failure status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "chat-failed-turn") == 0,
              "Failed relay response should preserve message request trace id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_success_count") == 0,
              "Failed relay should report no successful turn");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_failure_count") == 1,
              "Failed relay should report one failed turn");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_turn_count") == 0,
              "Failed relays should not increment turn count");
    ref_history = ar_data__get_map_data(ref_observer_memory, "last_history");
    AR_ASSERT(ref_history != NULL, "Failed relay response should include history");
    AR_ASSERT(ar_data__list_count(ref_history) == 0,
              "Failed relays should not append to history");

    // Cleanup
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_conversation_context);
    ar_data__destroy(own_participant_a_context);
    ar_data__destroy(own_participant_b_context);
    ar_data__destroy(own_observer_context);
}

int main(void) {
    printf("Running conversation method tests...\n\n");
    test_conversation__coordinates_two_participant_agents();
    printf("\nAll conversation method tests passed!\n");
    return 0;
}
