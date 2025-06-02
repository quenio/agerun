#ifndef AGERUN_FOUNDATION_FIXTURE_H
#define AGERUN_FOUNDATION_FIXTURE_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_expression.h"

/**
 * @file agerun_foundation_fixture.h
 * @brief Foundation fixture for AgeRun foundation module testing infrastructure
 * 
 * This module provides a proper abstraction for foundation module test setup and
 * teardown operations, eliminating repetitive patterns in data structure creation
 * and expression context setup. It focuses on patterns common to foundation modules
 * like data, expression, and instruction that don't require system initialization.
 */

/* Opaque foundation fixture type */
typedef struct foundation_fixture_s foundation_fixture_t;

/**
 * Creates a new test fixture for AgeRun foundation module tests
 * @param ref_test_name Name of the test for identification
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
foundation_fixture_t* ar_foundation_fixture_create(const char *ref_test_name);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture and all tracked resources
 */
void ar_foundation_fixture_destroy(foundation_fixture_t *own_fixture);

/**
 * Creates an expression context with standard test data
 * @param mut_fixture The fixture managing the test
 * @param ref_expression The expression string to evaluate
 * @return A newly created expression context
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 * @note The context is created with pre-populated memory, context, and message maps
 */
expression_context_t* ar_foundation_fixture_create_expression_context(
    foundation_fixture_t *mut_fixture,
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
expression_context_t* ar_foundation_fixture_create_custom_expression_context(
    foundation_fixture_t *mut_fixture,
    data_t *mut_memory,
    const data_t *ref_context,
    const data_t *ref_message,
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
data_t* ar_foundation_fixture_create_test_map(
    foundation_fixture_t *mut_fixture,
    const char *ref_name
);

/**
 * Creates an empty data map
 * @param mut_fixture The fixture managing the test
 * @return A newly created empty data map
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 */
data_t* ar_foundation_fixture_create_empty_map(
    foundation_fixture_t *mut_fixture
);

/**
 * Creates a test data list with sample values
 * @param mut_fixture The fixture managing the test
 * @return A newly created data list
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 */
data_t* ar_foundation_fixture_create_test_list(
    foundation_fixture_t *mut_fixture
);

/**
 * Gets the test name from a fixture
 * @param ref_fixture The fixture to query
 * @return The test name (borrowed reference)
 */
const char* ar_foundation_fixture_get_name(const foundation_fixture_t *ref_fixture);

/**
 * Checks if there were any memory leaks during the test
 * @param ref_fixture The fixture to check
 * @return true if no memory leaks, false if leaks detected
 */
bool ar_foundation_fixture_check_memory(const foundation_fixture_t *ref_fixture);

/**
 * Tracks a data object for automatic cleanup
 * @param mut_fixture The fixture managing the test
 * @param own_data The data object to track
 * @note Ownership: Takes ownership of the data object
 * @note Use this for data objects created outside the fixture helpers
 */
void ar_foundation_fixture_track_data(
    foundation_fixture_t *mut_fixture,
    data_t *own_data
);

/**
 * Tracks an expression context for automatic cleanup
 * @param mut_fixture The fixture managing the test
 * @param own_context The expression context to track
 * @note Ownership: Takes ownership of the expression context
 * @note Use this for contexts created outside the fixture helpers
 */
void ar_foundation_fixture_track_expression_context(
    foundation_fixture_t *mut_fixture,
    expression_context_t *own_context
);

#endif /* AGERUN_FOUNDATION_FIXTURE_H */
