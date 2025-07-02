#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ar_log.h"
#include "ar_heap.h"
#include "ar_event.h"

// Test function declarations
static void test_log__create_destroy(void);
static void test_log__error(void);
static void test_log__warning_and_info(void);
static void test_log__position_variants(void);
static void test_log__get_last_event_by_type(void);
static void test_log__get_last_error_message(void);
static void test_log__get_last_error_position(void);
static void test_log__buffer_overflow_triggers_flush(void);
static void test_log__destroy_flushes_buffer(void);

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
    test_log__get_last_event_by_type();
    test_log__get_last_error_message();
    test_log__get_last_error_position();
    test_log__buffer_overflow_triggers_flush();
    test_log__destroy_flushes_buffer();
    
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
    
    // And the file should contain the single error event (destroy flushes buffer)
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        return;
    }
    
    // Check that file contains exactly one line
    int line_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        line_count++;
        if (line_count == 1 && strstr(line, "ERROR: Test error message") == NULL) {
            fprintf(stderr, "    FAIL: Expected error message not found in log file\n");
            fclose(file);
            remove("agerun.log");
            return;
        }
    }
    fclose(file);
    
    if (line_count != 1) {
        fprintf(stderr, "    FAIL: Expected 1 event in file, but found %d\n", line_count);
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
    
    // And the file should contain both events (destroy flushes buffer)
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        return;
    }
    
    // Check that file contains exactly two lines
    int line_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        line_count++;
        if (line_count == 1 && strstr(line, "WARNING: Test warning message") == NULL) {
            fprintf(stderr, "    FAIL: Expected warning message not found in log file\n");
            fclose(file);
            remove("agerun.log");
            return;
        }
        if (line_count == 2 && strstr(line, "INFO: Test info message") == NULL) {
            fprintf(stderr, "    FAIL: Expected info message not found in log file\n");
            fclose(file);
            remove("agerun.log");
            return;
        }
    }
    fclose(file);
    
    if (line_count != 2) {
        fprintf(stderr, "    FAIL: Expected 2 events in file, but found %d\n", line_count);
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
    
    // And the file should contain all three events with positions (destroy flushes buffer)
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        return;
    }
    
    // Check that file contains exactly three lines with position info
    int line_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        line_count++;
        if (line_count == 1) {
            if (strstr(line, "ERROR: Parse error at position") == NULL || 
                strstr(line, "(at position 42)") == NULL) {
                fprintf(stderr, "    FAIL: Expected error with position 42 not found\n");
                fclose(file);
                remove("agerun.log");
                return;
            }
        }
        if (line_count == 2) {
            if (strstr(line, "WARNING: Unexpected token") == NULL || 
                strstr(line, "(at position 100)") == NULL) {
                fprintf(stderr, "    FAIL: Expected warning with position 100 not found\n");
                fclose(file);
                remove("agerun.log");
                return;
            }
        }
        if (line_count == 3) {
            if (strstr(line, "INFO: Processing started") == NULL || 
                strstr(line, "(at position 0)") == NULL) {
                fprintf(stderr, "    FAIL: Expected info with position 0 not found\n");
                fclose(file);
                remove("agerun.log");
                return;
            }
        }
    }
    fclose(file);
    
    if (line_count != 3) {
        fprintf(stderr, "    FAIL: Expected 3 events in file, but found %d\n", line_count);
        remove("agerun.log");
        return;
    }
    
    // Clean up
    remove("agerun.log");
    printf("    PASS\n");
}

