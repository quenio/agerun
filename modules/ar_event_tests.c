#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_event.h"
#include "ar_heap.h"

// Test function declarations
static void test_event__create_destroy(void);
static void test_event__create_with_message(void);
static void test_event__create_with_different_types(void);
static void test_event__create_with_position(void);
static void test_event__has_timestamp(void);
static void test_event__memory_stress(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_event_tests\n");
            return 1;
        }
    }

    printf("Running event module tests...\n");
    
    // Run tests
    test_event__create_destroy();
    test_event__create_with_message();
    test_event__create_with_different_types();
    test_event__create_with_position();
    test_event__has_timestamp();
    test_event__memory_stress();
    
    printf("All event tests passed!\n");
    return 0;
}

static void test_event__create_destroy(void) {
    printf("  test_event__create_destroy...\n");
    
    // Given nothing special
    
    // When creating an event
    ar_event_t *own_event = ar_event__create();
    
    // Then the event should be created successfully
    if (!own_event) {
        fprintf(stderr, "    FAIL: Event creation returned NULL\n");
        return;
    }
    
    // And it should be destroyed without issues
    ar_event__destroy(own_event);
}

static void test_event__create_with_message(void) {
    printf("  test_event__create_with_message...\n");
    
    // Given a test message
    const char *ref_test_message = "Test error message";
    
    // When creating an event with a message
    ar_event_t *own_event = ar_event__create_with_message(ref_test_message);
    
    // Then the event should be created successfully
    if (!own_event) {
        fprintf(stderr, "    FAIL: Event creation with message returned NULL\n");
        return;
    }
    
    // And the message should be retrievable
    const char *ref_message = ar_event__get_message(own_event);
    if (!ref_message) {
        fprintf(stderr, "    FAIL: Event message is NULL\n");
        ar_event__destroy(own_event);
        return;
    }
    
    // And the message should match what was provided
    if (strcmp(ref_message, ref_test_message) != 0) {
        fprintf(stderr, "    FAIL: Event message mismatch. Expected: '%s', Got: '%s'\n",
                ref_test_message, ref_message);
        ar_event__destroy(own_event);
        return;
    }
    
    // Clean up
    ar_event__destroy(own_event);
}

static void test_event__create_with_different_types(void) {
    printf("  test_event__create_with_different_types...\n");
    
    // Given different event types and messages
    const char *ref_error_msg = "Error occurred";
    const char *ref_warning_msg = "Warning: check this";
    const char *ref_info_msg = "Info: process started";
    
    // When creating events with different types
    ar_event_t *own_error = ar_event__create_typed(AR_EVENT_ERROR, ref_error_msg);
    ar_event_t *own_warning = ar_event__create_typed(AR_EVENT_WARNING, ref_warning_msg);
    ar_event_t *own_info = ar_event__create_typed(AR_EVENT_INFO, ref_info_msg);
    
    // Then all events should be created successfully
    if (!own_error || !own_warning || !own_info) {
        fprintf(stderr, "    FAIL: Event creation with type returned NULL\n");
        if (own_error) ar_event__destroy(own_error);
        if (own_warning) ar_event__destroy(own_warning);
        if (own_info) ar_event__destroy(own_info);
        return;
    }
    
    // And types should be retrievable and correct
    ar_event_type_t error_type = ar_event__get_type(own_error);
    if (error_type != AR_EVENT_ERROR) {
        fprintf(stderr, "    FAIL: Error event type mismatch. Expected: %d, Got: %d\n",
                AR_EVENT_ERROR, error_type);
        ar_event__destroy(own_error);
        ar_event__destroy(own_warning);
        ar_event__destroy(own_info);
        return;
    }
    
    ar_event_type_t warning_type = ar_event__get_type(own_warning);
    if (warning_type != AR_EVENT_WARNING) {
        fprintf(stderr, "    FAIL: Warning event type mismatch. Expected: %d, Got: %d\n",
                AR_EVENT_WARNING, warning_type);
        ar_event__destroy(own_error);
        ar_event__destroy(own_warning);
        ar_event__destroy(own_info);
        return;
    }
    
    ar_event_type_t info_type = ar_event__get_type(own_info);
    if (info_type != AR_EVENT_INFO) {
        fprintf(stderr, "    FAIL: Info event type mismatch. Expected: %d, Got: %d\n",
                AR_EVENT_INFO, info_type);
        ar_event__destroy(own_error);
        ar_event__destroy(own_warning);
        ar_event__destroy(own_info);
        return;
    }
    
    // And messages should still be correct
    if (strcmp(ar_event__get_message(own_error), ref_error_msg) != 0 ||
        strcmp(ar_event__get_message(own_warning), ref_warning_msg) != 0 ||
        strcmp(ar_event__get_message(own_info), ref_info_msg) != 0) {
        fprintf(stderr, "    FAIL: Event messages don't match\n");
        ar_event__destroy(own_error);
        ar_event__destroy(own_warning);
        ar_event__destroy(own_info);
        return;
    }
    
    // Clean up
    ar_event__destroy(own_error);
    ar_event__destroy(own_warning);
    ar_event__destroy(own_info);
}

