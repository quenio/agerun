/**
 * @file ar_instruction_evaluator.h
 * @brief Instruction evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate instruction AST nodes
 * and execute them within the AgeRun runtime. It handles all instruction
 * types including assignments, function calls, and control flow.
 * 
 * The evaluator works with instruction AST nodes (from instruction_ast module)
 * and uses the expression_evaluator for evaluating expressions within
 * instructions.
 */

#ifndef AGERUN_INSTRUCTION_EVALUATOR_H
#define AGERUN_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_instruction_ast.h"
#include "ar_data.h"
#include "ar_expression_evaluator.h"
#include "ar_log.h"
#include "ar_frame.h"

/* Forward declarations */
typedef struct ar_agency_s ar_agency_t;
typedef struct ar_delegation_s ar_delegation_t;

/**
 * Opaque type for instruction evaluator
 */
typedef struct ar_instruction_evaluator_s ar_instruction_evaluator_t;

/**
 * Creates a new instruction evaluator
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_agency The agency instance to use for agent/method operations (borrowed reference)
 * @param ref_delegation The delegation instance to use for delegate operations (borrowed reference)
 * @return A new evaluator instance
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function does not take ownership of the log, agency, or delegation parameters.
 *       The evaluator creates and owns its expression evaluator internally.
 */
ar_instruction_evaluator_t* ar_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
);

/**
 * Destroys an instruction evaluator
 * @param own_evaluator The evaluator to destroy
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar_instruction_evaluator__destroy(ar_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates any instruction AST node
 * @param ref_evaluator The evaluator instance (borrowed reference)
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note This is a facade method that dispatches to the appropriate specialized evaluator
 *       based on the instruction type. It handles all instruction types including
 *       assignments, function calls, and control flow.
 */
bool ar_instruction_evaluator__evaluate(
    const ar_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);

#endif /* AGERUN_INSTRUCTION_EVALUATOR_H */