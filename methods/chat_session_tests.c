#include <stdio.h>
#include <string.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_assert.h"

static ar_data_t *create_chat_session_context(void) {
    ar_data_t *own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Chat session context should be created");
    ar_data__set_map_string(own_context, "bot_name", "support-bot");
    return own_context;
}

static ar_data_t *create_start_message(void) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Start message should be created");
    ar_data__set_map_string(own_message, "action", "start");
    ar_data__set_map_string(own_message, "session_id", "sess-1001");
    ar_data__set_map_string(own_message, "user_id", "user-42");
    ar_data__set_map_string(own_message, "channel", "web");
    ar_data__set_map_string(own_message, "content", "");
    ar_data__set_map_string(own_message, "intent", "");
    ar_data__set_map_integer(own_message, "sender", 0);
    return own_message;
}

static ar_data_t *create_message_event(const char *ref_content, const char *ref_intent) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Message event should be created");
    ar_data__set_map_string(own_message, "action", "message");
    ar_data__set_map_string(own_message, "session_id", "sess-1001");
    ar_data__set_map_string(own_message, "user_id", "user-42");
    ar_data__set_map_string(own_message, "channel", "web");
    ar_data__set_map_string(own_message, "content", ref_content);
    ar_data__set_map_string(own_message, "intent", ref_intent);
    ar_data__set_map_integer(own_message, "sender", 0);
    return own_message;
}

static ar_data_t *create_action_message(const char *ref_action) {
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_message != NULL, "Action message should be created");
    ar_data__set_map_string(own_message, "action", ref_action);
    ar_data__set_map_string(own_message, "session_id", "sess-1001");
    ar_data__set_map_string(own_message, "user_id", "user-42");
    ar_data__set_map_string(own_message, "channel", "web");
    ar_data__set_map_string(own_message, "content", "");
    ar_data__set_map_string(own_message, "intent", "");
    ar_data__set_map_integer(own_message, "sender", 0);
    return own_message;
}

static void test_chat_session__start_initializes_session_state(void) {
    printf("Testing chat-session start initializes session state...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("chat_session_start");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "chat-session",
        "../../methods/chat-session-1.0.0.method",
        "1.0.0"
    ), "chat-session method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Fixture should provide agency");

    ar_data_t *own_context = create_chat_session_context();
    int64_t session_agent_id = ar_agency__create_agent(mut_agency, "chat-session", "1.0.0", own_context);
    AR_ASSERT(session_agent_id > 0, "Chat session agent should be created");

    ar_data_t *own_message = create_start_message();
    bool sent = ar_agency__send_to_agent(mut_agency, session_agent_id, own_message);
    AR_ASSERT(sent, "Start message should be queued");
    own_message = NULL;

    bool processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Start message should be processed");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, session_agent_id);
    AR_ASSERT(ref_memory != NULL, "Agent memory should exist");

    const ar_data_t *ref_state = ar_data__get_map_data(ref_memory, "state");
    AR_ASSERT(ref_state != NULL, "State should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_state), "active") == 0, "State should be active");

    const ar_data_t *ref_session_id = ar_data__get_map_data(ref_memory, "session_id");
    AR_ASSERT(ref_session_id != NULL, "Session id should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_session_id), "sess-1001") == 0,
              "Session id should match message");

    const ar_data_t *ref_turn_count = ar_data__get_map_data(ref_memory, "turn_count");
    AR_ASSERT(ref_turn_count != NULL, "Turn count should be stored");
    AR_ASSERT(ar_data__get_integer(ref_turn_count) == 0, "Turn count should start at 0");

    const ar_data_t *ref_last_reply = ar_data__get_map_data(ref_memory, "last_reply");
    AR_ASSERT(ref_last_reply != NULL, "Last reply should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_last_reply), "session_started") == 0,
              "Start should set session_started reply");

    ar_agency__destroy_agent(mut_agency, session_agent_id);
    while (ar_method_fixture__process_next_message(own_fixture)) {
    }
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "Fixture should report no leaks");
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
}

