#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_proxy.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include "ar_log.h"
#include "ar_data.h"

// Test function declarations
static void test_proxy__create_and_destroy(void);
static void test_proxy__stores_log_and_type(void);
static void test_proxy__handle_message_returns_false(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_proxy_tests\n");
            return 1;
        }
    }

    printf("Running proxy module tests...\n");

    // Run tests
    test_proxy__create_and_destroy();
    test_proxy__stores_log_and_type();
    test_proxy__handle_message_returns_false();

    printf("All proxy tests passed!\n");
    return 0;
}

static void test_proxy__create_and_destroy(void) {
    printf("  test_proxy__create_and_destroy...\n");

    // Given no prerequisites

    // When creating a proxy with NULL parameters
    ar_proxy_t *own_proxy = ar_proxy__create(NULL, NULL);

    // Then the proxy should be created successfully
    AR_ASSERT(own_proxy != NULL, "Proxy creation should succeed with NULL parameters");

    // Clean up
    ar_proxy__destroy(own_proxy);

    printf("    PASS\n");
}

static void test_proxy__stores_log_and_type(void) {
    printf("  test_proxy__stores_log_and_type...\n");

    // Given a log instance and proxy type
    ar_log_t *own_log = ar_log__create();
    const char *type = "file";

    // When creating a proxy with log and type
    ar_proxy_t *own_proxy = ar_proxy__create(own_log, type);
    AR_ASSERT(own_proxy != NULL, "Proxy creation should succeed");

    // Then the proxy should store the log reference
    ar_log_t *ref_stored_log = ar_proxy__get_log(own_proxy);
    AR_ASSERT(ref_stored_log == own_log,
              "Stored log should match the log passed to create");

    // Then the proxy should store the type identifier
    const char *ref_stored_type = ar_proxy__get_type(own_proxy);
    AR_ASSERT(ref_stored_type != NULL, "Stored type should not be NULL");
    AR_ASSERT(strcmp(ref_stored_type, "file") == 0,
              "Stored type should be 'file'");

    // Clean up
    ar_proxy__destroy(own_proxy);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_proxy__handle_message_returns_false(void) {
    printf("  test_proxy__handle_message_returns_false...\n");

    // Given a proxy instance and a test message
    ar_proxy_t *own_proxy = ar_proxy__create(NULL, "test");
    AR_ASSERT(own_proxy != NULL, "Proxy creation should succeed");

    ar_data_t *own_message = ar_data__create_string("test message");
    AR_ASSERT(own_message != NULL, "Message creation should succeed");

    // When calling handle_message with no handler set
    int64_t sender_id = 123;
    bool result = ar_proxy__handle_message(own_proxy, own_message, sender_id);

    // Then it should return false (no handler configured)
    AR_ASSERT(result == false, "handle_message should return false when no handler is set");

    // Clean up
    ar_data__destroy(own_message);
    ar_proxy__destroy(own_proxy);

    printf("    PASS\n");
}
