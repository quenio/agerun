#ifndef AGERUN_MODULE_TEST_FIXTURE_H
#define AGERUN_MODULE_TEST_FIXTURE_H

#include <stdbool.h>
#include "agerun_method.h"

/**
 * @file agerun_module_test_fixture.h
 * @brief Module test fixture for AgeRun module testing infrastructure
 * 
 * This module provides a proper abstraction for module test setup and teardown operations,
 * eliminating the need for helper functions scattered across test files.
 * It focuses on patterns common to module tests, such as method registration,
 * agent lifecycle management, and system state initialization.
 */

/* Opaque module test fixture type */
typedef struct module_test_fixture_s module_test_fixture_t;

/**
 * Creates a new test fixture for AgeRun module tests
 * @param ref_test_name Name of the test for identification
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
module_test_fixture_t* ar_module_test_fixture_create(const char *ref_test_name);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture
 */
void ar_module_test_fixture_destroy(module_test_fixture_t *own_fixture);

/**
 * Initializes the test environment for module tests
 * @param mut_fixture The fixture to use for initialization
 * @return true if initialization succeeded, false otherwise
 * @note This ensures system is in clean state and initializes required components
 */
bool ar_module_test_fixture_initialize(module_test_fixture_t *mut_fixture);

/**
 * Registers a method with the methodology
 * @param mut_fixture The fixture managing the test
 * @param ref_method_name Name to register the method as
 * @param ref_instructions The method instructions
 * @param ref_version Version string for the method
 * @return A newly created method object
 * @note Ownership: Returns an owned method that is automatically transferred to methodology
 */
method_t* ar_module_test_fixture_register_method(module_test_fixture_t *mut_fixture,
                                                const char *ref_method_name,
                                                const char *ref_instructions,
                                                const char *ref_version);

/**
 * Gets the test name from a fixture
 * @param ref_fixture The fixture to query
 * @return The test name (borrowed reference)
 */
const char* ar_module_test_fixture_get_name(const module_test_fixture_t *ref_fixture);

/**
 * Checks if there were any memory leaks during the test
 * @param ref_fixture The fixture to check
 * @return true if no memory leaks, false if leaks detected
 */
bool ar_module_test_fixture_check_memory(const module_test_fixture_t *ref_fixture);

/**
 * Resets the system to a clean state
 * @param mut_fixture The fixture to use
 * @note This is useful for tests that need to reinitialize after persistence operations
 */
void ar_module_test_fixture_reset_system(module_test_fixture_t *mut_fixture);

#endif /* AGERUN_MODULE_TEST_FIXTURE_H */
