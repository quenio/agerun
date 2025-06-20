/**
 * @file agerun_method_instruction_evaluator.c
 * @brief Implementation of the method instruction evaluator module
 */

#include "agerun_method_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include <string.h>
#include <stdio.h>

/**
 * Internal structure for method instruction evaluator
 * 
 * Note: This struct does not store a methodology reference because
 * ar__methodology__register_method() uses a global singleton internally.
 */
struct ar_method_instruction_evaluator_s {
    expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
};

/**
 * Creates a new method instruction evaluator instance
 */
ar_method_instruction_evaluator_t* ar__method_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    // Validate required parameters
    if (ref_expr_evaluator == NULL || mut_memory == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    ar_method_instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(
        sizeof(ar_method_instruction_evaluator_t), 
        "method_instruction_evaluator"
    );
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    evaluator->mut_memory = mut_memory;
    
    return evaluator;
}

/**
 * Destroys a method instruction evaluator instance
 */
void ar__method_instruction_evaluator__destroy(
    ar_method_instruction_evaluator_t *own_evaluator
) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}

/* Constants */
static const char* MEMORY_PREFIX = "memory.";
static const size_t MEMORY_PREFIX_LEN = 7;

/* Helper function to check if a path starts with "memory." and return the key path */
static const char* _get_memory_key_path(const char *ref_path) {
    if (!ref_path) {
        return NULL;
    }
    
    if (strncmp(ref_path, MEMORY_PREFIX, MEMORY_PREFIX_LEN) != 0) {
        return NULL;
    }
    
    return ref_path + MEMORY_PREFIX_LEN;
}

/* Helper function to extract function arguments and validate count */
static void** _extract_function_args(const instruction_ast_t *ref_ast, size_t expected_count, list_t **out_args_list) {
    if (!ref_ast || !out_args_list) {
        return NULL;
    }
    
    *out_args_list = ar__instruction_ast__get_function_args(ref_ast);
    if (!*out_args_list) {
        return NULL;
    }
    
    if (ar__list__count(*out_args_list) != expected_count) {
        ar__list__destroy(*out_args_list);
        *out_args_list = NULL;
        return NULL;
    }
    
    void **items = ar__list__items(*out_args_list);
    if (!items) {
        ar__list__destroy(*out_args_list);
        *out_args_list = NULL;
        return NULL;
    }
    
    return items;
}

/* Helper function to clean up function args */
static void _cleanup_function_args(void **items, list_t *own_args) {
    if (items) {
        AR__HEAP__FREE(items);
    }
    if (own_args) {
        ar__list__destroy(own_args);
    }
}

/* Helper function to create a deep copy of data value */
static data_t* _copy_data_value(const data_t *ref_value) {
    if (!ref_value) {
        return NULL;
    }
    
    switch (ar__data__get_type(ref_value)) {
        case DATA_INTEGER:
            return ar__data__create_integer(ar__data__get_integer(ref_value));
        case DATA_DOUBLE:
            return ar__data__create_double(ar__data__get_double(ref_value));
        case DATA_STRING:
            return ar__data__create_string(ar__data__get_string(ref_value));
        case DATA_MAP:
            {
                // Create a new map and copy all key-value pairs
                data_t *new_map = ar__data__create_map();
                if (!new_map) return NULL;
                
                // Get all keys from the original map
                data_t *keys = ar__data__get_map_keys(ref_value);
                if (!keys) {
                    ar__data__destroy(new_map);
                    return NULL;
                }
                
                // Copy each key-value pair
                size_t count = ar__data__list_count(keys);
                for (size_t i = 0; i < count; i++) {
                    // Get the key
                    data_t *key_data = ar__data__list_first(keys);
                    if (!key_data) break;
                    
                    const char *key = ar__data__get_string(key_data);
                    if (!key) {
                        data_t *removed = ar__data__list_remove_first(keys);
                        ar__data__destroy(removed);
                        continue;
                    }
                    
                    // Get the value from the original map
                    data_t *orig_value = ar__data__get_map_data(ref_value, key);
                    if (orig_value) {
                        // Recursively copy the value
                        data_t *copy_value = _copy_data_value(orig_value);
                        if (copy_value) {
                            bool success = ar__data__set_map_data(new_map, key, copy_value);
                            if (!success) {
                                fprintf(stderr, "ERROR: Failed to set map data for key '%s'\n", key);
                                ar__data__destroy(copy_value);
                            }
                        }
                    }
                    
                    // Remove and destroy the processed key
                    data_t *removed_key = ar__data__list_remove_first(keys);
                    ar__data__destroy(removed_key);
                }
                
                // Clean up the keys list
                ar__data__destroy(keys);
                
                return new_map;
            }
        case DATA_LIST:
            // TODO: Implement deep copy for lists
            return ar__data__create_list();
        default:
            return NULL;
    }
}

/* Helper function to evaluate an expression AST node using the expression evaluator */
static data_t* _evaluate_expression_ast(
    expression_evaluator_t *mut_expr_evaluator,
    expression_ast_t *ref_ast
) {
    if (!ref_ast) {
        return NULL;
    }
    
    expression_ast_type_t type = ar__expression_ast__get_type(ref_ast);
    
    switch (type) {
        case EXPR_AST_LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_expr_evaluator, ref_ast);
            
        case EXPR_AST_MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy
            {
                data_t *ref_value = ar__expression_evaluator__evaluate_memory_access(mut_expr_evaluator, ref_ast);
                if (!ref_value) return NULL;
                
                // Create a deep copy of the value
                return _copy_data_value(ref_value);
            }
            
        case EXPR_AST_BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_expr_evaluator, ref_ast);
            
        default:
            return NULL;
    }
}

