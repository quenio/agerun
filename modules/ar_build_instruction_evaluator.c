/**
 * @file agerun_build_instruction_evaluator.c
 * @brief Implementation of the build instruction evaluator
 */

#include "ar_build_instruction_evaluator.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_expression_evaluator.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_heap.h"
#include "ar_io.h"


/* Memory prefix for path extraction */
static const char MEMORY_PREFIX[] = "memory.";
static const size_t MEMORY_PREFIX_LEN = sizeof(MEMORY_PREFIX) - 1;

/* Opaque struct definition */
struct ar_build_instruction_evaluator_s {
    ar_expression_evaluator_t *ref_expr_evaluator;
    data_t *mut_memory;
};

/**
 * Creates a new build instruction evaluator
 */
ar_build_instruction_evaluator_t* ar_build_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    if (!ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_build_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(sizeof(ar_build_instruction_evaluator_t), "build_instruction_evaluator");
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator->mut_memory = mut_memory;
    
    return own_evaluator;  // Ownership transferred to caller
}

/**
 * Destroys a build instruction evaluator
 */
void ar_build_instruction_evaluator__destroy(
    ar_build_instruction_evaluator_t *own_evaluator
) {
    if (!own_evaluator) {
        return;
    }
    
    AR__HEAP__FREE(own_evaluator);
}


/**
 * Gets memory key path by removing "memory." prefix
 * 
 * @param ref_path The full path (e.g., "memory.foo.bar")
 * @return The key path without prefix (e.g., "foo.bar") or NULL
 */
static const char* _get_memory_key_path(const char *ref_path) {
    if (!ref_path) {
        return NULL;
    }
    
    // Check if path starts with "memory."
    if (strncmp(ref_path, MEMORY_PREFIX, MEMORY_PREFIX_LEN) != 0) {
        return NULL;
    }
    
    // Return pointer after "memory."
    return ref_path + MEMORY_PREFIX_LEN;
}

/**
 * Converts data value to string representation
 * 
 * @param ref_data The data value to convert (borrowed reference)
 * @param buffer Buffer to store numeric conversions
 * @param buffer_size Size of the buffer
 * @return String representation or NULL
 */
static const char* _data_to_string(const data_t *ref_data, char *buffer, size_t buffer_size) {
    if (!ref_data || !buffer || buffer_size == 0) {
        return NULL;
    }
    
    switch (ar_data__get_type(ref_data)) {
        case DATA_STRING:
            return ar_data__get_string(ref_data);
        case DATA_INTEGER:
            snprintf(buffer, buffer_size, "%d", ar_data__get_integer(ref_data));
            return buffer;
        case DATA_DOUBLE:
            snprintf(buffer, buffer_size, "%g", ar_data__get_double(ref_data));
            return buffer;
        default:
            return NULL;
    }
}

/**
 * Ensures buffer has sufficient capacity
 * 
 * @param own_buffer Current buffer (owned, may be NULL)
 * @param mut_capacity Current capacity (will be updated)
 * @param required_size Required size
 * @return New buffer or NULL on error
 * @note Ownership: Takes ownership of own_buffer, returns owned buffer
 */
static char* _ensure_buffer_capacity(char *own_buffer, size_t *mut_capacity, size_t required_size) {
    if (required_size <= *mut_capacity) {
        return own_buffer;
    }
    
    // Double the capacity until it's sufficient
    size_t new_capacity = *mut_capacity;
    while (new_capacity < required_size) {
        new_capacity *= 2;
    }
    
    char *new_buffer = AR__HEAP__MALLOC(new_capacity, "Buffer resize");
    if (!new_buffer) {
        return NULL;
    }
    
    // Copy existing content
    if (own_buffer) {
        memcpy(new_buffer, own_buffer, *mut_capacity);
        AR__HEAP__FREE(own_buffer);
    }
    
    *mut_capacity = new_capacity;
    return new_buffer;
}

/**
 * Processes a placeholder in the template
 * 
 * @param ref_template_ptr Current position in template (at '{')
 * @param ref_values Map of values for substitution
 * @param mut_result_str Result string buffer (mutable reference to owned buffer)
 * @param mut_result_size Result buffer size
 * @param mut_result_pos Current position in result buffer
 * @param mut_template_ptr Template pointer to update
 * @return true if placeholder was processed, false if no closing brace found
 */
