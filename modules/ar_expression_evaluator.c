/**
 * @file agerun_expression_evaluator.c
 * @brief Implementation of expression evaluator module
 */

#include "ar_expression_evaluator.h"
#include "ar_heap.h"
#include "ar_io.h"
#include "ar_frame.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Internal structure for expression evaluator
 */
struct expression_evaluator_s {
    data_t *ref_memory;    /**< Memory map with variables (borrowed) */
    data_t *ref_context;   /**< Optional context map (borrowed, may be NULL) */
    bool is_stateless;     /**< True if evaluator is stateless (uses frames) */
};

ar_expression_evaluator_t* ar__expression_evaluator__create(
    data_t *ref_memory,
    data_t *ref_context)
{
    if (!ref_memory) {
        ar__io__error("ar__expression_evaluator__create: NULL memory");
        return NULL;
    }

    ar_expression_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(ar_expression_evaluator_t), "expression_evaluator");
    if (!evaluator) {
        ar__io__error("ar__expression_evaluator__create: Failed to allocate evaluator");
        return NULL;
    }

    evaluator->ref_memory = ref_memory;
    evaluator->ref_context = ref_context;
    evaluator->is_stateless = false;

    return evaluator;
}

ar_expression_evaluator_t* ar__expression_evaluator__create_stateless(void)
{
    ar_expression_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(ar_expression_evaluator_t), "expression_evaluator");
    if (!evaluator) {
        ar__io__error("ar__expression_evaluator__create_stateless: Failed to allocate evaluator");
        return NULL;
    }

    evaluator->ref_memory = NULL;
    evaluator->ref_context = NULL;
    evaluator->is_stateless = true;

    return evaluator;
}

void ar__expression_evaluator__destroy(ar_expression_evaluator_t *own_evaluator)
{
    if (own_evaluator) {
        AR__HEAP__FREE(own_evaluator);
    }
}

data_t* ar__expression_evaluator__evaluate_literal_int(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_literal_int: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is an integer literal
    if (ar__expression_ast__get_type(ref_node) != AR_EXPR__LITERAL_INT) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the integer value and create a data_t
    int value = ar__expression_ast__get_int_value(ref_node);
    return ar__data__create_integer(value);
}

data_t* ar__expression_evaluator__evaluate_literal_double(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_literal_double: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a double literal
    if (ar__expression_ast__get_type(ref_node) != AR_EXPR__LITERAL_DOUBLE) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the double value and create a data_t
    double value = ar__expression_ast__get_double_value(ref_node);
    return ar__data__create_double(value);
}

data_t* ar__expression_evaluator__evaluate_literal_string(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_literal_string: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a string literal
    if (ar__expression_ast__get_type(ref_node) != AR_EXPR__LITERAL_STRING) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the string value and create a data_t
    const char *value = ar__expression_ast__get_string_value(ref_node);
    return ar__data__create_string(value);
}

data_t* ar__expression_evaluator__evaluate_memory_access(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_memory_access: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a memory access
    if (ar__expression_ast__get_type(ref_node) != AR_EXPR__MEMORY_ACCESS) {
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
    const data_t *current = map;
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
    // Cast away const - safe because caller shouldn't modify or destroy the reference
    return (data_t *)(uintptr_t)current;
}

/**
 * Forward declaration of frame-based evaluation helper
 */
static data_t* _evaluate_expression_with_frame(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node,
    const ar_frame_t *ref_frame);

/**
 * Forward declaration of common binary operation logic
 */
static data_t* _apply_binary_operation(
    ar_binary_operator_t op,
    data_t *own_left,
    data_t *own_right);

/**
 * Helper function to evaluate any expression AST node
 */
static data_t* _evaluate_expression(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!ref_node) return NULL;
    
    ar_expression_ast_type_t type = ar__expression_ast__get_type(ref_node);
    
    switch (type) {
        case AR_EXPR__LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_evaluator, ref_node);
        case AR_EXPR__LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_evaluator, ref_node);
        case AR_EXPR__LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_evaluator, ref_node);
        case AR_EXPR__MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy for binary ops
            {
                data_t *ref_value = ar__expression_evaluator__evaluate_memory_access(mut_evaluator, ref_node);
                if (!ref_value) return NULL;
                
                // Create a copy based on type
                switch (ar__data__get_type(ref_value)) {
                    case DATA_INTEGER:
                        return ar__data__create_integer(ar__data__get_integer(ref_value));
                    case DATA_DOUBLE:
                        return ar__data__create_double(ar__data__get_double(ref_value));
                    case DATA_STRING:
                        return ar__data__create_string(ar__data__get_string(ref_value));
                    default:
                        ar__io__error("_evaluate_expression: Unsupported data type for copy");
                        return NULL;
                }
            }
        case AR_EXPR__BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_evaluator, ref_node);
        default:
            ar__io__error("_evaluate_expression: Unknown expression type");
            return NULL;
    }
}

