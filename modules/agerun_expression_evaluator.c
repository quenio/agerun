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
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_literal_string: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a string literal
    if (ar__expression_ast__get_type(ref_node) != EXPR_AST_LITERAL_STRING) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the string value and create a data_t
    const char *value = ar__expression_ast__get_string_value(ref_node);
    return ar__data__create_string(value);
}

data_t* ar__expression_evaluator__evaluate_memory_access(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_memory_access: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a memory access
    if (ar__expression_ast__get_type(ref_node) != EXPR_AST_MEMORY_ACCESS) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the base accessor (should be "memory" or "context")
    const char *base = ar__expression_ast__get_memory_base(ref_node);
    if (!base) {
        ar__io__error("ar__expression_evaluator__evaluate_memory_access: No base accessor");
        return NULL;
    }
    
    // Get the path components
    size_t path_count = 0;
    char **path = ar__expression_ast__get_memory_path(ref_node, &path_count);
    
    // Determine which map to use based on the base
    data_t *map = NULL;
    if (strcmp(base, "memory") == 0) {
        map = mut_evaluator->ref_memory;
    } else if (strcmp(base, "context") == 0) {
        map = mut_evaluator->ref_context;
    } else {
        ar__io__error("ar__expression_evaluator__evaluate_memory_access: Invalid base accessor '%s'", base);
        if (path) AR__HEAP__FREE(path);
        return NULL;
    }
    
    // If context was requested but is NULL, return NULL
    if (!map) {
        if (path) AR__HEAP__FREE(path);
        return NULL;
    }
    
    // Navigate through the path
    data_t *current = map;
    for (size_t i = 0; i < path_count; i++) {
        if (ar__data__get_type(current) != DATA_MAP) {
            // Can't navigate further if not a map
            AR__HEAP__FREE(path);
            return NULL;
        }
        
        current = ar__data__get_map_data(current, path[i]);
        if (!current) {
            // Key not found
            AR__HEAP__FREE(path);
            return NULL;
        }
    }
    
    // Clean up the path array (but not the strings - they're borrowed)
    if (path) AR__HEAP__FREE(path);
    
    // Return the found value (it's a reference, not owned)
    return current;
}

data_t* ar__expression_evaluator__evaluate_binary_op(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    (void)mut_evaluator;
    (void)ref_node;
    return NULL;
}
