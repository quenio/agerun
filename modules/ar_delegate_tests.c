#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_delegate.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include "ar_log.h"
#include "ar_data.h"

// Test function declarations
static void test_delegate__create_and_destroy(void);
static void test_delegate__stores_log_and_type(void);
static void test_delegate__handle_message_returns_false(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_delegate_tests\n");
            return 1;
        }
    }

    printf("Running delegate module tests...\n");

    // Run tests
    test_delegate__create_and_destroy();
    test_delegate__stores_log_and_type();
    test_delegate__handle_message_returns_false();

    printf("All delegate tests passed!\n");
    return 0;
}

static void test_delegate__create_and_destroy(void) {
    printf("  test_delegate__create_and_destroy...\n");

    // Given no prerequisites

    // When creating a delegate with NULL parameters
    ar_delegate_t *own_delegate = ar_delegate__create(NULL, NULL);

    // Then the delegate should be created successfully
    AR_ASSERT(own_delegate != NULL, "Delegate creation should succeed with NULL parameters");

    // Clean up
    ar_delegate__destroy(own_delegate);

    printf("    PASS\n");
}

static void test_delegate__stores_log_and_type(void) {
    printf("  test_delegate__stores_log_and_type...\n");

    // Given a log instance and delegate type
    ar_log_t *own_log = ar_log__create();
    const char *type = "file";

    // When creating a delegate with log and type
    ar_delegate_t *own_delegate = ar_delegate__create(own_log, type);
    AR_ASSERT(own_delegate != NULL, "Delegate creation should succeed");

    // Then the delegate should store the log reference
    ar_log_t *ref_stored_log = ar_delegate__get_log(own_delegate);
    AR_ASSERT(ref_stored_log == own_log,
              "Stored log should match the log passed to create");

    // Then the delegate should store the type identifier
    const char *ref_stored_type = ar_delegate__get_type(own_delegate);
    AR_ASSERT(ref_stored_type != NULL, "Stored type should not be NULL");
    AR_ASSERT(strcmp(ref_stored_type, "file") == 0,
              "Stored type should be 'file'");

    // Clean up
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_delegate__handle_message_returns_false(void) {
    printf("  test_delegate__handle_message_returns_false...\n");

    // Given a delegate instance and a test message
    ar_delegate_t *own_delegate = ar_delegate__create(NULL, "test");
    AR_ASSERT(own_delegate != NULL, "Delegate creation should succeed");

    ar_data_t *own_message = ar_data__create_string("test message");
    AR_ASSERT(own_message != NULL, "Message creation should succeed");

    // When calling handle_message with no handler set
    int64_t sender_id = 123;
    bool result = ar_delegate__handle_message(own_delegate, own_message, sender_id);

    // Then it should return false (no handler configured)
    AR_ASSERT(result == false, "handle_message should return false when no handler is set");

    // Clean up
    ar_data__destroy(own_message);
    ar_delegate__destroy(own_delegate);

    printf("    PASS\n");
}
