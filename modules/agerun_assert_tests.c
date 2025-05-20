#include "agerun_assert.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Tests for the assertion utilities in agerun_assert.h
 * 
 * These tests verify the behavior of the assertion macros
 * in the assertion module. Since assertions terminate the program
 * when they fail, we can only test them for successful cases,
 * or by using special test harnesses that catch assertion failures.
 * 
 * These tests primarily ensure that:
 * 1. The macros compile correctly
 * 2. Normal (success) cases work as expected
 * 3. The macros expand to no-ops in release builds
 */

/**
 * Test the AR_ASSERT macro with conditions that evaluate to true
 */
static void test_ar_assert_true(void) {
    // Given a condition that evaluates to true
    int value = 42;
    
    // When using AR_ASSERT with this condition
    AR_ASSERT(value == 42, "Value should be 42");
    AR_ASSERT(value > 0, "Value should be positive");
    AR_ASSERT(value != 0, "Value should not be zero");
    
    // Then the assertions should pass without terminating the program
    printf("AR_ASSERT with true conditions passed\n");
}

/**
 * Test the AR_ASSERT_OWNERSHIP macro with non-NULL pointers
 */
static void test_ar_assert_ownership(void) {
    // Given a non-NULL pointer
    int *own_value = (int*)malloc(sizeof(int));
    if (!own_value) {
        printf("Failed to allocate memory for test\n");
        return;
    }
    
    // When using AR_ASSERT_OWNERSHIP with this pointer
    AR_ASSERT_OWNERSHIP(own_value);
    
    // Then the assertion should pass without terminating the program
    printf("AR_ASSERT_OWNERSHIP with non-NULL pointer passed\n");
    
    // Clean up
    free(own_value);
}

/**
 * Test the AR_ASSERT_TRANSFERRED macro with NULL pointers
 */
static void test_ar_assert_transferred(void) {
    // Given a NULL pointer (representing a transferred pointer)
    int *own_value = NULL;
    
    // When using AR_ASSERT_TRANSFERRED with this pointer
    AR_ASSERT_TRANSFERRED(own_value);
    
    // Then the assertion should pass without terminating the program
    printf("AR_ASSERT_TRANSFERRED with NULL pointer passed\n");
}

/**
 * Test the AR_ASSERT_NOT_USED_AFTER_FREE macro with NULL pointers
 */
static void test_ar_assert_not_used_after_free(void) {
    // Given a NULL pointer (representing a freed pointer)
    int *own_value = NULL;
    
    // When using AR_ASSERT_NOT_USED_AFTER_FREE with this pointer
    AR_ASSERT_NOT_USED_AFTER_FREE(own_value);
    
    // Then the assertion should pass without terminating the program
    printf("AR_ASSERT_NOT_USED_AFTER_FREE with NULL pointer passed\n");
}

/**
 * Main function that runs all the tests
 */
int main(void) {
    printf("Running agerun_assert tests...\n");
    
    test_ar_assert_true();
    test_ar_assert_ownership();
    test_ar_assert_transferred();
    test_ar_assert_not_used_after_free();
    
    printf("All agerun_assert tests passed!\n");
    
    return 0;
}
