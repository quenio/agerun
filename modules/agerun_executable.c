#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include "agerun_executable.h"
#include "agerun_heap.h"

/* Message strings */
static const char *ref_wake_message = "__wake__";
static const char *ref_increment_message = "increment";
static const char *ref_get_message = "get";

int ar__executable__main(void) {
    printf("Agerun Example Application\n");
    printf("==========================\n\n");
    
    // Initialize the runtime
    printf("Initializing runtime...\n");
    int64_t initial_agent = ar__system__init(NULL, NULL);
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        ar__system__shutdown();
        return 1;
    }
    printf("Runtime initialized successfully\n\n");
    
    // Create a simple echo method
    printf("Creating echo method...\n");
    const char *ref_echo_instructions = "send(0, message)";
    
    // Create method using methodology module (simplified API)
    const char *ref_echo_version = "1.0.0";
    if (!ar__methodology__create_method("echo", ref_echo_instructions, ref_echo_version)) {
        printf("Failed to create echo method\n");
        ar__system__shutdown();
        return 1;
    }
    
    printf("Echo method created with version %s\n\n", ref_echo_version);
    
    // Create a simplified counter method that just echoes back messages
    printf("Creating counter method...\n");
    const char *ref_counter_code = "send(0, \"Hello from counter!\")";
    
    // Create method using methodology module (simplified API)
    const char *ref_counter_version = "1.0.0";
    if (!ar__methodology__create_method("counter", ref_counter_code, ref_counter_version)) {
        printf("Failed to create counter method\n");
        ar__system__shutdown();
        return 1;
    }
    
    printf("Counter method created with version %s\n\n", ref_counter_version);
    
    // Create the initial agent
    printf("Creating initial agent...\n");
    initial_agent = ar__agency__create_agent("echo", ref_echo_version, NULL);
    if (initial_agent == 0) {
        printf("Failed to create initial agent\n");
        ar__system__shutdown();
        return 1;
    }
    
    // Send wake message to initial agent
    data_t *own_wake_data = ar__data__create_string(ref_wake_message);
    if (own_wake_data) {
        ar__agency__send_to_agent(initial_agent, own_wake_data);
        // Ownership transferred to agent
        own_wake_data = NULL; // Mark as transferred
    }
    printf("Initial agent created with ID: %lld\n\n", initial_agent);
    
    // Process the __wake__ message sent to the initial agent
    printf("Processing initial __wake__ message...\n");
    int processed = ar__system__process_all_messages();
    printf("Processed %d messages\n\n", processed);
    
    // Create a counter agent
    printf("Creating counter agent...\n");
    int64_t counter_id = ar__agency__create_agent("counter", ref_counter_version, NULL);
    if (counter_id == 0) {
        printf("Failed to create counter agent\n");
        ar__system__shutdown();
        return 1;
    }
    printf("Counter agent created with ID: %lld\n\n", counter_id);
    
    // Send some messages to the counter agent
    printf("Sending messages to counter agent...\n");
    data_t *own_incr_msg1 = ar__data__create_string(ref_increment_message);
    data_t *own_incr_msg2 = ar__data__create_string(ref_increment_message);
    data_t *own_incr_msg3 = ar__data__create_string(ref_increment_message);
    data_t *own_get_msg1 = ar__data__create_string(ref_get_message);
    
    if (own_incr_msg1 && own_incr_msg2 && own_incr_msg3 && own_get_msg1) {
        ar__agency__send_to_agent(counter_id, own_incr_msg1);
        // Ownership transferred to agent
        own_incr_msg1 = NULL; // Mark as transferred
        
        ar__agency__send_to_agent(counter_id, own_incr_msg2);
        // Ownership transferred to agent
        own_incr_msg2 = NULL; // Mark as transferred
        
        ar__agency__send_to_agent(counter_id, own_incr_msg3);
        // Ownership transferred to agent
        own_incr_msg3 = NULL; // Mark as transferred
        
        ar__agency__send_to_agent(counter_id, own_get_msg1);
        // Ownership transferred to agent
        own_get_msg1 = NULL; // Mark as transferred
    }
    
    // Process all messages
    printf("Processing messages...\n");
    processed = ar__system__process_all_messages();
    printf("Processed %d messages\n\n", processed);
    
    // Send more messages
    printf("Sending more messages...\n");
    data_t *own_incr_msg4 = ar__data__create_string(ref_increment_message);
    data_t *own_incr_msg5 = ar__data__create_string(ref_increment_message);
    data_t *own_get_msg2 = ar__data__create_string(ref_get_message);
    
    if (own_incr_msg4 && own_incr_msg5 && own_get_msg2) {
        ar__agency__send_to_agent(counter_id, own_incr_msg4);
        // Ownership transferred to agent
        own_incr_msg4 = NULL; // Mark as transferred
        
        ar__agency__send_to_agent(counter_id, own_incr_msg5);
        // Ownership transferred to agent
        own_incr_msg5 = NULL; // Mark as transferred
        
        ar__agency__send_to_agent(counter_id, own_get_msg2);
        // Ownership transferred to agent
        own_get_msg2 = NULL; // Mark as transferred
    }
    
    // Process all messages
    printf("Processing messages...\n");
    processed = ar__system__process_all_messages();
    printf("Processed %d messages\n\n", processed);
    
    // Save agents and methods to disk
    printf("Saving agents and methods to disk...\n");
    bool saved_agents = ar__agency__save_agents();
    bool saved_methods = ar__methodology__save_methods();
    printf("Agents saved: %s\n", saved_agents ? "yes" : "no");
    printf("Methods saved: %s\n\n", saved_methods ? "yes" : "no");
    
    // Shutdown the runtime
    printf("Shutting down runtime...\n");
    ar__system__shutdown();
    printf("Runtime shutdown complete\n\n");
    
    // Demonstrate loading from disk in a new runtime session
    printf("Starting new runtime session...\n");
    printf("Loading methods from disk...\n");
    bool loaded_methods = ar__methodology__load_methods();
    printf("Methods loaded: %s\n", loaded_methods ? "yes" : "no");
    
    // Initialize with echo method again
    initial_agent = ar__system__init("echo", ref_echo_version);
    if (initial_agent == 0) {
        printf("Failed to initialize runtime\n");
        return 1;
    }
    printf("Runtime initialized with initial agent ID: %lld\n", initial_agent);
    
    // Check if the persistent counter agent was loaded
    printf("Checking if counter agent was restored...\n");
    int agent_count = ar__agency__count_agents();
    printf("Total active agents: %d\n", agent_count);
    
    // Should have at least 2 agents (initial agent and counter)
    if (agent_count >= 2) {
        printf("Counter agent was restored\n");
        
        // Send a message to get the current count
        printf("Sending 'get' message to counter agent...\n");
        data_t *own_get_msg3 = ar__data__create_string(ref_get_message);
        if (own_get_msg3) {
            ar__agency__send_to_agent(counter_id, own_get_msg3);
            // Ownership transferred to agent
            own_get_msg3 = NULL; // Mark as transferred
        }
        
        // Process the message
        printf("Processing messages...\n");
        processed = ar__system__process_all_messages();
        printf("Processed %d messages\n", processed);
    } else {
        printf("Counter agent was not restored\n");
    }
    
    // Memory leak fixed - now properly freeing memory
    char *temp = AR__HEAP__MALLOC(256, "Temporary allocation for testing");
    if (temp) {
        strcpy(temp, "This memory will be properly freed");
        AR__HEAP__FREE(temp);
    }
    
    // Final shutdown
    printf("\nFinal shutdown...\n");
    ar__system__shutdown();
    printf("Runtime shutdown complete\n");
    
    return 0;
}

int main(void) {
    return ar__executable__main();
}
