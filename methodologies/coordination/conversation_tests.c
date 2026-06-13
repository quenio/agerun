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

static void append_agent_id(ar_data_t *mut_list, int64_t agent_id) {
    AR_ASSERT(ar_data__list_add_last_integer(mut_list, checked_agent_id(agent_id)),
              "Agent id should be appended");
}

static ar_data_t *create_participants(int64_t first,
                                      int64_t second,
                                      int64_t third) {
    ar_data_t *own_participants = ar_data__create_list();
    AR_ASSERT(own_participants != NULL, "Participants list should be created");
    append_agent_id(own_participants, first);
    append_agent_id(own_participants, second);
    if (third > 0) {
        append_agent_id(own_participants, third);
    }
    return own_participants;
}

static void register_record_receiver(ar_agency_t *mut_agency) {
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    const char *ref_instructions =
        "memory.last_action := message.action\n"
        "memory.last_request := message.request\n"
        "memory.last_response := message.response\n"
        "memory.last_source := message.source\n"
        "memory.last_trace_id := message.trace_id\n"
        "memory.last_session_id := message.session_id\n"
        "memory.last_participant := message.participant\n"
        "memory.last_state := message.state\n"
        "memory.last_status := message.status\n"
        "memory.last_result := message.result\n"
        "memory.last_success_count := message.success_count\n"
        "memory.last_failure_count := message.failure_count\n"
        "memory.last_payload := message.payload\n"
        "memory.last_turn_count := message.turn_count\n"
        "memory.last_history := message.history\n"
        "memory.last_participants := message.participants\n"
        "memory.last_last_sender := message.last_sender\n"
        "memory.last_last_payload := message.last_payload\n";

    AR_ASSERT(ar_methodology__create_method(mut_methodology,
                                            "record-receiver",
                                            ref_instructions,
                                            "1.0.0"),
              "record-receiver should be registered");
    verify_method_parses(mut_methodology, "record-receiver");
}

