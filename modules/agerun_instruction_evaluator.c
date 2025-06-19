/**
 * @file agerun_instruction_evaluator.c
 * @brief Implementation of the instruction evaluator module
 */

#include "agerun_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_agency.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_assignment_instruction_evaluator.h"
#include "agerun_send_instruction_evaluator.h"
#include "agerun_condition_instruction_evaluator.h"
#include "agerun_parse_instruction_evaluator.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

/**
 * Internal structure for instruction evaluator
 */
struct instruction_evaluator_s {
    expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
    data_t *ref_context;                         /* Context map (borrowed reference, can be NULL) */
    data_t *ref_message;                         /* Message data (borrowed reference, can be NULL) */
};

/**
 * Creates a new instruction evaluator
 */
instruction_evaluator_t* ar__instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
) {
    // Validate required parameters
    if (ref_expr_evaluator == NULL || mut_memory == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(instruction_evaluator_t), "instruction_evaluator");
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    evaluator->mut_memory = mut_memory;
    evaluator->ref_context = ref_context;
    evaluator->ref_message = ref_message;
    
    return evaluator;
}

/**
 * Destroys an instruction evaluator
 */
void ar__instruction_evaluator__destroy(instruction_evaluator_t *own_evaluator) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}

/* Forward declarations */
static data_t* _evaluate_expression_ast(instruction_evaluator_t *mut_evaluator, expression_ast_t *ref_ast);
static void** _extract_function_args(const instruction_ast_t *ref_ast, size_t expected_count, list_t **out_args_list);
static void _cleanup_function_args(void **items, list_t *own_args);
static data_t* _parse_and_evaluate_expression(instruction_evaluator_t *mut_evaluator, const char *ref_expr);

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

