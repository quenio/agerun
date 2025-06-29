#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ar_log.h"
#include "ar_heap.h"

// Test function declarations
static void test_log__create_destroy(void);
static void test_log__error(void);
static void test_log__warning_and_info(void);
static void test_log__position_variants(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_log_tests\n");
            return 1;
        }
    }

    printf("Running log module tests...\n");
    
    // Run tests
    test_log__create_destroy();
    test_log__error();
    test_log__warning_and_info();
    test_log__position_variants();
    
    printf("All log tests passed!\n");
    return 0;
}

static void test_log__create_destroy(void) {
    printf("  test_log__create_destroy...\n");
    
    // Given the log file doesn't exist
    remove("agerun.log");
    
    // When creating a log
    ar_log_t *own_log = ar_log__create();
    
    // Then the log should be created successfully
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    // And the log file should exist
    struct stat st;
    if (stat("agerun.log", &st) != 0) {
        fprintf(stderr, "    FAIL: Log file 'agerun.log' was not created\n");
        ar_log__destroy(own_log);
        return;
    }
    
    // Clean up
    ar_log__destroy(own_log);
    
    // Clean up log file for next test
    remove("agerun.log");
    printf("    PASS\n");
}

static void test_log__error(void) {
    printf("  test_log__error...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log and logging an error
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    ar_log__error(own_log, "Test error message");
    
    // Then the log should be destroyed successfully
    ar_log__destroy(own_log);
    
    // And since buffer wasn't full, file should exist but be empty
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        return;
    }
    
    // Check if file is empty (no flush occurred)
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    
    if (size != 0) {
        fprintf(stderr, "    FAIL: Log file should be empty (buffer not flushed), but has %ld bytes\n", size);
        remove("agerun.log");
        return;
    }
    
    // Clean up
    remove("agerun.log");
    printf("    PASS\n");
}

static void test_log__warning_and_info(void) {
    printf("  test_log__warning_and_info...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log and logging warning and info messages
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    ar_log__warning(own_log, "Test warning message");
    ar_log__info(own_log, "Test info message");
    
    // Then the log should be destroyed successfully
    ar_log__destroy(own_log);
    
    // And since buffer wasn't full, file should exist but be empty
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        return;
    }
    
    // Check if file is empty (no flush occurred)
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    
    if (size != 0) {
        fprintf(stderr, "    FAIL: Log file should be empty (buffer not flushed), but has %ld bytes\n", size);
        remove("agerun.log");
        return;
    }
    
    // Clean up
    remove("agerun.log");
    printf("    PASS\n");
}

static void test_log__position_variants(void) {
    printf("  test_log__position_variants...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log and logging messages with positions
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    ar_log__error_at(own_log, "Parse error at position", 42);
    ar_log__warning_at(own_log, "Unexpected token", 100);
    ar_log__info_at(own_log, "Processing started", 0);
    
    // Then the log should be destroyed successfully
    ar_log__destroy(own_log);
    
    // And since buffer wasn't full, file should exist but be empty
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        return;
    }
    
    // Check if file is empty (no flush occurred)
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    
    if (size != 0) {
        fprintf(stderr, "    FAIL: Log file should be empty (buffer not flushed), but has %ld bytes\n", size);
        remove("agerun.log");
        return;
    }
    
    // Clean up
    remove("agerun.log");
    printf("    PASS\n");
}