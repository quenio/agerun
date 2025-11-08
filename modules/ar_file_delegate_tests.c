#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_file_delegate.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include "ar_log.h"

// Test function declarations
static void test_file_delegate__create_and_destroy(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_file_delegate_tests\n");
            return 1;
        }
    }

    printf("Running file_delegate module tests...\n");

    // Run tests
    test_file_delegate__create_and_destroy();

    printf("All file_delegate tests passed!\n");
    return 0;
}

static void test_file_delegate__create_and_destroy(void) {
    printf("  test_file_delegate__create_and_destroy...\n");

    // Given a log instance for the delegate
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Setup: log should be created");

    // When creating a FileDelegate
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, "/tmp/allowed");

    // Then it should return non-NULL
    AR_ASSERT(own_delegate != NULL, "FileDelegate should be created");

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

