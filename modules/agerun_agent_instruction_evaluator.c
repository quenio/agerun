/**
 * @file agerun_agent_instruction_evaluator.c
 * @brief Implementation of the agent instruction evaluator module
 */

#include "agerun_agent_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_agency.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

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

/* Helper function to get a reference to memory/context without copying */
static const data_t* _get_memory_or_context_reference(
    data_t *mut_memory,
    data_t *ref_context,
    const char *ref_expr
) {
    if (!ref_expr) {
        return NULL;
    }
    
    // Check if it's a simple "memory" or "context" expression
    if (strcmp(ref_expr, "memory") == 0) {
        return mut_memory;
    } else if (strcmp(ref_expr, "context") == 0) {
        return ref_context;
    }
    
    // Check if it's a memory.path or context.path expression
    if (strncmp(ref_expr, "memory.", 7) == 0) {
        const char *key_path = ref_expr + 7;
        return ar__data__get_map_data(mut_memory, key_path);
    } else if (strncmp(ref_expr, "context.", 8) == 0) {
        if (!ref_context) {
            return NULL;
        }
        const char *key_path = ref_expr + 8;
        return ar__data__get_map_data(ref_context, key_path);
    }
    
    // Not a simple memory/context access
    return NULL;
}

bool ar__agent_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    const instruction_ast_t *ref_ast
) {
    if (!mut_expr_evaluator || !mut_memory || !ref_ast) {
        return false;
    }
    
    // Validate AST type
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_AGENT) {
        return false;
    }
    
    // Extract arguments
    list_t *own_args = NULL;
    void **items = _extract_function_args(ref_ast, 3, &own_args);
    if (!items) {
        return false;
    }
    
    // Parse and evaluate arguments
    const char *ref_method_expr = (const char*)items[0];
    const char *ref_version_expr = (const char*)items[1];
    const char *ref_context_expr = (const char*)items[2];
    
    data_t *own_method_name = _parse_and_evaluate_expression(mut_expr_evaluator, ref_method_expr);
    data_t *own_version = _parse_and_evaluate_expression(mut_expr_evaluator, ref_version_expr);
    
    // For context, first try to get a direct reference to avoid copying
    const data_t *ref_context_data = _get_memory_or_context_reference(mut_memory, ref_context, ref_context_expr);
    data_t *own_context = NULL;
    
    if (!ref_context_data) {
        // Not a simple memory/context access, evaluate the expression
        own_context = _parse_and_evaluate_expression(mut_expr_evaluator, ref_context_expr);
        ref_context_data = own_context; // Use the evaluated result
    }
    
    _cleanup_function_args(items, own_args);
    
    int64_t agent_id = 0;
    bool success = false;
    
    // Validate method name and version are strings
    if (own_method_name && own_version &&
        ar__data__get_type(own_method_name) == DATA_STRING &&
        ar__data__get_type(own_version) == DATA_STRING) {
        
        // Validate context - must be a map (since parser requires 3 args)
        bool context_valid = false;
        if (ref_context_data && ar__data__get_type(ref_context_data) == DATA_MAP) {
            context_valid = true;
        }
        
        if (context_valid) {
            const char *method_name = ar__data__get_string(own_method_name);
            const char *version = ar__data__get_string(own_version);
            
            // Check if method exists
            method_t *ref_method = ar__methodology__get_method(method_name, version);
            if (ref_method) {
                // Create the agent - context is borrowed, not owned
                agent_id = ar__agency__create_agent(method_name, version, ref_context_data);
                if (agent_id > 0) {
                    success = true;
                }
            }
        }
    }
    
    // Clean up evaluated arguments
    if (own_method_name) ar__data__destroy(own_method_name);
    if (own_version) ar__data__destroy(own_version);
    if (own_context) ar__data__destroy(own_context); // Only destroy if ownership wasn't transferred
    
    // Store result if assigned
    if (ar__instruction_ast__has_result_assignment(ref_ast)) {
        data_t *own_result = ar__data__create_integer((int)agent_id);
        if (own_result) {
            _store_result_if_assigned(mut_memory, ref_ast, own_result);
        }
    }
    
    return success;
}