#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "ar_system.h"
#include "ar_method.h"
#include "ar_agent.h"
#include "ar_agency.h"
#include "ar_methodology.h"
#include "ar_executable.h"
#include "ar_heap.h"

/* Message strings */
static const char *ref_increment_message = "increment";
static const char *ref_get_message = "get";

int ar_executable__main(void) {
    printf("Agerun Example Application\n");
    printf("==========================\n\n");
    
    // Create the system instance
    printf("Creating system instance...\n");
    ar_system_t *mut_system = ar_system__create();
    if (!mut_system) {
        printf("Error: Failed to create system instance\n");
        return 1;
    }
    
    // Initialize the runtime
    printf("Initializing runtime...\n");
    int64_t initial_agent = ar_system__init_with_instance(mut_system, NULL, NULL);
    if (initial_agent != 0) {
        printf("Error: Unexpected agent created during initialization\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    printf("Runtime initialized successfully\n\n");
    
    // Get agency instance from the system
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency) {
        printf("Error: Failed to get agency instance\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Get methodology instance from the agency
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    if (!mut_methodology) {
        printf("Error: Failed to get methodology instance\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Create a simple echo method that handles wake messages properly
    printf("Creating echo method...\n");
    const char *ref_echo_instructions = "if(message = \"__wake__\", send(0, \"Echo agent is awake\"), send(0, message))";
    
    // Create method using methodology instance
    const char *ref_echo_version = "1.0.0";
    if (!ar_methodology__create_method_with_instance(mut_methodology, "echo", ref_echo_instructions, ref_echo_version)) {
        printf("Failed to create echo method\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    printf("Echo method created with version %s\n\n", ref_echo_version);
    
    // Create a simplified counter method that just echoes back messages
    printf("Creating counter method...\n");
    const char *ref_counter_code = "send(0, \"Hello from counter!\")";
    
    // Create method using methodology instance
    const char *ref_counter_version = "1.0.0";
    if (!ar_methodology__create_method_with_instance(mut_methodology, "counter", ref_counter_code, ref_counter_version)) {
        printf("Failed to create counter method\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    printf("Counter method created with version %s\n\n", ref_counter_version);
    
    // Create the initial agent with proper context
    printf("Creating initial agent...\n");
    ar_data_t *own_echo_context = ar_data__create_map();
    if (!own_echo_context) {
        printf("Failed to create context for echo agent\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    initial_agent = ar_agency__create_agent_with_instance(mut_agency, "echo", ref_echo_version, own_echo_context);
    if (initial_agent == 0) {
        printf("Failed to create initial agent\n");
        ar_data__destroy(own_echo_context);
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    // Note: Agent stores reference to context, don't destroy it here
    
    printf("Initial agent created with ID: %" PRId64 "\n\n", initial_agent);
    
    // Process the __wake__ message that the agent sent to itself
    printf("Processing initial __wake__ message...\n");
    int processed = ar_system__process_all_messages_with_instance(mut_system);
    printf("Processed %d messages\n\n", processed);
    
    // Create a counter agent with proper context
    printf("Creating counter agent...\n");
    ar_data_t *own_counter_context = ar_data__create_map();
    if (!own_counter_context) {
        printf("Failed to create context for counter agent\n");
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    
    int64_t counter_id = ar_agency__create_agent_with_instance(mut_agency, "counter", ref_counter_version, own_counter_context);
    if (counter_id == 0) {
        printf("Failed to create counter agent\n");
        ar_data__destroy(own_counter_context);
        ar_system__shutdown_with_instance(mut_system);
        ar_system__destroy(mut_system);
        return 1;
    }
    // Note: Agent stores reference to context, don't destroy it here
    printf("Counter agent created with ID: %" PRId64 "\n\n", counter_id);
    
    // Send some messages to the counter agent
    printf("Sending messages to counter agent...\n");
    ar_data_t *own_incr_msg1 = ar_data__create_string(ref_increment_message);
    ar_data_t *own_incr_msg2 = ar_data__create_string(ref_increment_message);
    ar_data_t *own_incr_msg3 = ar_data__create_string(ref_increment_message);
    ar_data_t *own_get_msg1 = ar_data__create_string(ref_get_message);
    
    if (own_incr_msg1 && own_incr_msg2 && own_incr_msg3 && own_get_msg1) {
        ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_incr_msg1);
        // Ownership transferred to agent
        own_incr_msg1 = NULL; // Mark as transferred
        
        ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_incr_msg2);
        // Ownership transferred to agent
        own_incr_msg2 = NULL; // Mark as transferred
        
        ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_incr_msg3);
        // Ownership transferred to agent
        own_incr_msg3 = NULL; // Mark as transferred
        
        ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_get_msg1);
        // Ownership transferred to agent
        own_get_msg1 = NULL; // Mark as transferred
    }
    
    // Process all messages
    printf("Processing messages...\n");
    processed = ar_system__process_all_messages_with_instance(mut_system);
    printf("Processed %d messages\n\n", processed);
    
    // Send more messages
    printf("Sending more messages...\n");
    ar_data_t *own_incr_msg4 = ar_data__create_string(ref_increment_message);
    ar_data_t *own_incr_msg5 = ar_data__create_string(ref_increment_message);
    ar_data_t *own_get_msg2 = ar_data__create_string(ref_get_message);
    
    if (own_incr_msg4 && own_incr_msg5 && own_get_msg2) {
        ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_incr_msg4);
        // Ownership transferred to agent
        own_incr_msg4 = NULL; // Mark as transferred
        
        ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_incr_msg5);
        // Ownership transferred to agent
        own_incr_msg5 = NULL; // Mark as transferred
        
        ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_get_msg2);
        // Ownership transferred to agent
        own_get_msg2 = NULL; // Mark as transferred
    }
    
    // Process all messages
    printf("Processing messages...\n");
    processed = ar_system__process_all_messages_with_instance(mut_system);
    printf("Processed %d messages\n\n", processed);
    
    // Save agents and methods to disk
    printf("Saving agents and methods to disk...\n");
    bool saved_agents = ar_agency__save_agents_with_instance(mut_agency, NULL);
    bool saved_methods = ar_methodology__save_methods_with_instance(mut_methodology, "methodology.agerun");
    printf("Agents saved: %s\n", saved_agents ? "yes" : "no");
    printf("Methods saved: %s\n\n", saved_methods ? "yes" : "no");
    
    // Shutdown the runtime
    printf("Shutting down runtime...\n");
    ar_system__shutdown_with_instance(mut_system);
    
    // Now that agents are destroyed, we can safely clean up the contexts we created
    ar_data__destroy(own_echo_context);
    ar_data__destroy(own_counter_context);
    
    ar_system__destroy(mut_system);
    printf("Runtime shutdown complete\n\n");
    
    // Demonstrate loading from disk in a new runtime session
    printf("Starting new runtime session...\n");
    
    // Create new system instance for second session
    mut_system = ar_system__create();
    if (!mut_system) {
        printf("Error: Failed to create system instance\n");
        return 1;
    }
    
    // Get new agency instance
    mut_agency = ar_system__get_agency(mut_system);
    if (!mut_agency) {
        printf("Error: Failed to get agency instance\n");
        ar_system__destroy(mut_system);
        return 1;
    }
    
    // Get new methodology instance for loading
    mut_methodology = ar_agency__get_methodology(mut_agency);
    if (!mut_methodology) {
        printf("Error: Failed to get methodology instance\n");
        ar_system__destroy(mut_system);
        return 1;
    }
    
    printf("Loading methods from disk...\n");
    bool loaded_methods = ar_methodology__load_methods_with_instance(mut_methodology, "methodology.agerun");
    printf("Methods loaded: %s\n", loaded_methods ? "yes" : "no");
    
    // Initialize with echo method again
    initial_agent = ar_system__init_with_instance(mut_system, "echo", ref_echo_version);
    if (initial_agent == 0) {
        printf("Failed to initialize runtime\n");
        ar_system__destroy(mut_system);
        return 1;
    }
    printf("Runtime initialized with initial agent ID: %" PRId64 "\n", initial_agent);
    
    // Check if the persistent counter agent was loaded
    printf("Checking if counter agent was restored...\n");
    int agent_count = ar_agency__count_agents_with_instance(mut_agency);
    printf("Total active agents: %d\n", agent_count);
    
    // Should have at least 2 agents (initial agent and counter)
    if (agent_count >= 2) {
        printf("Counter agent was restored\n");
        
        // Send a message to get the current count
        printf("Sending 'get' message to counter agent...\n");
        ar_data_t *own_get_msg3 = ar_data__create_string(ref_get_message);
        if (own_get_msg3) {
            ar_agency__send_to_agent_with_instance(mut_agency, counter_id, own_get_msg3);
            // Ownership transferred to agent
            own_get_msg3 = NULL; // Mark as transferred
        }
        
        // Process the message
        printf("Processing messages...\n");
        processed = ar_system__process_all_messages_with_instance(mut_system);
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
    ar_system__shutdown_with_instance(mut_system);
    ar_system__destroy(mut_system);
    
    printf("Runtime shutdown complete\n");
    
    return 0;
}

int main(void) {
    return ar_executable__main();
}
