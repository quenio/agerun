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

/**
 * Helper function to evaluate any expression AST node
 */
static data_t* _evaluate_expression(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    if (!ref_node) return NULL;
    
    expression_ast_type_t type = ar__expression_ast__get_type(ref_node);
    
    switch (type) {
        case EXPR_AST_LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_evaluator, ref_node);
        case EXPR_AST_LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_evaluator, ref_node);
        case EXPR_AST_LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_evaluator, ref_node);
        case EXPR_AST_MEMORY_ACCESS:
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
        case EXPR_AST_BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_evaluator, ref_node);
        default:
            ar__io__error("_evaluate_expression: Unknown expression type");
            return NULL;
    }
}

data_t* ar__expression_evaluator__evaluate_binary_op(
    expression_evaluator_t *mut_evaluator,
    const expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        ar__io__error("ar__expression_evaluator__evaluate_binary_op: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a binary operation
    if (ar__expression_ast__get_type(ref_node) != EXPR_AST_BINARY_OP) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the operator and operands
    binary_operator_t op = ar__expression_ast__get_operator(ref_node);
    const expression_ast_t *left_node = ar__expression_ast__get_left(ref_node);
    const expression_ast_t *right_node = ar__expression_ast__get_right(ref_node);
    
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
    
    // Get the types of both operands
    data_type_t left_type = ar__data__get_type(left);
    data_type_t right_type = ar__data__get_type(right);
    
    data_t *result = NULL;
    
    // Handle operations based on types
    if (left_type == DATA_INTEGER && right_type == DATA_INTEGER) {
        // Integer operations
        int left_val = ar__data__get_integer(left);
        int right_val = ar__data__get_integer(right);
        
        switch (op) {
            case OP_ADD:
                result = ar__data__create_integer(left_val + right_val);
                break;
            case OP_SUBTRACT:
                result = ar__data__create_integer(left_val - right_val);
                break;
            case OP_MULTIPLY:
                result = ar__data__create_integer(left_val * right_val);
                break;
            case OP_DIVIDE:
                if (right_val == 0) {
                    ar__io__error("ar__expression_evaluator__evaluate_binary_op: Division by zero");
                } else {
                    result = ar__data__create_integer(left_val / right_val);
                }
                break;
            case OP_EQUAL:
                result = ar__data__create_integer(left_val == right_val ? 1 : 0);
                break;
            case OP_NOT_EQUAL:
                result = ar__data__create_integer(left_val != right_val ? 1 : 0);
                break;
            case OP_LESS:
                result = ar__data__create_integer(left_val < right_val ? 1 : 0);
                break;
            case OP_GREATER:
                result = ar__data__create_integer(left_val > right_val ? 1 : 0);
                break;
            case OP_LESS_EQ:
                result = ar__data__create_integer(left_val <= right_val ? 1 : 0);
                break;
            case OP_GREATER_EQ:
                result = ar__data__create_integer(left_val >= right_val ? 1 : 0);
                break;
            default:
                ar__io__error("ar__expression_evaluator__evaluate_binary_op: Unknown operator for integers");
                break;
        }
    } else if (left_type == DATA_DOUBLE || right_type == DATA_DOUBLE) {
        // Convert to doubles if either operand is a double
        double left_val = (left_type == DATA_DOUBLE) ? 
            ar__data__get_double(left) : (double)ar__data__get_integer(left);
        double right_val = (right_type == DATA_DOUBLE) ? 
            ar__data__get_double(right) : (double)ar__data__get_integer(right);
        
        switch (op) {
            case OP_ADD:
                result = ar__data__create_double(left_val + right_val);
                break;
            case OP_SUBTRACT:
                result = ar__data__create_double(left_val - right_val);
                break;
            case OP_MULTIPLY:
                result = ar__data__create_double(left_val * right_val);
                break;
            case OP_DIVIDE:
                if (right_val == 0.0) {
                    ar__io__error("ar__expression_evaluator__evaluate_binary_op: Division by zero");
                } else {
                    result = ar__data__create_double(left_val / right_val);
                }
                break;
            case OP_EQUAL:
                result = ar__data__create_integer(left_val == right_val ? 1 : 0);
                break;
            case OP_NOT_EQUAL:
                result = ar__data__create_integer(left_val != right_val ? 1 : 0);
                break;
            case OP_LESS:
                result = ar__data__create_integer(left_val < right_val ? 1 : 0);
                break;
            case OP_GREATER:
                result = ar__data__create_integer(left_val > right_val ? 1 : 0);
                break;
            case OP_LESS_EQ:
                result = ar__data__create_integer(left_val <= right_val ? 1 : 0);
                break;
            case OP_GREATER_EQ:
                result = ar__data__create_integer(left_val >= right_val ? 1 : 0);
                break;
            default:
                ar__io__error("ar__expression_evaluator__evaluate_binary_op: Unknown operator for doubles");
                break;
        }
    } else if (left_type == DATA_STRING && right_type == DATA_STRING) {
        // String operations
        const char *left_str = ar__data__get_string(left);
        const char *right_str = ar__data__get_string(right);
        
        switch (op) {
            case OP_ADD:
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
            case OP_EQUAL:
                result = ar__data__create_integer(strcmp(left_str, right_str) == 0 ? 1 : 0);
                break;
            case OP_NOT_EQUAL:
                result = ar__data__create_integer(strcmp(left_str, right_str) != 0 ? 1 : 0);
                break;
            default:
                ar__io__error("ar__expression_evaluator__evaluate_binary_op: Unsupported operator for strings");
                break;
        }
    } else {
        ar__io__error("ar__expression_evaluator__evaluate_binary_op: Type mismatch in binary operation");
    }
    
    // Clean up operand values
    ar__data__destroy(left);
    ar__data__destroy(right);
    
    return result;
}