static void test_log__get_last_event_by_type(void) {
    printf("  test_log__get_last_event_by_type...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log and logging various messages
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    // Add multiple events of different types
    ar_log__error(own_log, "First error");
    ar_log__warning(own_log, "First warning");
    ar_log__info(own_log, "First info");
    ar_log__error(own_log, "Second error");
    ar_log__warning(own_log, "Second warning");
    ar_log__info(own_log, "Second info");
    ar_log__error(own_log, "Last error");
    
    // Then we should be able to get the last event of each type
    ar_event_t *ref_last_error = ar_log__get_last_error(own_log);
    if (!ref_last_error) {
        fprintf(stderr, "    FAIL: get_last_error returned NULL\n");
        ar_log__destroy(own_log);
        return;
    }
    if (strcmp(ar_event__get_message(ref_last_error), "Last error") != 0) {
        fprintf(stderr, "    FAIL: Expected 'Last error', got '%s'\n", 
                ar_event__get_message(ref_last_error));
        ar_log__destroy(own_log);
        return;
    }
    
    ar_event_t *ref_last_warning = ar_log__get_last_warning(own_log);
    if (!ref_last_warning) {
        fprintf(stderr, "    FAIL: get_last_warning returned NULL\n");
        ar_log__destroy(own_log);
        return;
    }
    if (strcmp(ar_event__get_message(ref_last_warning), "Second warning") != 0) {
        fprintf(stderr, "    FAIL: Expected 'Second warning', got '%s'\n", 
                ar_event__get_message(ref_last_warning));
        ar_log__destroy(own_log);
        return;
    }
    
    ar_event_t *ref_last_info = ar_log__get_last_info(own_log);
    if (!ref_last_info) {
        fprintf(stderr, "    FAIL: get_last_info returned NULL\n");
        ar_log__destroy(own_log);
        return;
    }
    if (strcmp(ar_event__get_message(ref_last_info), "Second info") != 0) {
        fprintf(stderr, "    FAIL: Expected 'Second info', got '%s'\n", 
                ar_event__get_message(ref_last_info));
        ar_log__destroy(own_log);
        return;
    }
    
    // Clean up
    ar_log__destroy(own_log);
    remove("agerun.log");
    
    // Test with no events of a specific type
    own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Second log creation returned NULL\n");
        return;
    }
    
    // Add only errors
    ar_log__error(own_log, "Only error");
    
    // Should get NULL for warning and info
    ref_last_warning = ar_log__get_last_warning(own_log);
    if (ref_last_warning != NULL) {
        fprintf(stderr, "    FAIL: Expected NULL for warning, but got an event\n");
        ar_log__destroy(own_log);
        return;
    }
    
    ref_last_info = ar_log__get_last_info(own_log);
    if (ref_last_info != NULL) {
        fprintf(stderr, "    FAIL: Expected NULL for info, but got an event\n");
        ar_log__destroy(own_log);
        return;
    }
    
    // Clean up
    ar_log__destroy(own_log);
    remove("agerun.log");
    
    printf("    PASS\n");
}