static bool _process_placeholder(
    const char *ref_template_ptr,
    const data_t *ref_values,
    char **mut_result_str,
    size_t *mut_result_size,
    size_t *mut_result_pos,
    const char **mut_template_ptr
) {
    // Find closing brace
    const char *placeholder_end = strchr(ref_template_ptr + 1, '}');
    if (!placeholder_end) {
        return false;
    }
    
    // Extract variable name
    size_t var_len = (size_t)(placeholder_end - ref_template_ptr - 1);
    char *var_name = AR__HEAP__MALLOC(var_len + 1, "Build variable name");
    if (!var_name) {
        return false;
    }
    
    memcpy(var_name, ref_template_ptr + 1, var_len);
    var_name[var_len] = '\0';
    
    // Look up value and convert to string
    const data_t *ref_value = ar_data__get_map_data(ref_values, var_name);
    char value_buffer[256];
    const char *value_str = ref_value ? _data_to_string(ref_value, value_buffer, sizeof(value_buffer)) : NULL;
    
    // Determine what to append (value or original placeholder)
    const char *append_str;
    size_t append_len;
    if (value_str) {
        append_str = value_str;
        append_len = strlen(value_str);
    } else {
        // Keep original placeholder
        append_str = ref_template_ptr;
        append_len = var_len + 2;  // {varname}
    }
    
    // Ensure buffer capacity
    char *new_buffer = _ensure_buffer_capacity(*mut_result_str, mut_result_size, 
                                               *mut_result_pos + append_len + 1);
    if (!new_buffer) {
        AR__HEAP__FREE(var_name);
        // Note: caller still owns *mut_result_str and must free it
        return false;
    }
    *mut_result_str = new_buffer;
    
    // Append to result
    memcpy(*mut_result_str + *mut_result_pos, append_str, append_len);
    *mut_result_pos += append_len;
    
    // Update template pointer
    *mut_template_ptr = value_str ? placeholder_end + 1 : placeholder_end + 1;
    
    AR__HEAP__FREE(var_name);
    return true;
}

/* Helper function to create a deep copy of data value */
static data_t* _copy_data_value(const data_t *ref_value) {
    if (!ref_value) {
        return NULL;
    }
    
    switch (ar_data__get_type(ref_value)) {
        case DATA_INTEGER:
            return ar_data__create_integer(ar_data__get_integer(ref_value));
        case DATA_DOUBLE:
            return ar_data__create_double(ar_data__get_double(ref_value));
        case DATA_STRING:
            return ar_data__create_string(ar_data__get_string(ref_value));
        case DATA_MAP:
            {
                // Create a new map and copy all key-value pairs
                data_t *new_map = ar_data__create_map();
                if (!new_map) return NULL;
                
                // Get all keys from the original map
                data_t *keys = ar_data__get_map_keys(ref_value);
                if (!keys) {
                    ar_data__destroy(new_map);
                    return NULL;
                }
                
                // Copy each key-value pair
                size_t count = ar_data__list_count(keys);
                for (size_t i = 0; i < count; i++) {
                    // Get the key
                    data_t *key_data = ar_data__list_first(keys);
                    if (!key_data) break;
                    
                    const char *key = ar_data__get_string(key_data);
                    if (!key) {
                        data_t *removed = ar_data__list_remove_first(keys);
                        ar_data__destroy(removed);
                        continue;
                    }
                    
                    // Get the value from the original map
                    data_t *orig_value = ar_data__get_map_data(ref_value, key);
                    if (orig_value) {
                        // Recursively copy the value
                        data_t *copy_value = _copy_data_value(orig_value);
                        if (copy_value) {
                            bool success = ar_data__set_map_data(new_map, key, copy_value);
                            if (!success) {
                                fprintf(stderr, "ERROR: Failed to set map data for key '%s'\n", key);
                                ar_data__destroy(copy_value);
                            }
                        }
                    }
                    
                    // Remove and destroy the processed key
                    data_t *removed_key = ar_data__list_remove_first(keys);
                    ar_data__destroy(removed_key);
                }
                
                // Clean up the keys list
                ar_data__destroy(keys);
                
                return new_map;
            }
        case DATA_LIST:
            // TODO: Implement deep copy for lists
            return ar_data__create_list();
        default:
            return NULL;
    }
}


/**
 * Stores result if instruction has assignment
 * 
 * @param mut_memory Memory map (mutable reference)
 * @param ref_ast The instruction AST (borrowed reference)
 * @param own_result The result to store (owned value)
 * @return true if successful, false otherwise
 * @note Ownership: Takes ownership of own_result
 */
static bool _store_result_if_assigned(
    data_t *mut_memory,
    const ar_instruction_ast_t *ref_ast,
    data_t *own_result
) {
    const char *ref_result_path = ar_instruction_ast__get_function_result_path(ref_ast);
    if (!ref_result_path) {
        // No assignment, just destroy the result
        ar_data__destroy(own_result);
        return true;
    }
    
    // Get memory key path
    const char *key_path = _get_memory_key_path(ref_result_path);
    if (!key_path) {
        ar_data__destroy(own_result);
        return false;
    }
    
    // Store the result (transfers ownership)
    bool store_success = ar_data__set_map_data(mut_memory, key_path, own_result);
    if (!store_success) {
        ar_data__destroy(own_result);
        return false;
    }
    
    return true;
}

/**
 * Evaluates a build instruction using the stored dependencies
 */