data_t* ar__expression_evaluator__evaluate_binary_op(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_binary_op: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a binary operation
    if (ar__expression_ast__get_type(ref_node) != AR_EXPR__BINARY_OP) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the operator and operands
    ar_binary_operator_t op = ar__expression_ast__get_operator(ref_node);
    const ar_expression_ast_t *left_node = ar__expression_ast__get_left(ref_node);
    const ar_expression_ast_t *right_node = ar__expression_ast__get_right(ref_node);
    
    if (!left_node || !right_node) {
        ar__io__error("ar__expression_evaluator__evaluate_binary_op: Missing operands");
        return NULL;
    }
    
    // Recursively evaluate both operands
    data_t *left = _evaluate_expression(mut_evaluator, left_node);
    if (!left) {
        ar__io__error("ar__expression_evaluator__evaluate_binary_op: Failed to evaluate left operand");
        return NULL;
    }
    
    data_t *right = _evaluate_expression(mut_evaluator, right_node);
    if (!right) {
        ar__data__destroy(left);
        ar__io__error("ar__expression_evaluator__evaluate_binary_op: Failed to evaluate right operand");
        return NULL;
    }
    
    // Use common binary operation logic
    data_t *result = _apply_binary_operation(op, left, right);
    
    // Clean up operand values
    ar__data__destroy(left);
    ar__data__destroy(right);
    
    return result;
}

data_t* ar__expression_evaluator__evaluate(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_ast)
{
    if (!mut_evaluator) {
        ar__io__error("ar__expression_evaluator__evaluate: NULL evaluator");
        return NULL;
    }
    
    return _evaluate_expression(mut_evaluator, ref_ast);
}

data_t* ar__expression_evaluator__evaluate_with_frame(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_ast,
    const ar_frame_t *ref_frame)
{
    if (!mut_evaluator) {
        ar__io__error("ar__expression_evaluator__evaluate_with_frame: NULL evaluator");
        return NULL;
    }
    
    if (!ref_frame) {
        ar__io__error("ar__expression_evaluator__evaluate_with_frame: NULL frame");
        return NULL;
    }
    
    return _evaluate_expression_with_frame(mut_evaluator, ref_ast, ref_frame);
}

/**
 * Frame-based memory access evaluation
 */
static data_t* _evaluate_memory_access_with_frame(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node,
    const ar_frame_t *ref_frame)
{
    if (!mut_evaluator || !ref_node || !ref_frame) {
        ar__io__error("_evaluate_memory_access_with_frame: NULL parameters");
        return NULL;
    }
    
    // Check if the node is a memory access
    if (ar__expression_ast__get_type(ref_node) != AR_EXPR__MEMORY_ACCESS) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the base accessor (should be "memory" or "context")
    const char *base = ar__expression_ast__get_memory_base(ref_node);
    if (!base) {
        ar__io__error("_evaluate_memory_access_with_frame: No base accessor");
        return NULL;
    }
    
    // Get the path components
    size_t path_count = 0;
    char **path = ar__expression_ast__get_memory_path(ref_node, &path_count);
    
    // Determine which map to use based on the base
    const data_t *map = NULL;
    if (strcmp(base, "memory") == 0) {
        map = ar_frame__get_memory(ref_frame);
    } else if (strcmp(base, "context") == 0) {
        map = ar_frame__get_context(ref_frame);
    } else {
        ar__io__error("_evaluate_memory_access_with_frame: Invalid base accessor '%s'", base);
        if (path) AR__HEAP__FREE(path);
        return NULL;
    }
    
    // If context was requested but is NULL, return NULL
    if (!map) {
        if (path) AR__HEAP__FREE(path);
        return NULL;
    }
    
    // Navigate through the path
    const data_t *current = map;
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
    // Cast away const - safe because caller shouldn't modify or destroy the reference
    return (data_t *)(uintptr_t)current;
}