static void test_log__get_last_error_message(void) {
    printf("  test_log__get_last_error_message...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    // Test when no errors exist
    const char *ref_message = ar_log__get_last_error_message(own_log);
    if (ref_message != NULL) {
        fprintf(stderr, "    FAIL: Expected NULL when no errors, got '%s'\n", ref_message);
        ar_log__destroy(own_log);
        return;
    }
    
    // Add some non-error events
    ar_log__warning(own_log, "A warning");
    ar_log__info(own_log, "Some info");
    
    // Should still get NULL
    ref_message = ar_log__get_last_error_message(own_log);
    if (ref_message != NULL) {
        fprintf(stderr, "    FAIL: Expected NULL when only warnings/info exist, got '%s'\n", ref_message);
        ar_log__destroy(own_log);
        return;
    }
    
    // Add an error
    ar_log__error(own_log, "First error message");
    
    // Should get the error message
    ref_message = ar_log__get_last_error_message(own_log);
    if (!ref_message) {
        fprintf(stderr, "    FAIL: Expected error message, got NULL\n");
        ar_log__destroy(own_log);
        return;
    }
    if (strcmp(ref_message, "First error message") != 0) {
        fprintf(stderr, "    FAIL: Expected 'First error message', got '%s'\n", ref_message);
        ar_log__destroy(own_log);
        return;
    }
    
    // Add another error
    ar_log__error(own_log, "Second error message");
    
    // Should get the most recent error
    ref_message = ar_log__get_last_error_message(own_log);
    if (!ref_message) {
        fprintf(stderr, "    FAIL: Expected second error message, got NULL\n");
        ar_log__destroy(own_log);
        return;
    }
    if (strcmp(ref_message, "Second error message") != 0) {
        fprintf(stderr, "    FAIL: Expected 'Second error message', got '%s'\n", ref_message);
        ar_log__destroy(own_log);
        return;
    }
    
    // Clean up
    ar_log__destroy(own_log);
    remove("agerun.log");
    
    printf("    PASS\n");
}

static void test_log__get_last_error_position(void) {
    printf("  test_log__get_last_error_position...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    // Test when no errors exist
    int position = ar_log__get_last_error_position(own_log);
    if (position != 0) {
        fprintf(stderr, "    FAIL: Expected 0 when no errors, got %d\n", position);
        ar_log__destroy(own_log);
        return;
    }
    
    // Add an error without position
    ar_log__error(own_log, "Error without position");
    
    // Should get 0 (no position set)
    position = ar_log__get_last_error_position(own_log);
    if (position != 0) {
        fprintf(stderr, "    FAIL: Expected 0 for error without position, got %d\n", position);
        ar_log__destroy(own_log);
        return;
    }
    
    // Add an error with position
    ar_log__error_at(own_log, "Error at position 42", 42);
    
    // Should get the position
    position = ar_log__get_last_error_position(own_log);
    if (position != 42) {
        fprintf(stderr, "    FAIL: Expected 42, got %d\n", position);
        ar_log__destroy(own_log);
        return;
    }
    
    // Add another error with different position
    ar_log__error_at(own_log, "Error at position 100", 100);
    
    // Should get the most recent position
    position = ar_log__get_last_error_position(own_log);
    if (position != 100) {
        fprintf(stderr, "    FAIL: Expected 100, got %d\n", position);
        ar_log__destroy(own_log);
        return;
    }
    
    // Add a warning with position (should not affect error position)
    ar_log__warning_at(own_log, "Warning at position 200", 200);
    
    // Should still get the last error position
    position = ar_log__get_last_error_position(own_log);
    if (position != 100) {
        fprintf(stderr, "    FAIL: Expected 100 after warning, got %d\n", position);
        ar_log__destroy(own_log);
        return;
    }
    
    // Clean up
    ar_log__destroy(own_log);
    remove("agerun.log");
    
    printf("    PASS\n");
}

static void test_log__buffer_overflow_triggers_flush(void) {
    printf("  test_log__buffer_overflow_triggers_flush...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log and adding exactly 10 events (buffer limit)
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    // Add 10 events to fill the buffer
    for (int i = 1; i <= 10; i++) {
        char message[50];
        snprintf(message, sizeof(message), "Event %d", i);
        ar_log__error(own_log, message);
    }
    
    // File should still be empty (buffer not yet full)
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        ar_log__destroy(own_log);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long size_before = ftell(file);
    fclose(file);
    
    if (size_before != 0) {
        fprintf(stderr, "    FAIL: Log file should be empty before overflow, but has %ld bytes\n", size_before);
        ar_log__destroy(own_log);
        remove("agerun.log");
        return;
    }
    
    // When adding the 11th event (triggers flush)
    ar_log__error(own_log, "Event 11 - This should trigger flush");
    
    // Then the file should contain the first 10 events
    file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist after flush\n");
        ar_log__destroy(own_log);
        return;
    }
    
    // Count lines in the file
    int line_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        line_count++;
    }
    fclose(file);
    
    if (line_count != 10) {
        fprintf(stderr, "    FAIL: Expected 10 events in file after flush, but found %d\n", line_count);
        ar_log__destroy(own_log);
        remove("agerun.log");
        return;
    }
    
    // The buffer should now contain only the 11th event
    ar_event_t *ref_last_error = ar_log__get_last_error(own_log);
    if (!ref_last_error) {
        fprintf(stderr, "    FAIL: get_last_error returned NULL after flush\n");
        ar_log__destroy(own_log);
        remove("agerun.log");
        return;
    }
    
    if (strcmp(ar_event__get_message(ref_last_error), "Event 11 - This should trigger flush") != 0) {
        fprintf(stderr, "    FAIL: Expected last event to be 'Event 11 - This should trigger flush', got '%s'\n", 
                ar_event__get_message(ref_last_error));
        ar_log__destroy(own_log);
        remove("agerun.log");
        return;
    }
    
    // Clean up
    ar_log__destroy(own_log);
    remove("agerun.log");
    
    printf("    PASS\n");
}

static void test_log__destroy_flushes_buffer(void) {
    printf("  test_log__destroy_flushes_buffer...\n");
    
    // Given a clean state
    remove("agerun.log");
    
    // When creating a log and adding some events (less than buffer limit)
    ar_log_t *own_log = ar_log__create();
    if (!own_log) {
        fprintf(stderr, "    FAIL: Log creation returned NULL\n");
        return;
    }
    
    // Add 5 events (less than buffer limit of 10)
    ar_log__error(own_log, "Error message 1");
    ar_log__warning(own_log, "Warning message");
    ar_log__info(own_log, "Info message");
    ar_log__error_at(own_log, "Error at position", 42);
    ar_log__warning_at(own_log, "Warning at position", 100);
    
    // File should still be empty (buffer not full)
    FILE *file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist\n");
        ar_log__destroy(own_log);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long size_before = ftell(file);
    fclose(file);
    
    if (size_before != 0) {
        fprintf(stderr, "    FAIL: Log file should be empty before destroy, but has %ld bytes\n", size_before);
        ar_log__destroy(own_log);
        remove("agerun.log");
        return;
    }
    
    // When destroying the log (should flush buffered events)
    ar_log__destroy(own_log);
    
    // Then the file should contain all 5 events
    file = fopen("agerun.log", "r");
    if (!file) {
        fprintf(stderr, "    FAIL: Log file does not exist after destroy\n");
        remove("agerun.log");
        return;
    }
    
    // Count lines in the file
    int line_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        line_count++;
        // Verify format of first line
        if (line_count == 1) {
            if (strstr(line, "ERROR: Error message 1") == NULL) {
                fprintf(stderr, "    FAIL: First line doesn't contain expected error message\n");
                fclose(file);
                remove("agerun.log");
                return;
            }
        }
        // Verify format of position line
        if (line_count == 4) {
            if (strstr(line, "(at position 42)") == NULL) {
                fprintf(stderr, "    FAIL: Fourth line doesn't contain expected position\n");
                fclose(file);
                remove("agerun.log");
                return;
            }
        }
    }
    fclose(file);
    
    if (line_count != 5) {
        fprintf(stderr, "    FAIL: Expected 5 events in file after destroy, but found %d\n", line_count);
        remove("agerun.log");
        return;
    }
    
    // Clean up
    remove("agerun.log");
    
    printf("    PASS\n");
}