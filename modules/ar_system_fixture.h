#ifndef AGERUN_SYSTEM_FIXTURE_H
#define AGERUN_SYSTEM_FIXTURE_H

#include <stdbool.h>
#include "ar_method.h"
#include "ar_system.h"

/**
 * @file ar_system_fixture.h
 * @brief System fixture for AgeRun system module testing infrastructure
 * 
 * This module provides a proper abstraction for system module test setup and teardown
 * operations, eliminating the need for helper functions scattered across test files.
 * It focuses on patterns common to system module tests that require the full AgeRun
 * runtime, such as method registration, agent lifecycle management, and system state
 * initialization.
 */

/* Opaque system fixture type */
typedef struct ar_system_fixture_s ar_system_fixture_t;

/**
 * Creates a new test fixture for AgeRun system module tests
 * @param ref_test_name Name of the test for identification
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
ar_system_fixture_t* ar_system_fixture__create(const char *ref_test_name);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture
 */
void ar_system_fixture__destroy(ar_system_fixture_t *own_fixture);

/**
 * Initializes the test environment for system module tests
 * @param mut_fixture The fixture to use for initialization
 * @return true if initialization succeeded, false otherwise
 * @note This ensures system is in clean state and initializes required components
 */
bool ar_system_fixture__initialize(ar_system_fixture_t *mut_fixture);

/**
 * Registers a method with the methodology
 * @param mut_fixture The fixture managing the test
 * @param ref_method_name Name to register the method as
 * @param ref_instructions The method instructions
 * @param ref_version Version string for the method
 * @return A newly created method object
 * @note Ownership: Returns an owned method that is automatically transferred to methodology
 */
ar_method_t* ar_system_fixture__register_method(ar_system_fixture_t *mut_fixture,
                                                const char *ref_method_name,
                                                const char *ref_instructions,
                                                const char *ref_version);

/**
 * Gets the test name from a fixture
 * @param ref_fixture The fixture to query
 * @return The test name (borrowed reference)
 */
const char* ar_system_fixture__get_name(const ar_system_fixture_t *ref_fixture);

/**
 * Checks if there were any memory leaks during the test
 * @param ref_fixture The fixture to check
 * @return true if no memory leaks, false if leaks detected
 */
bool ar_system_fixture__check_memory(const ar_system_fixture_t *ref_fixture);

/**
 * Resets the system to a clean state
 * @param mut_fixture The fixture to use
 * @note This is useful for tests that need to reinitialize after persistence operations
 */
void ar_system_fixture__reset_system(ar_system_fixture_t *mut_fixture);

/**
 * Shuts down the system but preserves persistence files
 * @param mut_fixture The fixture to use
 * @note This is useful for persistence tests that need to verify data survives restarts
 */
void ar_system_fixture__shutdown_preserve_files(ar_system_fixture_t *mut_fixture);

/**
 * Process the next message in the system
 * @param mut_fixture The fixture managing the system
 * @return true if a message was processed, false if no messages
 */
bool ar_system_fixture__process_next_message(ar_system_fixture_t *mut_fixture);

/**
 * Process all pending messages in the system
 * @param mut_fixture The fixture managing the system
 * @return Number of messages processed
 */
int ar_system_fixture__process_all_messages(ar_system_fixture_t *mut_fixture);

/**
 * Get the agency from the fixture's system
 * @param ref_fixture The fixture to query
 * @return The system's agency (borrowed reference)
 */
ar_agency_t* ar_system_fixture__get_agency(ar_system_fixture_t *ref_fixture);

#endif /* AGERUN_SYSTEM_FIXTURE_H */
