/**
 * @file agerun_expression_evaluator.c
 * @brief Implementation of expression evaluator module
 */

#include "agerun_expression_evaluator.h"
#include "agerun_heap.h"
#include "agerun_io.h"
#include <string.h>

/**
 * Internal structure for expression evaluator
 */
struct expression_evaluator_s {
    data_t *ref_memory;    /**< Memory map with variables (borrowed) */
    data_t *ref_context;   /**< Optional context map (borrowed, may be NULL) */
};

expression_evaluator_t* ar__expression_evaluator__create(
    data_t *ref_memory,
    data_t *ref_context)
{
    if (!ref_memory) {
        ar__io__error("ar__expression_evaluator__create: NULL memory");
        return NULL;
    }

    expression_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(expression_evaluator_t), "expression_evaluator");
    if (!evaluator) {
        ar__io__error("ar__expression_evaluator__create: Failed to allocate evaluator");
        return NULL;
    }

    evaluator->ref_memory = ref_memory;
    evaluator->ref_context = ref_context;

    return evaluator;
}

void ar__expression_evaluator__destroy(expression_evaluator_t *own_evaluator)
{
    if (own_evaluator) {
        AR__HEAP__FREE(own_evaluator);
    }
}

data_t* ar__expression_evaluator__evaluate_literal_int(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_literal_int: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is an integer literal
    if (ar__expression_ast__get_type(ref_node) != EXPR_AST_LITERAL_INT) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the integer value and create a data_t
    int value = ar__expression_ast__get_int_value(ref_node);
    return ar__data__create_integer(value);
}

data_t* ar__expression_evaluator__evaluate_literal_double(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_literal_double: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a double literal
    if (ar__expression_ast__get_type(ref_node) != EXPR_AST_LITERAL_DOUBLE) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the double value and create a data_t
    double value = ar__expression_ast__get_double_value(ref_node);
    return ar__data__create_double(value);
}

data_t* ar__expression_evaluator__evaluate_literal_string(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    (void)mut_evaluator;
    (void)ref_node;
    return NULL;
}

data_t* ar__expression_evaluator__evaluate_memory_access(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    (void)mut_evaluator;
    (void)ref_node;
    return NULL;
}

data_t* ar__expression_evaluator__evaluate_binary_op(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    (void)mut_evaluator;
    (void)ref_node;
    return NULL;
}
