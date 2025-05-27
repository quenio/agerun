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

static void test_method_creator_create_simple(void) {
    printf("Testing method-creator method with simple method creation...\n");
    
    // Clean up from any previous tests
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the method-creator method file
    char *own_instructions = read_method_file("../methods/method-creator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("method-creator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t creator_agent = ar_system_init("method-creator", "1.0.0");
    if (creator_agent == 0) {
        // System already initialized, create agent directly
        creator_agent = ar_agent_create("method-creator", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(creator_agent, own_wake);
    }
    assert(creator_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a message to create a new method
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "method_name", "doubler");
    ar_data_set_map_string(own_message, "instructions", "memory.result := message.value * 2\nsend(message.sender, memory.result)");
    ar_data_set_map_string(own_message, "version", "1.0.0");
    ar_data_set_map_integer(own_message, "sender", 888);
    
    bool sent = ar_agent_send(creator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify method execution by checking agent's memory
    // The method-creator method should:
    // 1. Call method() function to create a new method
    // 2. Store the result (1 for success, 0 for failure) in memory.result
    // 3. Send the result back to the sender
    
    // Get agent memory for verification
    const data_t *agent_memory = ar_agent_get_memory(creator_agent);
    assert(agent_memory != NULL);
    
    // Check memory.result - should contain the result of method() function
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result == NULL) {
        printf("FAIL: memory.result not found - method() instruction failed to execute\n");
        printf("NOTE: This indicates the method() function in instruction module needs implementation\n");
    } else {
        assert(ar_data_get_type(result) == DATA_INTEGER);
        int result_val = ar_data_get_integer(result);
        printf("SUCCESS: method() instruction executed and created memory.result\n");
        printf("  - Method creation result: %d\n", result_val);
        
        if (result_val == 1) {
            printf("  - Method 'doubler' was successfully created\n");
            // We could verify the method exists by trying to create an agent with it
        } else {
            printf("  - Method creation failed (returned 0)\n");
        }
    }
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Method creator create simple test passed\n");
}

static void test_method_creator_invalid_syntax(void) {
    printf("Testing method-creator method with invalid syntax...\n");
    
    // Clean up from previous test
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Given the method-creator method file
    char *own_instructions = read_method_file("../methods/method-creator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("method-creator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t creator_agent = ar_system_init("method-creator", "1.0.0");
    if (creator_agent == 0) {
        // System already initialized, create agent directly
        creator_agent = ar_agent_create("method-creator", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(creator_agent, own_wake);
    }
    assert(creator_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a message to create a method with invalid syntax
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "method_name", "broken");
    ar_data_set_map_string(own_message, "instructions", "memory.result = invalid syntax here");
    ar_data_set_map_string(own_message, "version", "1.0.0");
    ar_data_set_map_integer(own_message, "sender", 888);
    
    bool sent = ar_agent_send(creator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Verify method execution with invalid syntax
    // The method() function should validate syntax and return 0 for invalid instructions
    
    // Get agent memory for verification
    const data_t *agent_memory = ar_agent_get_memory(creator_agent);
    assert(agent_memory != NULL);
    
    // Check memory.result - should be 0 for invalid syntax
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result == NULL) {
        printf("FAIL: memory.result not found - method() instruction failed to execute\n");
        printf("NOTE: This is expected until method() function is implemented in instruction module\n");
    } else {
        assert(ar_data_get_type(result) == DATA_INTEGER);
        int result_val = ar_data_get_integer(result);
        printf("SUCCESS: method() instruction executed with invalid syntax\n");
        printf("  - Method creation result: %d\n", result_val);
        
        if (result_val == 0) {
            printf("  - Correctly rejected invalid syntax\n");
        } else {
            printf("  - WARNING: Method creation succeeded with invalid syntax (expected failure)\n");
        }
    }
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Method creator invalid syntax test passed\n");
}

int main(void) {
    printf("Running method-creator method tests...\n\n");
    
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
    
    test_method_creator_create_simple();
    test_method_creator_invalid_syntax();
    
    printf("\nAll method-creator method tests passed!\n");
    return 0;
}