/* Helper function to parse and evaluate an expression string */
static data_t* _parse_and_evaluate_expression(
    expression_evaluator_t *mut_expr_evaluator,
    const char *ref_expr
) {
    if (!ref_expr) {
        return NULL;
    }
    
    expression_parser_t *parser = ar__expression_parser__create(ref_expr);
    if (!parser) {
        return NULL;
    }
    
    expression_ast_t *ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!ast) {
        return NULL;
    }
    
    data_t *result = _evaluate_expression_ast(mut_expr_evaluator, ast);
    ar__expression_ast__destroy(ast);
    
    return result;
}

/* Helper function to store result in memory if assignment path is provided */
static bool _store_result_if_assigned(
    data_t *mut_memory,
    const instruction_ast_t *ref_ast,
    data_t *own_result
) {
    const char *ref_result_path = ar__instruction_ast__get_function_result_path(ref_ast);
    if (!ref_result_path) {
        // No assignment, just destroy the result
        ar__data__destroy(own_result);
        return true;
    }
    
    // Get memory key path
    const char *key_path = _get_memory_key_path(ref_result_path);
    if (!key_path) {
        ar__data__destroy(own_result);
        return false;
    }
    
    // Store the result (transfers ownership)
    bool store_success = ar__data__set_map_data(mut_memory, key_path, own_result);
    if (!store_success) {
        ar__data__destroy(own_result);
        return false;
    }
    
    return true;
}

/* Helper function to evaluate three string arguments from a function call */
static bool _evaluate_three_string_args(
    expression_evaluator_t *mut_expr_evaluator,
    const instruction_ast_t *ref_ast,
    size_t expected_arg_count,
    data_t **out_arg1,
    data_t **out_arg2,
    data_t **out_arg3
) {
    // Extract arguments
    list_t *own_args = NULL;
    void **items = _extract_function_args(ref_ast, expected_arg_count, &own_args);
    if (!items) {
        return false;
    }
    
    // Parse and evaluate arguments
    const char *ref_expr1 = (const char*)items[0];
    const char *ref_expr2 = (const char*)items[1];
    const char *ref_expr3 = (const char*)items[2];
    
    *out_arg1 = _parse_and_evaluate_expression(mut_expr_evaluator, ref_expr1);
    *out_arg2 = _parse_and_evaluate_expression(mut_expr_evaluator, ref_expr2);
    *out_arg3 = _parse_and_evaluate_expression(mut_expr_evaluator, ref_expr3);
    
    _cleanup_function_args(items, own_args);
    
    // Validate all arguments are strings
    if (*out_arg1 && *out_arg2 && *out_arg3 &&
        ar__data__get_type(*out_arg1) == DATA_STRING &&
        ar__data__get_type(*out_arg2) == DATA_STRING &&
        ar__data__get_type(*out_arg3) == DATA_STRING) {
        return true;
    }
    
    return false;
}

bool ar__method_instruction_evaluator__evaluate(
    ar_method_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Extract dependencies from the evaluator instance
    expression_evaluator_t *mut_expr_evaluator = mut_evaluator->ref_expr_evaluator;
    data_t *mut_memory = mut_evaluator->mut_memory;
    
    if (!mut_expr_evaluator || !mut_memory) {
        return false;
    }
    
    // Validate AST type
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_METHOD) {
        return false;
    }
    
    // Evaluate three string arguments
    data_t *own_method_name = NULL;
    data_t *own_instructions = NULL;
    data_t *own_version = NULL;
    
    bool args_valid = _evaluate_three_string_args(
        mut_expr_evaluator, ref_ast, 3,
        &own_method_name, &own_instructions, &own_version
    );
    
    bool success = false;
    
    if (args_valid) {
        const char *method_name = ar__data__get_string(own_method_name);
        const char *instructions = ar__data__get_string(own_instructions);
        const char *version = ar__data__get_string(own_version);
        
        // Create and register the method
        method_t *own_method = ar__method__create(method_name, instructions, version);
        if (own_method) {
            ar__methodology__register_method(own_method);
            // Ownership transferred to methodology
            own_method = NULL;
            success = true;
        }
    }
    
    // Clean up evaluated arguments
    if (own_method_name) ar__data__destroy(own_method_name);
    if (own_instructions) ar__data__destroy(own_instructions);
    if (own_version) ar__data__destroy(own_version);
    
    // Store result if assigned
    if (ar__instruction_ast__has_result_assignment(ref_ast)) {
        data_t *own_result = ar__data__create_integer(success ? 1 : 0);
        if (own_result) {
            _store_result_if_assigned(mut_memory, ref_ast, own_result);
        }
    }
    
    return success;
}

bool ar_method_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
) {
    // Create a temporary evaluator instance
    ar_method_instruction_evaluator_t *evaluator = ar__method_instruction_evaluator__create(
        mut_expr_evaluator, mut_memory
    );
    if (!evaluator) {
        return false;
    }
    
    // Call the new evaluate function
    bool result = ar__method_instruction_evaluator__evaluate(evaluator, ref_ast);
    
    // Destroy the temporary instance
    ar__method_instruction_evaluator__destroy(evaluator);
    
    return result;
}