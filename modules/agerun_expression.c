#include "agerun_system.h"
#include "agerun_expression.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_list.h"
#include "agerun_map.h"
#include "agerun_heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Full definition of the expression context structure.
 * This is only visible in the implementation file.
 */
struct expression_context_s {
    data_t *mut_memory;         /* The agent's memory (mutable reference) */
    const data_t *ref_context;  /* The agent's context (borrowed reference) */
    const data_t *ref_message;  /* The message being processed (borrowed reference) */
    const char *ref_expr;       /* The expression to evaluate (borrowed reference) */
    int offset;                 /* Current position in the expression */
    list_t *own_results;        /* List of results owned by this context (to be freed when context is destroyed) */
};

/**
 * Creates a new expression evaluation context.
 *
 * @param mut_memory The agent's memory data (mutable reference, can be NULL if not needed)
 * @param ref_context The agent's context data (borrowed reference, can be NULL if not needed)
 * @param ref_message The message being processed (borrowed reference, can be NULL if not needed)
 * @param ref_expr The expression string to evaluate (borrowed reference)
 * @return Newly created expression context (owned by caller), or NULL on failure
 */
expression_context_t* ar_expression_create_context(data_t *mut_memory, const data_t *ref_context, const data_t *ref_message, const char *ref_expr) {
    if (!ref_expr) {
        return NULL;
    }
    
    expression_context_t *own_ctx = AR_HEAP_MALLOC(sizeof(expression_context_t), "Expression context");
    if (!own_ctx) {
        return NULL;
    }
    
    own_ctx->mut_memory = mut_memory;
    own_ctx->ref_context = ref_context;
    own_ctx->ref_message = ref_message;
    own_ctx->ref_expr = ref_expr;
    own_ctx->offset = 0;
    
    // Initialize list to track expression results
    own_ctx->own_results = ar_list_create();
    if (!own_ctx->own_results) {
        AR_HEAP_FREE(own_ctx);
        return NULL;
    }
    
    return own_ctx; // Ownership transferred to caller
}

/**
 * Destroys an expression context.
 * 
 * @param own_ctx The expression context to destroy (ownership transferred to function)
 */
void ar_expression_destroy_context(expression_context_t *own_ctx) {
    if (!own_ctx) {
        return;
    }
    
    // Free all results tracked by this context
    if (own_ctx->own_results) {
        // Get all items in the list
        void **own_items = ar_list_items(own_ctx->own_results);
        size_t count = ar_list_count(own_ctx->own_results);
        
        if (own_items && count > 0) {
            // Free each result that isn't a direct reference
            for (size_t i = 0; i < count; i++) {
                data_t *ref_result = (data_t *)own_items[i];
                if (ref_result) {
                    // Skip memory, context, and message - these are owned by caller
                    if (ref_result != own_ctx->mut_memory && 
                        ref_result != own_ctx->ref_context && 
                        ref_result != own_ctx->ref_message) {
                        ar_data_destroy(ref_result);
                    }
                }
            }
            
            // Free the items array
            AR_HEAP_FREE(own_items);
            own_items = NULL; // Mark as transferred
        }
        
        // Free the list itself
        ar_list_destroy(own_ctx->own_results);
        own_ctx->own_results = NULL; // Mark as transferred
    }
    
    // Free the context structure
    AR_HEAP_FREE(own_ctx);
    // No need to set own_ctx to NULL as it's a parameter and not accessible outside
}

/**
 * Gets the current parsing offset in the expression string.
 *
 * @param ref_ctx The expression context (borrowed reference)
 * @return Current offset in the expression string
 */
int ar_expression_offset(const expression_context_t *ref_ctx) {
    if (!ref_ctx) {
        return 0;
    }
    return ref_ctx->offset;
}

