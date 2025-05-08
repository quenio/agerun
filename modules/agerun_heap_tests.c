#include "agerun_heap.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Only needed functions in debug mode
#ifdef DEBUG
static int test_counter = 0;
static int passed_tests = 0;

static void start_test(const char *test_name) {
    printf("  Running test: %s...\n", test_name);
    test_counter++;
}

static void pass_test(const char *test_name) {
    printf("  Test passed: %s\n", test_name);
    passed_tests++;
}

/**
 * Test basic memory allocation and freeing with tracking
 * 
 * This test verifies that:
 * 1. Memory can be allocated using AR_MALLOC
 * 2. The allocated memory can be used normally
 * 3. The memory can be freed with AR_FREE
 * 4. The pointer can be set to NULL after freeing
 * 5. The AR_ASSERT_NOT_USED_AFTER_FREE macro correctly verifies the NULL state
 */
static void test_malloc_tracking(void) {
    start_test("test_malloc_tracking");
    
    // Allocate some memory with tracking
    char *own_buffer = AR_MALLOC(1024, "Test buffer");
    assert(own_buffer != NULL);
    
    // Use the memory
    memset(own_buffer, 'A', 1024);
    
    // Free the memory with tracking
    AR_FREE(own_buffer);
    own_buffer = NULL;
    
    // Verify that it's marked as freed
    AR_ASSERT_NOT_USED_AFTER_FREE(own_buffer);
    
    pass_test("test_malloc_tracking");
}

/**
 * Test AR_CALLOC and verify zero initialization
 */
static void test_calloc_tracking(void) {
    start_test("test_calloc_tracking");
    
    // Allocate some memory with tracking
    int *own_array = AR_CALLOC(10, sizeof(int), "Test array");
    assert(own_array != NULL);
    
    // Verify zero initialization
    for (int i = 0; i < 10; i++) {
        assert(own_array[i] == 0);
    }
    
    // Free the memory with tracking
    AR_FREE(own_array);
    own_array = NULL;
    
    // Verify that it's marked as freed
    AR_ASSERT_NOT_USED_AFTER_FREE(own_array);
    
    pass_test("test_calloc_tracking");
}

/**
 * Test AR_STRDUP
 */
static void test_strdup_tracking(void) {
    start_test("test_strdup_tracking");
    
    const char *original = "Hello, World!";
    
    // Duplicate the string with tracking
    char *own_copy = AR_STRDUP(original, "Test string");
    assert(own_copy != NULL);
    
    // Verify the copy
    assert(strcmp(own_copy, original) == 0);
    
    // Free the memory with tracking
    AR_FREE(own_copy);
    own_copy = NULL;
    
    // Verify that it's marked as freed
    AR_ASSERT_NOT_USED_AFTER_FREE(own_copy);
    
    pass_test("test_strdup_tracking");
}

/**
 * Test AR_REALLOC
 */
static void test_realloc_tracking(void) {
    start_test("test_realloc_tracking");
    
    // Allocate some memory with tracking
    char *own_buffer = AR_MALLOC(10, "Initial buffer");
    assert(own_buffer != NULL);
    
    // Fill it with data
    memset(own_buffer, 'A', 10);
    
    // Reallocate with tracking
    own_buffer = AR_REALLOC(own_buffer, 20, "Expanded buffer");
    assert(own_buffer != NULL);
    
    // Verify the data is preserved
    for (int i = 0; i < 10; i++) {
        assert(own_buffer[i] == 'A');
    }
    
    // Fill the new space
    memset(own_buffer + 10, 'B', 10);
    
    // Free the memory with tracking
    AR_FREE(own_buffer);
    own_buffer = NULL;
    
    // Verify that it's marked as freed
    AR_ASSERT_NOT_USED_AFTER_FREE(own_buffer);
    
    pass_test("test_realloc_tracking");
}

/**
 * Test assertion macros for ownership
 */
