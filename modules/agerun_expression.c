#include "agerun_system.h"
#include "agerun_expression.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_list.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

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
static data_t* parse_expression(agent_t *agent, const data_t *message, const char *expr, int *offset);
static data_t* parse_primary(agent_t *agent, const data_t *message, const char *expr, int *offset);
static data_t* parse_string_literal(agent_t *agent, const data_t *message, const char *expr, int *offset);
static data_t* parse_number_literal(agent_t *agent, const data_t *message, const char *expr, int *offset);
static data_t* parse_memory_access(agent_t *agent, const data_t *message, const char *expr, int *offset);
static data_t* parse_function_call(agent_t *agent, const data_t *message, const char *expr, int *offset);
static void skip_whitespace(const char *expr, int *offset);
static bool is_comparison_operator(const char *expr, int offset);
static bool is_arithmetic_operator(char c);
static bool match(const char *expr, int *offset, const char *to_match);
static bool match_char(const char *expr, int *offset, char to_match);
static bool expect_char(const char *expr, int *offset, char expected);
static bool is_identifier_start(char c);
static bool is_identifier_part(char c);
static char* parse_identifier(const char *expr, int *offset);
static bool is_digit(char c);

// Skip whitespace characters in the expression
static void skip_whitespace(const char *expr, int *offset) {
    while (expr[*offset] && ar_string_isspace(expr[*offset])) {
        (*offset)++;
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
static char* parse_identifier(const char *expr, int *offset) {
    int start = *offset;
    
    // First character must be a letter
    if (!is_identifier_start(expr[*offset])) {
        return NULL;
    }
    
    (*offset)++;
    
    // Rest of identifier can include letters, digits, and underscore
    while (expr[*offset] && is_identifier_part(expr[*offset])) {
        (*offset)++;
    }
    
    int length = *offset - start;
    char *identifier = malloc((size_t)length + 1);
    if (!identifier) {
        return NULL;
    }
    
    strncpy(identifier, expr + start, (size_t)length);
    identifier[length] = '\0';
    
    return identifier;
}


// Check if the next character in expr matches the expected one
// If it does, advance offset and return true
static bool match_char(const char *expr, int *offset, char to_match) {
    if (expr[*offset] == to_match) {
        (*offset)++;
        return true;
    }
    return false;
}

// Check if the string at the current offset matches the expected string
// If it does, advance offset past the matched string and return true
static bool match(const char *expr, int *offset, const char *to_match) {
    size_t len = strlen(to_match);
    if (strncmp(expr + *offset, to_match, len) == 0) {
        // Make sure it's not part of a longer identifier
        if (to_match[len-1] == '.' || !expr[*offset + (int)len] || 
            !is_identifier_part(expr[*offset + (int)len])) {
            *offset += (int)len;
            return true;
        }
    }
    return false;
}

// Expect a specific character, advance offset if found, return success/failure
static bool expect_char(const char *expr, int *offset, char expected) {
    if (expr[*offset] == expected) {
        (*offset)++;
        return true;
    }
    return false;
}


// Check if the next character is an arithmetic operator
static bool is_arithmetic_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

// Check if the next sequence of characters is a comparison operator
static bool is_comparison_operator(const char *expr, int offset) {
    if (expr[offset] == '=') {
        return true;
    }
    if (expr[offset] == '<') {
        if (expr[offset + 1] == '>' || expr[offset + 1] == '=') {
            return true;
        }
        return true;
    }
    if (expr[offset] == '>') {
        if (expr[offset + 1] == '=') {
            return true;
        }
        return true;
    }
    return false;
}

// Parse a string literal from the expression
static data_t* parse_string_literal(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    (void)agent; // Unused parameter
    (void)message; // Unused parameter

    if (expr[*offset] != '"') {
        return NULL;
    }
    
    (*offset)++; // Skip opening quote
    
    // Find the closing quote and count the length
    int start = *offset;
    int len = 0;
    
    while (expr[*offset] && expr[*offset] != '"') {
        (*offset)++;
        len++;
    }
    
    if (expr[*offset] != '"') {
        // Unterminated string literal
        return NULL;
    }
    
    // Allocate and create the string
    char *temp_str = malloc((size_t)len + 1);
    if (!temp_str) {
        return NULL;
    }
    
    strncpy(temp_str, expr + start, (size_t)len);
    temp_str[len] = '\0';
    
    (*offset)++; // Skip closing quote
    
    data_t *result = ar_data_create_string(temp_str);
    free(temp_str);
    
    return result;
}

// Parse a number literal (integer or double) from the expression
static data_t* parse_number_literal(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    (void)agent; // Unused parameter
    (void)message; // Unused parameter

    bool is_negative = false;
    if (expr[*offset] == '-') {
        is_negative = true;
        (*offset)++;
    }
    
    if (!is_digit(expr[*offset])) {
        return NULL;
    }
    
    // Parse integer part
    int value = 0;
    while (expr[*offset] && is_digit(expr[*offset])) {
        value = value * 10 + (expr[*offset] - '0');
        (*offset)++;
    }
    
    // Check for decimal point for double
    if (expr[*offset] == '.') {
        (*offset)++; // Skip decimal point
        
        if (!is_digit(expr[*offset])) {
            // Malformed double, must have at least one digit after decimal
            return NULL;
        }
        
        double double_value = (double)value;
        double decimal_place = 0.1;
        
        // Parse decimal part
        while (expr[*offset] && is_digit(expr[*offset])) {
            double_value += (expr[*offset] - '0') * decimal_place;
            decimal_place *= 0.1;
            (*offset)++;
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
static data_t* parse_memory_access(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    enum {
        ACCESS_TYPE_MESSAGE,
        ACCESS_TYPE_MEMORY,
        ACCESS_TYPE_CONTEXT
    } access_type;
    
    // Determine which type of access we're dealing with
    if (match(expr, offset, "message")) {
        access_type = ACCESS_TYPE_MESSAGE;
    } else if (match(expr, offset, "memory")) {
        access_type = ACCESS_TYPE_MEMORY;
    } else if (match(expr, offset, "context")) {
        access_type = ACCESS_TYPE_CONTEXT;
    } else {
        return NULL;
    }
    
    // Handle root access (no nested fields)
    if (expr[*offset] != '.') {
        switch (access_type) {
            case ACCESS_TYPE_MESSAGE:
                if (message) {
                    // Make a copy of the message data
                    data_type_t msg_type = ar_data_get_type(message);
                    switch (msg_type) {
                        case DATA_INTEGER:
                            return ar_data_create_integer(ar_data_get_integer(message));
                        case DATA_DOUBLE:
                            return ar_data_create_double(ar_data_get_double(message));
                        case DATA_STRING:
                            return ar_data_create_string(ar_data_get_string(message));
                        case DATA_LIST:
                            // For list, we need a deep copy - not implemented yet
                            return ar_data_create_string("[List data]");
                        case DATA_MAP:
                            // For map, we need a deep copy - not implemented yet
                            return ar_data_create_string("{Map data}");
                        default:
                            return ar_data_create_integer(0);
                    }
                } else {
                    return ar_data_create_string("");
                }
            case ACCESS_TYPE_MEMORY:
                if (agent && agent->memory) {
                    // Not ideal, but we return a string for now
                    return ar_data_create_string("{Memory map}");
                }
                return ar_data_create_map();
            case ACCESS_TYPE_CONTEXT:
                if (agent && agent->context) {
                    // Not ideal, but we return a string for now
                    return ar_data_create_string("{Context map}");
                }
                return ar_data_create_map();
        }
    }
    
    // Handle nested field access with dot notation
    char path[256] = "";
    int path_len = 0;
    
    while (expr[*offset] == '.') {
        (*offset)++; // Skip the dot
        
        char *id = parse_identifier(expr, offset);
        if (!id) {
            return ar_data_create_integer(0); // Invalid path
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
    const data_t *source = NULL;
    switch (access_type) {
        case ACCESS_TYPE_MESSAGE:
            source = message;
            break;
        case ACCESS_TYPE_MEMORY:
            source = agent ? agent->memory : NULL;
            break;
        case ACCESS_TYPE_CONTEXT:
            source = agent ? agent->context : NULL;
            break;
    }
    
    if (!source) {
        return ar_data_create_integer(0);
    }
    
    // Look up the data by path
    data_t *value = NULL;
    data_type_t source_type = ar_data_get_type(source);
    
    if (source_type == DATA_MAP) {
        // For map type, use the map access function
        value = ar_data_get_map_data(source, path);
        if (value) {
            // Make a copy of the value
            data_type_t value_type = ar_data_get_type(value);
            switch (value_type) {
                case DATA_INTEGER:
                    return ar_data_create_integer(ar_data_get_integer(value));
                case DATA_DOUBLE:
                    return ar_data_create_double(ar_data_get_double(value));
                case DATA_STRING:
                    return ar_data_create_string(ar_data_get_string(value));
                case DATA_LIST:
                    // For list, we need a deep copy - not implemented yet
                    return ar_data_create_string("[List data]");
                case DATA_MAP:
                    // For map, we need a deep copy - not implemented yet
                    return ar_data_create_string("{Map data}");
                default:
                    return ar_data_create_integer(0);
            }
        }
    }
    
    // Default to 0 for missing or invalid paths
    return ar_data_create_integer(0);
}

// Parse a function call expression
static data_t* parse_function_call(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    // Parse function name
    char *func_name = parse_identifier(expr, offset);
    if (!func_name) {
        return NULL;
    }
    
    skip_whitespace(expr, offset);
    
    // Expect opening parenthesis
    if (!expect_char(expr, offset, '(')) {
        free(func_name);
        return NULL;
    }
    
    // Parse arguments
    data_t *args[10] = {NULL}; // Support up to 10 arguments
    int arg_count = 0;
    
    skip_whitespace(expr, offset);
    
    // Check for empty argument list
    if (expr[*offset] != ')') {
        do {
            // Skip whitespace and comma
            skip_whitespace(expr, offset);
            
            // Parse argument expression
            if (arg_count < 10 && expr[*offset] && expr[*offset] != ')') {
                args[arg_count] = parse_expression(agent, message, expr, offset);
                if (!args[arg_count]) {
                    // Clean up previously parsed args
                    for (int i = 0; i < arg_count; i++) {
                        ar_data_destroy(args[i]);
                    }
                    free(func_name);
                    return NULL;
                }
                arg_count++;
            }
            
            skip_whitespace(expr, offset);
            
        } while (match_char(expr, offset, ',') && arg_count < 10);
    }
    
    // Expect closing parenthesis
    if (!expect_char(expr, offset, ')')) {
        // Clean up
        for (int i = 0; i < arg_count; i++) {
            ar_data_destroy(args[i]);
        }
        free(func_name);
        return NULL;
    }
    
    // Process specific functions
    data_t *result = NULL;
    
    if (strcmp(func_name, "send") == 0) {
        // send(agent_id, message)
        if (arg_count >= 2 && args[0] && args[1]) {
            agent_id_t target_id = 0;
            data_t *send_message = NULL;
            
            // Get target agent ID
            data_type_t arg0_type = ar_data_get_type(args[0]);
            if (arg0_type == DATA_INTEGER) {
                target_id = (agent_id_t)ar_data_get_integer(args[0]);
            } else if (arg0_type == DATA_STRING) {
                const char *id_str = ar_data_get_string(args[0]);
                if (id_str) {
                    target_id = (agent_id_t)atoll(id_str);
                }
            }
            
            // Create a copy of the message to send
            data_type_t arg1_type = ar_data_get_type(args[1]);
            switch (arg1_type) {
                case DATA_INTEGER:
                    send_message = ar_data_create_integer(ar_data_get_integer(args[1]));
                    break;
                case DATA_DOUBLE:
                    send_message = ar_data_create_double(ar_data_get_double(args[1]));
                    break;
                case DATA_STRING:
                    send_message = ar_data_create_string(ar_data_get_string(args[1]));
                    break;
                case DATA_LIST:
                    // For complex types, we'll send a string representation for now
                    send_message = ar_data_create_string("[List data]");
                    break;
                case DATA_MAP:
                    // For complex types, we'll send a string representation for now
                    send_message = ar_data_create_string("{Map data}");
                    break;
                default:
                    send_message = NULL;
                    break;
            }
            
            // Send the message (ownership transferred to ar_agent_send)
            if (target_id != 0 && send_message) {
                bool sent = ar_agent_send(target_id, send_message);
                
                // Set result to success/failure
                result = ar_data_create_integer(sent ? 1 : 0);
            } else if (target_id == 0) {
                // Special case: sending to agent_id 0 is a no-op that returns true
                result = ar_data_create_integer(1);
                // Destroy the message since we won't be sending it
                ar_data_destroy(send_message);
            } else {
                // Set result to failure
                result = ar_data_create_integer(0);
            }
        }
    } else if (strcmp(func_name, "parse") == 0) {
        // parse(template, input)
        if (arg_count >= 2 && args[0] && args[1]) {
            // Template must be a string
            if (ar_data_get_type(args[0]) != DATA_STRING) {
                // Return empty map if template is not a string
                result = ar_data_create_map();
            } else {
                const char *template = ar_data_get_string(args[0]);
                
                // Input should be a string, if not, return empty map
                if (ar_data_get_type(args[1]) != DATA_STRING) {
                    // Return empty map if input is not a string
                    result = ar_data_create_map();
                } else {
                    const char *input = ar_data_get_string(args[1]);
                    
                    if (template && input) {
                        // Create a map to store the parsed values
                        result = ar_data_create_map();
                        if (result) {
                            // Very simple parsing implementation
                            // TODO: Implement proper parsing using template
                            
                            // For now, just split input by spaces and use as key=value
                            char input_copy[1024];
                            strncpy(input_copy, input, sizeof(input_copy) - 1);
                            input_copy[sizeof(input_copy) - 1] = '\0';
                            
                            char *token = strtok(input_copy, " ");
                            while (token) {
                                char *equals = strchr(token, '=');
                                if (equals) {
                                    *equals = '\0'; // Split at the equals sign
                                    char *key = token;
                                    char *value = equals + 1;
                                    
                                    // Try to parse value as integer or double
                                    char *endptr;
                                    long long_val = strtol(value, &endptr, 10);
                                    if (*endptr == '\0') {
                                        // Integer value
                                        data_t *val_data = ar_data_create_integer((int)long_val);
                                        ar_data_set_map_data(result, key, val_data);
                                    } else {
                                        // Try as double
                                        double double_val = strtod(value, &endptr);
                                        if (*endptr == '\0') {
                                            // Double value
                                            data_t *val_data = ar_data_create_double(double_val);
                                            ar_data_set_map_data(result, key, val_data);
                                        } else {
                                            // String value
                                            data_t *val_data = ar_data_create_string(value);
                                            ar_data_set_map_data(result, key, val_data);
                                        }
                                    }
                                }
                                token = strtok(NULL, " ");
                            }
                        }
                    }
                }
            }
        }
        
        // If we didn't create a result, return empty map for parse failures
        if (!result) {
            result = ar_data_create_map();
        }
    } else if (strcmp(func_name, "build") == 0) {
        // build(template, values)
        if (arg_count >= 2 && args[0] && args[1]) {
            // Template must be a string
            if (ar_data_get_type(args[0]) == DATA_STRING) {
                const char *template = ar_data_get_string(args[0]);
                
                // Values should be a map
                if (template) {
                    char result_str[1024] = {0};
                    int result_pos = 0;
                    
                    // Simple template processing
                    // For now, just replace placeholders with values from args
                    for (int i = 0; template[i] && (size_t)result_pos < sizeof(result_str) - 1; i++) {
                        if (template[i] == '{' && template[i+1] == '}') {
                            // Replace {} with next argument
                            if (arg_count > 1 && args[1]) {
                                data_type_t arg_type = ar_data_get_type(args[1]);
                                
                                if (arg_type == DATA_MAP) {
                                    // TODO: Handle map values properly
                                    const char *str_val = "{Map data}";
                                    size_t str_len = strlen(str_val);
                                    if ((size_t)result_pos + str_len < sizeof(result_str) - 1) {
                                        strcpy(result_str + result_pos, str_val);
                                        result_pos += (int)str_len;
                                    }
                                } else if (arg_type == DATA_STRING) {
                                    const char *str_val = ar_data_get_string(args[1]);
                                    if (str_val) {
                                        size_t str_len = strlen(str_val);
                                        if ((size_t)result_pos + str_len < sizeof(result_str) - 1) {
                                            strcpy(result_str + result_pos, str_val);
                                            result_pos += (int)str_len;
                                        }
                                    }
                                } else if (arg_type == DATA_INTEGER) {
                                    char temp[32];
                                    int len = snprintf(temp, sizeof(temp), "%d", ar_data_get_integer(args[1]));
                                    if ((size_t)(result_pos + len) < sizeof(result_str) - 1) {
                                        strcpy(result_str + result_pos, temp);
                                        result_pos += len;
                                    }
                                } else if (arg_type == DATA_DOUBLE) {
                                    char temp[32];
                                    int len = snprintf(temp, sizeof(temp), "%f", ar_data_get_double(args[1]));
                                    if ((size_t)(result_pos + len) < sizeof(result_str) - 1) {
                                        strcpy(result_str + result_pos, temp);
                                        result_pos += len;
                                    }
                                }
                            }
                            i++; // Skip the closing brace
                        } else {
                            // Copy character as is
                            result_str[result_pos++] = template[i];
                        }
                    }
                    
                    result_str[result_pos] = '\0';
                    
                    // Return the built string
                    result = ar_data_create_string(result_str);
                }
            }
        }
        
        // Default to empty string if build fails
        if (!result) {
            result = ar_data_create_string("");
        }
    } else if (strcmp(func_name, "if") == 0) {
        // if(condition, true_value, false_value)
        if (arg_count >= 3 && args[0] && args[1] && args[2]) {
            // Evaluate condition
            bool condition = false;
            data_type_t cond_type = ar_data_get_type(args[0]);
            
            if (cond_type == DATA_INTEGER) {
                condition = (ar_data_get_integer(args[0]) != 0);
            } else if (cond_type == DATA_DOUBLE) {
                condition = (ar_data_get_double(args[0]) != 0.0);
            } else if (cond_type == DATA_STRING) {
                const char *str = ar_data_get_string(args[0]);
                condition = (str && *str); // True if non-empty string
            } else {
                condition = false; // Default for other types
            }
            
            // Return appropriate value based on condition
            if (condition) {
                data_type_t true_type = ar_data_get_type(args[1]);
                switch (true_type) {
                    case DATA_INTEGER:
                        result = ar_data_create_integer(ar_data_get_integer(args[1]));
                        break;
                    case DATA_DOUBLE:
                        result = ar_data_create_double(ar_data_get_double(args[1]));
                        break;
                    case DATA_STRING:
                        result = ar_data_create_string(ar_data_get_string(args[1]));
                        break;
                    case DATA_LIST:
                    case DATA_MAP:
                        // For complex types, we return a string representation for now
                        result = ar_data_create_string(true_type == DATA_LIST ? "[List data]" : "{Map data}");
                        break;
                    default:
                        result = ar_data_create_integer(0);
                        break;
                }
            } else {
                data_type_t false_type = ar_data_get_type(args[2]);
                switch (false_type) {
                    case DATA_INTEGER:
                        result = ar_data_create_integer(ar_data_get_integer(args[2]));
                        break;
                    case DATA_DOUBLE:
                        result = ar_data_create_double(ar_data_get_double(args[2]));
                        break;
                    case DATA_STRING:
                        result = ar_data_create_string(ar_data_get_string(args[2]));
                        break;
                    case DATA_LIST:
                    case DATA_MAP:
                        // For complex types, we return a string representation for now
                        result = ar_data_create_string(false_type == DATA_LIST ? "[List data]" : "{Map data}");
                        break;
                    default:
                        result = ar_data_create_integer(0);
                        break;
                }
            }
        }
    } else if (strcmp(func_name, "method") == 0) {
        // method(name, version, instructions)
        // Implementation would go here - placeholder for now
        result = ar_data_create_integer(0);
    } else if (strcmp(func_name, "agent") == 0) {
        // agent(method_name, version, context)
        // Implementation would go here - placeholder for now
        result = ar_data_create_integer(0);
    } else if (strcmp(func_name, "destroy") == 0) {
        // destroy(id) or destroy(method_name, version)
        // Implementation would go here - placeholder for now
        result = ar_data_create_integer(0);
    }
    
    // Free arguments and function name
    for (int i = 0; i < arg_count; i++) {
        ar_data_destroy(args[i]);
    }
    free(func_name);
    
    // Default result if none was created
    if (!result) {
        result = ar_data_create_integer(0);
    }
    
    return result;
}

// Parse a primary expression (literal, memory access, or function call)
static data_t* parse_primary(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    skip_whitespace(expr, offset);
    
    // Check for string literal
    if (expr[*offset] == '"') {
        return parse_string_literal(agent, message, expr, offset);
    }
    
    // Check for number literal (including negative numbers)
    if (is_digit(expr[*offset]) || (expr[*offset] == '-' && is_digit(expr[*offset + 1]))) {
        return parse_number_literal(agent, message, expr, offset);
    }
    
    // Check for memory access (message, memory, context)
    if (strncmp(expr + *offset, "message", 7) == 0 ||
        strncmp(expr + *offset, "memory", 6) == 0 ||
        strncmp(expr + *offset, "context", 7) == 0) {
        return parse_memory_access(agent, message, expr, offset);
    }
    
    // Check for function call
    if (is_identifier_start(expr[*offset])) {
        return parse_function_call(agent, message, expr, offset);
    }
    
    // If we get here, it's not a valid primary expression
    return NULL;
}

// Parse and evaluate a comparison expression
static data_t* parse_comparison(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    // First parse the left operand
    data_t *left = parse_primary(agent, message, expr, offset);
    if (!left) {
        return NULL;
    }
    
    skip_whitespace(expr, offset);
    
    // Check if there's a comparison operator
    if (!is_comparison_operator(expr, *offset)) {
        return left; // No comparison, just return the left operand
    }
    
    // Get the comparison operator
    char op[3] = {0};
    op[0] = expr[*offset];
    (*offset)++;
    
    // Check for two-character operators (<>, <=, >=)
    if ((op[0] == '<' && (expr[*offset] == '>' || expr[*offset] == '=')) ||
        (op[0] == '>' && expr[*offset] == '=')) {
        op[1] = expr[*offset];
        (*offset)++;
    }
    
    skip_whitespace(expr, offset);
    
    // Parse the right operand
    data_t *right = parse_primary(agent, message, expr, offset);
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
            snprintf(left_str, sizeof(left_str), "%f", ar_data_get_double(left));
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
            snprintf(right_str, sizeof(right_str), "%f", ar_data_get_double(right));
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

// Parse and evaluate an arithmetic expression
static data_t* parse_arithmetic(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    // First parse the left operand as a comparison (which might be just a primary)
    data_t *left = parse_comparison(agent, message, expr, offset);
    if (!left) {
        return NULL;
    }
    
    skip_whitespace(expr, offset);
    
    // Check if there's an arithmetic operator
    if (!is_arithmetic_operator(expr[*offset])) {
        return left; // No arithmetic, just return the left operand
    }
    
    // Get the arithmetic operator
    char op = expr[*offset];
    (*offset)++;
    
    skip_whitespace(expr, offset);
    
    // Parse the right operand
    data_t *right = parse_comparison(agent, message, expr, offset);
    if (!right) {
        ar_data_destroy(left);
        return NULL;
    }
    
    // Perform the arithmetic operation
    data_type_t left_type = ar_data_get_type(left);
    data_type_t right_type = ar_data_get_type(right);
    data_t *result = NULL;
    
    // Both operands are integers
    if (left_type == DATA_INTEGER && right_type == DATA_INTEGER) {
        int left_val = ar_data_get_integer(left);
        int right_val = ar_data_get_integer(right);
        int result_val = 0;
        
        switch (op) {
            case '+': result_val = left_val + right_val; break;
            case '-': result_val = left_val - right_val; break;
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
            case '+': result_val = left_val + right_val; break;
            case '-': result_val = left_val - right_val; break;
            case '*': result_val = left_val * right_val; break;
            case '/': 
                if (right_val != 0.0) {
                    result_val = left_val / right_val;
                }
                break;
        }
        
        result = ar_data_create_double(result_val);
    }
    // String concatenation with +
    else if (op == '+' && (left_type == DATA_STRING || right_type == DATA_STRING)) {
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
            snprintf(left_str, sizeof(left_str), "%f", ar_data_get_double(left));
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
            snprintf(right_str, sizeof(right_str), "%f", ar_data_get_double(right));
        }
        
        // Concatenate the strings
        char result_str[1024] = {0};
        snprintf(result_str, sizeof(result_str), "%s%s", left_str, right_str);
        
        result = ar_data_create_string(result_str);
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
    
    return result;
}

// Parse and evaluate an expression
static data_t* parse_expression(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    return parse_arithmetic(agent, message, expr, offset);
}

// Public function to evaluate an expression
data_t* ar_expression_evaluate(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    if (!expr || !offset) {
        return ar_data_create_integer(0);
    }
    
    // Save the original offset to restore in case of parsing failure
    int original_offset = *offset;
    
    // Parse the expression
    data_t *result = parse_expression(agent, message, expr, offset);
    
    // If parsing failed, reset the offset and return a default value
    if (!result) {
        *offset = original_offset;
        return ar_data_create_integer(0);
    }
    
    return result;
}
