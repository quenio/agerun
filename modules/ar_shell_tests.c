#include "ar_shell.h"
#include "ar_agency.h"
#include "ar_assert.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void test_shell__create_stores_default_mode_and_system(void);
static void test_shell__start_session_creates_receiving_agent(void);

int main(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            return 1;
        }
    }

    printf("Running shell module tests...\n");
    test_shell__create_stores_default_mode_and_system();
    test_shell__start_session_creates_receiving_agent();
    printf("All shell module tests passed!\n");
    return 0;
}

static void test_shell__create_stores_default_mode_and_system(void) {
    ar_shell_t *own_shell;

    printf("  test_shell__create_stores_default_mode_and_system...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");
    AR_ASSERT(ar_shell__get_default_mode(own_shell) == AR_SHELL_MODE__VERBOSE,
              "Shell should retain the requested default mode");
    AR_ASSERT(ar_shell__get_system(own_shell) != NULL,
              "Shell should wrap an owned system instance");

    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}

static void test_shell__start_session_creates_receiving_agent(void) {
    ar_shell_t *own_shell;
    ar_shell_session_t *ref_session;
    ar_agency_t *mut_agency;
    int64_t agent_id;

    printf("  test_shell__start_session_creates_receiving_agent...\n");

    own_shell = ar_shell__create(AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_shell != NULL, "Shell creation should succeed");

    ref_session = ar_shell__start_session(own_shell, AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(ref_session != NULL, "Shell should create a session");
    AR_ASSERT(ar_shell__get_session_count(own_shell) == 1,
              "Shell should track the created session");
    AR_ASSERT(ar_shell_session__get_mode(ref_session) == AR_SHELL_MODE__VERBOSE,
              "Session should retain the requested mode");
    AR_ASSERT(ar_shell_session__is_active(ref_session),
              "Session should become active after startup");

    agent_id = ar_shell_session__get_agent_id(ref_session);
    AR_ASSERT(agent_id > 0, "Shell startup should create a receiving agent");

    mut_agency = ar_system__get_agency(ar_shell__get_system(own_shell));
    AR_ASSERT(mut_agency != NULL, "Shell should expose its wrapped agency");
    AR_ASSERT(ar_agency__agent_exists(mut_agency, agent_id),
              "Created receiving agent should exist in the wrapped system");

    ar_shell__destroy(own_shell);
    printf("    PASS\n");
}