/**
 * Frame-based binary operation evaluation
 */
static data_t* _evaluate_binary_op_with_frame(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node,
    const ar_frame_t *ref_frame)
{
    if (!mut_evaluator || !ref_node || !ref_frame) {
        ar__io__error("_evaluate_binary_op_with_frame: NULL parameters");
        return NULL;
    }
    
    // Check if the node is a binary operation
    if (ar__expression_ast__get_type(ref_node) != AR_EXPR__BINARY_OP) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the operator and operands
    ar_binary_operator_t op = ar__expression_ast__get_operator(ref_node);
    const ar_expression_ast_t *left_node = ar__expression_ast__get_left(ref_node);
    const ar_expression_ast_t *right_node = ar__expression_ast__get_right(ref_node);
    
    if (!left_node || !right_node) {
        ar__io__error("_evaluate_binary_op_with_frame: Missing operands");
        return NULL;
    }
    
    // Recursively evaluate both operands with frame
    data_t *left = _evaluate_expression_with_frame(mut_evaluator, left_node, ref_frame);
    if (!left) {
        ar__io__error("_evaluate_binary_op_with_frame: Failed to evaluate left operand");
        return NULL;
    }
    
    data_t *right = _evaluate_expression_with_frame(mut_evaluator, right_node, ref_frame);
    if (!right) {
        ar__data__destroy(left);
        ar__io__error("_evaluate_binary_op_with_frame: Failed to evaluate right operand");
        return NULL;
    }
    
    // Use common binary operation logic
    data_t *result = _apply_binary_operation(op, left, right);
    
    // Clean up operand values
    ar__data__destroy(left);
    ar__data__destroy(right);
    
    return result;
}

/**
 * Helper function to evaluate any expression AST node with frame
 */
static data_t* _evaluate_expression_with_frame(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node,
    const ar_frame_t *ref_frame)
{
    if (!ref_node) return NULL;
    
    ar_expression_ast_type_t type = ar__expression_ast__get_type(ref_node);
    
    switch (type) {
        case AR_EXPR__LITERAL_INT:
            // Literals don't need frame
            return ar__expression_evaluator__evaluate_literal_int(mut_evaluator, ref_node);
        case AR_EXPR__LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_evaluator, ref_node);
        case AR_EXPR__LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_evaluator, ref_node);
        case AR_EXPR__MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy for operations
            {
                data_t *ref_value = _evaluate_memory_access_with_frame(mut_evaluator, ref_node, ref_frame);
                if (!ref_value) return NULL;
                
                // Create a copy based on type
                switch (ar__data__get_type(ref_value)) {
                    case DATA_INTEGER:
                        return ar__data__create_integer(ar__data__get_integer(ref_value));
                    case DATA_DOUBLE:
                        return ar__data__create_double(ar__data__get_double(ref_value));
                    case DATA_STRING:
                        return ar__data__create_string(ar__data__get_string(ref_value));
                    default:
                        ar__io__error("_evaluate_expression_with_frame: Unsupported data type for copy");
                        return NULL;
                }
            }
        case AR_EXPR__BINARY_OP:
            return _evaluate_binary_op_with_frame(mut_evaluator, ref_node, ref_frame);
        default:
            ar__io__error("_evaluate_expression_with_frame: Unknown expression type");
            return NULL;
    }
}

/**
 * Common binary operation logic shared between regular and frame-based evaluation
 */
