#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_assert.h"
#include "ar_methodology.h"
#include "ar_method.h"


static void test_bootstrap_runs_chat_session_demo_on_boot(void) {
    printf("Testing bootstrap runs chat-session demo on boot message...\n");
    
    // GIVEN: Bootstrap and chat-session methods are loaded
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("bootstrap_boot");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Directory");
    
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "bootstrap", 
              "../../methods/bootstrap-1.0.0.method", "1.0.0"), "Load bootstrap");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "chat-session", 
              "../../methods/chat-session-1.0.0.method", "1.0.0"), "Load chat-session");
    
    // GIVEN: Bootstrap agent exists
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    ar_data__set_map_string(own_context, "bot_name", "support-bot");
    int64_t bootstrap_id = ar_agency__create_agent(
        mut_agency, "bootstrap", "1.0.0", own_context);
    AR_ASSERT(bootstrap_id == 1, "Bootstrap should be agent 1");
    
    // WHEN: Bootstrap receives "__boot__" message
    ar_data_t *own_boot = ar_data__create_string("__boot__");
    ar_agency__send_to_agent(mut_agency, bootstrap_id, own_boot);
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Boot message should be processed");
    
    // THEN: Bootstrap should queue a chat-session demo
    const ar_data_t *bootstrap_memory = ar_agency__get_agent_memory(
        mut_agency, bootstrap_id);
    
    const ar_data_t *is_boot = ar_data__get_map_data(bootstrap_memory, "is_boot");
    AR_ASSERT(is_boot != NULL && ar_data__get_integer(is_boot) == 1, 
              "Bootstrap: is_boot should be 1 after boot");
    
    const ar_data_t *method_name = ar_data__get_map_data(bootstrap_memory, "method_name");
    AR_ASSERT(method_name != NULL && strcmp(ar_data__get_string(method_name), "chat-session") == 0,
              "Bootstrap: method_name should be 'chat-session'");
    
    const ar_data_t *chat_session_id = ar_data__get_map_data(bootstrap_memory, "chat_session_id");
    AR_ASSERT(chat_session_id != NULL && ar_data__get_integer(chat_session_id) == 2,
              "Bootstrap: chat_session_id should be 2");
    
    const ar_data_t *start_message = ar_data__get_map_data(bootstrap_memory, "start_message");
    AR_ASSERT(start_message != NULL && ar_data__get_type(start_message) == AR_DATA_TYPE__MAP,
              "Bootstrap: start_message should be a map");
    const ar_data_t *start_action = ar_data__get_map_data(start_message, "action");
    AR_ASSERT(start_action != NULL && strcmp(ar_data__get_string(start_action), "start") == 0,
              "Bootstrap: start_message.action should be 'start'");
    
    const ar_data_t *user_message = ar_data__get_map_data(bootstrap_memory, "user_message");
    AR_ASSERT(user_message != NULL && ar_data__get_type(user_message) == AR_DATA_TYPE__MAP,
              "Bootstrap: user_message should be a map");
    const ar_data_t *user_action = ar_data__get_map_data(user_message, "action");
    AR_ASSERT(user_action != NULL && strcmp(ar_data__get_string(user_action), "message") == 0,
              "Bootstrap: user_message.action should be 'message'");
    
    const ar_data_t *summary_message = ar_data__get_map_data(bootstrap_memory, "summary_message");
    AR_ASSERT(summary_message != NULL && ar_data__get_type(summary_message) == AR_DATA_TYPE__MAP,
              "Bootstrap: summary_message should be a map");
    const ar_data_t *summary_action = ar_data__get_map_data(summary_message, "action");
    AR_ASSERT(summary_action != NULL && strcmp(ar_data__get_string(summary_action), "summary") == 0,
              "Bootstrap: summary_message.action should be 'summary'");
    
    const ar_data_t *demo_status = ar_data__get_map_data(bootstrap_memory, "demo_status");
    AR_ASSERT(demo_status != NULL && strcmp(ar_data__get_string(demo_status), "Chat session demo queued") == 0,
              "Bootstrap: demo_status should confirm queued demo");
    
    // WHEN: Chat-session processes start, message, and summary
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Chat-session should process start message");
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Chat-session should process user message");
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Chat-session should process summary message");
    
    // THEN: Chat-session memory should reflect the sample conversation
    const ar_data_t *chat_memory = ar_agency__get_agent_memory(
        mut_agency, 2);
    AR_ASSERT(chat_memory != NULL, "Chat-session agent should exist");
    
    const ar_data_t *session_id = ar_data__get_map_data(chat_memory, "session_id");
    AR_ASSERT(session_id != NULL && strcmp(ar_data__get_string(session_id), "demo-session") == 0,
              "Chat-session: session_id should be demo-session");
    
    const ar_data_t *state = ar_data__get_map_data(chat_memory, "state");
    AR_ASSERT(state != NULL && strcmp(ar_data__get_string(state), "active") == 0,
              "Chat-session: state should remain active");
    
    const ar_data_t *turn_count = ar_data__get_map_data(chat_memory, "turn_count");
    AR_ASSERT(turn_count != NULL && ar_data__get_integer(turn_count) == 1,
              "Chat-session: turn_count should be 1");
    
    const ar_data_t *last_user_message = ar_data__get_map_data(chat_memory, "last_user_message");
    AR_ASSERT(last_user_message != NULL && strcmp(ar_data__get_string(last_user_message), "Need_help") == 0,
              "Chat-session: last_user_message should match demo content");
    
    const ar_data_t *summary = ar_data__get_map_data(chat_memory, "summary");
    AR_ASSERT(summary != NULL && strcmp(ar_data__get_string(summary),
              "session=demo-session user=demo-user channel=web state=active turns=1 escalation=0") == 0,
              "Chat-session: summary should describe the demo conversation");
    
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(!processed, "No extra messages should remain after demo");
    
    // Cleanup
    ar_data__destroy(own_context);
    ar_method_fixture__destroy(own_fixture);
    printf("PASS\n");
}


int main(void) {
    printf("Bootstrap Method Tests\n");
    printf("======================\n\n");
    
    test_bootstrap_runs_chat_session_demo_on_boot();
    
    printf("\nAll bootstrap tests passed!\n");
    return 0;
}