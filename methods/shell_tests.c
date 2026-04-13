#include <stdio.h>
#include <string.h>
#include "ar_method_fixture.h"
#include "ar_agency.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_assert.h"

static void test_shell__method_asset_loads_and_can_create_agent(void);

int main(void) {
    printf("Shell Method Tests\n");
    printf("==================\n\n");

    test_shell__method_asset_loads_and_can_create_agent();

    printf("All shell method tests passed!\n");
    return 0;
}

static void test_shell__method_asset_loads_and_can_create_agent(void) {
    ar_method_fixture_t *own_fixture;
    ar_agency_t *mut_agency;
    ar_methodology_t *mut_methodology;
    ar_method_t *ref_method;
    int64_t agent_id;

    printf("Testing shell method asset loads and can create an agent...\n");

    own_fixture = ar_method_fixture__create("shell_method_load");
    AR_ASSERT(own_fixture != NULL, "Method fixture should be created");
    AR_ASSERT(ar_method_fixture__initialize(own_fixture), "Method fixture should initialize");
    AR_ASSERT(ar_method_fixture__verify_directory(own_fixture), "Method tests should run from bin");
    AR_ASSERT(ar_method_fixture__load_method(
        own_fixture,
        AR_SHELL_METHOD_NAME,
        "../../methods/shell-1.0.0.method",
        AR_SHELL_METHOD_VERSION
    ), "Shell method asset should load into the methodology");

    mut_agency = ar_method_fixture__get_agency(own_fixture);
    AR_ASSERT(mut_agency != NULL, "Method fixture should expose its agency");
    mut_methodology = ar_agency__get_methodology(mut_agency);
    AR_ASSERT(mut_methodology != NULL, "Agency should expose its methodology");

    ref_method = ar_methodology__get_method(mut_methodology, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION);
    AR_ASSERT(ref_method != NULL, "Shell method should be registered");
    AR_ASSERT(ar_method__get_ast(ref_method) != NULL, "Shell method asset should parse to an AST");

    agent_id = ar_agency__create_agent(mut_agency, AR_SHELL_METHOD_NAME, AR_SHELL_METHOD_VERSION, NULL);
    AR_ASSERT(agent_id > 0, "Shell method should support creating a receiving agent");
    AR_ASSERT(ar_agency__destroy_agent(mut_agency, agent_id), "Shell agent should be destroyable");

    while (ar_method_fixture__process_next_message(own_fixture)) {
    }
    AR_ASSERT(ar_method_fixture__check_memory(own_fixture), "Method fixture should report no leaks");
    ar_method_fixture__destroy(own_fixture);
}
