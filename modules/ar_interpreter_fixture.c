#include "ar_interpreter_fixture.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_agency.h"
#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_system.h"
#include "ar_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

/**
 * Interpreter fixture structure (private implementation)
 */
struct ar_interpreter_fixture_s {
    char *own_test_name;              // Name of the test
    ar_log_t *own_log;                // Log instance for the interpreter
    ar_interpreter_t *own_interpreter;   // The interpreter instance
    ar_list_t *own_tracked_data;         // List of data objects to destroy
    ar_list_t *own_agent_ids;            // List of agent IDs to destroy
};

/**
 * Creates a new test fixture for AgeRun interpreter module tests
 */
ar_interpreter_fixture_t* ar_interpreter_fixture__create(const char *ref_test_name) {
    if (!ref_test_name) {
        return NULL;
    }
    
    ar_interpreter_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(ar_interpreter_fixture_t), "Interpreter fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    // Initialize fields
    own_fixture->own_test_name = AR__HEAP__STRDUP(ref_test_name, "Test name");
    if (!own_fixture->own_test_name) {
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    // Create log
    own_fixture->own_log = ar_log__create();
    if (!own_fixture->own_log) {
        AR__HEAP__FREE(own_fixture->own_test_name);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    // Create interpreter
    own_fixture->own_interpreter = ar_interpreter__create(own_fixture->own_log);
    if (!own_fixture->own_interpreter) {
        ar_log__destroy(own_fixture->own_log);
        AR__HEAP__FREE(own_fixture->own_test_name);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    // Create tracking lists
    own_fixture->own_tracked_data = ar_list__create();
    if (!own_fixture->own_tracked_data) {
        ar_interpreter__destroy(own_fixture->own_interpreter);
        ar_log__destroy(own_fixture->own_log);
        AR__HEAP__FREE(own_fixture->own_test_name);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_agent_ids = ar_list__create();
    if (!own_fixture->own_agent_ids) {
        ar_list__destroy(own_fixture->own_tracked_data);
        ar_interpreter__destroy(own_fixture->own_interpreter);
        ar_log__destroy(own_fixture->own_log);
        AR__HEAP__FREE(own_fixture->own_test_name);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    return own_fixture;
}

/**
 * Destroys a test fixture and performs cleanup
 */
void ar_interpreter_fixture__destroy(ar_interpreter_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // Destroy all tracked agents
    size_t agent_count = ar_list__count(own_fixture->own_agent_ids);
    if (agent_count > 0) {
        void **items = ar_list__items(own_fixture->own_agent_ids);
        if (items) {
            for (size_t i = 0; i < agent_count; i++) {
                ar_data_t *own_id_data = (ar_data_t *)items[i];
                if (own_id_data && ar_data__get_type(own_id_data) == AR_DATA_TYPE__INTEGER) {
                    int64_t agent_id = (int64_t)ar_data__get_integer(own_id_data);
                    ar_agency__destroy_agent(agent_id);
                    // Process any remaining messages after destroying each agent
                    while (ar_system__process_next_message()) {
                        // Keep processing
                    }
                    ar_data__destroy(own_id_data); // Destroy the integer data object
                }
            }
            AR__HEAP__FREE(items); // Free the array
        }
    }
    ar_list__destroy(own_fixture->own_agent_ids);
    
    // Destroy all tracked data
    size_t data_count = ar_list__count(own_fixture->own_tracked_data);
    if (data_count > 0) {
        void **items = ar_list__items(own_fixture->own_tracked_data);
        if (items) {
            for (size_t i = 0; i < data_count; i++) {
                ar_data_t *own_data = (ar_data_t *)items[i];
                if (own_data) {
                    ar_data__destroy(own_data);
                }
            }
            AR__HEAP__FREE(items); // Free the array
        }
    }
    ar_list__destroy(own_fixture->own_tracked_data);
    
    // Destroy interpreter
    ar_interpreter__destroy(own_fixture->own_interpreter);
    
    // Destroy log
    ar_log__destroy(own_fixture->own_log);
    
    // Free test name and fixture
    AR__HEAP__FREE(own_fixture->own_test_name);
    AR__HEAP__FREE(own_fixture);
}

/**
 * Gets the interpreter managed by the fixture
 */
ar_interpreter_t* ar_interpreter_fixture__get_interpreter(const ar_interpreter_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    return ref_fixture->own_interpreter;
}

/**
 * Gets the log managed by the fixture
 */
ar_log_t* ar_interpreter_fixture__get_log(const ar_interpreter_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    return ref_fixture->own_log;
}

/**
 * Creates a test agent with the given method
 */
int64_t ar_interpreter_fixture__create_agent(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions,
    const char *ref_version) {
    
    if (!mut_fixture || !ref_method_name || !ref_instructions) {
        return 0;
    }
    
    const char *version = ref_version ? ref_version : "1.0.0";
    
    // Create and register method
    ar_method_t *own_method = ar_method__create(ref_method_name, ref_instructions, version);
    if (!own_method) {
        return 0;
    }
    
    ar_methodology__register_method(own_method);
    // Ownership transferred to methodology
    
    // Create empty context for agent
    ar_data_t *own_context = ar_data__create_map();
    if (!own_context) {
        return 0;
    }
    
    // Create agent with context (agency takes reference, not ownership)
    int64_t agent_id = ar_agency__create_agent(ref_method_name, version, own_context);
    if (agent_id == 0) {
        ar_data__destroy(own_context);
        return 0;
    }
    
    // Track the context for cleanup
    ar_list__add_last(mut_fixture->own_tracked_data, own_context);
    
    // Track agent ID
    ar_data_t *own_id_data = ar_data__create_integer((int)agent_id);
    if (own_id_data) {
        ar_list__add_last(mut_fixture->own_agent_ids, own_id_data);
    }
    
    // Process wake message
    ar_system__process_next_message();
    
    return agent_id;
}

/**
 * Executes an instruction in the context of an agent
 */
int64_t ar_interpreter_fixture__execute_instruction(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_instruction) {
    
    return ar_interpreter_fixture__execute_with_message(mut_fixture, ref_instruction, NULL);
}

/**
 * Executes an instruction with a custom message
 * 
 * This function creates a temporary agent with a method containing the given instruction,
 * then executes it.
 */
int64_t ar_interpreter_fixture__execute_with_message(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_instruction,
    const ar_data_t *ref_message) {
    
    if (!mut_fixture || !ref_instruction) {
        return 0;
    }
    
    // Create a unique method name for this test instruction
    static int test_counter = 0;
    char method_name[256];
    snprintf(method_name, sizeof(method_name), "__test_instruction_%d__", test_counter++);
    
    // Create and register a temporary method with the single instruction
    ar_method_t *own_temp_method = ar_method__create(
        method_name,
        ref_instruction,
        "1.0.0"
    );
    
    if (!own_temp_method) {
        return 0;
    }
    
    // Register the method
    ar_methodology__register_method(own_temp_method);
    // Ownership transferred to methodology
    
    // Create a default context for the agent
    ar_data_t *own_default_context = ar_data__create_map();
    if (!own_default_context) {
        ar_methodology__unregister_method(method_name, "1.0.0");
        return 0;
    }
    
    // Create a temporary agent with this method
    int64_t temp_agent_id = ar_agency__create_agent(method_name, "1.0.0", own_default_context);
    // Ownership of context transferred to agency
    
    if (temp_agent_id == 0) {
        // Unregister the method since agent creation failed
        ar_data__destroy(own_default_context);
        ar_methodology__unregister_method(method_name, "1.0.0");
        return 0;
    }
    
    // Process the wake message
    ar_system__process_next_message();
    
    // Debug: Check if message is set
    if (ref_message) {
        fprintf(stderr, "DEBUG: Message is provided to execute_method\n");
    } else {
        fprintf(stderr, "DEBUG: Message is NULL\n");
    }
    
    // Create a default message if none provided
    ar_data_t *own_default_message = NULL;
    const ar_data_t *message_to_use = ref_message;
    if (!message_to_use) {
        own_default_message = ar_data__create_map();
        message_to_use = own_default_message;
    }
    
    // Execute the temporary agent's method
    fprintf(stderr, "DEBUG: Executing instruction via temporary agent: '%s'\n", ref_instruction);
    bool result = ar_interpreter__execute_method(
        mut_fixture->own_interpreter, 
        temp_agent_id, 
        message_to_use
    );
    fprintf(stderr, "DEBUG: Instruction result: %s\n", result ? "true" : "false");
    
    // Clean up default message if created
    if (own_default_message) {
        ar_data__destroy(own_default_message);
    }
    
    // If execution failed, clean up and return 0
    if (!result) {
        ar_agency__destroy_agent(temp_agent_id);
        ar_methodology__unregister_method(method_name, "1.0.0");
        return 0;
    }
    
    // Return the temporary agent ID for the caller to use
    // Caller is responsible for calling ar_interpreter_fixture__destroy_temp_agent
    return temp_agent_id;
}

/**
 * Creates and registers a method for testing
 */
bool ar_interpreter_fixture__create_method(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions,
    const char *ref_version) {
    
    if (!mut_fixture || !ref_method_name || !ref_instructions) {
        return false;
    }
    
    const char *version = ref_version ? ref_version : "1.0.0";
    
    // Create method
    ar_method_t *own_method = ar_method__create(ref_method_name, ref_instructions, version);
    if (!own_method) {
        return false;
    }
    
    // Register method
    ar_methodology__register_method(own_method);
    // Ownership transferred to methodology
    
    return true;
}

/**
 * Gets an agent's memory
 */
ar_data_t* ar_interpreter_fixture__get_agent_memory(
    const ar_interpreter_fixture_t *ref_fixture,
    int64_t agent_id) {
    
    (void)ref_fixture; // Not used, but kept for API consistency
    return ar_agency__get_agent_mutable_memory(agent_id);
}

/**
 * Sends a message to an agent and processes it
 */
bool ar_interpreter_fixture__send_message(
    ar_interpreter_fixture_t *mut_fixture,
    int64_t agent_id,
    ar_data_t *own_message) {
    
    (void)mut_fixture; // Not used, but kept for API consistency
    
    if (agent_id == 0 || !own_message) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    
    // Send message
    bool sent = ar_agency__send_to_agent(agent_id, own_message);
    if (!sent) {
        return false;
    }
    
    // Process the message
    return ar_system__process_next_message();
}

/**
 * Creates a test data map with common test values
 */
ar_data_t* ar_interpreter_fixture__create_test_map(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_name) {
    
    if (!mut_fixture) {
        return NULL;
    }
    
    // Create map
    ar_data_t *own_map = ar_data__create_map();
    if (!own_map) {
        return NULL;
    }
    
    // Add common test values
    ar_data_t *own_name = ar_data__create_string(ref_name ? ref_name : "test");
    if (own_name) {
        ar_data__set_map_data(own_map, "name", own_name);
    }
    
    ar_data_t *own_count = ar_data__create_integer(42);
    if (own_count) {
        ar_data__set_map_data(own_map, "count", own_count);
    }
    
    ar_data_t *own_value = ar_data__create_double(3.14);
    if (own_value) {
        ar_data__set_map_data(own_map, "value", own_value);
    }
    
    ar_data_t *own_flag = ar_data__create_integer(1);
    if (own_flag) {
        ar_data__set_map_data(own_map, "flag", own_flag);
    }
    
    // Track the map
    ar_list__add_last(mut_fixture->own_tracked_data, own_map);
    
    return own_map;
}

/**
 * Tracks a data object for automatic cleanup
 */
void ar_interpreter_fixture__track_data(
    ar_interpreter_fixture_t *mut_fixture,
    ar_data_t *own_data) {
    
    if (!mut_fixture || !own_data) {
        return;
    }
    
    ar_list__add_last(mut_fixture->own_tracked_data, own_data);
}

/**
 * Gets the test name from a fixture
 */
const char* ar_interpreter_fixture__get_name(const ar_interpreter_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    return ref_fixture->own_test_name;
}

/**
 * Destroys a temporary agent created by execute functions
 */
void ar_interpreter_fixture__destroy_temp_agent(
    ar_interpreter_fixture_t *mut_fixture,
    int64_t temp_agent_id) {
    
    if (!mut_fixture || temp_agent_id == 0) {
        return;
    }
    
    // Get the agent's method info to unregister it
    const char *method_name = NULL;
    const char *method_version = NULL;
    ar_agency__get_agent_method_info(temp_agent_id, &method_name, &method_version);
    
    // Get the agent's context before destroying the agent
    // The agent doesn't own the context, so we need to destroy it ourselves
    const ar_data_t *ref_context = ar_agency__get_agent_context(temp_agent_id);
    
    // Destroy the agent first
    ar_agency__destroy_agent(temp_agent_id);
    
    // Process any remaining messages (including sleep messages)
    // Need to process all messages since destroy might generate multiple
    while (ar_system__process_next_message()) {
        // Keep processing
    }
    
    // Destroy the context that we created for this temporary agent
    // We can cast away const because we are the ones who created this context
    if (ref_context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-qual"
        ar_data__destroy((ar_data_t*)ref_context);  // Cast away const - we own this
        #pragma GCC diagnostic pop
    }
    
    // Unregister the temporary method
    if (method_name && method_version) {
        ar_methodology__unregister_method(method_name, method_version);
    }
}
