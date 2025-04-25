#include "agerun_system.h"
#include "agerun_expression.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_list.h"
#include "agerun_map.h"

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
    data_t *memory;     /* The agent's memory */
    data_t *context;    /* The agent's context */
    data_t *message;    /* The message being processed */
    const char *expr;   /* The expression to evaluate */
    int offset;         /* Current position in the expression */
};

/**
 * Creates a new expression evaluation context.
 */
expression_context_t* ar_expression_create_context(data_t *memory, data_t *context, data_t *message, const char *expr) {
    if (!expr) {
        return NULL;
    }
    
    expression_context_t *ctx = malloc(sizeof(expression_context_t));
    if (!ctx) {
        return NULL;
    }
    
    ctx->memory = memory;
    ctx->context = context;
    ctx->message = message;
    ctx->expr = expr;
    ctx->offset = 0;
    
    return ctx;
}

/**
 * Destroys an expression context.
 * Note: This only frees the context structure itself, not the memory, context, or message
 * data structures which are owned by the caller.
 */
void ar_expression_destroy_context(expression_context_t *ctx) {
    free(ctx);
}

/**
 * Gets the current parsing offset in the expression string.
 */
int ar_expression_offset(const expression_context_t *ctx) {
    if (!ctx) {
        return 0;
    }
    return ctx->offset;
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
static data_t* parse_expression(expression_context_t *ctx);
static data_t* parse_primary(expression_context_t *ctx);
static data_t* parse_string_literal(expression_context_t *ctx);
static data_t* parse_number_literal(expression_context_t *ctx);
static data_t* parse_memory_access(expression_context_t *ctx);
static void skip_whitespace(expression_context_t *ctx);
static bool is_comparison_operator(expression_context_t *ctx);
static bool match(expression_context_t *ctx, const char *to_match);
static bool is_identifier_start(char c);
static bool is_identifier_part(char c);
static char* parse_identifier(expression_context_t *ctx);
static bool is_digit(char c);

// Skip whitespace characters in the expression
static void skip_whitespace(expression_context_t *ctx) {
    while (ctx->expr[ctx->offset] && ar_string_isspace(ctx->expr[ctx->offset])) {
        ctx->offset++;
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
static char* parse_identifier(expression_context_t *ctx) {
    int start = ctx->offset;
    
    // First character must be a letter
    if (!is_identifier_start(ctx->expr[ctx->offset])) {
        return NULL;
    }
    
    ctx->offset++;
    
    // Rest of identifier can include letters, digits, and underscore
    while (ctx->expr[ctx->offset] && is_identifier_part(ctx->expr[ctx->offset])) {
        ctx->offset++;
    }
    
    int length = ctx->offset - start;
    char *identifier = malloc((size_t)length + 1);
    if (!identifier) {
        return NULL;
    }
    
    strncpy(identifier, ctx->expr + start, (size_t)length);
    identifier[length] = '\0';
    
    return identifier;
}


// Check if the string at the current offset matches the expected string
// If it does, advance offset past the matched string and return true
static bool match(expression_context_t *ctx, const char *to_match) {
    size_t len = strlen(to_match);
    if (strncmp(ctx->expr + ctx->offset, to_match, len) == 0) {
        // Make sure it's not part of a longer identifier
        if (to_match[len-1] == '.' || !ctx->expr[ctx->offset + (int)len] || 
            !is_identifier_part(ctx->expr[ctx->offset + (int)len])) {
            ctx->offset += (int)len;
            return true;
        }
    }
    return false;
}



// Check if the next sequence of characters is a comparison operator
static bool is_comparison_operator(expression_context_t *ctx) {
    if (ctx->expr[ctx->offset] == '=') {
        return true;
    }
    if (ctx->expr[ctx->offset] == '<') {
        if (ctx->expr[ctx->offset + 1] == '>' || ctx->expr[ctx->offset + 1] == '=') {
            return true;
        }
        return true;
    }
    if (ctx->expr[ctx->offset] == '>') {
        if (ctx->expr[ctx->offset + 1] == '=') {
            return true;
        }
        return true;
    }
    return false;
}

// Parse a string literal from the expression
static data_t* parse_string_literal(expression_context_t *ctx) {
    if (ctx->expr[ctx->offset] != '"') {
        return NULL;
    }
    
    ctx->offset++; // Skip opening quote
    
    // Find the closing quote and count the length
    int start = ctx->offset;
    int len = 0;
    
    while (ctx->expr[ctx->offset] && ctx->expr[ctx->offset] != '"') {
        ctx->offset++;
        len++;
    }
    
    if (ctx->expr[ctx->offset] != '"') {
        // Unterminated string literal
        return NULL;
    }
    
    // Allocate and create the string
    char *temp_str = malloc((size_t)len + 1);
    if (!temp_str) {
        return NULL;
    }
    
    strncpy(temp_str, ctx->expr + start, (size_t)len);
    temp_str[len] = '\0';
    
    ctx->offset++; // Skip closing quote
    
    data_t *result = ar_data_create_string(temp_str);
    free(temp_str);
    
    return result;
}

// Parse a number literal (integer or double) from the expression
static data_t* parse_number_literal(expression_context_t *ctx) {
    bool is_negative = false;
    if (ctx->expr[ctx->offset] == '-') {
        is_negative = true;
        ctx->offset++;
    }
    
    if (!is_digit(ctx->expr[ctx->offset])) {
        return NULL;
    }
    
    // Parse integer part
    int value = 0;
    while (ctx->expr[ctx->offset] && is_digit(ctx->expr[ctx->offset])) {
        value = value * 10 + (ctx->expr[ctx->offset] - '0');
        ctx->offset++;
    }
    
    // Check for decimal point for double
    if (ctx->expr[ctx->offset] == '.') {
        ctx->offset++; // Skip decimal point
        
        if (!is_digit(ctx->expr[ctx->offset])) {
            // Malformed double, must have at least one digit after decimal
            return NULL;
        }
        
        double double_value = (double)value;
        double decimal_place = 0.1;
        
        // Parse decimal part
        while (ctx->expr[ctx->offset] && is_digit(ctx->expr[ctx->offset])) {
            double_value += (ctx->expr[ctx->offset] - '0') * decimal_place;
            decimal_place *= 0.1;
            ctx->offset++;
        }
        
        if (is_negative) {
            double_value = -double_value;
        }
        
        return ar_data_create_double(double_value);
    }
    
    // It's an integer
    if (is_negative) {
        value = -value;
    }
    
    return ar_data_create_integer(value);
}

// Parse a memory access (message, memory, context) expression
static data_t* parse_memory_access(expression_context_t *ctx) {
    enum {
        ACCESS_TYPE_MESSAGE,
        ACCESS_TYPE_MEMORY,
        ACCESS_TYPE_CONTEXT
    } access_type;
    
    // Determine which type of access we're dealing with
    if (match(ctx, "message")) {
        access_type = ACCESS_TYPE_MESSAGE;
    } else if (match(ctx, "memory")) {
        access_type = ACCESS_TYPE_MEMORY;
    } else if (match(ctx, "context")) {
        access_type = ACCESS_TYPE_CONTEXT;
    } else {
        return NULL;
    }
    
    // Handle root access (no nested fields)
    if (ctx->expr[ctx->offset] != '.') {
        switch (access_type) {
            case ACCESS_TYPE_MESSAGE:
                if (ctx->message) {
                    // Return the message directly, not a copy
                    return ctx->message;
                } else {
                    // Return NULL for non-existent message
                    return NULL;
                }
            case ACCESS_TYPE_MEMORY:
                if (ctx->memory) {
                    return ctx->memory;
                }
                // Return NULL for non-existent memory
                return NULL;
            case ACCESS_TYPE_CONTEXT:
                if (ctx->context) {
                    return ctx->context;
                }
                // Return NULL for non-existent context
                return NULL;
        }
    }
    
    // Handle nested field access with dot notation
    char path[256] = "";
    int path_len = 0;
    
    while (ctx->expr[ctx->offset] == '.') {
        ctx->offset++; // Skip the dot
        
        char *id = parse_identifier(ctx);
        if (!id) {
            return NULL; // Invalid identifier in path is a syntax error
        }
        
        // Append to path
        size_t id_len = strlen(id);
        if ((size_t)path_len + id_len + 1U < sizeof(path)) {
            if (path_len > 0) {
                path[path_len++] = '.';
            }
            strcpy(path + path_len, id);
            path_len += (int)id_len;
        }
        
        free(id);
    }
    
    // Now we have the full path, get the data
    data_t *source = NULL;
    switch (access_type) {
        case ACCESS_TYPE_MESSAGE:
            source = ctx->message;
            break;
        case ACCESS_TYPE_MEMORY:
            source = ctx->memory;
            break;
        case ACCESS_TYPE_CONTEXT:
            source = ctx->context;
            break;
    }
    
    if (!source) {
        // Return NULL for non-existent source
        return NULL;
    }
    
    // Look up the data by path
    data_t *value = NULL;
    data_type_t source_type = ar_data_get_type(source);
    
    if (source_type == DATA_MAP) {
        // For map type, use the map access function
        value = ar_data_get_map_data(source, path);
        if (value) {
            // Return the value directly, not a copy
            return value;
        }
    }
    
    // Return NULL for missing or invalid paths
    return NULL;
}


// Parse a primary expression (literal or memory access)
static data_t* parse_primary(expression_context_t *ctx) {
    skip_whitespace(ctx);
    
    // Check for string literal
    if (ctx->expr[ctx->offset] == '"') {
        return parse_string_literal(ctx);
    }
    
    // Check for number literal (including negative numbers)
    if (is_digit(ctx->expr[ctx->offset]) || (ctx->expr[ctx->offset] == '-' && is_digit(ctx->expr[ctx->offset + 1]))) {
        return parse_number_literal(ctx);
    }
    
    // Check for memory access (message, memory, context)
    if (strncmp(ctx->expr + ctx->offset, "message", 7) == 0 ||
        strncmp(ctx->expr + ctx->offset, "memory", 6) == 0 ||
        strncmp(ctx->expr + ctx->offset, "context", 7) == 0) {
        return parse_memory_access(ctx);
    }
    
    // Check for function call - which is a syntax error in expressions
    if (is_identifier_start(ctx->expr[ctx->offset])) {
        // Save the position at the start of the function name
        int func_name_start = ctx->offset;
        
        // Skip over function name
        while (ctx->expr[ctx->offset] && is_identifier_part(ctx->expr[ctx->offset])) {
            ctx->offset++;
        }
        
        skip_whitespace(ctx);
        
        // If we find an opening parenthesis, it's a function call - syntax error
        if (ctx->expr[ctx->offset] == '(') {
            // Reset offset to the start of the function name
            ctx->offset = func_name_start;
            // Return NULL to indicate a syntax error
            return NULL;
        }
        
        // Not a function call, reset offset and continue
        ctx->offset = func_name_start;
    }
    
    // If we get here, it's not a valid primary expression
    return NULL;
}

// Forward declarations for recursive descent functions
static data_t* parse_multiplicative(expression_context_t *ctx);
static data_t* parse_additive(expression_context_t *ctx);
static data_t* parse_comparison(expression_context_t *ctx);

// Parse a multiplicative expression (higher precedence: *, /)
static data_t* parse_multiplicative(expression_context_t *ctx) {
    // Parse the left operand
    data_t *left = parse_primary(ctx);
    if (!left) {
        return NULL;
    }
    
    skip_whitespace(ctx);
    
    // Check if there's a multiplicative operator (*, /)
    while (ctx->expr[ctx->offset] == '*' || ctx->expr[ctx->offset] == '/') {
        // Get the operator
        char op = ctx->expr[ctx->offset];
        ctx->offset++;
        
        skip_whitespace(ctx);
        
        // Parse the right operand (which is a primary)
        data_t *right = parse_primary(ctx);
        if (!right) {
            ar_data_destroy(left);
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
        
        // Clean up operands
        ar_data_destroy(left);
        ar_data_destroy(right);
        
        // Default result if operation failed
        if (!result) {
            result = ar_data_create_integer(0);
        }
        
        // The result becomes the new left operand for the next iteration
        left = result;
        
        skip_whitespace(ctx);
    }
    
    return left;
}

// Parse an additive expression (medium precedence: +, -)
static data_t* parse_additive(expression_context_t *ctx) {
    // Parse the left operand (which is a multiplicative expression)
    data_t *left = parse_multiplicative(ctx);
    if (!left) {
        return NULL;
    }
    
    skip_whitespace(ctx);
    
    // Check if there's an additive operator (+, -)
    while (ctx->expr[ctx->offset] == '+' || ctx->expr[ctx->offset] == '-') {
        // Get the operator
        char op = ctx->expr[ctx->offset];
        ctx->offset++;
        
        skip_whitespace(ctx);
        
        // Parse the right operand (which is a multiplicative expression)
        data_t *right = parse_multiplicative(ctx);
        if (!right) {
            ar_data_destroy(left);
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
                    strncpy(left_str, str, sizeof(left_str) - 1);
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
                    strncpy(right_str, str, sizeof(right_str) - 1);
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
        
        // Clean up operands
        ar_data_destroy(left);
        ar_data_destroy(right);
        
        // Default result if operation failed
        if (!result) {
            result = ar_data_create_integer(0);
        }
        
        // The result becomes the new left operand for the next iteration
        left = result;
        
        skip_whitespace(ctx);
    }
    
    return left;
}

// Parse a comparison expression (lowest precedence: =, <>, <, <=, >, >=)
static data_t* parse_comparison(expression_context_t *ctx) {
    // Parse the left operand (which is an additive expression)
    data_t *left = parse_additive(ctx);
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
    op[0] = ctx->expr[ctx->offset];
    ctx->offset++;
    
    // Check for two-character operators (<>, <=, >=)
    if ((op[0] == '<' && (ctx->expr[ctx->offset] == '>' || ctx->expr[ctx->offset] == '=')) ||
        (op[0] == '>' && ctx->expr[ctx->offset] == '=')) {
        op[1] = ctx->expr[ctx->offset];
        ctx->offset++;
    }
    
    skip_whitespace(ctx);
    
    // Parse the right operand (which is an additive expression)
    data_t *right = parse_additive(ctx);
    if (!right) {
        ar_data_destroy(left);
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
                strncpy(left_str, str, sizeof(left_str) - 1);
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
                strncpy(right_str, str, sizeof(right_str) - 1);
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
    
    // Clean up operands
    ar_data_destroy(left);
    ar_data_destroy(right);
    
    // Return the result as an integer (0 for false, 1 for true)
    return ar_data_create_integer(result ? 1 : 0);
}

// Parse and evaluate an expression
static data_t* parse_expression(expression_context_t *ctx) {
    return parse_comparison(ctx);
}

// Public function to evaluate an expression
data_t* ar_expression_evaluate(expression_context_t *ctx) {
    if (!ctx || !ctx->expr) {
        return NULL;
    }
    
    // Parse the expression
    data_t *result = parse_expression(ctx);
    
    // If parsing failed, return NULL to indicate a syntax error
    // The offset should already be at the position where the error was detected
    if (!result) {
        return NULL;
    }
    
    return result;
}
