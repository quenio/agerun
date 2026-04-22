#include <stdio.h>
#include <string.h>
#include "ar_agency.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_method.h"
#include "ar_method_fixture.h"
#include "ar_methodology.h"

static void test_boot_echo__method_asset_loads_and_can_create_agent(void);
static void test_boot_echo__boot_message_spawns_echo_and_queues_echo_payload(void);

int main(void) {
    printf("Boot Echo Method Tests\n");
    printf("======================\n\n");

    test_boot_echo__method_asset_loads_and_can_create_agent();
    test_boot_echo__boot_message_spawns_echo_and_queues_echo_payload();

    printf("All boot-echo method tests passed!\n");
    return 0;
}

static void test_boot_echo__method_asset_loads_and_can_create_agent(void) {
    ar_method_fixture_t *own_fixture;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_method_t *ref_method;
    int64_t agent_id;

    printf("Testing boot-echo method asset loads and can create an agent...\n");

    own_fixture = ar_method_fixture__create("boot_echo_method_load");
    AR_ASSERT(own_fixture != NULL, "Method fixture should be created");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Method fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Method tests should run from bin");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "boot-echo",
        "../../methods/boot-echo-1.0.0.method",
        "1.0.0"
    ), "boot-echo method asset should load into the methodology");

    mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Method fixture should expose its agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should expose its methodology");

    ref_method = ar_methodology__get_method(mut_methodology, "boot-echo", "1.0.0");
    AR_ASSERT(ref_method != NULL, "boot-echo method should be registered");
    AR_ASSERT(ar_method__get_ast(ref_method) != NULL, "boot-echo method should parse to an AST");

    agent_id = ar_agency__create_agent(mut_agency, "boot-echo", "1.0.0", NULL);
    AR_ASSERT(agent_id > 0, "boot-echo method should support creating a receiving agent");
    AR_ASSERT(ar_agency__destroy_agent(mut_agency, agent_id), "boot-echo agent should be destroyable");

    while (ar_method_fixture__process_next_message(own_fixture)) {
    }
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "Method fixture should report no leaks");
    ar_method_fixture__destroy(own_fixture);
}

static void test_boot_echo__boot_message_spawns_echo_and_queues_echo_payload(void) {
    ar_method_fixture_t *own_fixture;
    ar_agency_t *mut_agency;
    ar_data_t *own_context;
    ar_data_t *own_boot_message;
    const ar_data_t *ref_memory;
    const ar_data_t *ref_echo_payload;
    const ar_data_t *ref_echo_memory;
    int64_t boot_echo_id;
    int64_t echo_id;
    int extra_processed;

    printf("Testing boot-echo boot message spawns echo and queues echo payload...\n");

    own_fixture = ar_method_fixture__create("boot_echo_boot_message");
    AR_ASSERT(own_fixture != NULL, "Method fixture should be created");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Method fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Method tests should run from bin");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "echo",
        "../../methods/echo-1.0.0.method",
        "1.0.0"
    ), "echo method should load into the methodology");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        "boot-echo",
        "../../methods/boot-echo-1.0.0.method",
        "1.0.0"
    ), "boot-echo method should load into the methodology");

    mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Method fixture should expose its agency");

    own_context = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "boot-echo context should be created");

    boot_echo_id = ar_agency__create_agent(mut_agency, "boot-echo", "1.0.0", own_context);
    AR_ASSERT(boot_echo_id == 1, "boot-echo should be the first created agent");

    own_boot_message = ar_data__create_string("__boot__");
    AR_ASSERT(own_boot_message != NULL, "boot message should be created");
    AR_ASSERT(ar_agency__send_to_agent(mut_agency, boot_echo_id, own_boot_message),
              "boot message should be queued for the boot-echo agent");

    AR_ASSERT(ar_method_fixture__process_next_message(own_fixture),
              "boot-echo should process the __boot__ message");

    ref_memory = ar_agency__get_agent_memory(mut_agency, boot_echo_id);
    AR_ASSERT(ref_memory != NULL, "boot-echo memory should be available");
    AR_ASSERT(ar_data__get_map_integer(ref_memory, "is_boot") == 1,
              "boot-echo should mark the boot lifecycle message");

    echo_id = ar_data__get_map_integer(ref_memory, "echo_id");
    AR_ASSERT(echo_id == 2, "boot-echo should spawn an echo agent as agent 2");

    ref_echo_payload = ar_data__get_map_data(ref_memory, "echo_payload");
    AR_ASSERT(ref_echo_payload != NULL, "boot-echo should store the queued echo payload");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_echo_payload, "content"), "boot_echo_ready") == 0,
              "boot-echo should queue the expected content for echo");
    AR_ASSERT(ar_data__get_map_integer(ref_echo_payload, "sender") == 0,
              "boot-echo should queue the payload with sender 0 for echo reply no-op");
    AR_ASSERT(strcmp(ar_data__get_map_string(ref_memory, "demo_status"), "Boot echo queued") == 0,
              "boot-echo should record the queued demo status");

    extra_processed = 0;
    while (ar_method_fixture__process_next_message(own_fixture)) {
        extra_processed++;
    }
    AR_ASSERT(extra_processed == 1,
              "boot-echo should leave exactly one queued echo message for the child agent");

    ref_echo_memory = ar_agency__get_agent_memory(mut_agency, echo_id);
    AR_ASSERT(ref_echo_memory != NULL, "spawned echo agent memory should be available");

    AR_ASSERT(ar_agency__destroy_agent(mut_agency, echo_id), "spawned echo agent should be destroyable");
    AR_ASSERT(ar_agency__destroy_agent(mut_agency, boot_echo_id), "boot-echo agent should be destroyable");

    while (ar_method_fixture__process_next_message(own_fixture)) {
    }
    ar_data__destroy(own_context);
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "Method fixture should report no leaks");
    ar_method_fixture__destroy(own_fixture);
}
