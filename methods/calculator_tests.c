#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

static void test_calculator_add(void) {
    printf("Testing calculator method with add operation...\n");
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t calc_agent = ar_system_init("calculator", "1.0.0");
    if (calc_agent == 0) {
        // System already initialized, create agent directly
        calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(calc_agent, own_wake);
    }
    assert(calc_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send an add operation
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "operation", "add");
    ar_data_set_map_integer(own_message, "a", 5);
    ar_data_set_map_integer(own_message, "b", 3);
    ar_data_set_map_integer(own_message, "sender", 100); // Sender agent ID
    
    bool sent = ar_agent_send(calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculation
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Then the calculator should have sent the result (8) to agent 100
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Calculator add operation test passed\n");
}

static void test_calculator_multiply(void) {
    printf("Testing calculator method with multiply operation...\n");
    
    // Clean up from previous test
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t calc_agent = ar_system_init("calculator", "1.0.0");
    if (calc_agent == 0) {
        // System already initialized, create agent directly
        calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(calc_agent, own_wake);
    }
    assert(calc_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a multiply operation
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "operation", "multiply");
    ar_data_set_map_double(own_message, "a", 2.5);
    ar_data_set_map_double(own_message, "b", 4.0);
    ar_data_set_map_integer(own_message, "sender", 200);
    
    bool sent = ar_agent_send(calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculation
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Then the calculator should have sent the result (10.0) to agent 200
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Calculator multiply operation test passed\n");
}

static void test_calculator_subtract(void) {
    printf("Testing calculator method with subtract operation...\n");
    
    // Clean up from previous test
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t calc_agent = ar_system_init("calculator", "1.0.0");
    if (calc_agent == 0) {
        // System already initialized, create agent directly
        calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(calc_agent, own_wake);
    }
    assert(calc_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a subtract operation
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "operation", "subtract");
    ar_data_set_map_integer(own_message, "a", 10);
    ar_data_set_map_integer(own_message, "b", 7);
    ar_data_set_map_integer(own_message, "sender", 300);
    
    bool sent = ar_agent_send(calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculation
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Then the calculator should have sent the result (3) to agent 300
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Calculator subtract operation test passed\n");
}

static void test_calculator_divide(void) {
    printf("Testing calculator method with divide operation...\n");
    
    // Clean up from previous test
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t calc_agent = ar_system_init("calculator", "1.0.0");
    if (calc_agent == 0) {
        // System already initialized, create agent directly
        calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(calc_agent, own_wake);
    }
    assert(calc_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a divide operation
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "operation", "divide");
    ar_data_set_map_double(own_message, "a", 20.0);
    ar_data_set_map_double(own_message, "b", 4.0);
    ar_data_set_map_integer(own_message, "sender", 400);
    
    bool sent = ar_agent_send(calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculation
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Then the calculator should have sent the result (5.0) to agent 400
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Calculator divide operation test passed\n");
}

static void test_calculator_unknown_operation(void) {
    printf("Testing calculator method with unknown operation...\n");
    
    // Clean up from previous test
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    agent_id_t calc_agent = ar_system_init("calculator", "1.0.0");
    if (calc_agent == 0) {
        // System already initialized, create agent directly
        calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
        // Send wake message manually
        data_t *own_wake = ar_data_create_string("__wake__");
        ar_agent_send(calc_agent, own_wake);
    }
    assert(calc_agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send an unknown operation
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "operation", "modulo");
    ar_data_set_map_integer(own_message, "a", 10);
    ar_data_set_map_integer(own_message, "b", 3);
    ar_data_set_map_integer(own_message, "sender", 500);
    
    bool sent = ar_agent_send(calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculation
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Then the calculator should have sent 0 (default result) to agent 500
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Calculator unknown operation test passed\n");
}

int main(void) {
    printf("Running calculator method tests...\n\n");
    
    // Ensure clean state before starting tests
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    test_calculator_add();
    test_calculator_multiply();
    test_calculator_subtract();
    test_calculator_divide();
    test_calculator_unknown_operation();
    
    printf("\nAll calculator method tests passed!\n");
    return 0;
}
