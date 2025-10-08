#ifndef AGERUN_EXECUTABLE_FIXTURE_H
#define AGERUN_EXECUTABLE_FIXTURE_H

#include <stdio.h>

/**
 * @file ar_executable_fixture.h
 * @brief Executable fixture module for AgeRun executable testing infrastructure
 * 
 * This module provides a proper abstraction for executable test setup and teardown operations,
 * managing temporary build and methods directories for test isolation.
 * It follows Parnas principles by hiding implementation details and providing
 * a cohesive interface for executable test management.
 */

/* Opaque executable fixture type */
typedef struct ar_executable_fixture_s ar_executable_fixture_t;

/**
 * Creates a new executable test fixture
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 * @note This also initializes the temporary build directory
 */
ar_executable_fixture_t* ar_executable_fixture__create(void);

/**
 * Destroys an executable test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture
 * @note This also cleans up the temporary build directory
 */
void ar_executable_fixture__destroy(ar_executable_fixture_t *own_fixture);

/**
 * Creates a temporary methods directory for test isolation
 * @param mut_fixture The fixture managing the test
 * @return Path to the temporary methods directory
 * @note Ownership: Returns an owned string that caller must destroy with ar_executable_fixture__destroy_methods_dir
 */
char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture);

/**
 * Builds and runs the executable with specified methods directory
 * @param ref_fixture The fixture managing the test
 * @param ref_methods_dir Path to the methods directory to use
 * @return FILE pointer from popen() for reading output, or NULL on failure
 * @note Caller must close the FILE* with pclose() when done
 */
FILE* ar_executable_fixture__build_and_run(const ar_executable_fixture_t *ref_fixture, 
                                           const char *ref_methods_dir);

/**
 * Destroys a temporary methods directory and frees the path string
 * @param mut_fixture The fixture managing the test
 * @param own_methods_dir Path to the methods directory to destroy
 * @note Ownership: Takes ownership of the path string and frees it
 * @note Prints a warning if deletion fails but does not abort
 */
void ar_executable_fixture__destroy_methods_dir(ar_executable_fixture_t *mut_fixture,
                                                char *own_methods_dir);

/**
 * Gets the temporary build directory path used by the fixture
 * @param ref_fixture The fixture to query
 * @return The temporary build directory path (borrowed reference)
 * @note Ownership: Returns a borrowed reference to the internal path
 */
const char* ar_executable_fixture__get_build_dir(const ar_executable_fixture_t *ref_fixture);

/**
 * Removes persisted files from the build directory to ensure clean test state
 * @param ref_fixture The fixture managing the test
 * @note Removes agerun.methodology and agerun.agency files if they exist
 * @note Does not fail if files don't exist
 */
void ar_executable_fixture__clean_persisted_files(const ar_executable_fixture_t *ref_fixture);

#endif /* AGERUN_EXECUTABLE_FIXTURE_H */