static data_t* _apply_binary_operation(
    ar_binary_operator_t op,
    data_t *own_left,
    data_t *own_right)
{
    // Get the types of both operands
    data_type_t left_type = ar__data__get_type(own_left);
    data_type_t right_type = ar__data__get_type(own_right);
    
    data_t *result = NULL;
    
    // Handle operations based on types
    if (left_type == DATA_INTEGER && right_type == DATA_INTEGER) {
        // Integer operations
        int left_val = ar__data__get_integer(own_left);
        int right_val = ar__data__get_integer(own_right);
        
        switch (op) {
            case AR_OP__ADD:
                result = ar__data__create_integer(left_val + right_val);
                break;
            case AR_OP__SUBTRACT:
                result = ar__data__create_integer(left_val - right_val);
                break;
            case AR_OP__MULTIPLY:
                result = ar__data__create_integer(left_val * right_val);
                break;
            case AR_OP__DIVIDE:
                if (right_val == 0) {
                    ar__io__error("_apply_binary_operation: Division by zero");
                } else {
                    result = ar__data__create_integer(left_val / right_val);
                }
                break;
            case AR_OP__EQUAL:
                result = ar__data__create_integer(left_val == right_val ? 1 : 0);
                break;
            case AR_OP__NOT_EQUAL:
                result = ar__data__create_integer(left_val != right_val ? 1 : 0);
                break;
            case AR_OP__LESS:
                result = ar__data__create_integer(left_val < right_val ? 1 : 0);
                break;
            case AR_OP__GREATER:
                result = ar__data__create_integer(left_val > right_val ? 1 : 0);
                break;
            case AR_OP__LESS_EQ:
                result = ar__data__create_integer(left_val <= right_val ? 1 : 0);
                break;
            case AR_OP__GREATER_EQ:
                result = ar__data__create_integer(left_val >= right_val ? 1 : 0);
                break;
            default:
                ar__io__error("_apply_binary_operation: Unknown operator for integers");
                break;
        }
    } else if (left_type == DATA_DOUBLE || right_type == DATA_DOUBLE) {
        // Convert to doubles if either operand is a double
        double left_val = (left_type == DATA_DOUBLE) ? 
            ar__data__get_double(own_left) : (double)ar__data__get_integer(own_left);
        double right_val = (right_type == DATA_DOUBLE) ? 
            ar__data__get_double(own_right) : (double)ar__data__get_integer(own_right);
        
        switch (op) {
            case AR_OP__ADD:
                result = ar__data__create_double(left_val + right_val);
                break;
            case AR_OP__SUBTRACT:
                result = ar__data__create_double(left_val - right_val);
                break;
            case AR_OP__MULTIPLY:
                result = ar__data__create_double(left_val * right_val);
                break;
            case AR_OP__DIVIDE:
                if (right_val == 0.0) {
                    ar__io__error("_apply_binary_operation: Division by zero");
                } else {
                    result = ar__data__create_double(left_val / right_val);
                }
                break;
            case AR_OP__EQUAL:
                result = ar__data__create_integer(left_val == right_val ? 1 : 0);
                break;
            case AR_OP__NOT_EQUAL:
                result = ar__data__create_integer(left_val != right_val ? 1 : 0);
                break;
            case AR_OP__LESS:
                result = ar__data__create_integer(left_val < right_val ? 1 : 0);
                break;
            case AR_OP__GREATER:
                result = ar__data__create_integer(left_val > right_val ? 1 : 0);
                break;
            case AR_OP__LESS_EQ:
                result = ar__data__create_integer(left_val <= right_val ? 1 : 0);
                break;
            case AR_OP__GREATER_EQ:
                result = ar__data__create_integer(left_val >= right_val ? 1 : 0);
                break;
            default:
                ar__io__error("_apply_binary_operation: Unknown operator for doubles");
                break;
        }
    } else if (left_type == DATA_STRING && right_type == DATA_STRING) {
        // String operations
        const char *left_str = ar__data__get_string(own_left);
        const char *right_str = ar__data__get_string(own_right);
        
        switch (op) {
            case AR_OP__ADD:
                // String concatenation
                {
                    size_t len = strlen(left_str) + strlen(right_str) + 1;
                    char *concat = AR__HEAP__MALLOC(len, "string concatenation");
                    if (concat) {
                        strcpy(concat, left_str);
                        strcat(concat, right_str);
                        result = ar__data__create_string(concat);
                        AR__HEAP__FREE(concat);
                    }
                }
                break;
            case AR_OP__EQUAL:
                result = ar__data__create_integer(strcmp(left_str, right_str) == 0 ? 1 : 0);
                break;
            case AR_OP__NOT_EQUAL:
                result = ar__data__create_integer(strcmp(left_str, right_str) != 0 ? 1 : 0);
                break;
            default:
                ar__io__error("_apply_binary_operation: Unsupported operator for strings");
                break;
        }
    } else {
        ar__io__error("_apply_binary_operation: Type mismatch in binary operation");
    }
    
    return result;
}