/*
 * Recursive Descent Parser for AgeRun expressions
 * Based on the BNF grammar:
 *
 * <expression> ::= <string-literal>
 *               | <number-literal>
 *               | <memory-access>
 *               | <arithmetic-expression>
 *               | <comparison-expression>
 *
 * <string-literal> ::= '"' <characters> '"'
 *
 * <number-literal> ::= <integer>
 *                   | <double>
 *
 * <integer> ::= ['-'] <digit> {<digit>}
 * <double>  ::= <integer> '.' <digit> {<digit>}
 *
 * <memory-access> ::= 'message' {'.' <identifier>}
 *                  | 'memory' {'.' <identifier>}
 *                  | 'context' {'.' <identifier>}
 *
 * <arithmetic-expression> ::= <expression> <arithmetic-operator> <expression>
 * <arithmetic-operator> ::= '+' | '-' | '*' | '/'
 *
 * <comparison-expression> ::= <expression> <comparison-operator> <expression>
 * <comparison-operator> ::= '=' | '<>' | '<' | '<=' | '>' | '>='
 */

// Forward declarations for recursive descent functions
static const data_t* parse_expression(expression_context_t *ctx);
static const data_t* parse_primary(expression_context_t *ctx);
static const data_t* parse_string_literal(expression_context_t *ctx);
static const data_t* parse_number_literal(expression_context_t *ctx);
static const data_t* parse_memory_access(expression_context_t *ctx);
static void skip_whitespace(expression_context_t *ctx);
static bool is_comparison_operator(expression_context_t *ctx);
static bool match(expression_context_t *ctx, const char *to_match);
static bool is_identifier_start(char c);
static bool is_identifier_part(char c);
static char* parse_identifier(expression_context_t *ctx);
static bool is_digit(char c);

// Skip whitespace characters in the expression
static void skip_whitespace(expression_context_t *mut_ctx) {
    while (mut_ctx->ref_expr[mut_ctx->offset] && ar_string_isspace(mut_ctx->ref_expr[mut_ctx->offset])) {
        mut_ctx->offset++;
    }
}

// Check if a character is a digit
static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

// Check if a character can start an identifier
static bool is_identifier_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Check if a character can be part of an identifier
static bool is_identifier_part(char c) {
    return is_identifier_start(c) || is_digit(c) || c == '_';
}

// Parse an identifier from the expression
static char* parse_identifier(expression_context_t *mut_ctx) {
    int start = mut_ctx->offset;
    
    // First character must be a letter
    if (!is_identifier_start(mut_ctx->ref_expr[mut_ctx->offset])) {
        return NULL;
    }
    
    mut_ctx->offset++;
    
    // Rest of identifier can include letters, digits, and underscore
    while (mut_ctx->ref_expr[mut_ctx->offset] && is_identifier_part(mut_ctx->ref_expr[mut_ctx->offset])) {
        mut_ctx->offset++;
    }
    
    int length = mut_ctx->offset - start;
    char *own_identifier = AR_HEAP_MALLOC((size_t)length + 1, "Expression identifier");
    if (!own_identifier) {
        return NULL;
    }
    
    // Use memcpy with explicit bounds checking
    if (length > 0) {
        memcpy(own_identifier, mut_ctx->ref_expr + start, (size_t)length);
    }
    own_identifier[length] = '\0';  // Always ensure null-termination
    
    return own_identifier; // Ownership transferred to caller
}


// Check if the string at the current offset matches the expected string
// If it does, advance offset past the matched string and return true
static bool match(expression_context_t *mut_ctx, const char *ref_to_match) {
    size_t len = strlen(ref_to_match);
    if (strncmp(mut_ctx->ref_expr + mut_ctx->offset, ref_to_match, len) == 0) {
        // Make sure it's not part of a longer identifier
        if (ref_to_match[len-1] == '.' || !mut_ctx->ref_expr[mut_ctx->offset + (int)len] || 
            !is_identifier_part(mut_ctx->ref_expr[mut_ctx->offset + (int)len])) {
            mut_ctx->offset += (int)len;
            return true;
        }
    }
    return false;
}



// Check if the next sequence of characters is a comparison operator
static bool is_comparison_operator(expression_context_t *ref_ctx) {
    if (ref_ctx->ref_expr[ref_ctx->offset] == '=') {
        return true;
    }
    if (ref_ctx->ref_expr[ref_ctx->offset] == '<') {
        if (ref_ctx->ref_expr[ref_ctx->offset + 1] == '>' || ref_ctx->ref_expr[ref_ctx->offset + 1] == '=') {
            return true;
        }
        return true;
    }
    if (ref_ctx->ref_expr[ref_ctx->offset] == '>') {
        if (ref_ctx->ref_expr[ref_ctx->offset + 1] == '=') {
            return true;
        }
        return true;
    }
    return false;
}

