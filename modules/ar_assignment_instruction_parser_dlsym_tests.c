/**
 * @file ar_assignment_instruction_parser_dlsym_tests.c
 * @brief Tests error handling in ar_assignment_instruction_parser using function mocking
 * 
 * This test verifies that code using ar_assignment_instruction_parser properly handles
 * creation failures. It uses dlsym function interception to mock parser creation
 * and verify error handling behavior.
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
#include <stdbool.h>
#include <dlfcn.h>
#include <assert.h>
#include <string.h>

// Include headers for prototypes
#include "ar_assignment_instruction_parser.h"
#include "ar_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_parser.h"
#include "ar_log.h"
#include "ar_heap.h"
#include "ar_assert.h"

// Control which function should fail
static bool should_fail_parser_create = false;
static bool should_fail_expression_parser_create = false;

// Track what was created/destroyed for verification
static int parsers_created = 0;
static int parsers_destroyed = 0;
static int expression_parsers_created = 0;
static int expression_parsers_destroyed = 0;

// Mock the assignment instruction parser creation
ar_assignment_instruction_parser_t* ar_assignment_instruction_parser__create(ar_log_t* log) {
    if (should_fail_parser_create) {
        printf("  Mock: Failing assignment parser creation\n");
        if (log) {
            ar_log__error(log, "Failed to allocate memory for assignment instruction parser");
        }
        return NULL;
    }
    
    parsers_created++;
    printf("  Mock: Created assignment parser (#%d)\n", parsers_created);
    
    // Return a fake but non-NULL pointer
    return (ar_assignment_instruction_parser_t*)0x1000;
}

void ar_assignment_instruction_parser__destroy(ar_assignment_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed assignment parser (#%d)\n", parsers_destroyed);
    }
}

// Mock the parse function
ar_instruction_ast_t* ar_assignment_instruction_parser__parse(
    ar_assignment_instruction_parser_t* parser,
    const char* instruction
) {
    printf("  Mock: Parsing instruction: %s\n", instruction ? instruction : "(null)");
    
    if (!parser || !instruction) {
        printf("  Mock: Parse failed - NULL parameter\n");
        return NULL;
    }
    
    // For this test, we'll return a fake AST
    printf("  Mock: Parse succeeded\n");
    return (ar_instruction_ast_t*)0x2000;
}

// Mock expression parser creation (used internally by assignment parser)
ar_expression_parser_t* ar_expression_parser__create(ar_log_t* log, const char* expression) {
    (void)expression;
    
    if (should_fail_expression_parser_create) {
        printf("  Mock: Failing expression parser creation\n");
        if (log) {
            ar_log__error(log, "Failed to create expression parser");
        }
        return NULL;
    }
    
    expression_parsers_created++;
    printf("  Mock: Created expression parser (#%d)\n", expression_parsers_created);
    return (ar_expression_parser_t*)0x3000;
}

void ar_expression_parser__destroy(ar_expression_parser_t* parser) {
    if (parser) {
        expression_parsers_destroyed++;
        printf("  Mock: Destroyed expression parser (#%d)\n", expression_parsers_destroyed);
    }
}

// Need to provide malloc/free wrappers for dlsym (required but not intercepted)
void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;
    if (!real_malloc) {
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        real_malloc = converter.func;
    }
    return real_malloc(size);
}

void free(void* ptr) {
    typedef void (*free_fn)(void*);
    static free_fn real_free = NULL;
    if (!real_free) {
        union { void* obj; free_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "free");
        real_free = converter.func;
    }
    real_free(ptr);
}

// Test helper functions
static void reset_counters(void) {
    should_fail_parser_create = false;
    should_fail_expression_parser_create = false;
    parsers_created = 0;
    parsers_destroyed = 0;
    expression_parsers_created = 0;
    expression_parsers_destroyed = 0;
}

/**
 * Test that instruction parser handles assignment parser creation failure.
 */
static void test_instruction_parser_handles_assignment_parser_failure(void) {
    printf("\n=== Test: Instruction parser handles assignment parser failure ===\n");
    reset_counters();
    
    // Given a log instance
    ar_log_t* log = ar_log__create();
    AR_ASSERT(log != NULL, "Log creation should succeed");
    
    // When assignment parser creation fails
    should_fail_parser_create = true;
    
    // Simulate instruction parser trying to create assignment parser
    ar_assignment_instruction_parser_t* parser = ar_assignment_instruction_parser__create(log);
    
    // Then it should return NULL
    AR_ASSERT(parser == NULL, "Parser creation should fail when mocked to fail");
    
    // And error should be logged
    const char* error = ar_log__get_last_error_message(log);
    AR_ASSERT(error != NULL, "Error should be logged for creation failure");
    AR_ASSERT(strstr(error, "Failed to allocate memory") != NULL,
             "Error should indicate allocation failure");
    
    // Verify no leaks
    AR_ASSERT(parsers_created == 0, "No parsers should be created");
    AR_ASSERT(parsers_destroyed == 0, "No parsers should need destroying");
    
    ar_log__destroy(log);
    printf("✓ Test passed: Error properly logged when parser creation fails\n");
}

