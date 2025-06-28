/**
 * @file ar_method_evaluator.c
 * @brief Implementation of the method evaluator module
 */

#include "ar_method_evaluator.h"
#include "ar_method.h"
#include "ar_method_ast.h"
#include "ar_instruction_ast.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_evaluator.h"
#include "ar_frame.h"
#include "ar_heap.h"
#include "ar_io.h"

/**
 * Internal structure for method evaluator
 */
struct ar_method_evaluator_s {
    const method_t *ref_method;                          /* Method to evaluate (borrowed reference) */
    ar_expression_evaluator_t *own_expr_evaluator;      /* Expression evaluator (owned) */
    instruction_evaluator_t *own_inst_evaluator;        /* Instruction evaluator (owned) */
};

/**
 * Creates a new method evaluator for a specific method
 */
ar_method_evaluator_t* ar_method_evaluator__create(
    const method_t *ref_method
) {
    if (!ref_method) {
        ar__io__error("ar_method_evaluator__create: method cannot be NULL");
        return NULL;
    }
    
    // Verify the method has a parsed AST
    const ar_method_ast_t *ast = ar__method__get_ast(ref_method);
    if (!ast) {
        ar__io__error("ar_method_evaluator__create: method has no parsed AST");
        return NULL;
    }
    
    // Allocate evaluator structure
    ar_method_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(ar_method_evaluator_t), "method_evaluator");
    if (!evaluator) {
        ar__io__error("ar_method_evaluator__create: Failed to allocate evaluator");
        return NULL;
    }
    
    // Store reference to method
    evaluator->ref_method = ref_method;
    
    // Create stateless expression evaluator
    evaluator->own_expr_evaluator = ar__expression_evaluator__create_stateless();
    if (!evaluator->own_expr_evaluator) {
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create stateless instruction evaluator
    evaluator->own_inst_evaluator = ar_instruction_evaluator__create_stateless(
        evaluator->own_expr_evaluator
    );
    if (!evaluator->own_inst_evaluator) {
        ar__expression_evaluator__destroy(evaluator->own_expr_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    return evaluator;
}

/**
 * Destroys a method evaluator
 */
void ar_method_evaluator__destroy(ar_method_evaluator_t *own_evaluator) {
    if (!own_evaluator) {
        return;
    }
    
    // Destroy owned evaluators
    if (own_evaluator->own_inst_evaluator) {
        ar_instruction_evaluator__destroy(own_evaluator->own_inst_evaluator);
    }
    if (own_evaluator->own_expr_evaluator) {
        ar__expression_evaluator__destroy(own_evaluator->own_expr_evaluator);
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Evaluates a method using the provided frame
 */
bool ar_method_evaluator__evaluate(
    ar_method_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame
) {
    if (!mut_evaluator || !ref_frame) {
        ar__io__error("ar_method_evaluator__evaluate: evaluator and frame cannot be NULL");
        return false;
    }
    
    // Get the method's AST
    const ar_method_ast_t *method_ast = ar__method__get_ast(mut_evaluator->ref_method);
    if (!method_ast) {
        ar__io__error("ar_method_evaluator__evaluate: method has no parsed AST");
        return false;
    }
    
    // Get the instruction count
    size_t count = ar_method_ast__get_instruction_count(method_ast);
    if (count == 0) {
        // No instructions to execute
        return true;
    }
    
    // Execute each instruction in sequence
    for (size_t i = 0; i < count; i++) {
        // Get instruction by line number (1-based)
        const ar_instruction_ast_t *inst_ast = ar_method_ast__get_instruction(method_ast, i + 1);
        if (!inst_ast) {
            ar__io__error("ar_method_evaluator__evaluate: Failed to get instruction %zu", i + 1);
            return false;
        }
        
        // Evaluate the instruction with the frame
        bool result = ar_instruction_evaluator__evaluate_with_frame(
            mut_evaluator->own_inst_evaluator,
            inst_ast,
            ref_frame
        );
        
        if (!result) {
            ar__io__error("ar_method_evaluator__evaluate: Failed to evaluate instruction %zu", i + 1);
            return false;
        }
    }
    
    return true;
}