// Parse a string literal from the expression
static const data_t* parse_string_literal(expression_context_t *mut_ctx) {
    if (mut_ctx->ref_expr[mut_ctx->offset] != '"') {
        return NULL;
    }
    
    mut_ctx->offset++; // Skip opening quote
    
    // Find the closing quote and count the length
    int start = mut_ctx->offset;
    int len = 0;
    
    while (mut_ctx->ref_expr[mut_ctx->offset] && mut_ctx->ref_expr[mut_ctx->offset] != '"') {
        mut_ctx->offset++;
        len++;
    }
    
    if (mut_ctx->ref_expr[mut_ctx->offset] != '"') {
        // Unterminated string literal
        return NULL;
    }
    
    // Allocate and create the string
    char *own_temp_str = AR_HEAP_MALLOC((size_t)len + 1, "Temporary string buffer");
    if (!own_temp_str) {
        return NULL;
    }
    
    strncpy(own_temp_str, mut_ctx->ref_expr + start, (size_t)len);
    own_temp_str[len] = '\0';
    
    mut_ctx->offset++; // Skip closing quote
    
    data_t *own_result = ar_data_create_string(own_temp_str);
    AR_HEAP_FREE(own_temp_str);
    own_temp_str = NULL; // Mark as transferred
    
    // Track this result since we created it
    if (own_result) {
        ar_list_add_last(mut_ctx->own_results, own_result);
    }
    
    return own_result; // Ownership retained by context
}

// Parse a number literal (integer or double) from the expression
static const data_t* parse_number_literal(expression_context_t *mut_ctx) {
    bool is_negative = false;
    if (mut_ctx->ref_expr[mut_ctx->offset] == '-') {
        is_negative = true;
        mut_ctx->offset++;
    }
    
    if (!is_digit(mut_ctx->ref_expr[mut_ctx->offset])) {
        return NULL;
    }
    
    // Parse integer part
    int value = 0;
    while (mut_ctx->ref_expr[mut_ctx->offset] && is_digit(mut_ctx->ref_expr[mut_ctx->offset])) {
        value = value * 10 + (mut_ctx->ref_expr[mut_ctx->offset] - '0');
        mut_ctx->offset++;
    }
    
    // Check for decimal point for double
    if (mut_ctx->ref_expr[mut_ctx->offset] == '.') {
        mut_ctx->offset++; // Skip decimal point
        
        if (!is_digit(mut_ctx->ref_expr[mut_ctx->offset])) {
            // Malformed double, must have at least one digit after decimal
            return NULL;
        }
        
        double double_value = (double)value;
        double decimal_place = 0.1;
        
        // Parse decimal part
        while (mut_ctx->ref_expr[mut_ctx->offset] && is_digit(mut_ctx->ref_expr[mut_ctx->offset])) {
            double_value += (mut_ctx->ref_expr[mut_ctx->offset] - '0') * decimal_place;
            decimal_place *= 0.1;
            mut_ctx->offset++;
        }
        
        if (is_negative) {
            double_value = -double_value;
        }
        
        data_t *own_result = ar_data_create_double(double_value);
        
        // Track this result since we created it
        if (own_result) {
            ar_list_add_last(mut_ctx->own_results, own_result);
        }
        
        return own_result; // Ownership retained by context
    }
    
    // It's an integer
    if (is_negative) {
        value = -value;
    }
    
    data_t *own_result = ar_data_create_integer(value);
    
    // Track this result since we created it
    if (own_result) {
        ar_list_add_last(mut_ctx->own_results, own_result);
    }
    
    return own_result; // Ownership retained by context
}

