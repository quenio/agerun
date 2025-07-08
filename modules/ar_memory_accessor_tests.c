/**
 * @file ar_memory_accessor_tests.c
 * @brief Tests for the memory accessor module
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_memory_accessor.h"
#include "ar_heap.h"

static void test_memory_accessor__null_input(void) {
    // Given a NULL input
    const char *ref_path = NULL;
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return NULL
    assert(result == NULL);
}

static void test_memory_accessor__non_memory_path(void) {
    // Given a path that doesn't start with "memory."
    const char *ref_path = "context.user.name";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return NULL
    assert(result == NULL);
}

static void test_memory_accessor__simple_memory_path(void) {
    // Given a simple memory path
    const char *ref_path = "memory.x";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return "x"
    assert(result != NULL);
    assert(strcmp(result, "x") == 0);
}

static void test_memory_accessor__nested_memory_path(void) {
    // Given a nested memory path
    const char *ref_path = "memory.user.name";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return "user.name"
    assert(result != NULL);
    assert(strcmp(result, "user.name") == 0);
}

static void test_memory_accessor__deeply_nested_memory_path(void) {
    // Given a deeply nested memory path
    const char *ref_path = "memory.a.b.c.d.e";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return "a.b.c.d.e"
    assert(result != NULL);
    assert(strcmp(result, "a.b.c.d.e") == 0);
}

static void test_memory_accessor__memory_only(void) {
    // Given just "memory" without a dot
    const char *ref_path = "memory";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return NULL (not a valid memory path)
    assert(result == NULL);
}

static void test_memory_accessor__memory_with_trailing_dot(void) {
    // Given "memory." with no key
    const char *ref_path = "memory.";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return an empty string
    assert(result != NULL);
    assert(strcmp(result, "") == 0);
}

static void test_memory_accessor__empty_string(void) {
    // Given an empty string
    const char *ref_path = "";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return NULL
    assert(result == NULL);
}

static void test_memory_accessor__similar_prefix(void) {
    // Given a path that starts with "mem" but not "memory."
    const char *ref_path = "mem.something";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return NULL
    assert(result == NULL);
}

static void test_memory_accessor__starts_with_memory_but_not_root(void) {
    // Given a path where first segment starts with "memory" but isn't exactly "memory"
    const char *ref_path = "memoryfoo.x";
    
    // When we get the memory key
    const char *result = ar_memory_accessor__get_key(ref_path);
    
    // Then it should return NULL
    assert(result == NULL);
}

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run from a bin directory\n");
            return 1;
        }
    }
    
    // Run tests
    test_memory_accessor__null_input();
    test_memory_accessor__non_memory_path();
    test_memory_accessor__simple_memory_path();
    test_memory_accessor__nested_memory_path();
    test_memory_accessor__deeply_nested_memory_path();
    test_memory_accessor__memory_only();
    test_memory_accessor__memory_with_trailing_dot();
    test_memory_accessor__empty_string();
    test_memory_accessor__similar_prefix();
    test_memory_accessor__starts_with_memory_but_not_root();
    
    printf("All ar_memory_accessor tests passed!\n");
    ar_heap__memory_report();
    
    return 0;
}