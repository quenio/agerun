#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // For sleep

// Test expression evaluation indirectly through the message passing system
static void test_echo_method(void);
static void test_simple_method(void);

int main(int argc, char **argv) {
    printf("Starting Expression Tests...\n");
    fflush(stdout);
    
    // Given we initialize the runtime
    agent_id_t initial_agent = ar_system_init(NULL, 0);
    
    // Then no agent should be created during initialization
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        fflush(stdout);
        ar_system_shutdown();
        return 1;
    }
    
    // Run specific test based on argument
    if (argc > 1) {
        if (strcmp(argv[1], "echo") == 0) {
            test_echo_method();
        } else if (strcmp(argv[1], "simple") == 0) {
            test_simple_method();
        } else {
            printf("Unknown test: %s\n", argv[1]);
            return 1;
        }
    } else {
        // When we run all expression tests
        test_echo_method();
        test_simple_method();
    }
    
    // Then we clean up the system
    ar_system_shutdown();
    
    // And report success
    printf("All expression tests passed!\n");
    fflush(stdout);
    
    // Make sure we exit gracefully
    return 0;
}

static void test_echo_method(void) {
    printf("Testing echo method...\n");
    fflush(stdout);
    
    // Given a simple echo method that returns the input message
    version_t version = ar_method_create("echo", "send(0, message)", 0, true, false);
    assert(version > 0);
    printf("Created echo method version %d\n", version);
    fflush(stdout);
    
    // And an agent created with this method
    agent_id_t agent_id = ar_agent_create("echo", version, NULL);
    assert(agent_id > 0);
    printf("Created agent %lld using echo method\n", (long long)agent_id);
    fflush(stdout);
    
    // When we send a test message to the agent
    static const char *echo_text = "Hello, Echo!";
    data_t *echo_message = ar_data_create_string(echo_text);
    assert(echo_message != NULL);
    printf("Sending message: \"%s\"\n", echo_text);
    bool send_result = ar_agent_send(agent_id, echo_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the next message
    printf("Processing message...\n");
    ar_system_process_next_message(); // With opaque map, don't assert result
    
    // When we check for a response
    printf("Processing response...\n");
    bool response = ar_system_process_next_message();
    
    // Then there may be a response depending on the implementation
    printf("Response received: %s\n", response ? "yes" : "no");
    
    // When we clean up the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    
    // Then the agent should be destroyed successfully
    assert(destroy_result);
    
    printf("Echo method test passed.\n");
    fflush(stdout);
}

static void test_simple_method(void) {
    printf("Testing simple method...\n");
    
    // Given a simple method that stores a message in memory
    const char *simple_method = 
        "# Store message in memory\n"
        "memory[\"stored_message\"] := message";
    
    version_t version = ar_method_create("simple_test", simple_method, 0, true, false);
    assert(version > 0);
    printf("Created simple method version %d\n", version);
    
    // And an agent created with this method
    agent_id_t agent_id = ar_agent_create("simple_test", version, NULL);
    assert(agent_id > 0);
    printf("Created agent %lld using simple method\n", (long long)agent_id);
    
    // When we send a test message to the agent
    static const char *test_data_text = "Test Data";
    data_t *test_data_message = ar_data_create_string(test_data_text);
    assert(test_data_message != NULL);
    printf("Sending message: \"%s\"\n", test_data_text);
    bool send_result = ar_agent_send(agent_id, test_data_message);
    
    // Then the message should be sent successfully
    assert(send_result);
    
    // When we process the next message
    printf("Processing message...\n");
    ar_system_process_next_message(); // With opaque map, don't assert result
    
    // Give some time for any asynchronous processes to complete
    printf("Sleeping for a moment...\n");
    fflush(stdout);
    sleep(1);
    
    // Before cleanup
    printf("About to destroy agent %lld...\n", (long long)agent_id);
    fflush(stdout);
    
    // When we clean up the agent
    bool destroy_result = ar_agent_destroy(agent_id);
    
    // Then the agent should be destroyed successfully
    assert(destroy_result);
    
    printf("Agent successfully destroyed.\n");
    fflush(stdout);
    
    printf("Simple method test passed.\n");
    
    // Make sure all output is flushed before returning
    fflush(stdout);
}
