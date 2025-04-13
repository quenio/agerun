#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/agerun.h"

int main() {
    printf("Agerun Example Application\n");
    printf("==========================\n\n");
    
    // Create a simple echo method
    printf("Creating echo method...\n");
    version_t echo_version = agerun_method("echo", "send(0, message)", 0, true, false);
    if (echo_version == 0) {
        printf("Failed to create echo method\n");
        return 1;
    }
    printf("Echo method created with version %d\n\n", echo_version);
    
    // Create a counter method that increments a counter on each message
    printf("Creating counter method...\n");
    const char *counter_code = 
        "if(message == \"__wake__\", memory[\"count\"] := 0, \"\")\n"
        "if(message == \"increment\", memory[\"count\"] := memory[\"count\"] + 1, \"\")\n"
        "if(message == \"get\", send(0, build(\"Count: {}\", memory[\"count\"])), \"\")";
    
    version_t counter_version = agerun_method("counter", counter_code, 0, true, true);
    if (counter_version == 0) {
        printf("Failed to create counter method\n");
        return 1;
    }
    printf("Counter method created with version %d\n\n", counter_version);
    
    // Initialize the runtime with the echo method
    printf("Initializing runtime...\n");
    agent_id_t initial_agent = agerun_init("echo", echo_version);
    if (initial_agent == 0) {
        printf("Failed to initialize runtime\n");
        return 1;
    }
    printf("Runtime initialized with initial agent ID: %ld\n\n", initial_agent);
    
    // Process the __wake__ message sent to the initial agent
    printf("Processing initial __wake__ message...\n");
    int processed = agerun_process_all_messages();
    printf("Processed %d messages\n\n", processed);
    
    // Create a counter agent
    printf("Creating counter agent...\n");
    agent_id_t counter_id = agerun_create("counter", counter_version, NULL);
    if (counter_id == 0) {
        printf("Failed to create counter agent\n");
        agerun_shutdown();
        return 1;
    }
    printf("Counter agent created with ID: %ld\n\n", counter_id);
    
    // Send some messages to the counter agent
    printf("Sending messages to counter agent...\n");
    agerun_send(counter_id, "increment");
    agerun_send(counter_id, "increment");
    agerun_send(counter_id, "increment");
    agerun_send(counter_id, "get");
    
    // Process all messages
    printf("Processing messages...\n");
    processed = agerun_process_all_messages();
    printf("Processed %d messages\n\n", processed);
    
    // Send more messages
    printf("Sending more messages...\n");
    agerun_send(counter_id, "increment");
    agerun_send(counter_id, "increment");
    agerun_send(counter_id, "get");
    
    // Process all messages
    printf("Processing messages...\n");
    processed = agerun_process_all_messages();
    printf("Processed %d messages\n\n", processed);
    
    // Save agents and methods to disk
    printf("Saving agents and methods to disk...\n");
    bool saved_agents = agerun_save_agents();
    bool saved_methods = agerun_save_methods();
    printf("Agents saved: %s\n", saved_agents ? "yes" : "no");
    printf("Methods saved: %s\n\n", saved_methods ? "yes" : "no");
    
    // Shutdown the runtime
    printf("Shutting down runtime...\n");
    agerun_shutdown();
    printf("Runtime shutdown complete\n\n");
    
    // Demonstrate loading from disk in a new runtime session
    printf("Starting new runtime session...\n");
    printf("Loading methods from disk...\n");
    bool loaded_methods = agerun_load_methods();
    printf("Methods loaded: %s\n", loaded_methods ? "yes" : "no");
    
    // Initialize with echo method again
    initial_agent = agerun_init("echo", echo_version);
    if (initial_agent == 0) {
        printf("Failed to initialize runtime\n");
        return 1;
    }
    printf("Runtime initialized with initial agent ID: %ld\n", initial_agent);
    
    // Check if the persistent counter agent was loaded
    printf("Checking if counter agent was restored...\n");
    int agent_count = agerun_count_agents();
    printf("Total active agents: %d\n", agent_count);
    
    // Should have at least 2 agents (initial agent and counter)
    if (agent_count >= 2) {
        printf("Counter agent was restored\n");
        
        // Send a message to get the current count
        printf("Sending 'get' message to counter agent...\n");
        agerun_send(counter_id, "get");
        
        // Process the message
        printf("Processing messages...\n");
        processed = agerun_process_all_messages();
        printf("Processed %d messages\n", processed);
    } else {
        printf("Counter agent was not restored\n");
    }
    
    // Final shutdown
    printf("\nFinal shutdown...\n");
    agerun_shutdown();
    printf("Runtime shutdown complete\n");
    
    return 0;
}