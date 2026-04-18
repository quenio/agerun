/**
 * @file ar_complete_instruction_evaluator.h
 * @brief Evaluator for complete() function instructions in the AgeRun language
 */

#ifndef AGERUN_COMPLETE_INSTRUCTION_EVALUATOR_H
#define AGERUN_COMPLETE_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_frame.h"

typedef struct ar_expression_evaluator_s ar_expression_evaluator_t;
typedef struct ar_instruction_ast_s ar_instruction_ast_t;
typedef struct ar_log_s ar_log_t;
typedef struct ar_local_completion_s ar_local_completion_t;

typedef struct ar_complete_instruction_evaluator_s ar_complete_instruction_evaluator_t;

ar_complete_instruction_evaluator_t* ar_complete_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_local_completion_t *ref_local_completion
);
void ar_complete_instruction_evaluator__destroy(ar_complete_instruction_evaluator_t *own_evaluator);
bool ar_complete_instruction_evaluator__evaluate(
    const ar_complete_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);

#endif