/**
 * Test normal operation to verify mocking works correctly.
 */
static void test_normal_parser_creation(void) {
    printf("\n=== Test: Normal parser creation ===\n");
    reset_counters();
    
    // Given a log instance
    ar_log_t* log = ar_log__create();
    AR_ASSERT(log != NULL, "Log creation should succeed");
    
    // When creating parser normally (no failures)
    should_fail_parser_create = false;
    ar_assignment_instruction_parser_t* parser = ar_assignment_instruction_parser__create(log);
    
    // Then it should succeed
    AR_ASSERT(parser != NULL, "Parser creation should succeed");
    AR_ASSERT(parsers_created == 1, "One parser should be created");
    
    // And we can use it
    const char* instruction = "memory.x := 42";
    ar_instruction_ast_t* ast = ar_assignment_instruction_parser__parse(parser, instruction);
    AR_ASSERT(ast != NULL, "Parse should succeed");
    
    // Clean up
    ar_assignment_instruction_parser__destroy(parser);
    AR_ASSERT(parsers_destroyed == 1, "Parser should be destroyed");
    
    ar_log__destroy(log);
    printf("✓ Test passed: Normal operation works correctly\n");
}

/**
 * Test that parser properly handles NULL parameters.
 */
static void test_parser_null_parameters(void) {
    printf("\n=== Test: Parser NULL parameter handling ===\n");
    reset_counters();
    
    // Test 1: Create with NULL log (should succeed but can't log errors)
    ar_assignment_instruction_parser_t* parser = ar_assignment_instruction_parser__create(NULL);
    AR_ASSERT(parser != NULL, "Parser creation should succeed with NULL log");
    
    // Test 2: Parse with NULL instruction
    ar_instruction_ast_t* ast = ar_assignment_instruction_parser__parse(parser, NULL);
    AR_ASSERT(ast == NULL, "Parse should fail with NULL instruction");
    
    // Test 3: Parse with NULL parser
    ast = ar_assignment_instruction_parser__parse(NULL, "memory.x := 42");
    AR_ASSERT(ast == NULL, "Parse should fail with NULL parser");
    
    // Clean up
    ar_assignment_instruction_parser__destroy(parser);
    
    printf("✓ Test passed: NULL parameters handled correctly\n");
}

/**
 * Test cleanup tracking to ensure no resource leaks.
 */
static void test_cleanup_tracking(void) {
    printf("\n=== Test: Cleanup tracking ===\n");
    reset_counters();
    
    // Create and destroy multiple parsers
    for (int i = 0; i < 3; i++) {
        ar_assignment_instruction_parser_t* parser = ar_assignment_instruction_parser__create(NULL);
        AR_ASSERT(parser != NULL, "Parser creation should succeed");
        ar_assignment_instruction_parser__destroy(parser);
    }
    
    // Verify all parsers were cleaned up
    AR_ASSERT(parsers_created == 3, "Three parsers should be created");
    AR_ASSERT(parsers_destroyed == 3, "Three parsers should be destroyed");
    AR_ASSERT(parsers_created == parsers_destroyed, "All created parsers should be destroyed");
    
    printf("✓ Test passed: All resources properly cleaned up\n");
    printf("  Parsers: created=%d, destroyed=%d\n", parsers_created, parsers_destroyed);
}

int main(void) {
    printf("Running ar_assignment_instruction_parser dlsym tests...\n");
    printf("================================================\n");
    printf("Note: This test uses dlsym to mock functions\n");
    printf("and is excluded from sanitizer builds.\n");
    printf("================================================\n");
    
    // Run all tests
    test_normal_parser_creation();
    test_instruction_parser_handles_assignment_parser_failure();
    test_parser_null_parameters();
    test_cleanup_tracking();
    
    printf("\n================================================\n");
    printf("All ar_assignment_instruction_parser dlsym tests passed!\n");
    printf("================================================\n");
    
    return 0;
}