// Parse a memory access (message, memory, context) expression
static const data_t* parse_memory_access(expression_context_t *mut_ctx) {
    enum {
        ACCESS_TYPE_MESSAGE,
        ACCESS_TYPE_MEMORY,
        ACCESS_TYPE_CONTEXT
    } access_type;
    
    // Determine which type of access we're dealing with
    if (match(mut_ctx, "message")) {
        access_type = ACCESS_TYPE_MESSAGE;
    } else if (match(mut_ctx, "memory")) {
        access_type = ACCESS_TYPE_MEMORY;
    } else if (match(mut_ctx, "context")) {
        access_type = ACCESS_TYPE_CONTEXT;
    } else {
        return NULL;
    }
    
    // Handle root access (no nested fields)
    if (mut_ctx->ref_expr[mut_ctx->offset] != '.') {
        switch (access_type) {
            case ACCESS_TYPE_MESSAGE:
                // Return the message directly as a const pointer
                return mut_ctx->ref_message;
            case ACCESS_TYPE_MEMORY:
                // Return memory as const even though it's mutable internally
                return mut_ctx->mut_memory;
            case ACCESS_TYPE_CONTEXT:
                // Return the context directly as a const pointer
                return mut_ctx->ref_context;
        }
    }
    
    // Handle nested field access with dot notation
    char path[256] = "";
    int path_len = 0;
    
    while (mut_ctx->ref_expr[mut_ctx->offset] == '.') {
        mut_ctx->offset++; // Skip the dot
        
        char *own_id = parse_identifier(mut_ctx);
        if (!own_id) {
            return NULL; // Invalid identifier in path is a syntax error
        }
        
        // Append to path
        size_t id_len = strlen(own_id);
        if ((size_t)path_len + id_len + 1U < sizeof(path)) {
            if (path_len > 0) {
                path[path_len++] = '.';
            }
            // Use strncpy for secure copy with explicit null termination
            size_t remaining_space = sizeof(path) - (size_t)path_len;
            strncpy(path + path_len, own_id, remaining_space - 1);
            
            // Calculate copy length safely (avoiding signedness conversion warnings)
            size_t copy_len = (id_len < remaining_space - 1) ? id_len : remaining_space - 1;
            size_t index = (size_t)path_len + copy_len;
            path[index] = '\0';
            path_len += (int)copy_len;
        }
        
        AR_HEAP_FREE(own_id);
        own_id = NULL; // Mark as transferred
    }
    
    // Now we have the full path, get the data
    const data_t *ref_source = NULL;
    switch (access_type) {
        case ACCESS_TYPE_MESSAGE:
            ref_source = mut_ctx->ref_message;
            break;
        case ACCESS_TYPE_MEMORY:
            ref_source = mut_ctx->mut_memory;
            break;
        case ACCESS_TYPE_CONTEXT:
            ref_source = mut_ctx->ref_context;
            break;
    }
    
    if (!ref_source) {
        // Return NULL for non-existent source
        return NULL;
    }
    
    // Look up the data by path
    data_t *ref_value = NULL;
    data_type_t source_type = ar_data_get_type(ref_source);
    
    if (source_type == DATA_MAP) {
        // For map type, use the map access function
        ref_value = ar_data_get_map_data(ref_source, path);
        if (ref_value) {
            // Return the value directly, not a copy
            // The caller is responsible for not destroying this reference
            return ref_value; // Borrowed reference
        }
    }
    
    // Return NULL for missing or invalid paths
    return NULL;
}


// Parse a primary expression (literal or memory access)
static const data_t* parse_primary(expression_context_t *mut_ctx) {
    skip_whitespace(mut_ctx);
    
    // Check for string literal
    if (mut_ctx->ref_expr[mut_ctx->offset] == '"') {
        return parse_string_literal(mut_ctx);
    }
    
    // Check for number literal (including negative numbers)
    if (is_digit(mut_ctx->ref_expr[mut_ctx->offset]) || 
        (mut_ctx->ref_expr[mut_ctx->offset] == '-' && 
         is_digit(mut_ctx->ref_expr[mut_ctx->offset + 1]))) {
        return parse_number_literal(mut_ctx);
    }
    
    // Check for memory access (message, memory, context)
    if (strncmp(mut_ctx->ref_expr + mut_ctx->offset, "message", 7) == 0 ||
        strncmp(mut_ctx->ref_expr + mut_ctx->offset, "memory", 6) == 0 ||
        strncmp(mut_ctx->ref_expr + mut_ctx->offset, "context", 7) == 0) {
        return parse_memory_access(mut_ctx);
    }
    
    // Check for function call - which is a syntax error in expressions
    if (is_identifier_start(mut_ctx->ref_expr[mut_ctx->offset])) {
        // Save the position at the start of the function name
        int func_name_start = mut_ctx->offset;
        
        // Skip over function name
        while (mut_ctx->ref_expr[mut_ctx->offset] && 
               is_identifier_part(mut_ctx->ref_expr[mut_ctx->offset])) {
            mut_ctx->offset++;
        }
        
        skip_whitespace(mut_ctx);
        
        // If we find an opening parenthesis, it's a function call - syntax error
        if (mut_ctx->ref_expr[mut_ctx->offset] == '(') {
            // Reset offset to the start of the function name
            mut_ctx->offset = func_name_start;
            // Return NULL to indicate a syntax error
            return NULL;
        }
        
        // Not a function call, reset offset and continue
        mut_ctx->offset = func_name_start;
    }
    
    // If we get here, it's not a valid primary expression
    return NULL;
}

