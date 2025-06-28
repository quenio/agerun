/**
 * @file ar_method_evaluator.h
 * @brief Method evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate parsed method ASTs,
 * executing all instructions within a method in sequence. It acts as
 * the bridge between the method parser and the interpreter, enabling
 * "parse once, evaluate many times" semantics.
 * 
 * The evaluator works with pre-parsed method ASTs and uses the
 * instruction evaluator to execute each instruction in the method.
 */

#ifndef AR_METHOD_EVALUATOR_H
#define AR_METHOD_EVALUATOR_H

#include <stdbool.h>
#include "ar_frame.h"

/* Forward declaration to avoid circular dependency */
typedef struct method_s method_t;

/**
 * Opaque type for method evaluator
 */
typedef struct ar_method_evaluator_s ar_method_evaluator_t;

/**
 * Creates a new method evaluator for a specific method
 * @param ref_method The method containing parsed ASTs to evaluate (borrowed reference)
 * @return A new evaluator instance or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The evaluator keeps a reference to the method but does not own it.
 */
ar_method_evaluator_t* ar_method_evaluator__create(
    const method_t *ref_method
);

/**
 * Destroys a method evaluator
 * @param own_evaluator The evaluator to destroy (owned, can be NULL)
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar_method_evaluator__destroy(ar_method_evaluator_t *own_evaluator);

/**
 * Evaluates a method using the provided frame
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_frame The frame containing memory, context, and message (borrowed reference)
 * @return true if all instructions executed successfully, false on error
 * @note The evaluator executes all instructions in the method in sequence.
 *       Execution stops on the first error.
 */
bool ar_method_evaluator__evaluate(
    ar_method_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame
);

#endif /* AR_METHOD_EVALUATOR_H */