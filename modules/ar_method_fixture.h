#ifndef AGERUN_METHOD_FIXTURE_H
#define AGERUN_METHOD_FIXTURE_H

#include <stdbool.h>
#include "ar_system.h"

/**
 * @file ar_method_fixture.h
 * @brief Method fixture module for AgeRun method testing infrastructure
 * 
 * This module provides a proper abstraction for test setup and teardown operations,
 * eliminating the need for helper functions scattered across test files.
 * It follows Parnas principles by hiding implementation details and providing
 * a cohesive interface for test management.
 */

/* Opaque method fixture type */
typedef struct ar_method_fixture_s ar_method_fixture_t;

/**
 * Creates a new test fixture for AgeRun tests
 * @param ref_test_name Name of the test for identification
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
ar_method_fixture_t* ar_method_fixture__create(const char *ref_test_name);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture
 */
void ar_method_fixture__destroy(ar_method_fixture_t *own_fixture);

/**
 * Initializes the test environment
 * @param mut_fixture The fixture to use for initialization
 * @return true if initialization succeeded, false otherwise
 * @note This ensures system is in clean state and initializes required components
 */
bool ar_method_fixture__initialize(ar_method_fixture_t *mut_fixture);

/**
 * Loads a method file and registers it with the methodology
 * @param mut_fixture The fixture managing the test
 * @param ref_method_name Name to register the method as
 * @param ref_method_file Path to the method file
 * @param ref_version Version string for the method
 * @return true if method was loaded and registered successfully
 */
bool ar_method_fixture__load_method(ar_method_fixture_t *mut_fixture,
                                const char *ref_method_name,
                                const char *ref_method_file,
                                const char *ref_version);

/**
 * Verifies the current directory is correct for method tests
 * @param ref_fixture The fixture to use for verification
 * @return true if in correct directory, false otherwise
 */
bool ar_method_fixture__verify_directory(const ar_method_fixture_t *ref_fixture);

/**
 * Gets the test name from a fixture
 * @param ref_fixture The fixture to query
 * @return The test name (borrowed reference)
 */
const char* ar_method_fixture__get_name(const ar_method_fixture_t *ref_fixture);

/**
 * Checks if there were any memory leaks during the test
 * @param ref_fixture The fixture to check
 * @return true if no memory leaks, false if leaks detected
 */
bool ar_method_fixture__check_memory(const ar_method_fixture_t *ref_fixture);

/**
 * Process the next message in the system
 * @param mut_fixture The fixture managing the system
 * @return true if a message was processed, false if no messages
 */
bool ar_method_fixture__process_next_message(ar_method_fixture_t *mut_fixture);

/**
 * Process all pending messages in the system
 * @param mut_fixture The fixture managing the system
 * @return Number of messages processed
 */
int ar_method_fixture__process_all_messages(ar_method_fixture_t *mut_fixture);

/**
 * Get the agency instance from the fixture's system
 * @param ref_fixture The fixture containing the system
 * @return The agency instance (borrowed reference), or NULL if fixture has no system
 * @note Ownership: Returns a borrowed reference - do not destroy
 */
ar_agency_t* ar_method_fixture__get_agency(const ar_method_fixture_t *ref_fixture);

#endif /* AGERUN_METHOD_FIXTURE_H */
