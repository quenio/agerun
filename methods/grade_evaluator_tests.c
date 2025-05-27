#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "agerun_system.h"
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include "agerun_data.h"
#include "agerun_io.h"
#include "agerun_heap.h"

/**
 * Reads a method file and returns its contents as a string
 * @param ref_filename Path to the method file
 * @return Newly allocated string with file contents, or NULL on error
 * @note Ownership: Returns an owned string that caller must free
 */
static char* read_method_file(const char *ref_filename) {
    FILE *fp = NULL;
    file_result_t result = ar_io_open_file(ref_filename, "r", &fp);
    if (result != FILE_SUCCESS) {
        ar_io_error("Failed to open method file %s: %s\n", 
                    ref_filename, ar_io_error_message(result));
        return NULL;
    }
    
    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate buffer
    char *own_content = (char*)AR_HEAP_MALLOC((size_t)(file_size + 1), "Method file content");
    if (!own_content) {
        ar_io_close_file(fp, ref_filename);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(own_content, 1, (size_t)file_size, fp);
    own_content[bytes_read] = '\0';
    
    ar_io_close_file(fp, ref_filename);
    return own_content; // Ownership transferred to caller
}

static void test_grade_evaluator_grades(void) {
    printf("Testing grade-evaluator method with grade evaluation...\n");
    
    // Clean up from any previous tests
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the grade-evaluator method file
    char *own_instructions = read_method_file("../methods/grade-evaluator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("grade-evaluator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    // Initialize system after shutdown
    agent_id_t init_result = ar_system_init(NULL, NULL);
    printf("DEBUG: System init result: %lld\n", (long long)init_result);
    
    // Always create agent directly since system might be initialized with different method
    agent_id_t evaluator_agent = ar_agent_create("grade-evaluator", "1.0.0", NULL);
    assert(evaluator_agent > 0);
    
    // Process wake message (ar_agent_create already sent it)
    printf("DEBUG: About to process wake message for agent %lld\n", (long long)evaluator_agent);
    printf("DEBUG: Agent exists: %s\n", ar_agent_exists(evaluator_agent) ? "yes" : "no");
    
    // Check if agent has messages before processing
    bool has_messages = ar_agency_agent_has_messages(evaluator_agent);
    printf("DEBUG: Agent has messages: %s\n", has_messages ? "yes" : "no");
    
    bool wake_processed = ar_system_process_next_message();
    if (wake_processed) {
        printf("Wake message processed successfully\n");
    } else {
        printf("WARNING: Failed to process wake message\n");
    }
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    // Test case 1: Grade A (90+)
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    ar_data_set_map_string(own_message, "type", "grade");
    ar_data_set_map_integer(own_message, "value", 95);
    ar_data_set_map_integer(own_message, "sender", 666);
    
    bool sent = ar_agent_send(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Get updated memory after processing
    agent_memory = ar_agent_get_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    // Verify memory for grade A
    // The grade-evaluator method uses multiple if() functions to determine grades
    const data_t *is_grade = ar_data_get_map_data(agent_memory, "is_grade");
    const data_t *grade_a = ar_data_get_map_data(agent_memory, "grade_a");
    const data_t *grade = ar_data_get_map_data(agent_memory, "grade");
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    
    if (is_grade == NULL) {
        printf("FAIL: memory.is_grade not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(is_grade) == DATA_INTEGER);
        assert(ar_data_get_integer(is_grade) == 1);
        printf("SUCCESS: if() correctly identified type = \"grade\"\n");
    }
    
    if (grade_a == NULL) {
        printf("FAIL: memory.grade_a not found - if() comparison failed\n");
    } else {
        assert(ar_data_get_type(grade_a) == DATA_INTEGER);
        assert(ar_data_get_integer(grade_a) == 1);
        printf("SUCCESS: if() correctly identified value >= 90\n");
    }
    
    if (grade == NULL) {
        printf("FAIL: memory.grade not found - conditional assignment failed\n");
    } else {
        assert(ar_data_get_type(grade) == DATA_STRING);
        assert(strcmp(ar_data_get_string(grade), "A") == 0);
        printf("SUCCESS: Grade correctly set to \"A\"\n");
    }
    
    if (result == NULL) {
        printf("FAIL: memory.result not found - final assignment failed\n");
    } else {
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "A") == 0);
        printf("SUCCESS: Grade for 95: %s\n", ar_data_get_string(result));
    }
    // Expected result: "A"
    
    // Test case 2: Grade B (80-89)
    own_message = ar_data_create_map();
    assert(own_message != NULL);
    ar_data_set_map_string(own_message, "type", "grade");
    ar_data_set_map_integer(own_message, "value", 85);
    ar_data_set_map_integer(own_message, "sender", 666);
    
    sent = ar_agent_send(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Get updated memory after processing
    agent_memory = ar_agent_get_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    // Verify memory for grade B
    result = ar_data_get_map_data(agent_memory, "result");
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "B") == 0);
        printf("Grade for 85: %s\n", ar_data_get_string(result));
    }
    // Expected result: "B"
    
    // Test case 3: Grade C (70-79)
    own_message = ar_data_create_map();
    assert(own_message != NULL);
    ar_data_set_map_string(own_message, "type", "grade");
    ar_data_set_map_integer(own_message, "value", 75);
    ar_data_set_map_integer(own_message, "sender", 666);
    
    sent = ar_agent_send(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Get updated memory after processing
    agent_memory = ar_agent_get_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    // Verify memory for grade C
    result = ar_data_get_map_data(agent_memory, "result");
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "C") == 0);
        printf("Grade for 75: %s\n", ar_data_get_string(result));
    }
    // Expected result: "C"
    
    // Test case 4: Grade F (below 70)
    own_message = ar_data_create_map();
    assert(own_message != NULL);
    ar_data_set_map_string(own_message, "type", "grade");
    ar_data_set_map_integer(own_message, "value", 65);
    ar_data_set_map_integer(own_message, "sender", 666);
    
    sent = ar_agent_send(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Get updated memory after processing
    agent_memory = ar_agent_get_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    // Verify memory for grade F
    const data_t *grade_c = ar_data_get_map_data(agent_memory, "grade_c");
    grade = ar_data_get_map_data(agent_memory, "grade");
    result = ar_data_get_map_data(agent_memory, "result");
    
    if (grade_c != NULL) {
        assert(ar_data_get_type(grade_c) == DATA_INTEGER);
        assert(ar_data_get_integer(grade_c) == 0); // Should be 0 for value < 70
    }
    
    if (grade != NULL) {
        assert(ar_data_get_type(grade) == DATA_STRING);
        assert(strcmp(ar_data_get_string(grade), "F") == 0);
    }
    
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "F") == 0);
        printf("Grade for 65: %s\n", ar_data_get_string(result));
    }
    // Expected result: "F"
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Grade evaluator grades test passed\n");
}

