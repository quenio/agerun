/**
 * @file agerun_build_instruction_evaluator.c
 * @brief Implementation of the build instruction evaluator
 */

#include "agerun_build_instruction_evaluator.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "agerun_instruction_ast.h"
#include "agerun_expression_ast.h"
#include "agerun_expression_evaluator.h"
#include "agerun_expression_parser.h"
#include "agerun_data.h"
#include "agerun_list.h"
#include "agerun_heap.h"
#include "agerun_io.h"

/* Memory prefix for path extraction */
static const char MEMORY_PREFIX[] = "memory.";
static const size_t MEMORY_PREFIX_LEN = sizeof(MEMORY_PREFIX) - 1;

/* Opaque struct definition */
struct ar_build_instruction_evaluator_s {
    expression_evaluator_t *ref_expr_evaluator;
    data_t *mut_memory;
};

/**
 * Creates a new build instruction evaluator
 */
ar_build_instruction_evaluator_t* ar__build_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
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
void ar__build_instruction_evaluator__destroy(
    ar_build_instruction_evaluator_t *own_evaluator
) {
    if (!own_evaluator) {
        return;
    }
    
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Extracts function arguments from AST node
 * 
 * @param ref_ast The AST node (borrowed reference)
 * @param expected_count Expected number of arguments
 * @param out_args_list Output parameter for the arguments list (owned by caller)
 * @return Array of argument items or NULL on error
 * @note Ownership: Returns borrowed items array, sets owned list in out_args_list
 */
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

/**
 * Cleans up function arguments
 * 
 * @param items The items array to free (can be NULL)
 * @param own_args The arguments list to destroy (can be NULL)
 */
static void _cleanup_function_args(void **items, list_t *own_args) {
    if (items) {
        AR__HEAP__FREE(items);
    }
    if (own_args) {
        ar__list__destroy(own_args);
    }
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

/**
 * Evaluates an expression AST node
 * 
 * @param mut_expr_evaluator Expression evaluator (mutable reference)
 * @param ref_ast The expression AST to evaluate (borrowed reference)
 * @return The evaluated data or NULL on error
 * @note Ownership: Returns owned value
 */
static data_t* _evaluate_expression_ast(expression_evaluator_t *mut_expr_evaluator, expression_ast_t *ref_ast) {
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
            return ar__expression_evaluator__evaluate_memory_access(mut_expr_evaluator, ref_ast);
        case EXPR_AST_BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_expr_evaluator, ref_ast);
        default:
            return NULL;
    }
}

/**
 * Checks if an expression is a simple memory reference
 * 
 * @param mut_memory The memory map (mutable reference)
 * @param ref_expr The expression string (borrowed reference)
 * @return The referenced data or NULL if not a simple reference
 * @note Ownership: Returns borrowed reference
 */
static const data_t* _get_memory_reference(
    data_t *mut_memory,
    const char *ref_expr
) {
    if (!ref_expr || !mut_memory) {
        return NULL;
    }
    
    // Check if it's a simple "memory" expression
    if (strcmp(ref_expr, "memory") == 0) {
        return mut_memory;
    }
    
    // Check if it's a memory.path expression
    if (strncmp(ref_expr, MEMORY_PREFIX, MEMORY_PREFIX_LEN) == 0) {
        const char *key_path = ref_expr + MEMORY_PREFIX_LEN;
        return ar__data__get_map_data(mut_memory, key_path);
    }
    
    // Not a simple memory access
    return NULL;
}

/**
 * Parses and evaluates an expression string
 * 
 * @param mut_expr_evaluator Expression evaluator (mutable reference)
 * @param ref_expr The expression string to parse and evaluate
 * @return The evaluated data or NULL on error
 * @note Ownership: Returns owned value
 */
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

/**
 * Evaluates a build instruction using the stored dependencies
 */
bool ar__build_instruction_evaluator__evaluate(
    ar_build_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    expression_evaluator_t *mut_expr_evaluator = mut_evaluator->ref_expr_evaluator;
    data_t *mut_memory = mut_evaluator->mut_memory;
    
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
    data_t *own_template_data = _parse_and_evaluate_expression(mut_expr_evaluator, ref_template_expr);
    if (!own_template_data || ar__data__get_type(own_template_data) != DATA_STRING) {
        if (own_template_data) ar__data__destroy(own_template_data);
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Check if values expression is a simple memory reference
    const data_t *ref_values_data = _get_memory_reference(mut_memory, ref_values_expr);
    data_t *own_values_data = NULL;
    
    if (!ref_values_data) {
        // Not a simple memory access, evaluate the expression
        own_values_data = _parse_and_evaluate_expression(mut_expr_evaluator, ref_values_expr);
        ref_values_data = own_values_data; // Use the evaluated result
    }
    
    // Validate it's a map
    if (!ref_values_data || ar__data__get_type(ref_values_data) != DATA_MAP) {
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
        if (own_values_data) ar__data__destroy(own_values_data);
        ar__data__destroy(own_template_data);
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
                    if (own_values_data) ar__data__destroy(own_values_data);
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
                if (own_values_data) ar__data__destroy(own_values_data);
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
        if (own_values_data) ar__data__destroy(own_values_data);
        ar__data__destroy(own_template_data);
        return false;
    }
    
    // Clean up
    if (own_values_data) ar__data__destroy(own_values_data);
    ar__data__destroy(own_template_data);
    
    // Store result if assigned, otherwise just destroy it
    return _store_result_if_assigned(mut_memory, ref_ast, own_result);
}

/**
 * Evaluates a build instruction (legacy interface)
 */
bool ar_build_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
) {
    ar_build_instruction_evaluator_t *evaluator = ar__build_instruction_evaluator__create(
        mut_expr_evaluator, mut_memory
    );
    if (!evaluator) {
        return false;
    }
    
    bool result = ar__build_instruction_evaluator__evaluate(evaluator, ref_ast);
    ar__build_instruction_evaluator__destroy(evaluator);
    
    return result;
}

/**
 * Evaluates a build instruction (old name for compatibility)
 */
bool ar_build_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
) {
    return ar_build_instruction_evaluator__evaluate_legacy(
        mut_expr_evaluator,
        mut_memory,
        ref_ast
    );
}