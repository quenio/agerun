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


static void test_bootstrap_spawns_echo_on_boot(void) {
    printf("Testing bootstrap spawns echo on boot message...\n");
    
    // GIVEN: Bootstrap and echo methods are loaded
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("bootstrap_boot");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Directory");
    
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "bootstrap", 
              "../../methods/bootstrap-1.0.0.method", "1.0.0"), "Load bootstrap");
    AR_ASSERT(ar_method_fixture__load_method(own_fixture, "echo", 
              "../../methods/echo-1.0.0.method", "1.0.0"), "Load echo");
    
    // GIVEN: Bootstrap agent exists
    ar_agency_t *mut_agency = ar_method_fixture__get_agency(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    int64_t bootstrap_id = ar_agency__create_agent_with_instance(
        mut_agency, "bootstrap", "1.0.0", own_context);
    AR_ASSERT(bootstrap_id == 1, "Bootstrap should be agent 1");
    
    // WHEN: Bootstrap receives "__boot__" message
    ar_data_t *own_boot = ar_data__create_string("__boot__");
    ar_agency__send_to_agent_with_instance(mut_agency, bootstrap_id, own_boot);
    bool processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Boot message should be processed");
    
    // THEN: Bootstrap memory should reflect boot processing
    const ar_data_t *bootstrap_memory = ar_agency__get_agent_memory_with_instance(
        mut_agency, bootstrap_id);
    
    const ar_data_t *is_boot = ar_data__get_map_data(bootstrap_memory, "is_boot");
    AR_ASSERT(is_boot != NULL && ar_data__get_integer(is_boot) == 1, 
              "Bootstrap: is_boot should be 1 after boot");
    
    const ar_data_t *method_name = ar_data__get_map_data(bootstrap_memory, "method_name");
    AR_ASSERT(method_name != NULL && strcmp(ar_data__get_string(method_name), "echo") == 0,
              "Bootstrap: method_name should be 'echo'");
    
    const ar_data_t *echo_id = ar_data__get_map_data(bootstrap_memory, "echo_id");
    AR_ASSERT(echo_id != NULL && ar_data__get_integer(echo_id) == 2,
              "Bootstrap: echo_id should be 2");
    
    const ar_data_t *echo_message = ar_data__get_map_data(bootstrap_memory, "echo_message");
    AR_ASSERT(echo_message != NULL && ar_data__get_type(echo_message) == AR_DATA_TYPE__MAP,
              "Bootstrap: echo_message should be a map");
    
    const ar_data_t *sender = ar_data__get_map_data(echo_message, "sender");
    AR_ASSERT(sender != NULL && ar_data__get_integer(sender) == 1,
              "Bootstrap: echo_message.sender should be 1");
    
    const ar_data_t *content = ar_data__get_map_data(echo_message, "content");
    AR_ASSERT(content != NULL && strcmp(ar_data__get_string(content), "__boomerang__") == 0,
              "Bootstrap: echo_message.content should be '__boomerang__'");
    
    const ar_data_t *reply_received = ar_data__get_map_data(bootstrap_memory, "reply_received");
    AR_ASSERT(reply_received != NULL && ar_data__get_integer(reply_received) == 0,
              "Bootstrap: reply_received should be 0 before reply");
    
    const ar_data_t *status = ar_data__get_map_data(bootstrap_memory, "status");
    AR_ASSERT(status != NULL && strcmp(ar_data__get_string(status), "Bootstrap ready") == 0,
              "Bootstrap: status should be 'Bootstrap ready'");
    
    // WHEN: Echo processes the boomerang message
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Echo should receive message");
    
    // THEN: Echo memory should exist
    const ar_data_t *echo_memory = ar_agency__get_agent_memory_with_instance(
        mut_agency, 2);  // Echo is agent 2
    AR_ASSERT(echo_memory != NULL, "Echo agent should exist");
    AR_ASSERT(ar_data__get_type(echo_memory) == AR_DATA_TYPE__MAP,
              "Echo: memory should be a map");
    
    // WHEN: Bootstrap processes the boomerang reply
    processed = ar_method_fixture__process_next_message(own_fixture);
    AR_ASSERT(processed, "Bootstrap should receive reply");
    
    // THEN: Bootstrap memory should reflect boomerang receipt
    bootstrap_memory = ar_agency__get_agent_memory_with_instance(
        mut_agency, bootstrap_id);
    
    is_boot = ar_data__get_map_data(bootstrap_memory, "is_boot");
    AR_ASSERT(is_boot != NULL && ar_data__get_integer(is_boot) == 0,
              "Bootstrap: is_boot should be 0 after boomerang");
    
    method_name = ar_data__get_map_data(bootstrap_memory, "method_name");
    AR_ASSERT(method_name != NULL && ar_data__get_integer(method_name) == 0,
              "Bootstrap: method_name should be 0 after boomerang");
    
    echo_id = ar_data__get_map_data(bootstrap_memory, "echo_id");
    AR_ASSERT(echo_id != NULL && ar_data__get_integer(echo_id) == 0,
              "Bootstrap: echo_id should be 0 (spawn returns 0 when method_name is 0)");
    
    reply_received = ar_data__get_map_data(bootstrap_memory, "reply_received");
    AR_ASSERT(reply_received != NULL && ar_data__get_integer(reply_received) == 1,
              "Bootstrap: reply_received should be 1 after boomerang");
    
    status = ar_data__get_map_data(bootstrap_memory, "status");
    AR_ASSERT(status != NULL && strcmp(ar_data__get_string(status), "Bootstrap received reply") == 0,
              "Bootstrap: status should be 'Bootstrap received reply'");
    
    // Cleanup
    ar_data__destroy(own_context);
    ar_method_fixture__destroy(own_fixture);
    printf("PASS\n");
}


int main(void) {
    printf("Bootstrap Method Tests\n");
    printf("======================\n\n");
    
    test_bootstrap_spawns_echo_on_boot();
    
    printf("\nAll bootstrap tests passed!\n");
    return 0;
}