static void test_ownership_assertions(void) {
    start_test("test_ownership_assertions");
    
    // Test AR_ASSERT_OWNERSHIP
    void *own_ptr = AR_MALLOC(10, "Ownership test");
    AR_ASSERT_OWNERSHIP(own_ptr);
    
    // Test AR_ASSERT_TRANSFERRED after setting to NULL
    AR_FREE(own_ptr);
    own_ptr = NULL;
    AR_ASSERT_TRANSFERRED(own_ptr);
    
    // AR_ASSERT_NOT_USED_AFTER_FREE should work with NULL pointer
    AR_ASSERT_NOT_USED_AFTER_FREE(own_ptr);
    
    pass_test("test_ownership_assertions");
}

/**
 * Test memory leak detection and reporting
 * 
 * This test verifies the leak detection capability of the heap module by
 * intentionally creating a memory leak. The test passes if the allocation
 * is correctly tracked, but the memory leak itself should appear in the
 * heap_memory_report.log file generated at program exit.
 * 
 * Note: This test is essential for verifying the leak detection system works correctly.
 * The intentional leak is a necessary part of the test suite and should not be "fixed"
 * as it would eliminate the test case for the leak detection capability.
 */
static void test_leak_reporting(void) {
    start_test("test_leak_reporting");
    
    // Intentionally leak memory to test the reporting
    char *leaked_buffer = AR_MALLOC(1024, "Intentional leak for testing");
    memset(leaked_buffer, 0, 1024);
    
    // Confirm memory was allocated
    AR_ASSERT_OWNERSHIP(leaked_buffer);
    
    // Do not free this memory to test leak detection
    // The memory report at program exit should show this leak
    
    printf("  Note: This test intentionally leaks memory to test the leak detection mechanism.\n");
    printf("  Check heap_memory_report.log after the tests complete.\n");
    
    pass_test("test_leak_reporting");
}

/**
 * Test memory report generation
 */
static void test_memory_report(void) {
    start_test("test_memory_report");
    
    // Allocate and free some memory to have tracking records
    for (int i = 0; i < 5; i++) {
        char desc[64];
        snprintf(desc, sizeof(desc), "Test allocation %d", i);
        
        void *ptr = AR_MALLOC(100, desc);
        AR_FREE(ptr);
    }
    
    // Manually generate a report
    ar_heap_memory_report();
    
    // Check if the report file exists
    FILE *report = fopen("heap_memory_report.log", "r");
    assert(report != NULL);
    fclose(report);
    
    pass_test("test_memory_report");
}

/**
 * Main entry point for the heap module test suite
 * 
 * This test suite verifies the functionality of the heap module:
 * - Basic memory allocation tracking (malloc, calloc, realloc, strdup)
 * - Memory deallocation tracking
 * - Ownership assertion macros
 * - Memory leak detection and reporting
 * 
 * Note that the test_leak_reporting test intentionally creates a memory leak
 * to verify the leak detection system. This leak should appear in the
 * heap_memory_report.log file after tests complete.
 * 
 * @return 0 if all tests pass, 1 if any test fails
 */
int main(void) {
    printf("Starting Heap Module Tests...\n");
    
    // Basic memory allocation tests
    test_malloc_tracking();
    test_calloc_tracking();
    test_strdup_tracking();
    test_realloc_tracking();
    
    // Ownership assertion tests
    test_ownership_assertions();
    
    // Memory leak detection tests
    test_leak_reporting();
    
    // Report generation test
    test_memory_report();
    
    // Print summary
    printf("\nHeap Module Tests: %d of %d tests passed.\n", passed_tests, test_counter);
    
    // The leak reporting test intentionally leaks memory
    // The leak should be detected by the memory report that's generated at exit
    
    return (passed_tests == test_counter) ? 0 : 1;
}
#else
// In release mode, just provide a minimal test
int main(void) {
    printf("Heap Module Tests skipped - DEBUG not defined\n");
    printf("Recompile with -DDEBUG to run the tests\n");
    return 0;
}
#endif /* DEBUG */
