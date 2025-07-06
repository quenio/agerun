#ifndef AGERUN_INSTRUCTION_FIXTURE_H
#define AGERUN_INSTRUCTION_FIXTURE_H

#include <stdbool.h>
#include "ar_data.h"
#include "ar_expression.h"
#include "ar_agent.h"
#include "ar_method.h"
#include "ar_system.h"

/**
 * @file agerun_instruction_fixture.h
 * @brief Test fixture for AgeRun instruction module tests
 * 
 * This module provides test infrastructure specifically for the instruction module's
 * test suite. It eliminates repetitive patterns in agent creation, method registration,
 * data structure creation, and expression context setup that are common across 
 * instruction module tests.
 */

/* Opaque instruction fixture type */
typedef struct ar_instruction_fixture_s ar_instruction_fixture_t;

/**
 * Creates a new test fixture for AgeRun instruction module tests
 * @param ref_test_name Name of the test for identification
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
ar_instruction_fixture_t* ar_instruction_fixture__create(const char *ref_test_name);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture and all tracked resources
 */
void ar_instruction_fixture__destroy(ar_instruction_fixture_t *own_fixture);

/**
 * Creates an expression context with standard test data
 * @param mut_fixture The fixture managing the test
 * @param ref_expression The expression string to evaluate
 * @return A newly created expression context
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 * @note The context is created with pre-populated memory, context, and message maps
 */
ar_expression_context_t* ar_instruction_fixture__create_expression_context(
    ar_instruction_fixture_t *mut_fixture,
    const char *ref_expression
);

/**
 * Creates an expression context with custom data
 * @param mut_fixture The fixture managing the test
 * @param mut_memory Memory map for the expression context (can be NULL)
 * @param ref_context Context map for the expression context (can be NULL)
 * @param ref_message Message map for the expression context (can be NULL)
 * @param ref_expression The expression string to evaluate
 * @return A newly created expression context
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 * @note The memory parameter is mutable because expression evaluation returns direct
 *       references to memory fields. While expressions don't modify memory, the API
 *       requires mutable access for type correctness when returning these references.
 */
ar_expression_context_t* ar_instruction_fixture__create_custom_expression_context(
    ar_instruction_fixture_t *mut_fixture,
    ar_data_t *mut_memory,
    const ar_data_t *ref_context,
    const ar_data_t *ref_message,
    const char *ref_expression
);

/**
 * Creates a test data map with common test values
 * @param mut_fixture The fixture managing the test
 * @param ref_name Name identifier for the map
 * @return A newly created data map
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 * @note The map is pre-populated with common test values
 */
ar_data_t* ar_instruction_fixture__create_test_map(
    ar_instruction_fixture_t *mut_fixture,
    const char *ref_name
);

/**
 * Creates an empty data map
 * @param mut_fixture The fixture managing the test
 * @return A newly created empty data map
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 */
ar_data_t* ar_instruction_fixture__create_empty_map(
    ar_instruction_fixture_t *mut_fixture
);

/**
 * Creates a test data list with sample values
 * @param mut_fixture The fixture managing the test
 * @return A newly created data list
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 */
ar_data_t* ar_instruction_fixture__create_test_list(
    ar_instruction_fixture_t *mut_fixture
);

/**
 * Gets the test name from a fixture
 * @param ref_fixture The fixture to query
 * @return The test name (borrowed reference)
 */
const char* ar_instruction_fixture__get_name(const ar_instruction_fixture_t *ref_fixture);

/**
 * Checks if there were any memory leaks during the test
 * @param ref_fixture The fixture to check
 * @return true if no memory leaks, false if leaks detected
 */
bool ar_instruction_fixture__check_memory(const ar_instruction_fixture_t *ref_fixture);

/**
 * Tracks a data object for automatic cleanup
 * @param mut_fixture The fixture managing the test
 * @param own_data The data object to track
 * @note Ownership: Takes ownership of the data object
 * @note Use this for data objects created outside the fixture helpers
 */
void ar_instruction_fixture__track_data(
    ar_instruction_fixture_t *mut_fixture,
    ar_data_t *own_data
);

/**
 * Tracks an expression context for automatic cleanup
 * @param mut_fixture The fixture managing the test
 * @param own_context The expression context to track
 * @note Ownership: Takes ownership of the expression context
 * @note Use this for contexts created outside the fixture helpers
 */
void ar_instruction_fixture__track_expression_context(
    ar_instruction_fixture_t *mut_fixture,
    ar_expression_context_t *own_context
);

/**
 * Creates a test agent with the given method
 * @param mut_fixture The fixture managing the test  
 * @param ref_method_name Name for the test method
 * @param ref_instructions Instructions for the test method
 * @return Agent ID if successful, 0 on error
 * @note The fixture handles method registration and agent cleanup
 * @note Processes the wake message automatically
 */
int64_t ar_instruction_fixture__create_test_agent(
    ar_instruction_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions
);

/**
 * Gets the agent ID created by the fixture
 * @param ref_fixture The fixture to query
 * @return Agent ID if created, 0 otherwise
 */
int64_t ar_instruction_fixture__get_agent(const ar_instruction_fixture_t *ref_fixture);

/**
 * Tracks a generic pointer for cleanup with a custom destructor
 * @param mut_fixture The fixture managing the test
 * @param own_resource The resource to track
 * @param destructor Function to call to destroy the resource
 * @note Ownership: Takes ownership of the resource
 */
void ar_instruction_fixture__track_resource(
    ar_instruction_fixture_t *mut_fixture,
    void *own_resource,
    void (*destructor)(void*)
);

/**
 * Sets up test system initialization
 * @param mut_fixture The fixture managing the test
 * @param ref_init_method_name Initial method name for system
 * @param ref_init_instructions Initial method instructions
 * @return true if successful, false on error
 * @note This must be called before creating agents if system needs initialization
 */
bool ar_instruction_fixture__init_system(
    ar_instruction_fixture_t *mut_fixture,
    const char *ref_init_method_name,
    const char *ref_init_instructions
);

#endif /* AGERUN_INSTRUCTION_FIXTURE_H */
