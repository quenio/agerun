/**
 * @file agerun_expression_evaluator.c
 * @brief Implementation of expression evaluator module
 */

#include "ar_expression_evaluator.h"
#include "ar_heap.h"
#include "ar_io.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Internal structure for expression evaluator
 */
struct expression_evaluator_s {
    ar_log_t *ref_log;     /**< Log instance for error reporting (borrowed) */
    data_t *ref_memory;    /**< Memory map with variables (borrowed) */
    data_t *ref_context;   /**< Optional context map (borrowed, may be NULL) */
};

/* Helper function to log error messages */
static void _log_error(ar_expression_evaluator_t *mut_evaluator, const char *message) {
    if (message && mut_evaluator && mut_evaluator->ref_log) {
        ar_log__error(mut_evaluator->ref_log, message);
    }
}

ar_expression_evaluator_t* ar_expression_evaluator__create(
    ar_log_t *ref_log,
    data_t *ref_memory,
    data_t *ref_context)
{
    if (!ref_log) {
        ar_io__error("ar_expression_evaluator__create: NULL log");
        return NULL;
    }
    
    if (!ref_memory) {
        ar_log__error(ref_log, "ar_expression_evaluator__create: NULL memory");
        return NULL;
    }

    ar_expression_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(ar_expression_evaluator_t), "expression_evaluator");
    if (!evaluator) {
        ar_log__error(ref_log, "ar_expression_evaluator__create: Failed to allocate evaluator");
        return NULL;
    }

    evaluator->ref_log = ref_log;
    evaluator->ref_memory = ref_memory;
    evaluator->ref_context = ref_context;

    return evaluator;
}

void ar_expression_evaluator__destroy(ar_expression_evaluator_t *own_evaluator)
{
    if (own_evaluator) {
        AR__HEAP__FREE(own_evaluator);
    }
}