bool ar_build_instruction_evaluator__evaluate(
    ar_build_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    ar_expression_evaluator_t *mut_expr_evaluator = mut_evaluator->ref_expr_evaluator;
    data_t *mut_memory = mut_evaluator->mut_memory;
    
    // Verify this is a build AST node
    if (ar_instruction_ast__get_type(ref_ast) != AR_INST__BUILD) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (!ref_arg_asts) {
        return false;
    }
    
    // Verify we have exactly 2 arguments
    if (ar_list__count(ref_arg_asts) != 2) {
        return false;
    }
    
    // Get the argument ASTs array
    void **items = ar_list__items(ref_arg_asts);
    if (!items) {
        return false;
    }
    
    const ar_expression_ast_t *ref_template_ast = (const ar_expression_ast_t*)items[0];
    const ar_expression_ast_t *ref_values_ast = (const ar_expression_ast_t*)items[1];
    
    if (!ref_template_ast || !ref_values_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate template expression AST
    data_t *template_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_template_ast);
    if (!template_result || ar_data__get_type(template_result) != DATA_STRING) {
        fprintf(stderr, "DEBUG: build evaluator - template evaluation failed or not string\n");
        if (template_result && ar_data__hold_ownership(template_result, mut_evaluator)) {
            ar_data__transfer_ownership(template_result, mut_evaluator);
            ar_data__destroy(template_result);
        }
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Get ownership of template data
    data_t *own_template_data;
    if (ar_data__hold_ownership(template_result, mut_evaluator)) {
        // We can claim ownership - it's an unowned value
        ar_data__transfer_ownership(template_result, mut_evaluator);
        own_template_data = template_result;
    } else {
        // It's owned by someone else - we need to make a copy
        own_template_data = _copy_data_value(template_result);
        if (!own_template_data) {
            AR__HEAP__FREE(items);
            return false;
        }
    }
    
    // Evaluate values expression AST to check for map
    data_t *values_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_values_ast);
    const data_t *ref_values_data = values_result;
    data_t *own_values_data = NULL;
    
    // Check if we need to make a copy (if result is owned by memory/context)
    // CRITICAL: Never try to take ownership of memory itself!
    if (values_result == mut_memory) {
        own_values_data = NULL;
    } else if (values_result && ar_data__hold_ownership(values_result, mut_evaluator)) {
        // We can claim ownership - it's an unowned value
        ar_data__transfer_ownership(values_result, mut_evaluator);
        own_values_data = values_result;
    }
    
    // Validate it's a map
    if (!ref_values_data || ar_data__get_type(ref_values_data) != DATA_MAP) {
        if (own_values_data) ar_data__destroy(own_values_data);
        ar_data__destroy(own_template_data);
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Clean up the items array as we're done with it
    AR__HEAP__FREE(items);
    
    const char *template_str = ar_data__get_string(own_template_data);
    
    // Build the string by replacing placeholders in template
    size_t result_size = strlen(template_str) * 2 + 256;
    char *own_result_str = AR__HEAP__MALLOC(result_size, "Build result");
    if (!own_result_str) {
        if (own_values_data) ar_data__destroy(own_values_data);
        ar_data__destroy(own_template_data);
        return false;
    }
    
    size_t result_pos = 0;
    const char *template_ptr = template_str;
    
    while (*template_ptr) {
        if (*template_ptr == '{') {
            // Process placeholder
            if (!_process_placeholder(template_ptr, ref_values_data, 
                                      &own_result_str, &result_size, &result_pos, &template_ptr)) {
                // If no closing brace found, just copy the '{' character
                char *new_buffer = _ensure_buffer_capacity(own_result_str, &result_size, result_pos + 2);
                if (!new_buffer) {
                    AR__HEAP__FREE(own_result_str);
                    if (own_values_data) ar_data__destroy(own_values_data);
                    ar_data__destroy(own_template_data);
                    return false;
                }
                own_result_str = new_buffer;
                own_result_str[result_pos++] = '{';
                template_ptr++;
            }
        } else {
            // Regular character, ensure capacity and copy
            char *new_buffer = _ensure_buffer_capacity(own_result_str, &result_size, result_pos + 2);
            if (!new_buffer) {
                AR__HEAP__FREE(own_result_str);
                if (own_values_data) ar_data__destroy(own_values_data);
                ar_data__destroy(own_template_data);
                return false;
            }
            own_result_str = new_buffer;
            own_result_str[result_pos++] = *template_ptr++;
        }
    }
    
    // Null-terminate the result
    own_result_str[result_pos] = '\0';
    
    // Create result data object
    data_t *own_result = ar_data__create_string(own_result_str);
    AR__HEAP__FREE(own_result_str);
    
    if (!own_result) {
        if (own_values_data) ar_data__destroy(own_values_data);
        ar_data__destroy(own_template_data);
        return false;
    }
    
    // Clean up
    if (own_values_data) ar_data__destroy(own_values_data);
    ar_data__destroy(own_template_data);
    
    // Store result if assigned, otherwise just destroy it
    return _store_result_if_assigned(mut_memory, ref_ast, own_result);
}


