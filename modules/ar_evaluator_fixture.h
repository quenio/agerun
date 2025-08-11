#ifndef AGERUN_INSTRUCTION_EVALUATOR_FIXTURE_H
#define AGERUN_INSTRUCTION_EVALUATOR_FIXTURE_H

#include <stdbool.h>
#include "ar_data.h"
#include "ar_log.h"
#include "ar_expression_evaluator.h"
#include "ar_agency.h"
#include "ar_methodology.h"
/* Individual evaluator headers should be included by test files */
#include "ar_frame.h"
#include "ar_instruction_ast.h"

/**
 * @file ar_evaluator_fixture.h
 * @brief Generic test fixture for instruction evaluator tests
 * 
 * This module provides test infrastructure for instruction evaluator test suites.
 * It eliminates repetitive patterns in evaluator creation, frame setup, AST creation,
 * and cleanup. This fixture is designed for the new frame-based evaluator pattern
 * where evaluators don't store memory internally.
 */

/* Opaque fixture type */
typedef struct ar_evaluator_fixture_s ar_evaluator_fixture_t;

/**
 * Creates a new test fixture for assignment instruction evaluator tests
 * @param ref_test_name Name of the test for identification
 * @return A newly created test fixture
 * @note Ownership: Returns an owned fixture that caller must destroy
 */
ar_evaluator_fixture_t* ar_evaluator_fixture__create(
    const char *ref_test_name
);

/**
 * Destroys a test fixture and performs cleanup
 * @param own_fixture The fixture to destroy
 * @note Ownership: Takes ownership and destroys the fixture and all tracked resources
 */
void ar_evaluator_fixture__destroy(
    ar_evaluator_fixture_t *own_fixture
);

/* Note: The fixture does not manage evaluators - tests create and manage them independently */

/**
 * Gets the expression evaluator from the fixture
 * @param ref_fixture The fixture to query
 * @return The expression evaluator (borrowed reference)
 * @note Ownership: Returns a borrowed reference; do not destroy
 */
ar_expression_evaluator_t* ar_evaluator_fixture__get_expression_evaluator(
    const ar_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the memory map from the fixture
 * @param ref_fixture The fixture to query
 * @return The memory map (borrowed reference)
 * @note Ownership: Returns a borrowed reference; do not destroy
 */
ar_data_t* ar_evaluator_fixture__get_memory(
    const ar_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the log instance from the fixture
 * @param ref_fixture The fixture to query
 * @return The log (borrowed reference)
 * @note Ownership: Returns a borrowed reference; do not destroy
 */
ar_log_t* ar_evaluator_fixture__get_log(
    const ar_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the agency instance from the fixture
 * @param ref_fixture The fixture to query
 * @return The agency (borrowed reference)
 * @note Ownership: Returns a borrowed reference; do not destroy
 */
ar_agency_t* ar_evaluator_fixture__get_agency(
    const ar_evaluator_fixture_t *ref_fixture
);

/**
 * Gets the methodology instance from the fixture
 * @param ref_fixture The fixture to query
 * @return The methodology (borrowed reference)
 * @note Ownership: Returns a borrowed reference; do not destroy
 */
ar_methodology_t* ar_evaluator_fixture__get_methodology(
    const ar_evaluator_fixture_t *ref_fixture
);

/**
 * Creates a frame for evaluation
 * @param mut_fixture The fixture managing the test
 * @return A newly created frame with memory from fixture
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 * @note The frame is created with empty context and message maps
 */
ar_frame_t* ar_evaluator_fixture__create_frame(
    ar_evaluator_fixture_t *mut_fixture
);

/**
 * Creates an assignment AST with integer value
 * @param mut_fixture The fixture managing the test
 * @param ref_path The memory path for assignment (e.g., "memory.count")
 * @param value The integer value to assign
 * @return A newly created assignment AST with expression
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 */
ar_instruction_ast_t* ar_evaluator_fixture__create_assignment_int(
    ar_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    int value
);

/**
 * Creates an assignment AST with string value
 * @param mut_fixture The fixture managing the test
 * @param ref_path The memory path for assignment (e.g., "memory.name")
 * @param ref_value The string value to assign
 * @return A newly created assignment AST with expression
 * @note Ownership: Returns a borrowed reference; fixture owns and will destroy it
 */
ar_instruction_ast_t* ar_evaluator_fixture__create_assignment_string(
    ar_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    const char *ref_value
);

/**
 * Creates an assignment AST with expression
 * @param mut_fixture The fixture managing the test
 * @param ref_path The memory path for assignment
 * @param own_expr_ast The expression AST to assign
 * @return A newly created assignment AST
 * @note Ownership: Takes ownership of expr_ast. Returns borrowed reference.
 */
ar_instruction_ast_t* ar_evaluator_fixture__create_assignment_expr(
    ar_evaluator_fixture_t *mut_fixture,
    const char *ref_path,
    ar_expression_ast_t *own_expr_ast
);

/**
 * Tracks an instruction AST for automatic cleanup
 * @param mut_fixture The fixture managing the test
 * @param own_ast The AST to track
 * @note Ownership: Takes ownership of the AST
 */
void ar_evaluator_fixture__track_ast(
    ar_evaluator_fixture_t *mut_fixture,
    ar_instruction_ast_t *own_ast
);

/**
 * Checks if there were any memory leaks during the test
 * @param ref_fixture The fixture to check
 * @return true if no memory leaks, false if leaks detected
 */
bool ar_evaluator_fixture__check_memory(
    const ar_evaluator_fixture_t *ref_fixture
);

/**
 * Processes the next message in the system queue
 * @param mut_fixture The fixture managing the test
 * @return true if a message was processed, false if queue is empty
 */
bool ar_evaluator_fixture__process_next_message(
    ar_evaluator_fixture_t *mut_fixture
);

#endif /* AGERUN_INSTRUCTION_EVALUATOR_FIXTURE_H */