static void test_event__create_with_position(void) {
    printf("  test_event__create_with_position...\n");
    
    // Given an error message and position information
    const char *ref_error_msg = "Syntax error: unexpected token";
    int position = 42;
    
    // When creating an event with position
    ar_event_t *own_event = ar_event__create_with_position(AR_EVENT_ERROR, ref_error_msg, position);
    
    // Then the event should be created successfully
    if (!own_event) {
        fprintf(stderr, "    FAIL: Event creation with position returned NULL\n");
        return;
    }
    
    // And the message should be retrievable
    const char *ref_message = ar_event__get_message(own_event);
    if (!ref_message || strcmp(ref_message, ref_error_msg) != 0) {
        fprintf(stderr, "    FAIL: Event message incorrect: %s\n", ref_message);
        ar_event__destroy(own_event);
        return;
    }
    
    // And the type should be correct
    if (ar_event__get_type(own_event) != AR_EVENT_ERROR) {
        fprintf(stderr, "    FAIL: Event type incorrect\n");
        ar_event__destroy(own_event);
        return;
    }
    
    // And the position should be retrievable
    int retrieved_position = ar_event__get_position(own_event);
    if (retrieved_position != position) {
        fprintf(stderr, "    FAIL: Event position incorrect. Expected: %d, Got: %d\n",
                position, retrieved_position);
        ar_event__destroy(own_event);
        return;
    }
    
    // And has_position should return true
    if (!ar_event__has_position(own_event)) {
        fprintf(stderr, "    FAIL: Event should have position\n");
        ar_event__destroy(own_event);
        return;
    }
    
    // Clean up
    ar_event__destroy(own_event);
    
    // Test event without position
    ar_event_t *own_event_no_pos = ar_event__create_typed(AR_EVENT_INFO, "Just info");
    if (!own_event_no_pos) {
        fprintf(stderr, "    FAIL: Event creation without position returned NULL\n");
        return;
    }
    
    // has_position should return false
    if (ar_event__has_position(own_event_no_pos)) {
        fprintf(stderr, "    FAIL: Event should not have position\n");
        ar_event__destroy(own_event_no_pos);
        return;
    }
    
    // Clean up
    ar_event__destroy(own_event_no_pos);
}

static void test_event__has_timestamp(void) {
    printf("  test_event__has_timestamp...\n");
    
    // Given an event
    ar_event_t *own_event = ar_event__create_typed(AR_EVENT_INFO, "System started");
    
    // Then the event should have a timestamp
    if (!own_event) {
        fprintf(stderr, "    FAIL: Event creation returned NULL\n");
        return;
    }
    
    // And the timestamp should be retrievable
    const char *ref_timestamp = ar_event__get_timestamp(own_event);
    if (!ref_timestamp) {
        fprintf(stderr, "    FAIL: Event timestamp is NULL\n");
        ar_event__destroy(own_event);
        return;
    }
    
    // And the timestamp should be in ISO 8601 format (YYYY-MM-DDTHH:MM:SS)
    // Basic validation: should be at least 19 characters
    if (strlen(ref_timestamp) < 19) {
        fprintf(stderr, "    FAIL: Event timestamp too short: %s\n", ref_timestamp);
        ar_event__destroy(own_event);
        return;
    }
    
    // Basic format check
    if (ref_timestamp[4] != '-' || ref_timestamp[7] != '-' || 
        ref_timestamp[10] != 'T' || ref_timestamp[13] != ':' || ref_timestamp[16] != ':') {
        fprintf(stderr, "    FAIL: Event timestamp has wrong format: %s\n", ref_timestamp);
        ar_event__destroy(own_event);
        return;
    }
    
    // Clean up
    ar_event__destroy(own_event);
}

static void test_event__memory_stress(void) {
    printf("  test_event__memory_stress...\n");
    
    // Given a need to create many events
    const int event_count = 100;
    ar_event_t *events[100];
    
    // When creating many events with different configurations
    for (int i = 0; i < event_count; i++) {
        if (i % 4 == 0) {
            // Basic event
            events[i] = ar_event__create();
        } else if (i % 4 == 1) {
            // Event with message
            char message[50];
            snprintf(message, sizeof(message), "Event #%d", i);
            events[i] = ar_event__create_with_message(message);
        } else if (i % 4 == 2) {
            // Event with type and message
            char message[50];
            snprintf(message, sizeof(message), "Warning #%d", i);
            events[i] = ar_event__create_typed(AR_EVENT_WARNING, message);
        } else {
            // Event with position
            char message[50];
            snprintf(message, sizeof(message), "Error at position %d", i * 10);
            events[i] = ar_event__create_with_position(AR_EVENT_ERROR, message, i * 10);
        }
        
        // Then each event should be created successfully
        if (!events[i]) {
            fprintf(stderr, "    FAIL: Event %d creation failed\n", i);
            // Clean up already created events
            for (int j = 0; j < i; j++) {
                ar_event__destroy(events[j]);
            }
            return;
        }
    }
    
    // And we should be able to access all events
    for (int i = 0; i < event_count; i++) {
        const char *timestamp = ar_event__get_timestamp(events[i]);
        if (!timestamp) {
            fprintf(stderr, "    FAIL: Event %d has no timestamp\n", i);
            // Clean up
            for (int j = 0; j < event_count; j++) {
                ar_event__destroy(events[j]);
            }
            return;
        }
    }
    
    // Clean up all events
    for (int i = 0; i < event_count; i++) {
        ar_event__destroy(events[i]);
    }
    
    // Memory report will verify no leaks
}