/**
 * @file ar_instruction_parser_dlsym_tests.c
 * @brief Tests error handling and logging in ar_instruction_parser using dlsym interception
 * 
 * This test verifies that ar_instruction_parser properly logs errors when
 * specialized parser creation fails at any point. It uses dlsym function 
 * interception to simulate failures and verify error logging.
 * 
 * Note: This test is excluded from sanitizer builds because dlsym interception
 * conflicts with sanitizer instrumentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <assert.h>
#include <string.h>

// Include headers for prototypes
#include "ar_instruction_parser.h"
#include "ar_log.h"
#include "ar_assignment_instruction_parser.h"
#include "ar_send_instruction_parser.h"
#include "ar_condition_instruction_parser.h"
#include "ar_parse_instruction_parser.h"
#include "ar_build_instruction_parser.h"
#include "ar_compile_instruction_parser.h"
#include "ar_spawn_instruction_parser.h"
#include "ar_exit_instruction_parser.h"
#include "ar_deprecate_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_heap.h"

// Control which parser creation should fail
static int fail_at_parser = -1;
static int current_parser = 0;

// Track what was created/destroyed for leak detection
static int parsers_created = 0;
static int parsers_destroyed = 0;

// Mock the specialized parser creation functions
ar_assignment_instruction_parser_t* ar_assignment_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing assignment parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created assignment parser (#%d)\n", current_parser);
    return (ar_assignment_instruction_parser_t*)0x1000; // Fake pointer
}

void ar_assignment_instruction_parser__destroy(ar_assignment_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed assignment parser\n");
    }
}

ar_send_instruction_parser_t* ar_send_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing send parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created send parser (#%d)\n", current_parser);
    return (ar_send_instruction_parser_t*)0x2000;
}

void ar_send_instruction_parser__destroy(ar_send_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed send parser\n");
    }
}

ar_condition_instruction_parser_t* ar_condition_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing condition parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created condition parser (#%d)\n", current_parser);
    return (ar_condition_instruction_parser_t*)0x3000;
}

void ar_condition_instruction_parser__destroy(ar_condition_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed condition parser\n");
    }
}

ar_parse_instruction_parser_t* ar_parse_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing parse parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created parse parser (#%d)\n", current_parser);
    return (ar_parse_instruction_parser_t*)0x4000;
}

void ar_parse_instruction_parser__destroy(ar_parse_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed parse parser\n");
    }
}

ar_build_instruction_parser_t* ar_build_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing build parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created build parser (#%d)\n", current_parser);
    return (ar_build_instruction_parser_t*)0x5000;
}

void ar_build_instruction_parser__destroy(ar_build_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed build parser\n");
    }
}

ar_compile_instruction_parser_t* ar_compile_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing compile parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created compile parser (#%d)\n", current_parser);
    return (ar_compile_instruction_parser_t*)0x6000;
}

void ar_compile_instruction_parser__destroy(ar_compile_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed compile parser\n");
    }
}

ar_spawn_instruction_parser_t* ar_spawn_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing spawn parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created spawn parser (#%d)\n", current_parser);
    return (ar_spawn_instruction_parser_t*)0x7000;
}

void ar_spawn_instruction_parser__destroy(ar_spawn_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed spawn parser\n");
    }
}

ar_exit_instruction_parser_t* ar_exit_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing exit parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created exit parser (#%d)\n", current_parser);
    return (ar_exit_instruction_parser_t*)0x8000;
}

void ar_exit_instruction_parser__destroy(ar_exit_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed exit parser\n");
    }
}

ar_deprecate_instruction_parser_t* ar_deprecate_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing deprecate parser creation (#%d)\n", current_parser);
        return NULL;
    }
    parsers_created++;
    printf("  Mock: Created deprecate parser (#%d)\n", current_parser);
    return (ar_deprecate_instruction_parser_t*)0x9000;
}

void ar_deprecate_instruction_parser__destroy(ar_deprecate_instruction_parser_t* parser) {
    if (parser) {
        parsers_destroyed++;
        printf("  Mock: Destroyed deprecate parser\n");
    }
}

// Mock parse functions (not needed for our test but required for linking)
// Assignment has only 2 params
ar_instruction_ast_t* ar_assignment_instruction_parser__parse(
    ar_assignment_instruction_parser_t* parser,
    const char* instruction) {
    (void)parser; (void)instruction;
    return NULL;
}

// These have 3 params with const char* result_path
#define MOCK_PARSE_CONST(type) \
    ar_instruction_ast_t* ar_##type##_instruction_parser__parse( \
        ar_##type##_instruction_parser_t* parser, \
        const char* instruction, \
        const char* result_path) { \
        (void)parser; (void)instruction; (void)result_path; \
        return NULL; \
    }

MOCK_PARSE_CONST(send)
MOCK_PARSE_CONST(condition)
MOCK_PARSE_CONST(parse)
MOCK_PARSE_CONST(build)
MOCK_PARSE_CONST(compile)
MOCK_PARSE_CONST(spawn)
MOCK_PARSE_CONST(exit)
MOCK_PARSE_CONST(deprecate)

// Need to provide malloc/free wrappers for dlsym
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
    current_parser = 0;
    parsers_created = 0;
    parsers_destroyed = 0;
}

static void run_test(const char* test_name, int fail_at, const char* expected_error) {
    printf("\n=== %s ===\n", test_name);
    
    reset_counters();
    fail_at_parser = fail_at;
    
    // Create log to capture error messages
    ar_log_t* log = ar_log__create();
    assert(log != NULL);
    
    // Try to create the instruction parser
    ar_instruction_parser_t* parser = ar_instruction_parser__create(log);
    
    if (parser) {
        printf("Result: SUCCESS - parser created (unexpected)\n");
        ar_instruction_parser__destroy(parser);
    } else {
        printf("Result: FAILED - parser is NULL (expected)\n");
        
        // Verify error was logged
        const char* error_msg = ar_log__get_last_error_message(log);
        if (error_msg && strstr(error_msg, expected_error)) {
            printf("✓ Error logged correctly: %s\n", error_msg);
        } else {
            printf("✗ Expected error not found. Got: %s\n", error_msg ? error_msg : "(null)");
        }
    }
    
    printf("\nCleanup tracking:\n");
    printf("  Parsers: created=%d, destroyed=%d\n", parsers_created, parsers_destroyed);
    
    int leaks = parsers_created - parsers_destroyed;
    if (leaks > 0) {
        printf("  LEAK DETECTED: %d parsers not cleaned up!\n", leaks);
    } else {
        printf("  OK: All parsers properly cleaned up\n");
    }
    
    ar_log__destroy(log);
}

// Test functions following project conventions
static void test_instruction_parser__logs_assignment_parser_failure(void) {
    run_test("Fail at assignment parser (#1)", 1, "Failed to create assignment instruction parser");
}

static void test_instruction_parser__logs_send_parser_failure(void) {
    run_test("Fail at send parser (#2)", 2, "Failed to create send instruction parser");
}

static void test_instruction_parser__logs_condition_parser_failure(void) {
    run_test("Fail at condition parser (#3)", 3, "Failed to create condition instruction parser");
}

static void test_instruction_parser__logs_parse_parser_failure(void) {
    run_test("Fail at parse parser (#4)", 4, "Failed to create parse instruction parser");
}

static void test_instruction_parser__logs_build_parser_failure(void) {
    run_test("Fail at build parser (#5)", 5, "Failed to create build instruction parser");
}

static void test_instruction_parser__logs_compile_parser_failure(void) {
    run_test("Fail at compile parser (#6)", 6, "Failed to create compile instruction parser");
}

static void test_instruction_parser__logs_spawn_parser_failure(void) {
    run_test("Fail at spawn parser (#7)", 7, "Failed to create spawn instruction parser");
}

static void test_instruction_parser__logs_exit_parser_failure(void) {
    run_test("Fail at exit parser (#8)", 8, "Failed to create exit instruction parser");
}

static void test_instruction_parser__logs_deprecate_parser_failure(void) {
    run_test("Fail at deprecate parser (#9)", 9, "Failed to create deprecate instruction parser");
}

static void test_instruction_parser__successful_creation(void) {
    printf("\n=== Successful creation (no failures) ===\n");
    
    reset_counters();
    fail_at_parser = -1; // Don't fail any
    
    ar_log_t* log = ar_log__create();
    assert(log != NULL);
    
    ar_instruction_parser_t* parser = ar_instruction_parser__create(log);
    assert(parser != NULL);
    
    printf("Result: SUCCESS - all parsers created\n");
    printf("  Parsers created: %d\n", parsers_created);
    
    ar_instruction_parser__destroy(parser);
    
    printf("  Parsers destroyed: %d\n", parsers_destroyed);
    assert(parsers_created == parsers_destroyed);
    
    ar_log__destroy(log);
}

int main(void) {
    printf("Running ar_instruction_parser dlsym tests...\n");
    printf("These tests use function interception to simulate parser creation failures.\n");
    
    // Test each parser creation failure
    test_instruction_parser__logs_assignment_parser_failure();
    test_instruction_parser__logs_send_parser_failure();
    test_instruction_parser__logs_condition_parser_failure();
    test_instruction_parser__logs_parse_parser_failure();
    test_instruction_parser__logs_build_parser_failure();
    test_instruction_parser__logs_compile_parser_failure();
    test_instruction_parser__logs_spawn_parser_failure();
    test_instruction_parser__logs_exit_parser_failure();
    test_instruction_parser__logs_deprecate_parser_failure();
    
    // Test successful creation
    test_instruction_parser__successful_creation();
    
    printf("\n=== All dlsym tests completed ===\n");
    return 0;
}