static void test_grade_evaluator_status(void) {
    printf("Testing grade-evaluator method with status evaluation...\n");
    
    // Clean up from previous test
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the grade-evaluator method file
    char *own_instructions = read_method_file("../methods/grade-evaluator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("grade-evaluator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    // Initialize system after shutdown
    agent_id_t init_result = ar_system_init(NULL, NULL);
    printf("DEBUG: System init result: %lld\n", (long long)init_result);
    
    // Always create agent directly since system might be initialized with different method
    agent_id_t evaluator_agent = ar_agent_create("grade-evaluator", "1.0.0", NULL);
    assert(evaluator_agent > 0);
    
    // Process wake message (ar_agent_create already sent it)
    printf("DEBUG: About to process wake message for agent %lld\n", (long long)evaluator_agent);
    printf("DEBUG: Agent exists: %s\n", ar_agent_exists(evaluator_agent) ? "yes" : "no");
    
    // Check if agent has messages before processing
    bool has_messages = ar_agency_agent_has_messages(evaluator_agent);
    printf("DEBUG: Agent has messages: %s\n", has_messages ? "yes" : "no");
    
    bool wake_processed = ar_system_process_next_message();
    if (wake_processed) {
        printf("Wake message processed successfully\n");
    } else {
        printf("WARNING: Failed to process wake message\n");
    }
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    // Test case 1: Active status (value > 0)
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    ar_data_set_map_string(own_message, "type", "status");
    ar_data_set_map_integer(own_message, "value", 5);
    ar_data_set_map_integer(own_message, "sender", 666);
    
    bool sent = ar_agent_send(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify memory for active status
    const data_t *is_status = ar_data_get_map_data(agent_memory, "is_status");
    const data_t *status = ar_data_get_map_data(agent_memory, "status");
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    
    if (is_status != NULL) {
        assert(ar_data_get_type(is_status) == DATA_INTEGER);
        assert(ar_data_get_integer(is_status) == 1); // Should be 1 for status type
    }
    
    if (status != NULL) {
        assert(ar_data_get_type(status) == DATA_STRING);
        assert(strcmp(ar_data_get_string(status), "active") == 0);
    }
    
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "active") == 0);
        printf("Status for value 5: %s\n", ar_data_get_string(result));
    }
    // Expected result: "active"
    
    // Test case 2: Inactive status (value <= 0)
    own_message = ar_data_create_map();
    assert(own_message != NULL);
    ar_data_set_map_string(own_message, "type", "status");
    ar_data_set_map_integer(own_message, "value", 0);
    ar_data_set_map_integer(own_message, "sender", 666);
    
    sent = ar_agent_send(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify memory for inactive status
    status = ar_data_get_map_data(agent_memory, "status");
    result = ar_data_get_map_data(agent_memory, "result");
    
    if (status != NULL) {
        assert(ar_data_get_type(status) == DATA_STRING);
        assert(strcmp(ar_data_get_string(status), "inactive") == 0);
    }
    
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "inactive") == 0);
        printf("Status for value 0: %s\n", ar_data_get_string(result));
    }
    // Expected result: "inactive"
    
    // Test case 3: Unknown type
    own_message = ar_data_create_map();
    assert(own_message != NULL);
    ar_data_set_map_string(own_message, "type", "unknown");
    ar_data_set_map_integer(own_message, "value", 50);
    ar_data_set_map_integer(own_message, "sender", 666);
    
    sent = ar_agent_send(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify memory for unknown type
    is_status = ar_data_get_map_data(agent_memory, "is_status");
    const data_t *is_grade = ar_data_get_map_data(agent_memory, "is_grade");
    result = ar_data_get_map_data(agent_memory, "result");
    
    if (is_status != NULL) {
        assert(ar_data_get_type(is_status) == DATA_INTEGER);
        assert(ar_data_get_integer(is_status) == 0); // Should be 0 for non-status type
    }
    
    if (is_grade != NULL) {
        assert(ar_data_get_type(is_grade) == DATA_INTEGER);
        assert(ar_data_get_integer(is_grade) == 0); // Should be 0 for non-grade type
    }
    
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "unknown") == 0);
        printf("Result for unknown type: %s\n", ar_data_get_string(result));
    }
    // Expected result: "unknown"
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Grade evaluator status test passed\n");
}

int main(void) {
    printf("Running grade-evaluator method tests...\n\n");
    
    // Verify we're running from the bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current directory: %s\n", cwd);
        // Check if we're in a directory ending with /bin
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            return 1;
        }
    } else {
        perror("getcwd() error");
        return 1;
    }
    
    // Ensure clean state before starting tests
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    test_grade_evaluator_grades();
    test_grade_evaluator_status();
    
    printf("\nAll grade-evaluator method tests passed!\n");
    return 0;
}
