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

static void test_echo_simple_message(void) {
    printf("Testing echo method with simple message...\n");
    
    // Initialize system (if not already initialized)
    if (ar_system_init(NULL, NULL) == 0) {
        // System already initialized
    }
    
    // Read echo method from file
    char *own_echo_instructions = read_method_file("../methods/echo-1.0.0.method");
    assert(own_echo_instructions != NULL);
    printf("Echo method instructions:\n%s\n", own_echo_instructions);
    
    // Register echo method
    bool registered = ar_methodology_create_method("echo", own_echo_instructions, "1.0.0");
    assert(registered);
    
    AR_HEAP_FREE(own_echo_instructions);
    
    // Create echo agent
    agent_id_t echo_agent = ar_agent_create("echo", "1.0.0", NULL);
    assert(echo_agent != 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(echo_agent);
    assert(agent_memory != NULL);
    
    // When we send a message with sender field
    data_t *own_message = ar_data_create_map();
    assert(own_message != NULL);
    
    // Add sender field (0 means system/test)
    data_t *own_sender = ar_data_create_integer(0);
    ar_data_set_map_data(own_message, "sender", own_sender);
    own_sender = NULL; // Ownership transferred
    
    // Add the actual message content
    data_t *own_content = ar_data_create_string("Hello, Echo!");
    ar_data_set_map_data(own_message, "content", own_content);
    own_content = NULL; // Ownership transferred
    
    bool sent = ar_agent_send(echo_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Echo method now only sends back message.content, doesn't store in memory
    
    // Process the return message (echo sends it back)
    processed = ar_system_process_next_message();
    
    // Clean up
    ar_agent_destroy(echo_agent);
    
    printf("PASS\n");
}

static void test_echo_map_message(void) {
    printf("Testing echo method with map message...\n");
    
    // Initialize system (if not already initialized)
    if (ar_system_init(NULL, NULL) == 0) {
        // System already initialized
    }
    
    // Read echo method from file (in case not loaded)
    char *own_echo_instructions = read_method_file("../methods/echo-1.0.0.method");
    if (own_echo_instructions) {
        ar_methodology_create_method("echo", own_echo_instructions, "1.0.0");
        AR_HEAP_FREE(own_echo_instructions);
    }
    
    // Create echo agent
    agent_id_t echo_agent = ar_agent_create("echo", "1.0.0", NULL);
    assert(echo_agent != 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // Verify agent memory was initialized
    const data_t *agent_memory = ar_agent_get_memory(echo_agent);
    assert(agent_memory != NULL);
    
    // When we send a map message
    data_t *own_map_message = ar_data_create_map();
    assert(own_map_message != NULL);
    ar_data_set_map_integer(own_map_message, "sender", 0); // Add sender field
    ar_data_set_map_string(own_map_message, "type", "greeting");
    ar_data_set_map_string(own_map_message, "content", "Hello from map!");
    ar_data_set_map_integer(own_map_message, "count", 42);
    
    bool sent = ar_agent_send(echo_agent, own_map_message);
    assert(sent);
    own_map_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Echo method now only sends back message.content, doesn't store in memory
    
    // Process the return message (echo sends it back)
    processed = ar_system_process_next_message();
    
    // Clean up
    ar_agent_destroy(echo_agent);
    
    printf("PASS\n");
}

int main(void) {
    printf("Running echo method tests...\n");
    
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
    
    test_echo_simple_message();
    test_echo_map_message();
    
    // Clean up
    ar_system_shutdown();
    
    printf("All tests passed!\n");
    return 0;
}