// Forward declarations for recursive descent functions
static const data_t* parse_multiplicative(expression_context_t *ctx);
static const data_t* parse_additive(expression_context_t *ctx);
static const data_t* parse_comparison(expression_context_t *ctx);

// Parse a multiplicative expression (higher precedence: *, /)
static const data_t* parse_multiplicative(expression_context_t *ctx) {
    // Parse the left operand
    const data_t *left = parse_primary(ctx);
    if (!left) {
        return NULL;
    }
    
    skip_whitespace(ctx);
    
    // Check if there's a multiplicative operator (*, /)
    while (ctx->ref_expr[ctx->offset] == '*' || ctx->ref_expr[ctx->offset] == '/') {
        // Get the operator
        char op = ctx->ref_expr[ctx->offset];
        ctx->offset++;
        
        skip_whitespace(ctx);
        
        // Parse the right operand (which is a primary)
        const data_t *right = parse_primary(ctx);
        if (!right) {
            // Don't destroy left - it's a borrowed reference
            return NULL;
        }
        
        // Perform the operation
        data_type_t left_type = ar_data_get_type(left);
        data_type_t right_type = ar_data_get_type(right);
        data_t *result = NULL;
        
        // Both operands are integers
        if (left_type == DATA_INTEGER && right_type == DATA_INTEGER) {
            int left_val = ar_data_get_integer(left);
            int right_val = ar_data_get_integer(right);
            int result_val = 0;
            
            switch (op) {
                case '*': result_val = left_val * right_val; break;
                case '/': 
                    if (right_val != 0) {
                        result_val = left_val / right_val;
                    }
                    break;
            }
            
            result = ar_data_create_integer(result_val);
        }
        // At least one operand is a double, result is a double
        else if ((left_type == DATA_INTEGER || left_type == DATA_DOUBLE) &&
                 (right_type == DATA_INTEGER || right_type == DATA_DOUBLE)) {
            
            double left_val = (left_type == DATA_INTEGER) ? 
                (double)ar_data_get_integer(left) : ar_data_get_double(left);
                
            double right_val = (right_type == DATA_INTEGER) ? 
                (double)ar_data_get_integer(right) : ar_data_get_double(right);
            
            double result_val = 0.0;
            
            switch (op) {
                case '*': result_val = left_val * right_val; break;
                case '/': 
                    if (right_val != 0.0) {
                        result_val = left_val / right_val;
                    }
                    break;
            }
            
            result = ar_data_create_double(result_val);
        }
        // Unsupported operation
        else {
            result = ar_data_create_integer(0);
        }
        
        // Default result if operation failed
        if (!result) {
            result = ar_data_create_integer(0);
        }
        
        // Track this result since we created it
        if (result) {
            ar_list_add_last(ctx->own_results, result);
        }
        
        // The result becomes the new left operand for the next iteration
        left = result;
        
        skip_whitespace(ctx);
    }
    
    return left;
}

