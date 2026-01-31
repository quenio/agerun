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
static int mut_fail_at_malloc = -1;
static int mut_fail_count = 1;  // Number of consecutive mallocs to fail
static int mut_current_malloc = 0;
static int mut_consecutive_failures = 0;
static int mut_resolving_malloc = 0;
static size_t mut_fail_size = 0;
static int mut_fail_size_remaining = 0;

// dlsym malloc wrapper
void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn ref_real_malloc = NULL;

    if (!ref_real_malloc) {
        if (mut_resolving_malloc) {
            return NULL;
        }
        mut_resolving_malloc = 1;
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        ref_real_malloc = converter.func;
        mut_resolving_malloc = 0;
    }

    mut_current_malloc++;
    
    if (mut_fail_size > 0 && size == mut_fail_size && mut_fail_size_remaining > 0) {
        mut_fail_size_remaining--;
        return NULL;
    }

    // Check if we should fail this malloc by count
    if (mut_fail_at_malloc > 0
        && mut_current_malloc >= mut_fail_at_malloc
        && mut_consecutive_failures < mut_fail_count) {
        mut_consecutive_failures++;
        return NULL;  // Simulate malloc failure
    }

    return ref_real_malloc(size);
}

// Reset counters before each test
static void reset_counters(void) {
    mut_fail_at_malloc = -1;
    mut_fail_count = 1;
    mut_current_malloc = 0;
    mut_consecutive_failures = 0;
    mut_resolving_malloc = 0;
    mut_fail_size = 0;
    mut_fail_size_remaining = 0;
}

// Test function declarations
static void test_file_delegate__create_handles_malloc_failure_delegate(void);
static void test_file_delegate__create_handles_malloc_failure_strdup(void);

int main(void) {
    // Directory check
    char mut_cwd[1024];
    if (getcwd(mut_cwd, sizeof(mut_cwd)) != NULL) {
        if (!strstr(mut_cwd, "/bin/") && !strstr(mut_cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", mut_cwd);
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
    mut_fail_at_malloc = 1;  // Fail the first malloc in ar_heap__malloc for the delegate struct
    mut_fail_count = 2;      // Fail 2 consecutive mallocs (#5 and #6) to defeat retry logic

    ar_log_t *ref_log = NULL;

    // When creating a FileDelegate (delegate struct allocation will fail)
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp", 0);

    // Then should return NULL and not crash
    AR_ASSERT(own_delegate == NULL, "Should handle delegate malloc failure");

    // Cleanup
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
    mut_fail_size = strlen("/tmp") + 1;
    mut_fail_size_remaining = 2;  // Fail both strdup attempts for the allowed path

    ar_log_t *ref_log = NULL;

    // When creating a FileDelegate (strdup malloc will fail)
    ar_file_delegate_t *own_delegate = ar_file_delegate__create(ref_log, "/tmp", 0);

    // Then should return NULL and clean up delegate (no memory leak)
    AR_ASSERT(own_delegate == NULL, "Should handle strdup failure and clean up");

    // Cleanup
    printf("    PASS\n");
}
