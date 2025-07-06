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
#include "ar_log.h"
#include "ar_memory_accessor.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Struct definition for parse instruction evaluator */
struct ar_parse_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    ar_data_t *mut_memory;                          /* Memory map (mutable reference) */
};


/* Forward declarations of helper functions */
static bool _store_result_if_assigned(ar_data_t *mut_memory, const ar_instruction_ast_t *ref_ast, ar_data_t *own_result);
static ar_data_t* _parse_value_string(const char *value_str);
static void _log_error(ar_parse_instruction_evaluator_t *mut_evaluator, const char *message);

/* Helper function to log error message */
static void _log_error(ar_parse_instruction_evaluator_t *mut_evaluator, const char *message) {
    if (message && mut_evaluator->ref_log) {
        ar_log__error(mut_evaluator->ref_log, message);
    }
}




/* Helper function to store result in memory if assignment path is provided */
static bool _store_result_if_assigned(
    ar_data_t *mut_memory,
    const ar_instruction_ast_t *ref_ast,
    ar_data_t *own_result
) {
    const char *ref_result_path = ar_instruction_ast__get_function_result_path(ref_ast);
    if (!ref_result_path) {
        // No assignment, just destroy the result
        ar_data__destroy(own_result);
        return true;
    }
    
    // Get memory key path
    const char *key_path = ar_memory_accessor__get_key(ref_result_path);
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


/* Helper function to parse a value string and determine its type */
static ar_data_t* _parse_value_string(const char *value_str) {
    if (!value_str || *value_str == '\0') {
        return ar_data__create_string("");
    }
    
    // Try to parse as integer first
    char *endptr;
    long int_val = strtol(value_str, &endptr, 10);
    if (*endptr == '\0' && value_str[0] != '\0') {
        return ar_data__create_integer((int)int_val);
    }
    
    // Try to parse as double
    double double_val = strtod(value_str, &endptr);
    if (*endptr == '\0' && value_str[0] != '\0' && strchr(value_str, '.')) {
        return ar_data__create_double(double_val);
    }
    
    // Otherwise treat as string
    return ar_data__create_string(value_str);
}

/**
 * Creates a new parse instruction evaluator
 */
ar_parse_instruction_evaluator_t* ar_parse_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory
) {
    if (!ref_log || !ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_parse_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_parse_instruction_evaluator_t),
        "parse_instruction_evaluator"
    );
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_log = ref_log;
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
    
    // Clear any previous error
    _log_error(mut_evaluator, NULL);
    
    // Verify this is a parse AST node
    if (ar_instruction_ast__get_type(ref_ast) != AR_INST__PARSE) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(ref_ast);
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
    const ar_expression_ast_t *ref_input_ast = (const ar_expression_ast_t*)items[1];
    
    if (!ref_template_ast || !ref_input_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate template expression AST
    ar_data_t *template_result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_template_ast);
    if (!template_result || ar_data__get_type(template_result) != DATA_STRING) {
        if (template_result && ar_data__hold_ownership(template_result, mut_evaluator)) {
            ar_data__transfer_ownership(template_result, mut_evaluator);
            ar_data__destroy(template_result);
        }
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Get ownership of template data
    ar_data_t *own_template_data;
    if (ar_data__hold_ownership(template_result, mut_evaluator)) {
        // We can claim ownership - it's an unowned value
        ar_data__transfer_ownership(template_result, mut_evaluator);
        own_template_data = template_result;
    } else {
        // It's owned by someone else - we need to make a copy
        own_template_data = ar_data__shallow_copy(template_result);
        if (!own_template_data) {
            _log_error(mut_evaluator, "Cannot parse with nested containers in template (no deep copy support)");
            AR__HEAP__FREE(items);
            return false;
        }
    }
    
    // Evaluate input expression AST
    ar_data_t *input_result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_input_ast);
    if (!input_result || ar_data__get_type(input_result) != DATA_STRING) {
        if (input_result && ar_data__hold_ownership(input_result, mut_evaluator)) {
            ar_data__transfer_ownership(input_result, mut_evaluator);
            ar_data__destroy(input_result);
        }
        ar_data__destroy(own_template_data);
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Get ownership of input data
    ar_data_t *own_input_data;
    if (ar_data__hold_ownership(input_result, mut_evaluator)) {
        // We can claim ownership - it's an unowned value
        ar_data__transfer_ownership(input_result, mut_evaluator);
        own_input_data = input_result;
    } else {
        // It's owned by someone else - we need to make a copy
        own_input_data = ar_data__shallow_copy(input_result);
        if (!own_input_data) {
            _log_error(mut_evaluator, "Cannot parse with nested containers in input (no deep copy support)");
            ar_data__destroy(own_template_data);
            AR__HEAP__FREE(items);
            return false;
        }
    }
    
    const char *template_str = ar_data__get_string(own_template_data);
    const char *input_str = ar_data__get_string(own_input_data);
    
    // Clean up the items array as we're done with it
    AR__HEAP__FREE(items);
    
    // Create result map
    ar_data_t *own_result = ar_data__create_map();
    if (!own_result) {
        ar_data__destroy(own_input_data);
        ar_data__destroy(own_template_data);
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
                ar_data__destroy(own_result);
                own_result = ar_data__create_map(); // Return empty map
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
            ar_data__destroy(own_result);
            ar_data__destroy(own_input_data);
            ar_data__destroy(own_template_data);
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
                ar_data__destroy(own_result);
                own_result = ar_data__create_map(); // Return empty map
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
                ar_data__destroy(own_result);
                ar_data__destroy(own_input_data);
                ar_data__destroy(own_template_data);
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
                ar_data__destroy(own_result);
                own_result = ar_data__create_map(); // Return empty map
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
            ar_data__destroy(own_result);
            ar_data__destroy(own_input_data);
            ar_data__destroy(own_template_data);
            return false;
        }
        memcpy(value_str, input_ptr, value_len);
        value_str[value_len] = '\0';
        
        // Parse the value and store in result map
        ar_data_t *own_value = _parse_value_string(value_str);
        AR__HEAP__FREE(value_str);
        
        if (own_value) {
            ar_data__set_map_data(own_result, var_name, own_value);
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
    ar_data__destroy(own_input_data);
    ar_data__destroy(own_template_data);
    
    // Store result if assigned, otherwise just destroy it
    return _store_result_if_assigned(mut_evaluator->mut_memory, ref_ast, own_result);
}

