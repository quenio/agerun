#ifndef AGERUN_INTERPRETER_FIXTURE_H
#define AGERUN_INTERPRETER_FIXTURE_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_interpreter.h"
#include "ar_instruction.h"
#include "ar_data.h"
#include "ar_method.h"

/**
 * @file ar_interpreter_fixture.h
 * @brief Test fixture for AgeRun interpreter module tests
 * 
 * This module provides test infrastructure specifically for the interpreter module's
 * test suite. It eliminates repetitive patterns in interpreter creation, agent setup,
 * method registration, and instruction execution that are common across 
 * interpreter module tests.
 */

/* Opaque interpreter fixture type */
typedef struct ar_interpreter_fixture_s ar_interpreter_fixture_t;

/**
 * Creates a new test fixture for AgeRun interpreter module tests
 * @param ref_test_name Name of the test for identification
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 * @note Automatically creates an interpreter instance and initializes the system
 */
ar_interpreter_fixture_t* ar_interpreter_fixture__create(const char *ref_test_name);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture and all tracked resources
 * @note Automatically destroys the interpreter and cleans up the system
 */
void ar_interpreter_fixture__destroy(ar_interpreter_fixture_t *own_fixture);

/**
 * Gets the interpreter managed by the fixture
 * @param ref_fixture The fixture to query
 * @return The interpreter instance (borrowed reference)
 * @note Ownership: Returns a borrowed reference; fixture owns the interpreter
 */
ar_interpreter_t* ar_interpreter_fixture__get_interpreter(const ar_interpreter_fixture_t *ref_fixture);

/**
 * Creates a test agent with the given method
 * @param mut_fixture The fixture managing the test  
 * @param ref_method_name Name for the test method
 * @param ref_instructions Instructions for the test method
 * @param ref_version Version of the method (defaults to "1.0.0" if NULL)
 * @return Agent ID if successful, 0 on error
 * @note The fixture handles method registration and agent cleanup
 * @note Processes the wake message automatically
 */
int64_t ar_interpreter_fixture__create_agent(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions,
    const char *ref_version
);

/**
 * Executes an instruction in the context of an agent
 * @param mut_fixture The fixture managing the test
 * @param agent_id The agent whose context to use
 * @param ref_instruction The instruction to execute
 * @return true if execution succeeded, false otherwise
 * @note Uses the fixture's interpreter to execute the instruction
 */
bool ar_interpreter_fixture__execute_instruction(
    ar_interpreter_fixture_t *mut_fixture,
    int64_t agent_id,
    const char *ref_instruction
);

/**
 * Executes an instruction with a custom message
 * @param mut_fixture The fixture managing the test
 * @param agent_id The agent whose context to use
 * @param ref_instruction The instruction to execute
 * @param ref_message The message to provide as context (can be NULL)
 * @return true if execution succeeded, false otherwise
 * @note The message is not destroyed by this function
 */
bool ar_interpreter_fixture__execute_with_message(
    ar_interpreter_fixture_t *mut_fixture,
    int64_t agent_id,
    const char *ref_instruction,
    const ar_data_t *ref_message
);

/**
 * Creates and registers a method for testing
 * @param mut_fixture The fixture managing the test
 * @param ref_method_name Name of the method
 * @param ref_instructions Method instructions
 * @param ref_version Version (defaults to "1.0.0" if NULL)
 * @return true if successful, false on error
 * @note The fixture tracks the method for cleanup
 */
bool ar_interpreter_fixture__create_method(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions,
    const char *ref_version
);

/**
 * Gets an agent's memory
 * @param ref_fixture The fixture to query
 * @param agent_id The agent whose memory to get
 * @return The agent's memory (borrowed reference) or NULL if not found
 * @note Ownership: Returns a mutable borrowed reference for testing
 */
ar_data_t* ar_interpreter_fixture__get_agent_memory(
    const ar_interpreter_fixture_t *ref_fixture,
    int64_t agent_id
);

/**
 * Sends a message to an agent and processes it
 * @param mut_fixture The fixture managing the test
 * @param agent_id The target agent
 * @param own_message The message to send (ownership transferred)
 * @return true if message was sent and processed successfully
 * @note Ownership: Takes ownership of the message
 */
bool ar_interpreter_fixture__send_message(
    ar_interpreter_fixture_t *mut_fixture,
    int64_t agent_id,
    ar_data_t *own_message
);

/**
 * Creates a test data map with common test values
 * @param mut_fixture The fixture managing the test
 * @param ref_name Name identifier for the map
 * @return A newly created data map
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 * @note The map is pre-populated with common test values
 */
ar_data_t* ar_interpreter_fixture__create_test_map(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_name
);

/**
 * Tracks a data object for automatic cleanup
 * @param mut_fixture The fixture managing the test
 * @param own_data The data object to track
 * @note Ownership: Takes ownership of the data object
 * @note Use this for data objects created outside the fixture helpers
 */
void ar_interpreter_fixture__track_data(
    ar_interpreter_fixture_t *mut_fixture,
    ar_data_t *own_data
);

/**
 * Gets the test name from a fixture
 * @param ref_fixture The fixture to query
 * @return The test name (borrowed reference)
 */
const char* ar_interpreter_fixture__get_name(const ar_interpreter_fixture_t *ref_fixture);

#endif /* AGERUN_INTERPRETER_FIXTURE_H */