// Parse an additive expression (medium precedence: +, -)
static const data_t* parse_additive(expression_context_t *ctx) {
    // Parse the left operand (which is a multiplicative expression)
    const data_t *left = parse_multiplicative(ctx);
    if (!left) {
        return NULL;
    }
    
    skip_whitespace(ctx);
    
    // Check if there's an additive operator (+, -)
    while (ctx->ref_expr[ctx->offset] == '+' || ctx->ref_expr[ctx->offset] == '-') {
        // Get the operator
        char op = ctx->ref_expr[ctx->offset];
        ctx->offset++;
        
        skip_whitespace(ctx);
        
        // Parse the right operand (which is a multiplicative expression)
        const data_t *right = parse_multiplicative(ctx);
        if (!right) {
            // Don't destroy left - it's a borrowed reference
            return NULL;
        }
        
        // Perform the operation
        data_type_t left_type = ar_data_get_type(left);
        data_type_t right_type = ar_data_get_type(right);
        data_t *result = NULL;
        
        // String concatenation with +
        if (op == '+' && (left_type == DATA_STRING || right_type == DATA_STRING)) {
            char left_str[512] = {0};
            char right_str[512] = {0};
            
            // Convert left operand to string
            if (left_type == DATA_STRING) {
                const char *str = ar_data_get_string(left);
                if (str) {
                    // Safe string copy with bounds check and explicit null termination
                    size_t copy_len = strlen(str);
                    if (copy_len >= sizeof(left_str)) {
                        copy_len = sizeof(left_str) - 1;
                    }
                    memcpy(left_str, str, copy_len);
                    left_str[copy_len] = '\0';  // Ensure null-termination
                }
            } else if (left_type == DATA_INTEGER) {
                snprintf(left_str, sizeof(left_str), "%d", ar_data_get_integer(left));
            } else if (left_type == DATA_DOUBLE) {
                snprintf(left_str, sizeof(left_str), "%.2f", ar_data_get_double(left));
            }
            
            // Convert right operand to string
            if (right_type == DATA_STRING) {
                const char *str = ar_data_get_string(right);
                if (str) {
                    // Safe string copy with bounds check and explicit null termination
                    size_t copy_len = strlen(str);
                    if (copy_len >= sizeof(right_str)) {
                        copy_len = sizeof(right_str) - 1;
                    }
                    memcpy(right_str, str, copy_len);
                    right_str[copy_len] = '\0';  // Ensure null-termination
                }
            } else if (right_type == DATA_INTEGER) {
                snprintf(right_str, sizeof(right_str), "%d", ar_data_get_integer(right));
            } else if (right_type == DATA_DOUBLE) {
                snprintf(right_str, sizeof(right_str), "%.2f", ar_data_get_double(right));
            }
            
            // Concatenate the strings
            char result_str[1024] = {0};
            snprintf(result_str, sizeof(result_str), "%s%s", left_str, right_str);
            
            result = ar_data_create_string(result_str);
        }
        // Both operands are integers
        else if (left_type == DATA_INTEGER && right_type == DATA_INTEGER) {
            int left_val = ar_data_get_integer(left);
            int right_val = ar_data_get_integer(right);
            int result_val = 0;
            
            switch (op) {
                case '+': result_val = left_val + right_val; break;
                case '-': result_val = left_val - right_val; break;
            }
            
            result = ar_data_create_integer(result_val);
        }
        // At least one operand is a double, result is a double
        else if ((left_type == DATA_INTEGER || left_type == DATA_DOUBLE) &&
                 (right_type == DATA_INTEGER || right_type == DATA_DOUBLE)) {
            
            double left_val = (left_type == DATA_INTEGER) ? 
                (double)ar_data_get_integer(left) : ar_data_get_double(left);
                
            double right_val = (right_type == DATA_INTEGER) ? 
                (double)ar_data_get_integer(right) : ar_data_get_double(right);
            
            double result_val = 0.0;
            
            switch (op) {
                case '+': result_val = left_val + right_val; break;
                case '-': result_val = left_val - right_val; break;
            }
            
            result = ar_data_create_double(result_val);
        }
        // Unsupported operation
        else {
            result = ar_data_create_integer(0);
        }
        
        // Default result if operation failed
        if (!result) {
            result = ar_data_create_integer(0);
        }
        
        // Track this result since we created it
        if (result) {
            ar_list_add_last(ctx->own_results, result);
        }
        
        // The result becomes the new left operand for the next iteration
        left = result;
        
        skip_whitespace(ctx);
    }
    
    return left;
}

