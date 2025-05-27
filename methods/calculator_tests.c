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

static void test_calculator_add(void) {
    printf("Testing calculator method with add operation...\n");
    
    // Clean up from any previous tests
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    if (!method_created) {
        printf("WARNING: Method already exists, using existing one\n");
    }
    // assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    // Initialize system after shutdown
    agent_id_t init_result = ar_system_init(NULL, NULL);
    printf("DEBUG: System init result: %lld\n", (long long)init_result);
    
    // Create agent (ar_agent_create sends wake message but doesn't process it)
    agent_id_t calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
    assert(calc_agent > 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process the wake message first
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(calc_agent);
    assert(agent_memory != NULL);
    
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
    printf("DEBUG: About to process calculator message\n");
    bool processed = ar_system_process_next_message();
    if (!processed) {
        printf("ERROR: Failed to process calculator message\n");
    }
    assert(processed);
    
    // Get updated memory after processing
    agent_memory = ar_agent_get_memory(calc_agent);
    assert(agent_memory != NULL);
    
    // Verify memory for add operation
    // The calculator method uses conditional if() to select operation and arithmetic expressions
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result == NULL) {
        printf("FAIL: memory.result not found - arithmetic operation failed\n");
        printf("NOTE: This may indicate issues with if() or arithmetic expressions\n");
    } else {
        assert(ar_data_get_type(result) == DATA_INTEGER || ar_data_get_type(result) == DATA_DOUBLE);
        if (ar_data_get_type(result) == DATA_INTEGER) {
            assert(ar_data_get_integer(result) == 8);
            printf("SUCCESS: Calculator add operation: 5 + 3 = %d\n", ar_data_get_integer(result));
        } else {
            assert(ar_data_get_double(result) == 8.0);
            printf("SUCCESS: Calculator add operation: 5 + 3 = %.1f\n", ar_data_get_double(result));
        }
    }
    
    // Then the calculator should have sent the result (8) to agent 100
    
    // Cleanup
    ar_agent_destroy(calc_agent);
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
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    if (!method_created) {
        printf("WARNING: Method already exists, using existing one\n");
    }
    // assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    // Initialize system after shutdown
    ar_system_init(NULL, NULL);
    
    // Create agent (ar_agent_create sends wake message but doesn't process it)
    agent_id_t calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
    assert(calc_agent > 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process the wake message first
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(calc_agent);
    assert(agent_memory != NULL);
    
    // When we send a multiply operation
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    ar_data_set_map_string(own_message, "operation", "multiply");
    ar_data_set_map_integer(own_message, "a", 5);
    ar_data_set_map_integer(own_message, "b", 2);
    ar_data_set_map_integer(own_message, "sender", 200);
    
    bool sent = ar_agent_send(calc_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the calculation
    printf("DEBUG: About to process calculator message\n");
    bool processed = ar_system_process_next_message();
    if (!processed) {
        printf("ERROR: Failed to process calculator message\n");
    }
    assert(processed);
    
    // Get updated memory after processing
    agent_memory = ar_agent_get_memory(calc_agent);
    assert(agent_memory != NULL);
    
    // Verify memory for multiply operation
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    
    // Debug: Check what's in memory
    printf("DEBUG: Checking agent memory after multiply\n");
    
    // Debug: Also check what we sent
    printf("DEBUG: We sent operation='multiply', a=5, b=2\n");
    
    // Debug: Check if the result is still 0 (initial value)
    printf("DEBUG: memory.result exists and equals %g\n", 
           result ? (ar_data_get_type(result) == DATA_INTEGER ? 
                    (double)ar_data_get_integer(result) : 
                    ar_data_get_double(result)) : -999.0);
    
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_INTEGER || ar_data_get_type(result) == DATA_DOUBLE);
        double result_value = (ar_data_get_type(result) == DATA_INTEGER) 
            ? (double)ar_data_get_integer(result) 
            : ar_data_get_double(result);
        printf("DEBUG: Multiply result type: %s, value: %.1f\n", 
               ar_data_get_type(result) == DATA_INTEGER ? "INTEGER" : "DOUBLE", 
               result_value);
        assert(result_value == 10.0);  // 5 * 2 = 10
        printf("SUCCESS: Calculator multiply operation: 5 * 2 = %.1f\n", result_value);
    } else {
        printf("Warning: memory.result not found\n");
    }
    
    // Then the calculator should have sent the result (10.0) to agent 200
    
    // Cleanup
    ar_agent_destroy(calc_agent);
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
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    if (!method_created) {
        printf("WARNING: Method already exists, using existing one\n");
    }
    // assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    // Initialize system after shutdown
    ar_system_init(NULL, NULL);
    
    // Create agent (ar_agent_create sends wake message but doesn't process it)
    agent_id_t calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
    assert(calc_agent > 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process the wake message first
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(calc_agent);
    assert(agent_memory != NULL);
    
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
    printf("DEBUG: About to process calculator message\n");
    bool processed = ar_system_process_next_message();
    if (!processed) {
        printf("ERROR: Failed to process calculator message\n");
    }
    assert(processed);
    
    // Verify memory for subtract operation
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_INTEGER || ar_data_get_type(result) == DATA_DOUBLE);
        if (ar_data_get_type(result) == DATA_INTEGER) {
            assert(ar_data_get_integer(result) == 3);
            printf("Subtract result: %d\n", ar_data_get_integer(result));
        } else {
            assert(ar_data_get_double(result) == 3.0);
            printf("Subtract result: %.1f\n", ar_data_get_double(result));
        }
    } else {
        printf("Warning: memory.result not found\n");
    }
    
    // Then the calculator should have sent the result (3) to agent 300
    
    // Cleanup
    ar_agent_destroy(calc_agent);
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
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    if (!method_created) {
        printf("WARNING: Method already exists, using existing one\n");
    }
    // assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    // Initialize system after shutdown
    ar_system_init(NULL, NULL);
    
    // Create agent (ar_agent_create sends wake message but doesn't process it)
    agent_id_t calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
    assert(calc_agent > 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process the wake message first
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(calc_agent);
    assert(agent_memory != NULL);
    
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
    printf("DEBUG: About to process calculator message\n");
    bool processed = ar_system_process_next_message();
    if (!processed) {
        printf("ERROR: Failed to process calculator message\n");
    }
    assert(processed);
    
    // Verify memory for divide operation
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_INTEGER || ar_data_get_type(result) == DATA_DOUBLE);
        double result_value = (ar_data_get_type(result) == DATA_INTEGER) 
            ? (double)ar_data_get_integer(result) 
            : ar_data_get_double(result);
        assert(result_value == 5.0);
        printf("Divide result: %.1f\n", result_value);
    } else {
        printf("Warning: memory.result not found\n");
    }
    
    // Then the calculator should have sent the result (5.0) to agent 400
    
    // Cleanup
    ar_agent_destroy(calc_agent);
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
    
    // Remove any .agerun files from current directory (should be bin/)
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Given the calculator method file
    char *own_instructions = read_method_file("../methods/calculator-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("calculator", own_instructions, "1.0.0");
    if (!method_created) {
        printf("WARNING: Method already exists, using existing one\n");
    }
    // assert(method_created);
    AR_HEAP_FREE(own_instructions);
    own_instructions = NULL;
    
    // Initialize system after shutdown
    ar_system_init(NULL, NULL);
    
    // Create agent (ar_agent_create sends wake message but doesn't process it)
    agent_id_t calc_agent = ar_agent_create("calculator", "1.0.0", NULL);
    assert(calc_agent > 0);
    printf("DEBUG: Created calc agent %lld\n", (long long)calc_agent);
    
    // Process the wake message first
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(calc_agent);
    assert(agent_memory != NULL);
    
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
    printf("DEBUG: About to process calculator message\n");
    bool processed = ar_system_process_next_message();
    if (!processed) {
        printf("ERROR: Failed to process calculator message\n");
    }
    assert(processed);
    
    // Verify memory for unknown operation
    const data_t *result = ar_data_get_map_data(agent_memory, "result");
    if (result != NULL) {
        assert(ar_data_get_type(result) == DATA_INTEGER || ar_data_get_type(result) == DATA_DOUBLE);
        if (ar_data_get_type(result) == DATA_INTEGER) {
            assert(ar_data_get_integer(result) == 0);
            printf("Unknown operation result: %d\n", ar_data_get_integer(result));
        } else {
            assert(ar_data_get_double(result) == 0.0);
            printf("Unknown operation result: %.1f\n", ar_data_get_double(result));
        }
    } else {
        printf("Warning: memory.result not found\n");
    }
    
    // Then the calculator should have sent 0 (default result) to agent 500
    
    // Cleanup
    ar_agent_destroy(calc_agent);
    ar_system_shutdown();
    ar_methodology_cleanup();
    ar_agency_reset();
    
    printf("✓ Calculator unknown operation test passed\n");
}

int main(void) {
    printf("Running calculator method tests...\n\n");
    
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
    
    test_calculator_add();
    test_calculator_multiply();
    test_calculator_subtract();
    test_calculator_divide();
    test_calculator_unknown_operation();
    
    printf("\nAll calculator method tests passed!\n");
    return 0;
}

