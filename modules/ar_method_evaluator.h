/**
 * @file ar_method_evaluator.h
 * @brief Method evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate method AST nodes
 * using a frame-based execution model. It serves as the top-level
 * evaluator that coordinates the execution of all instructions
 * within a method.
 */

#ifndef AGERUN_METHOD_EVALUATOR_H
#define AGERUN_METHOD_EVALUATOR_H

#include <stdbool.h>
#include "ar_log.h"
#include "ar_instruction_evaluator.h"
#include "ar_method_ast.h"
#include "ar_frame.h"

/* Forward declarations */
typedef struct ar_agency_s ar_agency_t;
typedef struct ar_delegation_s ar_delegation_t;

/**
 * Opaque type for the method evaluator
 */
typedef struct ar_method_evaluator_s ar_method_evaluator_t;

/**
 * Creates a new method evaluator
 * @param ref_log The log instance for error reporting (borrowed reference)
 * @param ref_agency The agency instance to use for agent/method operations (borrowed reference)
 * @param ref_delegation The delegation instance to use for delegate operations (borrowed reference)
 * @return A new method evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The evaluator creates and owns its instruction evaluator internally.
 */
ar_method_evaluator_t* ar_method_evaluator__create(
    ar_log_t *ref_log,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
);

/**
 * Destroys a method evaluator
 * @param own_evaluator The evaluator to destroy (owned)
 * @note Takes ownership of the evaluator and frees all resources
 */
void ar_method_evaluator__destroy(
    ar_method_evaluator_t *own_evaluator
);

/**
 * Evaluates a method AST using the provided frame
 * @param ref_evaluator The method evaluator to use (borrowed reference)
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The method AST to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The method will execute all instructions in sequence until completion or error
 */
bool ar_method_evaluator__evaluate(
    const ar_method_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_method_ast_t *ref_ast
);

#endif /* AGERUN_METHOD_EVALUATOR_H */