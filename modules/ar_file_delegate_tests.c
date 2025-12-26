#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_file_delegate.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include "ar_log.h"

// Test function declarations
static void test_file_delegate__create_returns_non_null(void);
static void test_file_delegate__create_handles_null_log(void);
static void test_file_delegate__create_handles_null_path(void);
static void test_file_delegate__get_type(void);
static void test_file_delegate__destroy_cleans_up(void);
static void test_file_delegate__destroy_handles_null(void);

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
    test_file_delegate__create_returns_non_null();
    test_file_delegate__create_handles_null_log();
    test_file_delegate__create_handles_null_path();
    test_file_delegate__get_type();
    test_file_delegate__destroy_cleans_up();
    test_file_delegate__destroy_handles_null();

    printf("All file_delegate tests passed!\n");
    return 0;
}

static void test_file_delegate__create_returns_non_null(void) {
    printf("  test_file_delegate__create_returns_non_null...\n");

    // Given a log instance for the delegate
    ar_log_t *own_log = ar_log__create();

    // When creating a FileDelegate
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, "/tmp/allowed");

    // Then it should return non-NULL
    AR_ASSERT(own_delegate != NULL, "FileDelegate should be created");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__create_handles_null_log(void) {
    printf("  test_file_delegate__create_handles_null_log...\n");

    // Given a NULL log parameter (log module can handle NULL)
    ar_log_t *ref_log = NULL;

    // When creating a FileDelegate with NULL log
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp/allowed");

    // Then it should succeed (log module handles NULL gracefully)
    AR_ASSERT(own_delegate != NULL, "Should accept NULL log");  // ← FAILS (NULL check rejects it)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);

    printf("    PASS\n");
}

static void test_file_delegate__create_handles_null_path(void) {
    printf("  test_file_delegate__create_handles_null_path...\n");

    // Given a log instance and a NULL path parameter
    ar_log_t *own_log = ar_log__create();

    // When creating a FileDelegate with NULL path
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, NULL);

    // Then it should return NULL
    AR_ASSERT(own_delegate == NULL, "Should reject NULL path");  // ← FAILS (no NULL check)

    // Cleanup
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__get_type(void) {
    printf("  test_file_delegate__get_type...\n");

    // Given a FileDelegate instance
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, "/tmp/allowed");

    // When getting the delegate type
    const char *ref_type = ar_file_delegate__get_type(own_delegate);

    // Then it should return "file"
    AR_ASSERT(strcmp(ref_type, "file") == 0, "Type should be 'file'");  // ← FAILS (stub returns NULL)

    // Cleanup
    ar_file_delegate__destroy(own_delegate);
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__destroy_cleans_up(void) {
    printf("  test_file_delegate__destroy_cleans_up...\n");

    // Given a FileDelegate instance
    ar_log_t *own_log = ar_log__create();
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(own_log, "/tmp/allowed");
    AR_ASSERT(own_delegate != NULL, "Setup: delegate created");

    // When destroying the delegate
    ar_file_delegate__destroy(own_delegate);

    // Then no memory should leak (verified by memory report)
    // No explicit assertion needed - memory leak check validates this

    // Cleanup
    ar_log__destroy(own_log);

    printf("    PASS\n");
}

static void test_file_delegate__destroy_handles_null(void) {
    printf("  test_file_delegate__destroy_handles_null...\n");

    // Given a NULL delegate
    ar_file_delegate_t *own_delegate = NULL;

    // When destroying NULL delegate
    ar_file_delegate__destroy(own_delegate);

    // Then it should not crash (no assertion needed - just shouldn't crash)
    // Success = no segfault

    printf("    PASS\n");
}

