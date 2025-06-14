#include "agerun_interpreter_fixture.h"
#include "agerun_heap.h"
#include "agerun_list.h"
#include "agerun_agency.h"
#include "agerun_methodology.h"
#include "agerun_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Interpreter fixture structure (private implementation)
 */
struct interpreter_fixture_s {
    char *own_test_name;              // Name of the test
    interpreter_t *own_interpreter;   // The interpreter instance
    list_t *own_tracked_data;         // List of data objects to destroy
    list_t *own_agent_ids;            // List of agent IDs to destroy
};

/**
 * Creates a new test fixture for AgeRun interpreter module tests
 */
interpreter_fixture_t* ar__interpreter_fixture__create(const char *ref_test_name) {
    if (!ref_test_name) {
        return NULL;
    }
    
    interpreter_fixture_t *own_fixture = AR__HEAP__MALLOC(sizeof(interpreter_fixture_t), "Interpreter fixture");
    if (!own_fixture) {
        return NULL;
    }
    
    // Initialize fields
    own_fixture->own_test_name = AR__HEAP__STRDUP(ref_test_name, "Test name");
    if (!own_fixture->own_test_name) {
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    // Create interpreter
    own_fixture->own_interpreter = ar__interpreter__create();
    if (!own_fixture->own_interpreter) {
        AR__HEAP__FREE(own_fixture->own_test_name);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    // Create tracking lists
    own_fixture->own_tracked_data = ar__list__create();
    if (!own_fixture->own_tracked_data) {
        ar__interpreter__destroy(own_fixture->own_interpreter);
        AR__HEAP__FREE(own_fixture->own_test_name);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    own_fixture->own_agent_ids = ar__list__create();
    if (!own_fixture->own_agent_ids) {
        ar__list__destroy(own_fixture->own_tracked_data);
        ar__interpreter__destroy(own_fixture->own_interpreter);
        AR__HEAP__FREE(own_fixture->own_test_name);
        AR__HEAP__FREE(own_fixture);
        return NULL;
    }
    
    return own_fixture;
}

/**
 * Destroys a test fixture and performs cleanup
 */
void ar__interpreter_fixture__destroy(interpreter_fixture_t *own_fixture) {
    if (!own_fixture) {
        return;
    }
    
    // Destroy all tracked agents
    size_t agent_count = ar__list__count(own_fixture->own_agent_ids);
    if (agent_count > 0) {
        void **items = ar__list__items(own_fixture->own_agent_ids);
        if (items) {
            for (size_t i = 0; i < agent_count; i++) {
                data_t *own_id_data = (data_t *)items[i];
                if (own_id_data && ar__data__get_type(own_id_data) == DATA_INTEGER) {
                    int64_t agent_id = (int64_t)ar__data__get_integer(own_id_data);
                    ar__agency__destroy_agent(agent_id);
                    ar__data__destroy(own_id_data); // Destroy the integer data object
                }
            }
            AR__HEAP__FREE(items); // Free the array
        }
    }
    ar__list__destroy(own_fixture->own_agent_ids);
    
    // Destroy all tracked data
    size_t data_count = ar__list__count(own_fixture->own_tracked_data);
    if (data_count > 0) {
        void **items = ar__list__items(own_fixture->own_tracked_data);
        if (items) {
            for (size_t i = 0; i < data_count; i++) {
                data_t *own_data = (data_t *)items[i];
                if (own_data) {
                    ar__data__destroy(own_data);
                }
            }
            AR__HEAP__FREE(items); // Free the array
        }
    }
    ar__list__destroy(own_fixture->own_tracked_data);
    
    // Destroy interpreter
    ar__interpreter__destroy(own_fixture->own_interpreter);
    
    // Free test name and fixture
    AR__HEAP__FREE(own_fixture->own_test_name);
    AR__HEAP__FREE(own_fixture);
}

/**
 * Gets the interpreter managed by the fixture
 */
interpreter_t* ar__interpreter_fixture__get_interpreter(const interpreter_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    return ref_fixture->own_interpreter;
}

/**
 * Creates a test agent with the given method
 */
int64_t ar__interpreter_fixture__create_agent(
    interpreter_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions,
    const char *ref_version) {
    
    if (!mut_fixture || !ref_method_name || !ref_instructions) {
        return 0;
    }
    
    const char *version = ref_version ? ref_version : "1.0.0";
    
    // Create and register method
    method_t *own_method = ar__method__create(ref_method_name, ref_instructions, version);
    if (!own_method) {
        return 0;
    }
    
    ar__methodology__register_method(own_method);
    // Ownership transferred to methodology
    
    // Create agent
    int64_t agent_id = ar__agency__create_agent(ref_method_name, version, NULL);
    if (agent_id == 0) {
        return 0;
    }
    
    // Track agent ID
    data_t *own_id_data = ar__data__create_integer((int)agent_id);
    if (own_id_data) {
        ar__list__add_last(mut_fixture->own_agent_ids, own_id_data);
    }
    
    // Process wake message
    ar__system__process_next_message();
    
    return agent_id;
}

/**
 * Executes an instruction in the context of an agent
 */
bool ar__interpreter_fixture__execute_instruction(
    interpreter_fixture_t *mut_fixture,
    int64_t agent_id,
    const char *ref_instruction) {
    
    return ar__interpreter_fixture__execute_with_message(mut_fixture, agent_id, ref_instruction, NULL);
}

/**
 * Executes an instruction with a custom message
 */
bool ar__interpreter_fixture__execute_with_message(
    interpreter_fixture_t *mut_fixture,
    int64_t agent_id,
    const char *ref_instruction,
    const data_t *ref_message) {
    
    if (!mut_fixture || agent_id == 0 || !ref_instruction) {
        return false;
    }
    
    // Get agent memory and context
    data_t *mut_memory = ar__agency__get_agent_mutable_memory(agent_id);
    const data_t *ref_context = ar__agency__get_agent_context(agent_id);
    
    if (!mut_memory) {
        return false;
    }
    
    // Create instruction context
    instruction_context_t *own_ctx = ar__instruction__create_context(mut_memory, ref_context, ref_message);
    if (!own_ctx) {
        return false;
    }
    
    // Debug: Check if message is set
    if (ref_message) {
        fprintf(stderr, "DEBUG: Message is provided to context\n");
    } else {
        fprintf(stderr, "DEBUG: Message is NULL\n");
    }
    
    // Execute instruction
    fprintf(stderr, "DEBUG: Executing instruction: '%s'\n", ref_instruction);
    bool result = ar__interpreter__execute_instruction(mut_fixture->own_interpreter, own_ctx, ref_instruction);
    fprintf(stderr, "DEBUG: Instruction result: %s\n", result ? "true" : "false");
    
    // Clean up
    ar__instruction__destroy_context(own_ctx);
    
    return result;
}

/**
 * Creates and registers a method for testing
 */
bool ar__interpreter_fixture__create_method(
    interpreter_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions,
    const char *ref_version) {
    
    if (!mut_fixture || !ref_method_name || !ref_instructions) {
        return false;
    }
    
    const char *version = ref_version ? ref_version : "1.0.0";
    
    // Create method
    method_t *own_method = ar__method__create(ref_method_name, ref_instructions, version);
    if (!own_method) {
        return false;
    }
    
    // Register method
    ar__methodology__register_method(own_method);
    // Ownership transferred to methodology
    
    return true;
}

/**
 * Gets an agent's memory
 */
data_t* ar__interpreter_fixture__get_agent_memory(
    const interpreter_fixture_t *ref_fixture,
    int64_t agent_id) {
    
    (void)ref_fixture; // Not used, but kept for API consistency
    return ar__agency__get_agent_mutable_memory(agent_id);
}

/**
 * Sends a message to an agent and processes it
 */
bool ar__interpreter_fixture__send_message(
    interpreter_fixture_t *mut_fixture,
    int64_t agent_id,
    data_t *own_message) {
    
    (void)mut_fixture; // Not used, but kept for API consistency
    
    if (agent_id == 0 || !own_message) {
        if (own_message) {
            ar__data__destroy(own_message);
        }
        return false;
    }
    
    // Send message
    bool sent = ar__agency__send_to_agent(agent_id, own_message);
    if (!sent) {
        return false;
    }
    
    // Process the message
    return ar__system__process_next_message();
}

/**
 * Creates a test data map with common test values
 */
data_t* ar__interpreter_fixture__create_test_map(
    interpreter_fixture_t *mut_fixture,
    const char *ref_name) {
    
    if (!mut_fixture) {
        return NULL;
    }
    
    // Create map
    data_t *own_map = ar__data__create_map();
    if (!own_map) {
        return NULL;
    }
    
    // Add common test values
    data_t *own_name = ar__data__create_string(ref_name ? ref_name : "test");
    if (own_name) {
        ar__data__set_map_data(own_map, "name", own_name);
    }
    
    data_t *own_count = ar__data__create_integer(42);
    if (own_count) {
        ar__data__set_map_data(own_map, "count", own_count);
    }
    
    data_t *own_value = ar__data__create_double(3.14);
    if (own_value) {
        ar__data__set_map_data(own_map, "value", own_value);
    }
    
    data_t *own_flag = ar__data__create_integer(1);
    if (own_flag) {
        ar__data__set_map_data(own_map, "flag", own_flag);
    }
    
    // Track the map
    ar__list__add_last(mut_fixture->own_tracked_data, own_map);
    
    return own_map;
}

/**
 * Tracks a data object for automatic cleanup
 */
void ar__interpreter_fixture__track_data(
    interpreter_fixture_t *mut_fixture,
    data_t *own_data) {
    
    if (!mut_fixture || !own_data) {
        return;
    }
    
    ar__list__add_last(mut_fixture->own_tracked_data, own_data);
}

/**
 * Gets the test name from a fixture
 */
const char* ar__interpreter_fixture__get_name(const interpreter_fixture_t *ref_fixture) {
    if (!ref_fixture) {
        return NULL;
    }
    return ref_fixture->own_test_name;
}