/* Helper function to evaluate three string arguments from a function call */
static bool _evaluate_three_string_args(
    instruction_evaluator_t *mut_evaluator,
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
    
    *out_arg1 = _parse_and_evaluate_expression(mut_evaluator, ref_expr1);
    *out_arg2 = _parse_and_evaluate_expression(mut_evaluator, ref_expr2);
    *out_arg3 = _parse_and_evaluate_expression(mut_evaluator, ref_expr3);
    
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

/* Helper function to get a reference to memory/context without copying */
static const data_t* _get_memory_or_context_reference(
    instruction_evaluator_t *mut_evaluator,
    const char *ref_expr
) {
    if (!mut_evaluator || !ref_expr) {
        return NULL;
    }
    
    // Check if it's a simple "memory" or "context" expression
    if (strcmp(ref_expr, "memory") == 0) {
        return mut_evaluator->mut_memory;
    } else if (strcmp(ref_expr, "context") == 0) {
        return mut_evaluator->ref_context;
    }
    
    // Check if it's a memory.path or context.path expression
    if (strncmp(ref_expr, "memory.", 7) == 0) {
        const char *key_path = ref_expr + 7;
        return ar__data__get_map_data(mut_evaluator->mut_memory, key_path);
    } else if (strncmp(ref_expr, "context.", 8) == 0) {
        if (!mut_evaluator->ref_context) {
            return NULL;
        }
        const char *key_path = ref_expr + 8;
        return ar__data__get_map_data(mut_evaluator->ref_context, key_path);
    }
    
    // Not a simple memory/context access
    return NULL;
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

/* Helper function to ensure a dynamic buffer has enough capacity */
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

/* Helper function to store result in memory if assignment path is provided */
static bool _store_result_if_assigned(
    instruction_evaluator_t *mut_evaluator,
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
    bool store_success = ar__data__set_map_data(mut_evaluator->mut_memory, key_path, own_result);
    if (!store_success) {
        ar__data__destroy(own_result);
        return false;
    }
    
    return true;
}

/* Helper function to convert data value to string representation */
static const char* _data_to_string(const data_t *ref_data, char *buffer, size_t buffer_size) {
    if (!ref_data || !buffer || buffer_size == 0) {
        return NULL;
    }
    
    switch (ar__data__get_type(ref_data)) {
        case DATA_STRING:
            return ar__data__get_string(ref_data);
        case DATA_INTEGER:
            snprintf(buffer, buffer_size, "%d", ar__data__get_integer(ref_data));
            return buffer;
        case DATA_DOUBLE:
            snprintf(buffer, buffer_size, "%g", ar__data__get_double(ref_data));
            return buffer;
        default:
            return NULL;
    }
}

/* Helper function to parse and evaluate an expression string */
static data_t* _parse_and_evaluate_expression(instruction_evaluator_t *mut_evaluator, const char *ref_expr) {
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
    
    data_t *result = _evaluate_expression_ast(mut_evaluator, ast);
    ar__expression_ast__destroy(ast);
    
    return result;
}

/* Helper function to evaluate an expression AST node using the expression evaluator */
static data_t* _evaluate_expression_ast(instruction_evaluator_t *mut_evaluator, expression_ast_t *ref_ast) {
    if (!ref_ast) {
        return NULL;
    }
    
    expression_ast_type_t type = ar__expression_ast__get_type(ref_ast);
    
    switch (type) {
        case EXPR_AST_LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        case EXPR_AST_MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy
            {
                data_t *ref_value = ar__expression_evaluator__evaluate_memory_access(mut_evaluator->ref_expr_evaluator, ref_ast);
                if (!ref_value) return NULL;
                
                // Create a deep copy of the value
                return _copy_data_value(ref_value);
            }
            
        case EXPR_AST_BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        default:
            return NULL;
    }
}

bool ar__instruction_evaluator__evaluate_assignment(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the assignment instruction evaluator module
    return ar__assignment_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

bool ar__instruction_evaluator__evaluate_send(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the send instruction evaluator module
    return ar__send_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

bool ar__instruction_evaluator__evaluate_if(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the condition instruction evaluator module
    return ar__condition_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}


bool ar__instruction_evaluator__evaluate_parse(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the parse instruction evaluator module
    return ar__parse_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

/* Helper function to process a template placeholder */
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
    const data_t *ref_value = ar__data__get_map_data(ref_values, var_name);
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

bool ar__instruction_evaluator__evaluate_build(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Verify this is a build AST node
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_BUILD) {
        return false;
    }
    
    // Get function arguments
    list_t *own_args = NULL;
    void **items = _extract_function_args(ref_ast, 2, &own_args);
    if (!items) {
        return false;
    }
    
    const char *ref_template_expr = (const char*)items[0];
    const char *ref_values_expr = (const char*)items[1];
    
    if (!ref_template_expr || !ref_values_expr) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Parse and evaluate template expression
    data_t *own_template_data = _parse_and_evaluate_expression(mut_evaluator, ref_template_expr);
    if (!own_template_data || ar__data__get_type(own_template_data) != DATA_STRING) {
        if (own_template_data) ar__data__destroy(own_template_data);
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Parse and evaluate values expression (should be a map)
    data_t *own_values_data = _parse_and_evaluate_expression(mut_evaluator, ref_values_expr);
    if (!own_values_data || ar__data__get_type(own_values_data) != DATA_MAP) {
        if (own_values_data) ar__data__destroy(own_values_data);
        ar__data__destroy(own_template_data);
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Clean up items array and args list
    _cleanup_function_args(items, own_args);
    
    const char *template_str = ar__data__get_string(own_template_data);
    
    // Build the string by replacing placeholders in template
    size_t result_size = strlen(template_str) * 2 + 256;
    char *own_result_str = AR__HEAP__MALLOC(result_size, "Build result");
    if (!own_result_str) {
        ar__data__destroy(own_values_data);
        ar__data__destroy(own_template_data);
        return false;
    }
    
    size_t result_pos = 0;
    const char *template_ptr = template_str;
    
    while (*template_ptr) {
        if (*template_ptr == '{') {
            // Process placeholder
            if (!_process_placeholder(template_ptr, own_values_data, 
                                      &own_result_str, &result_size, &result_pos, &template_ptr)) {
                // If no closing brace found, just copy the '{' character
                char *new_buffer = _ensure_buffer_capacity(own_result_str, &result_size, result_pos + 2);
                if (!new_buffer) {
                    AR__HEAP__FREE(own_result_str);
                    ar__data__destroy(own_values_data);
                    ar__data__destroy(own_template_data);
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
                ar__data__destroy(own_values_data);
                ar__data__destroy(own_template_data);
                return false;
            }
            own_result_str = new_buffer;
            own_result_str[result_pos++] = *template_ptr++;
        }
    }
    
    // Null-terminate the result
    own_result_str[result_pos] = '\0';
    
    // Create result data object
    data_t *own_result = ar__data__create_string(own_result_str);
    AR__HEAP__FREE(own_result_str);
    
    if (!own_result) {
        ar__data__destroy(own_values_data);
        ar__data__destroy(own_template_data);
        return false;
    }
    
    // Clean up
    ar__data__destroy(own_values_data);
    ar__data__destroy(own_template_data);
    
    // Store result if assigned, otherwise just destroy it
    return _store_result_if_assigned(mut_evaluator, ref_ast, own_result);
}

bool ar__instruction_evaluator__evaluate_method(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
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
        mut_evaluator, ref_ast, 3,
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
            _store_result_if_assigned(mut_evaluator, ref_ast, own_result);
        }
    }
    
    return success;
}

bool ar__instruction_evaluator__evaluate_agent(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
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
    
    data_t *own_method_name = _parse_and_evaluate_expression(mut_evaluator, ref_method_expr);
    data_t *own_version = _parse_and_evaluate_expression(mut_evaluator, ref_version_expr);
    
    // For context, first try to get a direct reference to avoid copying
    const data_t *ref_context = _get_memory_or_context_reference(mut_evaluator, ref_context_expr);
    data_t *own_context = NULL;
    
    if (!ref_context) {
        // Not a simple memory/context access, evaluate the expression
        own_context = _parse_and_evaluate_expression(mut_evaluator, ref_context_expr);
        ref_context = own_context; // Use the evaluated result
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
        if (ref_context && ar__data__get_type(ref_context) == DATA_MAP) {
            context_valid = true;
        }
        
        if (context_valid) {
            const char *method_name = ar__data__get_string(own_method_name);
            const char *version = ar__data__get_string(own_version);
            
            // Check if method exists
            method_t *ref_method = ar__methodology__get_method(method_name, version);
            if (ref_method) {
                // Create the agent - context is borrowed, not owned
                agent_id = ar__agency__create_agent(method_name, version, ref_context);
                if (agent_id > 0) {
                    success = true;
                }
            }
        }
    }
    
    // Clean up evaluated arguments
    if (own_method_name) ar__data__destroy(own_method_name);
    if (own_version) ar__data__destroy(own_version);
    if (own_context) ar__data__destroy(own_context); // Only destroy if we created a copy
    
    // Store result if assigned
    if (ar__instruction_ast__has_result_assignment(ref_ast)) {
        data_t *own_result = ar__data__create_integer((int)agent_id);
        if (own_result) {
            _store_result_if_assigned(mut_evaluator, ref_ast, own_result);
        }
    }
    
    return success;
}

bool ar__instruction_evaluator__evaluate_destroy(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Validate AST type
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_DESTROY) {
        return false;
    }
    
    // Get function arguments
    list_t *own_args = ar__instruction_ast__get_function_args(ref_ast);
    if (!own_args) {
        return false;
    }
    
    size_t arg_count = ar__list__count(own_args);
    if (arg_count != 1 && arg_count != 2) {
        ar__list__destroy(own_args);
        return false;
    }
    
    bool success = false;
    bool destroy_result = false;
    
    // Get array of arguments
    void **items = ar__list__items(own_args);
    if (!items) {
        ar__list__destroy(own_args);
        return false;
    }
    
    if (arg_count == 1) {
        // destroy(agent_id)
        // Extract and evaluate agent ID argument
        const char *ref_agent_expr = (const char*)items[0];
        data_t *own_agent_id = _parse_and_evaluate_expression(mut_evaluator, ref_agent_expr);
        
        if (own_agent_id && ar__data__get_type(own_agent_id) == DATA_INTEGER) {
            int64_t agent_id = (int64_t)ar__data__get_integer(own_agent_id);
            destroy_result = ar__agency__destroy_agent(agent_id);
            success = true;
        }
        
        if (own_agent_id) {
            ar__data__destroy(own_agent_id);
        }
    } else {
        // destroy(method_name, method_version)
        // Extract arguments
        const char *ref_name_expr = (const char*)items[0];
        const char *ref_version_expr = (const char*)items[1];
        
        // Evaluate method name
        data_t *own_name = _parse_and_evaluate_expression(mut_evaluator, ref_name_expr);
        data_t *own_version = _parse_and_evaluate_expression(mut_evaluator, ref_version_expr);
        
        if (own_name && own_version &&
            ar__data__get_type(own_name) == DATA_STRING &&
            ar__data__get_type(own_version) == DATA_STRING) {
            
            const char *method_name = ar__data__get_string(own_name);
            const char *method_version = ar__data__get_string(own_version);
            
            // Get the method to check if agents are using it
            method_t *ref_method = ar__methodology__get_method(method_name, method_version);
            if (ref_method) {
                // Count agents using this method
                int agent_count = ar__agency__count_agents_using_method(ref_method);
                
                if (agent_count > 0) {
                    // Send __sleep__ messages to all agents using this method
                    int64_t agent_id = ar__agency__get_first_agent();
                    while (agent_id > 0) {
                        const method_t *agent_method = ar__agency__get_agent_method(agent_id);
                        if (agent_method == ref_method) {
                            data_t *sleep_msg = ar__data__create_string("__sleep__");
                            if (sleep_msg) {
                                bool sent = ar__agency__send_to_agent(agent_id, sleep_msg);
                                if (!sent) {
                                    // If send fails, we need to destroy the message ourselves
                                    ar__data__destroy(sleep_msg);
                                }
                            }
                        }
                        agent_id = ar__agency__get_next_agent(agent_id);
                    }
                    
                    // Now destroy each agent
                    agent_id = ar__agency__get_first_agent();
                    while (agent_id > 0) {
                        int64_t next_id = ar__agency__get_next_agent(agent_id);
                        const method_t *agent_method = ar__agency__get_agent_method(agent_id);
                        if (agent_method == ref_method) {
                            ar__agency__destroy_agent(agent_id);
                        }
                        agent_id = next_id;
                    }
                }
                
                // Now unregister the method
                destroy_result = ar__methodology__unregister_method(method_name, method_version);
                success = true;
            } else {
                // Method doesn't exist
                success = true;
                destroy_result = false;
            }
        }
        
        if (own_name) ar__data__destroy(own_name);
        if (own_version) ar__data__destroy(own_version);
    }
    
    // Free the items array (but not the items themselves)
    AR__HEAP__FREE(items);
    
    // Destroy the args list
    ar__list__destroy(own_args);
    
    // Store result if assigned
    if (success && ar__instruction_ast__has_result_assignment(ref_ast)) {
        data_t *own_result = ar__data__create_integer(destroy_result ? 1 : 0);
        if (own_result) {
            _store_result_if_assigned(mut_evaluator, ref_ast, own_result);
        }
    }
    
    return success;
}