static void test_chat_session__message_updates_turns_and_last_message(void) {
    printf("Testing chat-session message updates turns and last message...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("chat_session_message");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "chat-session",
        "../../methods/chat-session-1.0.0.method",
        "1.0.0"
    ), "chat-session method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Fixture should provide agency");

    ar_data_t *own_context = create_chat_session_context();
    int64_t session_agent_id = ar_agency__create_agent(mut_agency, "chat-session", "1.0.0", own_context);
    AR_ASSERT(session_agent_id > 0, "Chat session agent should be created");

    ar_data_t *own_start = create_start_message();
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, session_agent_id, own_start),
              "Start message should be queued");
    own_start = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Start should process");

    ar_data_t *own_message = create_message_event("I need help with my order", "general");
    bool sent = ar_agency__send_to_agent(mut_agency, session_agent_id, own_message);
    AR_ASSERT(sent, "User message should be queued");
    own_message = NULL;

    bool processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "User message should be processed");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, session_agent_id);
    AR_ASSERT(ref_memory != NULL, "Agent memory should exist");

    const ar_data_t *ref_turn_count = ar_data__get_map_data(ref_memory, "turn_count");
    AR_ASSERT(ref_turn_count != NULL, "Turn count should be stored");
    AR_ASSERT(ar_data__get_integer(ref_turn_count) == 1, "Turn count should increment");

    const ar_data_t *ref_last_user_message = ar_data__get_map_data(ref_memory, "last_user_message");
    AR_ASSERT(ref_last_user_message != NULL, "Last user message should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_last_user_message), "I need help with my order") == 0,
              "Last user message should match input");

    const ar_data_t *ref_last_reply = ar_data__get_map_data(ref_memory, "last_reply");
    AR_ASSERT(ref_last_reply != NULL, "Last reply should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_last_reply), "message_received") == 0,
              "General message should set message_received reply");

    const ar_data_t *ref_state = ar_data__get_map_data(ref_memory, "state");
    AR_ASSERT(ref_state != NULL, "State should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_state), "active") == 0,
              "General message should keep active state");

    ar_agency__destroy_agent(mut_agency, session_agent_id);
    while (ar_method_fixture__process_next_message(own_fixture)) {
    }
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "Fixture should report no leaks");
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
}

static void test_chat_session__human_handoff_and_summary(void) {
    printf("Testing chat-session handoff and summary generation...\n");

    ar_method_fixture_t *own_fixture = ar_method_fixture__create("chat_session_handoff");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Fixture directory should verify");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "chat-session",
        "../../methods/chat-session-1.0.0.method",
        "1.0.0"
    ), "chat-session method should load");

    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Fixture should provide agency");

    ar_data_t *own_context = create_chat_session_context();
    int64_t session_agent_id = ar_agency__create_agent(mut_agency, "chat-session", "1.0.0", own_context);
    AR_ASSERT(session_agent_id > 0, "Chat session agent should be created");

    ar_data_t *own_start = create_start_message();
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, session_agent_id, own_start),
              "Start message should be queued");
    own_start = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Start should process");

    ar_data_t *own_handoff = create_message_event("I want a human", "human");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, session_agent_id, own_handoff),
              "Human handoff message should be queued");
    own_handoff = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Handoff should process");

    ar_data_t *own_summary = create_action_message("summary");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, session_agent_id, own_summary),
              "Summary message should be queued");
    own_summary = NULL;
    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture), "Summary should process");

    const ar_data_t *ref_memory = ar_agency__get_agent_memory(mut_agency, session_agent_id);
    AR_ASSERT(ref_memory != NULL, "Agent memory should exist");

    const ar_data_t *ref_state = ar_data__get_map_data(ref_memory, "state");
    AR_ASSERT(ref_state != NULL, "State should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_state), "waiting_for_human") == 0,
              "Human handoff should update state");

    const ar_data_t *ref_escalation = ar_data__get_map_data(ref_memory, "escalation_requested");
    AR_ASSERT(ref_escalation != NULL, "Escalation flag should be stored");
    AR_ASSERT(ar_data__get_integer(ref_escalation) == 1, "Escalation flag should be 1");

    const ar_data_t *ref_last_reply = ar_data__get_map_data(ref_memory, "last_reply");
    AR_ASSERT(ref_last_reply != NULL, "Last reply should be stored");
    AR_ASSERT(strcmp(ar_data__get_string(ref_last_reply), "handoff_requested") == 0,
              "Human handoff should set handoff reply");

    const ar_data_t *ref_summary = ar_data__get_map_data(ref_memory, "summary");
    AR_ASSERT(ref_summary != NULL, "Summary should be stored");
    AR_ASSERT(strcmp(
        ar_data__get_string(ref_summary),
        "session=sess-1001 user=user-42 channel=web state=waiting_for_human turns=1 escalation=1"
    ) == 0, "Summary should include key session fields");

    ar_agency__destroy_agent(mut_agency, session_agent_id);
    while (ar_method_fixture__process_next_message(own_fixture)) {
    }
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "Fixture should report no leaks");
    ar_method_fixture__destroy(own_fixture);
    ar_data__destroy(own_context);
}

int main(void) {
    printf("Chat Session Method Tests\n");
    printf("=========================\n\n");

    test_chat_session__start_initializes_session_state();
    test_chat_session__message_updates_turns_and_last_message();
    test_chat_session__human_handoff_and_summary();

    printf("\nAll chat-session tests passed!\n");
    return 0;
}