static void test_conversation__broadcasts_turns_to_all_other_participants(void) {
    printf("Testing conversation broadcasts turns to all other participants...\n");

    // Given a conversation coordinator, three participants, and a response observer
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("conversation_multi_agent");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    load_method(own_fixture, "conversation");
    load_method(own_fixture, "broadcasting");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    register_record_receiver(mut_agency);

    ar_data_t *own_conversation_context = create_context();
    ar_data_t *own_participant_a_context = create_context();
    ar_data_t *own_participant_b_context = create_context();
    ar_data_t *own_participant_c_context = create_context();
    ar_data_t *own_observer_context = create_context();
    int64_t conversation_agent = ar_agency__create_agent(
        mut_agency, "conversation", "1.0.0", own_conversation_context);
    int64_t participant_a = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_participant_a_context);
    int64_t participant_b = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_participant_b_context);
    int64_t participant_c = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_participant_c_context);
    int64_t observer = ar_agency__create_agent(
        mut_agency, "record-receiver", "1.0.0", own_observer_context);

    // When the coordinator starts a conversation
    ar_data_t *own_start = ar_data__create_map();
    AR_ASSERT(own_start != NULL, "Conversation start should be created");
    ar_data__set_map_string(own_start, "request", "conversation_start");
    ar_data__set_map_string(own_start, "trace_id", "chat-trace-1");
    ar_data__set_map_string(own_start, "session_id", "chat-session-1");
    ar_data_t *own_participants = create_participants(participant_a, participant_b, participant_c);
    AR_ASSERT(ar_data__set_map_data(own_start, "participants", own_participants),
              "Conversation start should own participants list");
    own_participants = NULL;
    ar_data__set_map_integer(own_start, "source", checked_agent_id(observer));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_start),
              "Conversation start should queue");
    own_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    const ar_data_t *ref_observer_memory = ar_agency__get_agent_memory(mut_agency, observer);
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_response"),
                     "conversation_result") == 0,
              "Conversation start should emit a response");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "success") == 0,
              "Conversation start should report standard success status");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_result"), "active") == 0,
              "Conversation start should report active state");

    const ar_data_t *ref_conversation_memory = ar_agency__get_agent_memory(mut_agency,
                                                                          conversation_agent);
    int64_t first_broadcasting_agent =
        ar_data__get_map_integer(ref_conversation_memory, "broadcasting_agent");
    AR_ASSERT(first_broadcasting_agent > 0,
              "Conversation should spawn a broadcasting helper on start");

    // When participant A sends a turn
    ar_data_t *own_first = ar_data__create_map();
    AR_ASSERT(own_first != NULL, "First conversation message should be created");
    ar_data__set_map_string(own_first, "request", "conversation_message");
    ar_data__set_map_string(own_first, "trace_id", "chat-turn-1");
    ar_data__set_map_string(own_first, "session_id", "chat-session-1");
    ar_data__set_map_integer(own_first, "sender", checked_agent_id(participant_a));
    ar_data__set_map_string(own_first, "payload", "hello");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_first),
              "First conversation message should queue");
    own_first = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then participants B and C should receive the same turn while A is excluded
    const ar_data_t *ref_participant_a_memory = ar_agency__get_agent_memory(mut_agency,
                                                                           participant_a);
    const ar_data_t *ref_participant_b_memory = ar_agency__get_agent_memory(mut_agency,
                                                                           participant_b);
    const ar_data_t *ref_participant_c_memory = ar_agency__get_agent_memory(mut_agency,
                                                                           participant_c);
    AR_ASSERT(ar_data__get_map_data(ref_participant_a_memory, "last_request") == NULL,
              "Sender A should not receive its own turn");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_request"),
                     "conversation_turn") == 0,
              "Participant B should receive a conversation turn");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_c_memory, "last_request"),
                     "conversation_turn") == 0,
              "Participant C should receive a conversation turn");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_b_memory, "last_source") ==
                  checked_agent_id(conversation_agent),
              "Participant B turn should identify the conversation source");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_c_memory, "last_source") ==
                  checked_agent_id(conversation_agent),
              "Participant C turn should identify the conversation source");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_b_memory, "last_participant") ==
                  checked_agent_id(participant_a),
              "Participant B should see participant A as sender");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_c_memory, "last_participant") ==
                  checked_agent_id(participant_a),
              "Participant C should see participant A as sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_payload"),
                     "hello") == 0,
              "Participant B should receive participant A's payload");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_c_memory, "last_payload"),
                     "hello") == 0,
              "Participant C should receive participant A's payload");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_trace_id"),
                     "chat-turn-1") == 0,
              "Participant B turn should preserve first message trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_c_memory, "last_trace_id"),
                     "chat-turn-1") == 0,
              "Participant C turn should preserve first message trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_session_id"),
                     "chat-session-1") == 0,
              "Participant B turn should preserve conversation session id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_c_memory, "last_session_id"),
                     "chat-session-1") == 0,
              "Participant C turn should preserve conversation session id");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_b_memory, "last_turn_count") == 1,
              "Participant B should receive turn one");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_c_memory, "last_turn_count") == 1,
              "Participant C should receive turn one");

    // When participant B sends a turn
    ar_data_t *own_second = ar_data__create_map();
    AR_ASSERT(own_second != NULL, "Second conversation message should be created");
    ar_data__set_map_string(own_second, "request", "conversation_message");
    ar_data__set_map_string(own_second, "trace_id", "chat-turn-2");
    ar_data__set_map_string(own_second, "session_id", "chat-session-1");
    ar_data__set_map_integer(own_second, "sender", checked_agent_id(participant_b));
    ar_data__set_map_string(own_second, "payload", "reply");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_second),
              "Second conversation message should queue");
    own_second = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    // Then participants A and C should receive the turn, B should not, and broadcasting is reused
    ref_conversation_memory = ar_agency__get_agent_memory(mut_agency, conversation_agent);
    int64_t second_broadcasting_agent =
        ar_data__get_map_integer(ref_conversation_memory, "broadcasting_agent");
    AR_ASSERT(second_broadcasting_agent == first_broadcasting_agent,
              "Conversation should reuse the broadcasting helper across turns");

    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_b_memory, "last_trace_id"),
                     "chat-turn-1") == 0,
              "Sender B should not receive its own reply turn");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_a_memory, "last_request"),
                     "conversation_turn") == 0,
              "Participant A should receive participant B's reply turn");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_a_memory, "last_participant") ==
                  checked_agent_id(participant_b),
              "Participant A should see participant B as sender");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_c_memory, "last_participant") ==
                  checked_agent_id(participant_b),
              "Participant C should see participant B as sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_a_memory, "last_payload"),
                     "reply") == 0,
              "Participant A should receive participant B's payload");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_c_memory, "last_payload"),
                     "reply") == 0,
              "Participant C should receive participant B's payload");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_a_memory, "last_trace_id"),
                     "chat-turn-2") == 0,
              "Participant A turn should preserve second message trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_c_memory, "last_trace_id"),
                     "chat-turn-2") == 0,
              "Participant C turn should preserve second message trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_a_memory, "last_session_id"),
                     "chat-session-1") == 0,
              "Participant A turn should preserve conversation session id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_participant_c_memory, "last_session_id"),
                     "chat-session-1") == 0,
              "Participant C turn should preserve conversation session id");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_a_memory, "last_turn_count") == 2,
              "Participant A should receive turn two");
    AR_ASSERT(ar_data__get_map_integer(ref_participant_c_memory, "last_turn_count") == 2,
              "Participant C should receive turn two");

    // When a summary is requested
    ar_data_t *own_summary = ar_data__create_map();
    AR_ASSERT(own_summary != NULL, "Summary request should be created");
    ar_data__set_map_string(own_summary, "request", "conversation_summary");
    ar_data__set_map_string(own_summary, "trace_id", "chat-summary-1");
    ar_data__set_map_string(own_summary, "session_id", "chat-session-1");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_summary),
              "Summary request should queue");
    own_summary = NULL;

    ar_method_fixture__process_all_messages(own_fixture);

    // Then the observer should receive a structured summary with both turns
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_response"),
                     "conversation_result") == 0,
              "Conversation summary should be a response");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_source") ==
                  checked_agent_id(conversation_agent),
              "Conversation summary should identify the conversation source");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_status"), "success") == 0,
              "Conversation summary should report standard success status");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_turn_count") == 2,
              "Summary should report two turns");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_trace_id"),
                     "chat-summary-1") == 0,
              "Conversation summary should preserve summary request trace id");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_session_id"),
                     "chat-session-1") == 0,
              "Conversation summary should preserve conversation session id");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_success_count") == 2,
              "Conversation summary should report successful turn count");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_failure_count") == 0,
              "Conversation summary should report no failed turns");
    AR_ASSERT(ar_data__get_map_integer(ref_observer_memory, "last_last_sender") ==
                  checked_agent_id(participant_b),
              "Summary should report participant B as the last sender");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_last_payload"),
                     "reply") == 0,
              "Summary should report the last payload");
    ar_data_t *ref_participants = ar_data__get_map_data(ref_observer_memory, "last_participants");
    AR_ASSERT(ref_participants != NULL, "Summary should include participants");
    AR_ASSERT(ar_data__list_count(ref_participants) == 3,
              "Summary should include all participants");
    ar_data_t *ref_history = ar_data__get_map_data(ref_observer_memory, "last_history");
    AR_ASSERT(ref_history != NULL, "Summary should include history");
    AR_ASSERT(ar_data__list_count(ref_history) == 2, "Summary history should include both turns");

    // When a participant message cannot be delivered
    ar_data_t *own_failed_start = ar_data__create_map();
    AR_ASSERT(own_failed_start != NULL, "Failed relay conversation start should be created");
    ar_data__set_map_string(own_failed_start, "request", "conversation_start");
    ar_data__set_map_string(own_failed_start, "trace_id", "chat-2-start");
    ar_data__set_map_string(own_failed_start, "session_id", "chat-session-2");
    own_participants = create_participants(participant_a, 98765, 0);
    AR_ASSERT(ar_data__set_map_data(own_failed_start, "participants", own_participants),
              "Failed relay start should own participants list");
    own_participants = NULL;
    ar_data__set_map_integer(own_failed_start, "source", checked_agent_id(observer));
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, conversation_agent, own_failed_start),
              "Failed relay start should queue");
    own_failed_start = NULL;
    ar_method_fixture__process_all_messages(own_fixture);

    ar_data_t *own_failed_turn = ar_data__create_map();
    AR_ASSERT(own_failed_turn != NULL, "Failed relay message should be created");
    ar_data__set_map_string(own_failed_turn, "request", "conversation_message");
    ar_data__set_map_string(own_failed_turn, "trace_id", "chat-failed-turn");
    ar_data__set_map_string(own_failed_turn, "session_id", "chat-session-2");
    ar_data__set_map_integer(own_failed_turn, "sender", checked_agent_id(participant_a));
    ar_data__set_map_string(own_failed_turn, "payload", "undeliverable");
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
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_observer_memory, "last_session_id"),
                     "chat-session-2") == 0,
              "Failed relay response should preserve conversation session id");
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
    ar_data__destroy(own_participant_c_context);
    ar_data__destroy(own_observer_context);
}

int main(void) {
    printf("Running conversation method tests...\n\n");
    test_conversation__broadcasts_turns_to_all_other_participants();
    printf("\nAll conversation method tests passed!\n");
    return 0;
}