// Parse a comparison expression (lowest precedence: =, <>, <, <=, >, >=)
static const data_t* parse_comparison(expression_context_t *ctx) {
    // Parse the left operand (which is an additive expression)
    const data_t *left = parse_additive(ctx);
    if (!left) {
        return NULL;
    }
    
    skip_whitespace(ctx);
    
    // Check if there's a comparison operator
    if (!is_comparison_operator(ctx)) {
        return left; // No comparison, just return the left operand
    }
    
    // Get the comparison operator
    char op[3] = {0};
    op[0] = ctx->ref_expr[ctx->offset];
    ctx->offset++;
    
    // Check for two-character operators (<>, <=, >=)
    if ((op[0] == '<' && (ctx->ref_expr[ctx->offset] == '>' || ctx->ref_expr[ctx->offset] == '=')) ||
        (op[0] == '>' && ctx->ref_expr[ctx->offset] == '=')) {
        op[1] = ctx->ref_expr[ctx->offset];
        ctx->offset++;
    }
    
    skip_whitespace(ctx);
    
    // Parse the right operand (which is an additive expression)
    const data_t *right = parse_additive(ctx);
    if (!right) {
        // Don't destroy left - it's a borrowed reference
        return NULL;
    }
    
    // Perform the comparison
    data_type_t left_type = ar_data_get_type(left);
    data_type_t right_type = ar_data_get_type(right);
    bool result = false;
    
    // Handle case where both operands are numeric
    if ((left_type == DATA_INTEGER || left_type == DATA_DOUBLE) &&
        (right_type == DATA_INTEGER || right_type == DATA_DOUBLE)) {
        
        double left_val, right_val;
        
        if (left_type == DATA_INTEGER) {
            left_val = (double)ar_data_get_integer(left);
        } else {
            left_val = ar_data_get_double(left);
        }
        
        if (right_type == DATA_INTEGER) {
            right_val = (double)ar_data_get_integer(right);
        } else {
            right_val = ar_data_get_double(right);
        }
        
        // Compare based on operator
        if (strcmp(op, "=") == 0) {
            result = (left_val == right_val);
        } else if (strcmp(op, "<>") == 0) {
            result = (left_val != right_val);
        } else if (strcmp(op, "<") == 0) {
            result = (left_val < right_val);
        } else if (strcmp(op, "<=") == 0) {
            result = (left_val <= right_val);
        } else if (strcmp(op, ">") == 0) {
            result = (left_val > right_val);
        } else if (strcmp(op, ">=") == 0) {
            result = (left_val >= right_val);
        }
    }
    // Handle case where both operands are strings
    else if (left_type == DATA_STRING && right_type == DATA_STRING) {
        const char *left_str = ar_data_get_string(left);
        const char *right_str = ar_data_get_string(right);
        
        if (!left_str) left_str = "";
        if (!right_str) right_str = "";
        
        int cmp = strcmp(left_str, right_str);
        
        // Compare based on operator
        if (strcmp(op, "=") == 0) {
            result = (cmp == 0);
        } else if (strcmp(op, "<>") == 0) {
            result = (cmp != 0);
        } else if (strcmp(op, "<") == 0) {
            result = (cmp < 0);
        } else if (strcmp(op, "<=") == 0) {
            result = (cmp <= 0);
        } else if (strcmp(op, ">") == 0) {
            result = (cmp > 0);
        } else if (strcmp(op, ">=") == 0) {
            result = (cmp >= 0);
        }
    }
    // Mixed type comparisons - convert to strings and compare
    else if (left_type != right_type) {
        char left_str[64] = {0};
        char right_str[64] = {0};
        
        // Convert left operand to string
        if (left_type == DATA_STRING) {
            const char *str = ar_data_get_string(left);
            if (str) {
                // Safe string copy with bounds check and explicit null termination
                size_t copy_len = strlen(str);
                if (copy_len >= sizeof(left_str)) {
                    copy_len = sizeof(left_str) - 1;
                }
                memcpy(left_str, str, copy_len);
                left_str[copy_len] = '\0';  // Ensure null-termination
            }
        } else if (left_type == DATA_INTEGER) {
            snprintf(left_str, sizeof(left_str), "%d", ar_data_get_integer(left));
        } else if (left_type == DATA_DOUBLE) {
            snprintf(left_str, sizeof(left_str), "%.2f", ar_data_get_double(left));
        }
        
        // Convert right operand to string
        if (right_type == DATA_STRING) {
            const char *str = ar_data_get_string(right);
            if (str) {
                // Safe string copy with bounds check and explicit null termination
                size_t copy_len = strlen(str);
                if (copy_len >= sizeof(right_str)) {
                    copy_len = sizeof(right_str) - 1;
                }
                memcpy(right_str, str, copy_len);
                right_str[copy_len] = '\0';  // Ensure null-termination
            }
        } else if (right_type == DATA_INTEGER) {
            snprintf(right_str, sizeof(right_str), "%d", ar_data_get_integer(right));
        } else if (right_type == DATA_DOUBLE) {
            snprintf(right_str, sizeof(right_str), "%.2f", ar_data_get_double(right));
        }
        
        int cmp = strcmp(left_str, right_str);
        
        // Compare based on operator
        if (strcmp(op, "=") == 0) {
            result = (cmp == 0);
        } else if (strcmp(op, "<>") == 0) {
            result = (cmp != 0);
        } else if (strcmp(op, "<") == 0) {
            result = (cmp < 0);
        } else if (strcmp(op, "<=") == 0) {
            result = (cmp <= 0);
        } else if (strcmp(op, ">") == 0) {
            result = (cmp > 0);
        } else if (strcmp(op, ">=") == 0) {
            result = (cmp >= 0);
        }
    }
    
    // Create a new integer result (0 for false, 1 for true)
    data_t *comparison_result = ar_data_create_integer(result ? 1 : 0);
    
    // Track this result since we created it
    if (comparison_result) {
        ar_list_add_last(ctx->own_results, comparison_result);
    }
    
    return comparison_result;
}

