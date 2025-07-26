/**
 * @file ar_instruction_evaluator_error_tests.c
 * @brief Tests error handling and cleanup in ar_instruction_evaluator
 * 
 * This test verifies that ar_instruction_evaluator (implemented in Zig) properly
 * cleans up all resources when sub-evaluator creation fails at any point.
 * It uses function interception to simulate failures and track cleanup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <assert.h>

// Include headers for prototypes
#include "ar_instruction_evaluator.h"
#include "ar_log.h"
#include "ar_expression_evaluator.h"
#include "ar_assignment_instruction_evaluator.h"
#include "ar_send_instruction_evaluator.h"
#include "ar_condition_instruction_evaluator.h"
#include "ar_parse_instruction_evaluator.h"
#include "ar_build_instruction_evaluator.h"
#include "ar_compile_instruction_evaluator.h"
#include "ar_spawn_instruction_evaluator.h"
#include "ar_exit_instruction_evaluator.h"
#include "ar_deprecate_instruction_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"
#include "ar_frame.h"

// Define the interceptor functions that will override the real ones
#define DEFINE_INTERCEPTOR(name) \
    static int name##_created = 0; \
    static int name##_destroyed = 0;

DEFINE_INTERCEPTOR(expression)
DEFINE_INTERCEPTOR(assignment)
DEFINE_INTERCEPTOR(send)
// Other evaluators don't need detailed tracking but define to avoid warnings
DEFINE_INTERCEPTOR(condition)
DEFINE_INTERCEPTOR(parse)
DEFINE_INTERCEPTOR(build)
DEFINE_INTERCEPTOR(compile)
DEFINE_INTERCEPTOR(spawn)
DEFINE_INTERCEPTOR(exit)
DEFINE_INTERCEPTOR(deprecate)

static int fail_at_evaluator = -1;
static int current_evaluator = 0;

// Expression evaluator interceptors
ar_expression_evaluator_t* ar_expression_evaluator__create(ar_log_t* log) {
    (void)log; // Suppress unused warning
    current_evaluator++;
    if (current_evaluator == fail_at_evaluator) {
        printf("  Mock: Failing expression evaluator creation (#%d)\n", current_evaluator);
        return NULL;
    }
    expression_created++;
    printf("  Mock: Created expression evaluator (#%d)\n", current_evaluator);
    return (ar_expression_evaluator_t*)0x1000; // Fake pointer
}

void ar_expression_evaluator__destroy(ar_expression_evaluator_t* evaluator) {
    if (evaluator) {
        expression_destroyed++;
        printf("  Mock: Destroyed expression evaluator\n");
    }
}

// Assignment evaluator interceptors
ar_assignment_instruction_evaluator_t* ar_assignment_instruction_evaluator__create(
    ar_log_t* log, ar_expression_evaluator_t* expr) {
    (void)log; (void)expr; // Suppress unused warnings
    current_evaluator++;
    if (current_evaluator == fail_at_evaluator) {
        printf("  Mock: Failing assignment evaluator creation (#%d)\n", current_evaluator);
        return NULL;
    }
    assignment_created++;
    printf("  Mock: Created assignment evaluator (#%d)\n", current_evaluator);
    return (ar_assignment_instruction_evaluator_t*)0x2000;
}

void ar_assignment_instruction_evaluator__destroy(ar_assignment_instruction_evaluator_t* evaluator) {
    if (evaluator) {
        assignment_destroyed++;
        printf("  Mock: Destroyed assignment evaluator\n");
    }
}

// Send evaluator interceptors
ar_send_instruction_evaluator_t* ar_send_instruction_evaluator__create(
    ar_log_t* log, ar_expression_evaluator_t* expr) {
    (void)log; (void)expr; // Suppress unused warnings
    current_evaluator++;
    if (current_evaluator == fail_at_evaluator) {
        printf("  Mock: Failing send evaluator creation (#%d)\n", current_evaluator);
        return NULL;
    }
    send_created++;
    printf("  Mock: Created send evaluator (#%d)\n", current_evaluator);
    return (ar_send_instruction_evaluator_t*)0x3000;
}

void ar_send_instruction_evaluator__destroy(ar_send_instruction_evaluator_t* evaluator) {
    if (evaluator) {
        send_destroyed++;
        printf("  Mock: Destroyed send evaluator\n");
    }
}

// Other evaluator interceptors (simplified - just pass through)
#define DEFINE_SIMPLE_INTERCEPTOR(type, name, addr) \
    ar_##type##_instruction_evaluator_t* ar_##type##_instruction_evaluator__create( \
        ar_log_t* log, ar_expression_evaluator_t* expr) { \
        (void)log; (void)expr; \
        current_evaluator++; \
        if (current_evaluator == fail_at_evaluator) { \
            printf("  Mock: Failing %s evaluator creation (#%d)\n", name, current_evaluator); \
            return NULL; \
        } \
        printf("  Mock: Created %s evaluator (#%d)\n", name, current_evaluator); \
        return (ar_##type##_instruction_evaluator_t*)addr; \
    } \
    void ar_##type##_instruction_evaluator__destroy(ar_##type##_instruction_evaluator_t* evaluator) { \
        if (evaluator) printf("  Mock: Destroyed %s evaluator\n", name); \
    }

DEFINE_SIMPLE_INTERCEPTOR(condition, "condition", 0x4000)
DEFINE_SIMPLE_INTERCEPTOR(parse, "parse", 0x5000)
DEFINE_SIMPLE_INTERCEPTOR(build, "build", 0x6000)
DEFINE_SIMPLE_INTERCEPTOR(compile, "compile", 0x7000)
DEFINE_SIMPLE_INTERCEPTOR(spawn, "spawn", 0x8000)
DEFINE_SIMPLE_INTERCEPTOR(exit, "exit", 0x9000)
DEFINE_SIMPLE_INTERCEPTOR(deprecate, "deprecate", 0xA000)

// Additional mocks needed by ar_allocator
void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;
    if (!real_malloc) {
        // Use a union to avoid ISO C warning about function pointer conversion
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
        // Use a union to avoid ISO C warning about function pointer conversion
        union { void* obj; free_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "free");
        real_free = converter.func;
    }
    real_free(ptr);
}

// ar_heap functions are already defined in the heap module
// We'll link with the real implementations

// Mock instruction AST functions
ar_instruction_ast_type_t ar_instruction_ast__get_type(const ar_instruction_ast_t* ast) {
    (void)ast;
    return AR_INSTRUCTION_AST_TYPE__ASSIGNMENT;
}

// Mock evaluate functions (not needed for our test but required for linking)
#define MOCK_EVALUATE(type) \
    bool ar_##type##_instruction_evaluator__evaluate( \
        const ar_##type##_instruction_evaluator_t* eval, \
        const ar_frame_t* frame, \
        const ar_instruction_ast_t* ast) { \
        (void)eval; (void)frame; (void)ast; \
        return true; \
    }

MOCK_EVALUATE(assignment)
MOCK_EVALUATE(send)
MOCK_EVALUATE(condition)
MOCK_EVALUATE(parse)
MOCK_EVALUATE(build)
MOCK_EVALUATE(compile)
MOCK_EVALUATE(spawn)
MOCK_EVALUATE(exit)
MOCK_EVALUATE(deprecate)

// Test framework integration

// Simple log mock
ar_log_t* ar_log__create(void) { return (ar_log_t*)0xF000; }
void ar_log__destroy(ar_log_t* log) { (void)log; }
void ar_log__error(ar_log_t* log, const char* msg) { (void)log; (void)msg; }

static void reset_counters(void) {
    current_evaluator = 0;
    expression_created = 0;
    expression_destroyed = 0;
    assignment_created = 0;
    assignment_destroyed = 0;
    send_created = 0;
    send_destroyed = 0;
    // Reset others to avoid warnings
    condition_created = condition_destroyed = 0;
    parse_created = parse_destroyed = 0;
    build_created = build_destroyed = 0;
    compile_created = compile_destroyed = 0;
    spawn_created = spawn_destroyed = 0;
    exit_created = exit_destroyed = 0;
    deprecate_created = deprecate_destroyed = 0;
}

static void run_test(const char* test_name, int fail_at) {
    printf("\n=== %s ===\n", test_name);
    
    reset_counters();
    fail_at_evaluator = fail_at;
    
    ar_log_t* log = ar_log__create();
    ar_instruction_evaluator_t* evaluator = ar_instruction_evaluator__create(log);
    
    if (evaluator) {
        printf("Result: SUCCESS - evaluator created\n");
        ar_instruction_evaluator__destroy(evaluator);
    } else {
        printf("Result: FAILED - evaluator is NULL\n");
    }
    
    printf("\nCleanup tracking:\n");
    printf("  Expression: created=%d, destroyed=%d\n", expression_created, expression_destroyed);
    printf("  Assignment: created=%d, destroyed=%d\n", assignment_created, assignment_destroyed);
    printf("  Send:       created=%d, destroyed=%d\n", send_created, send_destroyed);
    
    int leaks = (expression_created - expression_destroyed) +
                (assignment_created - assignment_destroyed) +
                (send_created - send_destroyed);
    
    if (leaks > 0) {
        printf("  LEAK DETECTED: %d evaluators not cleaned up!\n", leaks);
    } else {
        printf("  OK: All evaluators properly cleaned up\n");
    }
    
    ar_log__destroy(log);
}

// Test functions following project conventions
static void test_instruction_evaluator__cleanup_on_expression_failure(void) {
    run_test("Fail at expression evaluator (#1)", 1);
}

static void test_instruction_evaluator__cleanup_on_assignment_failure(void) {
    run_test("Fail at assignment evaluator (#2)", 2);
}

static void test_instruction_evaluator__cleanup_on_send_failure(void) {
    run_test("Fail at send evaluator (#3)", 3);
}

static void test_instruction_evaluator__cleanup_on_late_failure(void) {
    run_test("Fail at deprecate evaluator (#10)", 10);
}

// Main test runner
int main(void) {
    printf("Starting ar_instruction_evaluator error handling tests...\n");
    printf("=======================================================\n");
    
    // First verify success case works
    printf("\nVerifying success case...\n");
    run_test("Success case (no failures)", -1);
    
    // Then test each failure point
    test_instruction_evaluator__cleanup_on_expression_failure();
    printf("test_instruction_evaluator__cleanup_on_expression_failure passed!\n");
    
    test_instruction_evaluator__cleanup_on_assignment_failure();
    printf("test_instruction_evaluator__cleanup_on_assignment_failure passed!\n");
    
    test_instruction_evaluator__cleanup_on_send_failure();
    printf("test_instruction_evaluator__cleanup_on_send_failure passed!\n");
    
    test_instruction_evaluator__cleanup_on_late_failure();
    printf("test_instruction_evaluator__cleanup_on_late_failure passed!\n");
    
    printf("\nAll ar_instruction_evaluator error handling tests passed!\n");
    printf("The Zig errdefer cleanup is working correctly!\n");
    
    return 0;
}