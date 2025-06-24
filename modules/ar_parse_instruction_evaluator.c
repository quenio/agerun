/**
 * @file agerun_parse_instruction_evaluator.c
 * @brief Implementation of the parse instruction evaluator
 */

#include "ar_parse_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_expression_evaluator.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Struct definition for parse instruction evaluator */
struct ar_parse_instruction_evaluator_s {
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
};

/* Constants */
static const char* MEMORY_PREFIX = "memory.";
static const size_t MEMORY_PREFIX_LEN = 7;

/* Forward declarations of helper functions */
static data_t* _evaluate_expression_ast(ar_expression_evaluator_t *mut_expr_evaluator, const ar_expression_ast_t *ref_ast);
static const char* _get_memory_key_path(const char *ref_path);
static bool _store_result_if_assigned(data_t *mut_memory, const ar_instruction_ast_t *ref_ast, data_t *own_result);
static data_t* _copy_data_value(const data_t *ref_value);
static data_t* _parse_value_string(const char *value_str);

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

/* Helper function to store result in memory if assignment path is provided */
static bool _store_result_if_assigned(
    data_t *mut_memory,
    const ar_instruction_ast_t *ref_ast,
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


/* Helper function to evaluate an expression AST node using the expression evaluator */
static data_t* _evaluate_expression_ast(ar_expression_evaluator_t *mut_expr_evaluator, const ar_expression_ast_t *ref_ast) {
    if (!ref_ast) {
        return NULL;
    }
    
    ar_expression_ast_type_t type = ar__expression_ast__get_type(ref_ast);
    
    switch (type) {
        case AR_EXPR__LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_expr_evaluator, ref_ast);
            
        case AR_EXPR__LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_expr_evaluator, ref_ast);
            
        case AR_EXPR__LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_expr_evaluator, ref_ast);
            
        case AR_EXPR__MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy
            {
                data_t *ref_value = ar__expression_evaluator__evaluate_memory_access(mut_expr_evaluator, ref_ast);
                if (!ref_value) return NULL;
                
                // Create a deep copy of the value
                return _copy_data_value(ref_value);
            }
            
        case AR_EXPR__BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_expr_evaluator, ref_ast);
            
        default:
            return NULL;
    }
}

/* Helper function to parse a value string and determine its type */
static data_t* _parse_value_string(const char *value_str) {
    if (!value_str || *value_str == '\0') {
        return ar__data__create_string("");
    }
    
    // Try to parse as integer first
    char *endptr;
    long int_val = strtol(value_str, &endptr, 10);
    if (*endptr == '\0' && value_str[0] != '\0') {
        return ar__data__create_integer((int)int_val);
    }
    
    // Try to parse as double
    double double_val = strtod(value_str, &endptr);
    if (*endptr == '\0' && value_str[0] != '\0' && strchr(value_str, '.')) {
        return ar__data__create_double(double_val);
    }
    
    // Otherwise treat as string
    return ar__data__create_string(value_str);
}

/**
 * Creates a new parse instruction evaluator
 */
ar_parse_instruction_evaluator_t* ar_parse_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    if (!ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_parse_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_parse_instruction_evaluator_t),
        "parse_instruction_evaluator"
    );
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator->mut_memory = mut_memory;
    
    // Ownership transferred to caller
    return own_evaluator;
}

/**
 * Destroys a parse instruction evaluator
 */
