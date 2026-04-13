#include "ar_shell_session.h"
#include "ar_assert.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void test_shell_session__create_initializes_memory_and_mode(void);
static void test_shell_session__activate_records_agent_id(void);

int main(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            return 1;
        }
    }

    printf("Running shell session tests...\n");
    test_shell_session__create_initializes_memory_and_mode();
    test_shell_session__activate_records_agent_id();
    printf("All shell session tests passed!\n");
    return 0;
}

static void test_shell_session__create_initializes_memory_and_mode(void) {
    ar_shell_session_t *own_session;

    printf("  test_shell_session__create_initializes_memory_and_mode...\n");

    own_session = ar_shell_session__create(7, AR_SHELL_MODE__VERBOSE);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");
    AR_ASSERT(ar_shell_session__get_session_id(own_session) == 7,
              "Shell session should retain its identifier");
    AR_ASSERT(ar_shell_session__get_mode(own_session) == AR_SHELL_MODE__VERBOSE,
              "Shell session should retain its startup mode");
    AR_ASSERT(ar_shell_session__get_memory(own_session) != NULL,
              "Shell session should create an owned session memory map");
    AR_ASSERT(!ar_shell_session__is_active(own_session),
              "Fresh shell session should not be active before activation");

    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}

static void test_shell_session__activate_records_agent_id(void) {
    ar_shell_session_t *own_session;

    printf("  test_shell_session__activate_records_agent_id...\n");

    own_session = ar_shell_session__create(8, AR_SHELL_MODE__NORMAL);
    AR_ASSERT(own_session != NULL, "Shell session creation should succeed");
    AR_ASSERT(ar_shell_session__activate(own_session, 42),
              "Activation should succeed with a receiving-agent identifier");
    AR_ASSERT(ar_shell_session__is_active(own_session),
              "Activated shell session should become active");
    AR_ASSERT(ar_shell_session__get_agent_id(own_session) == 42,
              "Activated shell session should retain the receiving-agent identifier");

    ar_shell_session__destroy(own_session);
    printf("    PASS\n");
}