// Parse and evaluate an expression
static const data_t* parse_expression(expression_context_t *ctx) {
    return parse_comparison(ctx);
}

/**
 * Evaluate an expression in the agent's context using recursive descent parsing.
 *
 * @param mut_ctx Pointer to the expression evaluation context (mutable reference)
 * @return Pointer to the evaluated data result, or NULL on failure
 */
const data_t* ar_expression_evaluate(expression_context_t *mut_ctx) {
    if (!mut_ctx || !mut_ctx->ref_expr) {
        return NULL;
    }
    
    // Parse the expression
    const data_t *ref_result = parse_expression(mut_ctx);
    
    // If parsing failed, return NULL to indicate a syntax error
    // The offset should already be at the position where the error was detected
    if (!ref_result) {
        return NULL;
    }
    
    return ref_result; // Result is owned by context unless ownership is transferred
}

/**
 * Take ownership of a result from the expression context.
 * 
 * This function removes the result from the context's tracked results list,
 * so it won't be destroyed when the context is destroyed. The caller
 * becomes responsible for destroying the result when no longer needed.
 *
 * @param mut_ctx Pointer to the expression evaluation context (mutable reference)
 * @param ref_result The result to take ownership of (becomes owned by caller)
 * @return true if ownership was successfully transferred, false otherwise
 */
data_t* ar_expression_take_ownership(expression_context_t *mut_ctx, const data_t *ref_result) {
    if (!mut_ctx || !ref_result || !mut_ctx->own_results) {
        return NULL;
    }
    
    // If result is a direct reference to memory, context, or message, we don't need to remove
    // it from the results list as these are already not freed by the context
    if (ref_result == mut_ctx->mut_memory) {
        // For memory we return the mutable version since it's already mutable
        return mut_ctx->mut_memory;
    } else if (ref_result == mut_ctx->ref_context || ref_result == mut_ctx->ref_message) {
        // For context and message we can't return non-const pointer since they are truly const
        return NULL;
    }
    
    // Remove the result from the list
    // The ar_list_remove function returns the removed item as a non-const pointer
    data_t *own_result = ar_list_remove(mut_ctx->own_results, ref_result);
    
    // If result was found and removed, ownership is now transferred to the caller
    return own_result;
}
