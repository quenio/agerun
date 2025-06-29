#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ar_frame.h"
#include "ar_data.h"
#include "ar_heap.h"

// Test function declarations
static void test_frame__create_destroy_with_all_parameters(void);
static void test_frame__getters_return_parameters(void);
static void test_frame__create_with_null_fields_not_allowed(void);

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd /Users/quenio/Repos/agerun/bin && ./ar_frame_tests\n");
            return 1;
        }
    }

    printf("Running frame module tests...\n");
    
    // Run tests
    test_frame__create_destroy_with_all_parameters();
    test_frame__getters_return_parameters();
    test_frame__create_with_null_fields_not_allowed();
    
    printf("All frame tests passed!\n");
    return 0;
}

static void test_frame__create_destroy_with_all_parameters(void) {
    printf("  test_frame__create_destroy_with_all_parameters...\n");
    
    // Given valid memory, context, and message data
    data_t *own_memory = ar_data__create_map();
    data_t *own_context = ar_data__create_map();
    data_t *own_message = ar_data__create_string("test message");
    
    // When creating a frame with all parameters
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Then the frame should be created successfully
    if (!own_frame) {
        fprintf(stderr, "    FAIL: Frame creation returned NULL\n");
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    // Clean up
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_memory);
    ar_data__destroy(own_context);
    ar_data__destroy(own_message);
    
    printf("    PASS\n");
}

static void test_frame__getters_return_parameters(void) {
    printf("  test_frame__getters_return_parameters...\n");
    
    // Given valid memory, context, and message data
    data_t *own_memory = ar_data__create_map();
    ar_data__set_map_integer(own_memory, "x", 42);
    
    data_t *own_context = ar_data__create_map();
    ar_data__set_map_string(own_context, "agent_id", "123");
    
    data_t *own_message = ar_data__create_string("test message");
    
    // When creating a frame and accessing via getters
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Then the getters should return the same references
    data_t *ref_memory = ar_frame__get_memory(own_frame);
    const data_t *ref_context = ar_frame__get_context(own_frame);
    const data_t *ref_message = ar_frame__get_message(own_frame);
    
    if (ref_memory != own_memory) {
        fprintf(stderr, "    FAIL: Memory getter returned different reference\n");
        ar_frame__destroy(own_frame);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    if (ref_context != own_context) {
        fprintf(stderr, "    FAIL: Context getter returned different reference\n");
        ar_frame__destroy(own_frame);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    if (ref_message != own_message) {
        fprintf(stderr, "    FAIL: Message getter returned different reference\n");
        ar_frame__destroy(own_frame);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    // Verify we can access data through the getters
    int x = ar_data__get_map_integer(ref_memory, "x");
    if (x != 42) {
        fprintf(stderr, "    FAIL: Memory value incorrect: %d\n", x);
        ar_frame__destroy(own_frame);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    const char *agent_id = ar_data__get_map_string(ref_context, "agent_id");
    if (!agent_id || strcmp(agent_id, "123") != 0) {
        fprintf(stderr, "    FAIL: Context value incorrect\n");
        ar_frame__destroy(own_frame);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    const char *msg = ar_data__get_string(ref_message);
    if (!msg || strcmp(msg, "test message") != 0) {
        fprintf(stderr, "    FAIL: Message value incorrect\n");
        ar_frame__destroy(own_frame);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    // Clean up
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_memory);
    ar_data__destroy(own_context);
    ar_data__destroy(own_message);
    
    printf("    PASS\n");
}

static void test_frame__create_with_null_fields_not_allowed(void) {
    printf("  test_frame__create_with_null_fields_not_allowed...\n");
    
    // Given valid data for testing
    data_t *own_memory = ar_data__create_map();
    data_t *own_context = ar_data__create_map();
    data_t *own_message = ar_data__create_string("test");
    
    // When creating a frame with null memory
    ar_frame_t *frame1 = ar_frame__create(NULL, own_context, own_message);
    if (frame1 != NULL) {
        fprintf(stderr, "    FAIL: Frame creation with NULL memory should fail\n");
        ar_frame__destroy(frame1);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    // When creating a frame with null context
    ar_frame_t *frame2 = ar_frame__create(own_memory, NULL, own_message);
    if (frame2 != NULL) {
        fprintf(stderr, "    FAIL: Frame creation with NULL context should fail\n");
        ar_frame__destroy(frame2);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    // When creating a frame with null message
    ar_frame_t *frame3 = ar_frame__create(own_memory, own_context, NULL);
    if (frame3 != NULL) {
        fprintf(stderr, "    FAIL: Frame creation with NULL message should fail\n");
        ar_frame__destroy(frame3);
        ar_data__destroy(own_memory);
        ar_data__destroy(own_context);
        ar_data__destroy(own_message);
        return;
    }
    
    // Clean up
    ar_data__destroy(own_memory);
    ar_data__destroy(own_context);
    ar_data__destroy(own_message);
    
    printf("    PASS\n");
}