void ar_parse_instruction_evaluator__destroy(
    ar_parse_instruction_evaluator_t *own_evaluator
) {
    if (!own_evaluator) {
        return;
    }
    
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Evaluates a parse instruction using the stored dependencies
 */
bool ar_parse_instruction_evaluator__evaluate(
    ar_parse_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Verify this is a parse AST node
    if (ar__instruction_ast__get_type(ref_ast) != AR_INST__PARSE) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const list_t *ref_arg_asts = ar__instruction_ast__get_function_arg_asts(ref_ast);
    if (!ref_arg_asts) {
        return false;
    }
    
    // Verify we have exactly 2 arguments
    if (ar__list__count(ref_arg_asts) != 2) {
        return false;
    }
    
    // Get the argument ASTs array
    void **items = ar__list__items(ref_arg_asts);
    if (!items) {
        return false;
    }
    
    const ar_expression_ast_t *ref_template_ast = (const ar_expression_ast_t*)items[0];
    const ar_expression_ast_t *ref_input_ast = (const ar_expression_ast_t*)items[1];
    
    if (!ref_template_ast || !ref_input_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate template expression AST
    data_t *own_template_data = _evaluate_expression_ast(mut_evaluator->ref_expr_evaluator, ref_template_ast);
    if (!own_template_data || ar__data__get_type(own_template_data) != DATA_STRING) {
        if (own_template_data) ar__data__destroy(own_template_data);
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate input expression AST
    data_t *own_input_data = _evaluate_expression_ast(mut_evaluator->ref_expr_evaluator, ref_input_ast);
    if (!own_input_data || ar__data__get_type(own_input_data) != DATA_STRING) {
        if (own_input_data) ar__data__destroy(own_input_data);
        ar__data__destroy(own_template_data);
        AR__HEAP__FREE(items);
        return false;
    }
    
    const char *template_str = ar__data__get_string(own_template_data);
    const char *input_str = ar__data__get_string(own_input_data);
    
    // Clean up the items array as we're done with it
    AR__HEAP__FREE(items);
    
    // Create result map
    data_t *own_result = ar__data__create_map();
    if (!own_result) {
        ar__data__destroy(own_input_data);
        ar__data__destroy(own_template_data);
        return false;
    }
    
    // Parse the template and input to extract values
    const char *template_ptr = template_str;
    const char *input_ptr = input_str;
    
    while (*template_ptr && *input_ptr) {
        // Look for {variable} pattern
        const char *var_start = strchr(template_ptr, '{');
        if (!var_start) {
            // No more variables, check if remaining template matches input
            if (strcmp(template_ptr, input_ptr) != 0) {
                // Template doesn't match input
                ar__data__destroy(own_result);
                own_result = ar__data__create_map(); // Return empty map
            }
            break;
        }
        
        const char *var_end = strchr(var_start + 1, '}');
        if (!var_end) {
            // Invalid template - no closing brace
            break;
        }
        
        // Extract variable name
        size_t var_len = (size_t)(var_end - var_start - 1);
        char *var_name = AR__HEAP__MALLOC(var_len + 1, "Parse variable name");
        if (!var_name) {
            ar__data__destroy(own_result);
            ar__data__destroy(own_input_data);
            ar__data__destroy(own_template_data);
            return false;
        }
        memcpy(var_name, var_start + 1, var_len);
        var_name[var_len] = '\0';
        
        // Get the literal text between current position and {
        size_t literal_len = (size_t)(var_start - template_ptr);
        
        // Match literal text in input
        if (literal_len > 0) {
            if (strncmp(template_ptr, input_ptr, literal_len) != 0) {
                // Literal doesn't match
                AR__HEAP__FREE(var_name);
                ar__data__destroy(own_result);
                own_result = ar__data__create_map(); // Return empty map
                break;
            }
            input_ptr += literal_len;
        }
        
        // Find the next literal after the variable
        template_ptr = var_end + 1;
        const char *next_var_start = strchr(template_ptr, '{');
        size_t next_literal_len = 0;
        
        if (next_var_start) {
            next_literal_len = (size_t)(next_var_start - template_ptr);
        } else {
            next_literal_len = strlen(template_ptr);
        }
        
        // Extract value from input
        const char *value_end;
        if (next_literal_len > 0) {
            // Extract next literal text to search for
            char *next_literal = AR__HEAP__MALLOC(next_literal_len + 1, "Parse next literal");
            if (!next_literal) {
                AR__HEAP__FREE(var_name);
                ar__data__destroy(own_result);
                ar__data__destroy(own_input_data);
                ar__data__destroy(own_template_data);
                return false;
            }
            memcpy(next_literal, template_ptr, next_literal_len);
            next_literal[next_literal_len] = '\0';
            
            // Find where the next literal starts in input
            const char *next_literal_pos = strstr(input_ptr, next_literal);
            AR__HEAP__FREE(next_literal);
            
            if (next_literal_pos) {
                value_end = next_literal_pos;
            } else {
                // Next literal not found
                AR__HEAP__FREE(var_name);
                ar__data__destroy(own_result);
                own_result = ar__data__create_map(); // Return empty map
                break;
            }
        } else {
            // No more literals, take rest of input
            value_end = input_ptr + strlen(input_ptr);
        }
        
        // Extract the value string
        size_t value_len = (size_t)(value_end - input_ptr);
        char *value_str = AR__HEAP__MALLOC(value_len + 1, "Parse value");
        if (!value_str) {
            AR__HEAP__FREE(var_name);
            ar__data__destroy(own_result);
            ar__data__destroy(own_input_data);
            ar__data__destroy(own_template_data);
            return false;
        }
        memcpy(value_str, input_ptr, value_len);
        value_str[value_len] = '\0';
        
        // Parse the value and store in result map
        data_t *own_value = _parse_value_string(value_str);
        AR__HEAP__FREE(value_str);
        
        if (own_value) {
            ar__data__set_map_data(own_result, var_name, own_value);
            // Ownership transferred
        }
        
        AR__HEAP__FREE(var_name);
        
        // Move input pointer past the value
        input_ptr = value_end;
        
        // Move template pointer past the literal (if any)
        if (next_literal_len > 0) {
            input_ptr += next_literal_len;
            template_ptr += next_literal_len;
        }
    }
    
    // Clean up
    ar__data__destroy(own_input_data);
    ar__data__destroy(own_template_data);
    
    // Store result if assigned, otherwise just destroy it
    return _store_result_if_assigned(mut_evaluator->mut_memory, ref_ast, own_result);
}

