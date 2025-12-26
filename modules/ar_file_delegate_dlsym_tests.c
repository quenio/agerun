/**
 * @file ar_file_delegate_dlsym_tests.c
 * @brief Tests malloc failure handling in ar_file_delegate using dlsym interception
 * 
 * This test verifies that ar_file_delegate__create() properly handles malloc failures
 * and returns NULL without crashing. It uses dlsym function interception to simulate
 * malloc failures at specific points.
 * 
 * Note: This test is excluded from sanitizer builds because dlsym interception
 * conflicts with sanitizer instrumentation.
 * 
 * Following patterns from:
 * - kb/dlsym-test-interception-technique.md
 * - kb/sanitizer-test-exclusion-pattern.md
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include "ar_file_delegate.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include "ar_log.h"

// Control variables for malloc failure injection
static int fail_at_malloc = -1;
static int fail_count = 1;  // Number of consecutive mallocs to fail
static int current_malloc = 0;
static int consecutive_failures = 0;

// dlsym malloc wrapper
void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;

    if (!real_malloc) {
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        real_malloc = converter.func;
    }

    current_malloc++;
    printf("  Mock: malloc #%d called (size=%zu)\n", current_malloc, size);
    
    // Check if we should fail this malloc
    if (fail_at_malloc > 0 && current_malloc >= fail_at_malloc && consecutive_failures < fail_count) {
        printf("  Mock: Failing malloc #%d (consecutive failure %d/%d)\n", 
               current_malloc, consecutive_failures + 1, fail_count);
        consecutive_failures++;
        return NULL;  // Simulate malloc failure
    }

    return real_malloc(size);
}

// Reset counters before each test
static void reset_counters(void) {
    fail_at_malloc = -1;
    fail_count = 1;
    current_malloc = 0;
    consecutive_failures = 0;
}

// Test function declarations
static void test_file_delegate__create_handles_malloc_failure_delegate(void);
static void test_file_delegate__create_handles_malloc_failure_strdup(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_file_delegate_dlsym_tests\n");
            return 1;
        }
    }

    printf("Running file_delegate dlsym tests...\n");
    printf("================================================\n");
    printf("Note: This test uses dlsym to mock malloc\n");
    printf("and is excluded from sanitizer builds.\n");
    printf("================================================\n");

    // Run tests
    test_file_delegate__create_handles_malloc_failure_delegate();
    test_file_delegate__create_handles_malloc_failure_strdup();

    printf("\n================================================\n");
    printf("All file_delegate dlsym tests passed!\n");
    printf("================================================\n");
    return 0;
}

// Test malloc failure at delegate struct allocation
static void test_file_delegate__create_handles_malloc_failure_delegate(void) {
    printf("  test_file_delegate__create_handles_malloc_failure_delegate...\n");

    // ar_heap__malloc retries on failure, so we need to fail both the initial malloc
    // and the retry. The delegate struct is 24 bytes.
    // From debugging: malloc #5 is 24 bytes and happens after #3/#4 failures
    // We need to fail both #5 and #6 (the retry) to truly fail the delegate struct allocation
    
    reset_counters();
    fail_at_malloc = 5;  // Start failing at malloc #5 (delegate struct)
    fail_count = 2;      // Fail 2 consecutive mallocs (#5 and #6) to defeat retry logic

    ar_log_t *ref_log = ar_log__create();

    // When creating a FileDelegate (malloc #3 and #4 will fail)
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Then should return NULL and not crash
    AR_ASSERT(own_delegate == NULL, "Should handle delegate malloc failure");

    // Cleanup
    ar_log__destroy(ref_log);

    printf("    PASS\n");
}

// Test malloc failure at strdup allocation (path string)
static void test_file_delegate__create_handles_malloc_failure_strdup(void) {
    printf("  test_file_delegate__create_handles_malloc_failure_strdup...\n");

    // ar_heap__strdup also retries on failure, so we need to fail both the initial malloc
    // and the retry. strdup happens after delegate struct allocation.
    // From debugging: delegate struct is #5/#6, then ar_delegate__create does allocations,
    // then strdup happens around #8/#9. We need to fail both to defeat retry logic.
    
    reset_counters();
    fail_at_malloc = 8;  // Start failing at malloc #8 (strdup for "/tmp" path)
    fail_count = 2;      // Fail 2 consecutive mallocs (#8 and #9) to defeat retry logic

    ar_log_t *ref_log = ar_log__create();

    // When creating a FileDelegate (strdup malloc will fail)
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp");

    // Then should return NULL and clean up delegate (no memory leak)
    AR_ASSERT(own_delegate == NULL, "Should handle strdup failure and clean up");

    // Cleanup
    ar_log__destroy(ref_log);

    printf("    PASS\n");
}