data_t* ar_expression_evaluator__evaluate_literal_int(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        _log_error(mut_evaluator, "evaluate_literal_int: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is an integer literal
    if (ar_expression_ast__get_type(ref_node) != AR_EXPR__LITERAL_INT) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the integer value and create a data_t
    int value = ar_expression_ast__get_int_value(ref_node);
    return ar_data__create_integer(value);
}

data_t* ar_expression_evaluator__evaluate_literal_double(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        _log_error(mut_evaluator, "evaluate_literal_double: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a double literal
    if (ar_expression_ast__get_type(ref_node) != AR_EXPR__LITERAL_DOUBLE) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the double value and create a data_t
    double value = ar_expression_ast__get_double_value(ref_node);
    return ar_data__create_double(value);
}

data_t* ar_expression_evaluator__evaluate_literal_string(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        _log_error(mut_evaluator, "evaluate_literal_string: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a string literal
    if (ar_expression_ast__get_type(ref_node) != AR_EXPR__LITERAL_STRING) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the string value and create a data_t
    const char *value = ar_expression_ast__get_string_value(ref_node);
    return ar_data__create_string(value);
}

data_t* ar_expression_evaluator__evaluate_memory_access(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        _log_error(mut_evaluator, "evaluate_memory_access: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a memory access
    if (ar_expression_ast__get_type(ref_node) != AR_EXPR__MEMORY_ACCESS) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the base accessor (should be "memory" or "context")
    const char *base = ar_expression_ast__get_memory_base(ref_node);
    if (!base) {
        _log_error(mut_evaluator, "evaluate_memory_access: No base accessor");
        return NULL;
    }
    
    // Get the path components
    size_t path_count = 0;
    char **path = ar_expression_ast__get_memory_path(ref_node, &path_count);
    
    // Determine which map to use based on the base
    data_t *map = NULL;
    if (strcmp(base, "memory") == 0) {
        map = mut_evaluator->ref_memory;
    } else if (strcmp(base, "context") == 0) {
        map = mut_evaluator->ref_context;
    } else {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "ar_expression_evaluator__evaluate_memory_access: Invalid base accessor '%s'", base);
        _log_error(mut_evaluator, error_msg);
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
        if (ar_data__get_type(current) != DATA_MAP) {
            // Can't navigate further if not a map
            AR__HEAP__FREE(path);
            return NULL;
        }
        
        current = ar_data__get_map_data(current, path[i]);
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

data_t* ar_expression_evaluator__evaluate(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_ast)
{
    if (!mut_evaluator || !ref_ast) {
        _log_error(mut_evaluator, "evaluate: NULL evaluator or AST");
        return NULL;
    }
    
    ar_expression_ast_type_t type = ar_expression_ast__get_type(ref_ast);
    
    switch (type) {
        case AR_EXPR__LITERAL_INT:
            return ar_expression_evaluator__evaluate_literal_int(mut_evaluator, ref_ast);
        case AR_EXPR__LITERAL_DOUBLE:
            return ar_expression_evaluator__evaluate_literal_double(mut_evaluator, ref_ast);
        case AR_EXPR__LITERAL_STRING:
            return ar_expression_evaluator__evaluate_literal_string(mut_evaluator, ref_ast);
        case AR_EXPR__MEMORY_ACCESS:
            // Memory access returns a reference owned by the memory/context map
            return ar_expression_evaluator__evaluate_memory_access(mut_evaluator, ref_ast);
        case AR_EXPR__BINARY_OP:
            return ar_expression_evaluator__evaluate_binary_op(mut_evaluator, ref_ast);
        default:
            _log_error(mut_evaluator, "evaluate: Unknown expression type");
            return NULL;
    }
}

/**
 * Helper function to evaluate any expression AST node
 * This is used internally by binary operations to evaluate operands
 */
static data_t* _evaluate_expression(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!ref_node) return NULL;
    
    ar_expression_ast_type_t type = ar_expression_ast__get_type(ref_node);
    
    switch (type) {
        case AR_EXPR__LITERAL_INT:
            return ar_expression_evaluator__evaluate_literal_int(mut_evaluator, ref_node);
        case AR_EXPR__LITERAL_DOUBLE:
            return ar_expression_evaluator__evaluate_literal_double(mut_evaluator, ref_node);
        case AR_EXPR__LITERAL_STRING:
            return ar_expression_evaluator__evaluate_literal_string(mut_evaluator, ref_node);
        case AR_EXPR__MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy for binary ops
            {
                data_t *ref_value = ar_expression_evaluator__evaluate_memory_access(mut_evaluator, ref_node);
                if (!ref_value) return NULL;
                
                // Create a copy based on type
                switch (ar_data__get_type(ref_value)) {
                    case DATA_INTEGER:
                        return ar_data__create_integer(ar_data__get_integer(ref_value));
                    case DATA_DOUBLE:
                        return ar_data__create_double(ar_data__get_double(ref_value));
                    case DATA_STRING:
                        return ar_data__create_string(ar_data__get_string(ref_value));
                    default:
                        _log_error(mut_evaluator, "_evaluate_expression: Unsupported data type for copy");
                        return NULL;
                }
            }
        case AR_EXPR__BINARY_OP:
            return ar_expression_evaluator__evaluate_binary_op(mut_evaluator, ref_node);
        default:
            _log_error(mut_evaluator, "_evaluate_expression: Unknown expression type");
            return NULL;
    }
}

data_t* ar_expression_evaluator__evaluate_binary_op(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node)
{
    if (!mut_evaluator || !ref_node) {
        _log_error(mut_evaluator, "evaluate_binary_op: NULL evaluator or node");
        return NULL;
    }
    
    // Check if the node is a binary operation
    if (ar_expression_ast__get_type(ref_node) != AR_EXPR__BINARY_OP) {
        // Not an error, just not the right type
        return NULL;
    }
    
    // Get the operator and operands
    ar_binary_operator_t op = ar_expression_ast__get_operator(ref_node);
    const ar_expression_ast_t *left_node = ar_expression_ast__get_left(ref_node);
    const ar_expression_ast_t *right_node = ar_expression_ast__get_right(ref_node);
    
    if (!left_node || !right_node) {
        _log_error(mut_evaluator, "evaluate_binary_op: Missing operands");
        return NULL;
    }
    
    // Recursively evaluate both operands
    data_t *left = _evaluate_expression(mut_evaluator, left_node);
    if (!left) {
        _log_error(mut_evaluator, "evaluate_binary_op: Failed to evaluate left operand");
        return NULL;
    }
    
    data_t *right = _evaluate_expression(mut_evaluator, right_node);
    if (!right) {
        ar_data__destroy(left);
        _log_error(mut_evaluator, "evaluate_binary_op: Failed to evaluate right operand");
        return NULL;
    }
    
    // Get the types of both operands
    ar_data_type_t left_type = ar_data__get_type(left);
    ar_data_type_t right_type = ar_data__get_type(right);
    
    data_t *result = NULL;
    
    // Handle operations based on types
    if (left_type == DATA_INTEGER && right_type == DATA_INTEGER) {
        // Integer operations
        int left_val = ar_data__get_integer(left);
        int right_val = ar_data__get_integer(right);
        
        switch (op) {
            case AR_OP__ADD:
                result = ar_data__create_integer(left_val + right_val);
                break;
            case AR_OP__SUBTRACT:
                result = ar_data__create_integer(left_val - right_val);
                break;
            case AR_OP__MULTIPLY:
                result = ar_data__create_integer(left_val * right_val);
                break;
            case AR_OP__DIVIDE:
                if (right_val == 0) {
                    _log_error(mut_evaluator, "evaluate_binary_op: Division by zero");
                } else {
                    result = ar_data__create_integer(left_val / right_val);
                }
                break;
            case AR_OP__EQUAL:
                result = ar_data__create_integer(left_val == right_val ? 1 : 0);
                break;
            case AR_OP__NOT_EQUAL:
                result = ar_data__create_integer(left_val != right_val ? 1 : 0);
                break;
            case AR_OP__LESS:
                result = ar_data__create_integer(left_val < right_val ? 1 : 0);
                break;
            case AR_OP__GREATER:
                result = ar_data__create_integer(left_val > right_val ? 1 : 0);
                break;
            case AR_OP__LESS_EQ:
                result = ar_data__create_integer(left_val <= right_val ? 1 : 0);
                break;
            case AR_OP__GREATER_EQ:
                result = ar_data__create_integer(left_val >= right_val ? 1 : 0);
                break;
            default:
                _log_error(mut_evaluator, "evaluate_binary_op: Unknown operator for integers");
                break;
        }
    } else if (left_type == DATA_DOUBLE || right_type == DATA_DOUBLE) {
        // Convert to doubles if either operand is a double
        double left_val = (left_type == DATA_DOUBLE) ? 
            ar_data__get_double(left) : (double)ar_data__get_integer(left);
        double right_val = (right_type == DATA_DOUBLE) ? 
            ar_data__get_double(right) : (double)ar_data__get_integer(right);
        
        switch (op) {
            case AR_OP__ADD:
                result = ar_data__create_double(left_val + right_val);
                break;
            case AR_OP__SUBTRACT:
                result = ar_data__create_double(left_val - right_val);
                break;
            case AR_OP__MULTIPLY:
                result = ar_data__create_double(left_val * right_val);
                break;
            case AR_OP__DIVIDE:
                if (right_val == 0.0) {
                    _log_error(mut_evaluator, "evaluate_binary_op: Division by zero");
                } else {
                    result = ar_data__create_double(left_val / right_val);
                }
                break;
            case AR_OP__EQUAL:
                result = ar_data__create_integer(left_val == right_val ? 1 : 0);
                break;
            case AR_OP__NOT_EQUAL:
                result = ar_data__create_integer(left_val != right_val ? 1 : 0);
                break;
            case AR_OP__LESS:
                result = ar_data__create_integer(left_val < right_val ? 1 : 0);
                break;
            case AR_OP__GREATER:
                result = ar_data__create_integer(left_val > right_val ? 1 : 0);
                break;
            case AR_OP__LESS_EQ:
                result = ar_data__create_integer(left_val <= right_val ? 1 : 0);
                break;
            case AR_OP__GREATER_EQ:
                result = ar_data__create_integer(left_val >= right_val ? 1 : 0);
                break;
            default:
                _log_error(mut_evaluator, "evaluate_binary_op: Unknown operator for doubles");
                break;
        }
    } else if (left_type == DATA_STRING && right_type == DATA_STRING) {
        // String operations
        const char *left_str = ar_data__get_string(left);
        const char *right_str = ar_data__get_string(right);
        
        switch (op) {
            case AR_OP__ADD:
                // String concatenation
                {
                    size_t len = strlen(left_str) + strlen(right_str) + 1;
                    char *concat = AR__HEAP__MALLOC(len, "string concatenation");
                    if (concat) {
                        strcpy(concat, left_str);
                        strcat(concat, right_str);
                        result = ar_data__create_string(concat);
                        AR__HEAP__FREE(concat);
                    }
                }
                break;
            case AR_OP__EQUAL:
                result = ar_data__create_integer(strcmp(left_str, right_str) == 0 ? 1 : 0);
                break;
            case AR_OP__NOT_EQUAL:
                result = ar_data__create_integer(strcmp(left_str, right_str) != 0 ? 1 : 0);
                break;
            default:
                _log_error(mut_evaluator, "evaluate_binary_op: Unsupported operator for strings");
                break;
        }
    } else {
        _log_error(mut_evaluator, "evaluate_binary_op: Type mismatch in binary operation");
    }
    
    // Clean up operand values
    ar_data__destroy(left);
    ar_data__destroy(right);
    
    return result;
}
