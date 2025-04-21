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

// Evaluate an expression in the agent's context
data_t* ar_expression_evaluate(agent_t *agent, const data_t *message, const char *expr, int *offset) {
    data_t *result = ar_data_create_integer(0);
    if (!result) {
        return NULL;
    }
    
    if (!expr || !offset) {
        return result;
    }
    
    // Skip leading whitespace
    while (expr[*offset] && ar_string_isspace(expr[*offset])) {
        (*offset)++;
    }
    
    // Handle empty expression
    if (!expr[*offset]) {
        return result;
    }
    
    // Check for string literal (enclosed in quotes)
    if (expr[*offset] == '"') {
        (*offset)++; // Skip opening quote
        
        // Count the length of the string
        int start = *offset;
        int len = 0;
        while (expr[*offset] && expr[*offset] != '"') {
            (*offset)++;
            len++;
        }
        
        // Allocate and create a string value
        char *temp_str = malloc((size_t)len + 1);
        if (temp_str) {
            strncpy(temp_str, expr + start, (size_t)len);
            temp_str[len] = '\0';
            
            // Replace our result with a new string data
            ar_data_destroy(result);
            result = ar_data_create_string(temp_str);
            free(temp_str);
            
            if (!result) {
                // If string creation failed, return a default integer
                result = ar_data_create_integer(0);
            }
        }
        
        if (expr[*offset] == '"') {
            (*offset)++; // Skip closing quote
        }
        
        return result;
    }
    
    // Check for message variable
    if (strncmp(expr + *offset, "message", 7) == 0 && 
        (!expr[*offset + 7] || !isalnum((unsigned char)expr[*offset + 7]))) {
        *offset += 7; // Skip "message"
        
        ar_data_destroy(result);
        
        // If message exists, return a copy of it, otherwise return empty string
        if (message) {
            // This will create a new data_t of the appropriate type
            data_type_t msg_type = ar_data_get_type(message);
            
            switch (msg_type) {
                case DATA_INTEGER:
                    result = ar_data_create_integer(ar_data_get_integer(message));
                    break;
                case DATA_DOUBLE:
                    result = ar_data_create_double(ar_data_get_double(message));
                    break;
                case DATA_STRING:
                    result = ar_data_create_string(ar_data_get_string(message));
                    break;
                case DATA_LIST:
                case DATA_MAP:
                    // For complex types, we'll return a string representation for now
                    result = ar_data_create_string(msg_type == DATA_LIST ? "[List data]" : "{Map data}");
                    break;
                default:
                    result = ar_data_create_integer(0);
                    break;
            }
        } else {
            result = ar_data_create_string("");
        }
        
        if (!result) {
            result = ar_data_create_integer(0);
        }
        
        return result;
    }
    
    // Check for memory access (memory["key"] or memory[key])
    if (strncmp(expr + *offset, "memory", 6) == 0 && expr[*offset + 6] == '[') {
        *offset += 7; // Skip "memory["
        
        // Evaluate the key expression
        int key_offset = 0;
        data_t *key_val = ar_expression_evaluate(agent, message, expr + *offset, &key_offset);
        *offset += key_offset;
        
        // Skip closing bracket
        while (expr[*offset] && expr[*offset] != ']') {
            (*offset)++;
        }
        if (expr[*offset] == ']') {
            (*offset)++;
        }
        
        // Convert key to string
        char *key = NULL;
        data_type_t key_type = ar_data_get_type(key_val);
        
        if (key_type == DATA_STRING) {
            key = strdup(ar_data_get_string(key_val));
        } else if (key_type == DATA_INTEGER) {
            char temp[32];
            snprintf(temp, sizeof(temp), "%d", ar_data_get_integer(key_val));
            key = strdup(temp);
        } else if (key_type == DATA_DOUBLE) {
            char temp[32];
            snprintf(temp, sizeof(temp), "%f", ar_data_get_double(key_val));
            key = strdup(temp);
        }
        
        // Destroy the key value as we've extracted what we need
        ar_data_destroy(key_val);
        
        if (key) {
            // Look up value in memory using the data API
            data_t *value = ar_data_get_map_data(agent->memory, key);
            if (value) {
                // Replace our result with a new data object based on the value
                ar_data_destroy(result);
                
                data_type_t value_type = ar_data_get_type(value);
                if (value_type == DATA_STRING) {
                    result = ar_data_create_string(ar_data_get_string(value));
                } else if (value_type == DATA_INTEGER) {
                    result = ar_data_create_integer(ar_data_get_integer(value));
                } else if (value_type == DATA_DOUBLE) {
                    result = ar_data_create_double(ar_data_get_double(value));
                } else if (value_type == DATA_MAP) {
                    // Create a new map
                    result = ar_data_create_map();
                    // We'd implement map copying here if needed
                }
                
                if (!result) {
                    // If creation failed, return a default integer
                    result = ar_data_create_integer(0);
                }
            }
            
            free(key);
        }
        
        return result;
    }
    
    // Check for integer literal
    if (isdigit((unsigned char)expr[*offset]) || 
        (expr[*offset] == '-' && isdigit((unsigned char)expr[*offset + 1]))) {
        bool is_negative = (expr[*offset] == '-');
        if (is_negative) {
            (*offset)++;
        }
        
        int value = 0;
        bool is_double = false;
        double double_value = 0.0;
        
        // Parse integer part
        while (expr[*offset] && isdigit((unsigned char)expr[*offset])) {
            value = value * 10 + (expr[*offset] - '0');
            (*offset)++;
        }
        
        // Check for decimal point
        if (expr[*offset] == '.') {
            is_double = true;
            double_value = (double)value;
            
            (*offset)++; // Skip decimal point
            double decimal_place = 0.1;
            
            // Parse decimal part
            while (expr[*offset] && isdigit((unsigned char)expr[*offset])) {
                double_value += (expr[*offset] - '0') * decimal_place;
                decimal_place *= 0.1;
                (*offset)++;
            }
        }
        
        // Replace our result with the appropriate numeric type
        ar_data_destroy(result);
        
        if (is_double) {
            double final_value = is_negative ? -double_value : double_value;
            result = ar_data_create_double(final_value);
        } else {
            int final_value = is_negative ? -value : value;
            result = ar_data_create_integer(final_value);
        }
        
        if (!result) {
            // If creation failed, return a default integer
            result = ar_data_create_integer(0);
        }
        
        return result;
    }
    
    // Check for function calls (function(arg1, arg2, ...))
    if (isalpha((unsigned char)expr[*offset])) {
        // Find function name
        int name_start = *offset;
        while (expr[*offset] && (isalnum((unsigned char)expr[*offset]) || expr[*offset] == '_')) {
            (*offset)++;
        }
        
        int name_len = *offset - name_start;
        char func_name[64];
        if ((size_t)name_len < sizeof(func_name)) {
            strncpy(func_name, expr + name_start, name_len);
            func_name[name_len] = '\0';
            
            // Skip whitespace
            while (expr[*offset] && ar_string_isspace(expr[*offset])) {
                (*offset)++;
            }
            
            // Check for opening parenthesis
            if (expr[*offset] == '(') {
                (*offset)++; // Skip opening parenthesis
                
                // Parse arguments
                data_t *args[10] = {NULL}; // Support up to 10 arguments
                int arg_count = 0;
                
                // Skip whitespace
                while (expr[*offset] && ar_string_isspace(expr[*offset])) {
                    (*offset)++;
                }
                
                // Check for empty argument list
                if (expr[*offset] != ')') {
                    do {
                        // Skip whitespace and comma
                        while (expr[*offset] && (ar_string_isspace(expr[*offset]) || expr[*offset] == ',')) {
                            (*offset)++;
                        }
                        
                        // Evaluate argument expression
                        if (arg_count < 10 && expr[*offset] && expr[*offset] != ')') {
                            int arg_offset = 0;
                            args[arg_count] = ar_expression_evaluate(agent, message, expr + *offset, &arg_offset);
                            *offset += arg_offset;
                            arg_count++;
                        }
                        
                        // Skip whitespace
                        while (expr[*offset] && ar_string_isspace(expr[*offset])) {
                            (*offset)++;
                        }
                        
                    } while (expr[*offset] == ',' && arg_count < 10);
                }
                
                // Skip closing parenthesis
                if (expr[*offset] == ')') {
                    (*offset)++;
                }
                
                // Process specific functions
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
                        if (target_id > 0 && send_message) {
                            bool sent = ar_agent_send(target_id, send_message);
                            
                            // Set result to success/failure
                            ar_data_destroy(result);
                            result = ar_data_create_integer(sent ? 1 : 0);
                            if (!result) {
                                result = ar_data_create_integer(0);
                            }
                        } else {
                            // Set result to failure
                            ar_data_destroy(result);
                            result = ar_data_create_integer(0);
                        }
                    }
                } else if (strcmp(func_name, "parse") == 0) {
                    // parse(template, input)
                    if (arg_count >= 2 && args[0] && args[1]) {
                        // Template must be a string
                        if (ar_data_get_type(args[0]) != DATA_STRING) {
                            // Return empty map if template is not a string
                            ar_data_destroy(result);
                            result = ar_data_create_map();
                        } else {
                            const char *template = ar_data_get_string(args[0]);
                            
                            // Input should be a string, if not, return empty map
                            if (ar_data_get_type(args[1]) != DATA_STRING) {
                                // Return empty map if input is not a string
                                ar_data_destroy(result);
                                result = ar_data_create_map();
                            } else {
                                const char *input = ar_data_get_string(args[1]);
                                
                                if (template && input) {
                                    // Create a map to store the parsed values
                                    data_t *map_data = ar_data_create_map();
                                    if (map_data) {
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
                                                    ar_data_set_map_data(map_data, key, val_data);
                                                } else {
                                                    // Try as double
                                                    double double_val = strtod(value, &endptr);
                                                    if (*endptr == '\0') {
                                                        // Double value
                                                        data_t *val_data = ar_data_create_double(double_val);
                                                        ar_data_set_map_data(map_data, key, val_data);
                                                    } else {
                                                        // String value
                                                        data_t *val_data = ar_data_create_string(value);
                                                        ar_data_set_map_data(map_data, key, val_data);
                                                    }
                                                }
                                            }
                                            token = strtok(NULL, " ");
                                        }
                                        
                                        // Return the map as the result
                                        ar_data_destroy(result);
                                        result = map_data;
                                    }
                                }
                            }
                        }
                    } else {
                        // Return empty map for parse failures
                        ar_data_destroy(result);
                        result = ar_data_create_map();
                    }
                } else if (strcmp(func_name, "equals") == 0) {
                    // equals(value1, value2)
                    // Compares two values for equality, handling different data types
                    if (arg_count >= 2 && args[0] && args[1]) {
                        data_type_t type1 = ar_data_get_type(args[0]);
                        data_type_t type2 = ar_data_get_type(args[1]);
                        
                        bool is_equal = false;
                        
                        // If types are the same, compare directly
                        if (type1 == type2) {
                            switch (type1) {
                                case DATA_INTEGER:
                                    is_equal = (ar_data_get_integer(args[0]) == ar_data_get_integer(args[1]));
                                    break;
                                case DATA_DOUBLE:
                                    is_equal = (ar_data_get_double(args[0]) == ar_data_get_double(args[1]));
                                    break;
                                case DATA_STRING:
                                    is_equal = (strcmp(ar_data_get_string(args[0]), ar_data_get_string(args[1])) == 0);
                                    break;
                                case DATA_LIST:
                                case DATA_MAP:
                                    // For now, complex types can only equal themselves (reference equality)
                                    is_equal = (args[0] == args[1]);
                                    break;
                                default:
                                    is_equal = false;
                                    break;
                            }
                        } else {
                            // Types are different - try some automatic conversions for comparison
                            if ((type1 == DATA_INTEGER && type2 == DATA_DOUBLE) ||
                                (type1 == DATA_DOUBLE && type2 == DATA_INTEGER)) {
                                // Compare as doubles
                                double val1 = (type1 == DATA_INTEGER) ? 
                                    (double)ar_data_get_integer(args[0]) : ar_data_get_double(args[0]);
                                double val2 = (type2 == DATA_INTEGER) ? 
                                    (double)ar_data_get_integer(args[1]) : ar_data_get_double(args[1]);
                                is_equal = (val1 == val2);
                            } else if (type1 == DATA_STRING || type2 == DATA_STRING) {
                                // If either is a string, convert both to strings and compare
                                char str1[64] = {0};
                                char str2[64] = {0};
                                
                                // Convert first argument to string
                                if (type1 == DATA_STRING) {
                                    strncpy(str1, ar_data_get_string(args[0]), sizeof(str1) - 1);
                                } else if (type1 == DATA_INTEGER) {
                                    snprintf(str1, sizeof(str1), "%d", ar_data_get_integer(args[0]));
                                } else if (type1 == DATA_DOUBLE) {
                                    snprintf(str1, sizeof(str1), "%f", ar_data_get_double(args[0]));
                                }
                                
                                // Convert second argument to string
                                if (type2 == DATA_STRING) {
                                    strncpy(str2, ar_data_get_string(args[1]), sizeof(str2) - 1);
                                } else if (type2 == DATA_INTEGER) {
                                    snprintf(str2, sizeof(str2), "%d", ar_data_get_integer(args[1]));
                                } else if (type2 == DATA_DOUBLE) {
                                    snprintf(str2, sizeof(str2), "%f", ar_data_get_double(args[1]));
                                }
                                
                                // Compare the string representations
                                is_equal = (strcmp(str1, str2) == 0);
                            }
                        }
                        
                        // Set result to boolean result (1 for true, 0 for false)
                        ar_data_destroy(result);
                        result = ar_data_create_integer(is_equal ? 1 : 0);
                        if (!result) {
                            result = ar_data_create_integer(0);
                        }
                    }
                } else if (strcmp(func_name, "build") == 0) {
                    // build(format, arg1, arg2, ...)
                    if (arg_count >= 1 && args[0] && ar_data_get_type(args[0]) == DATA_STRING) {
                        const char *format = ar_data_get_string(args[0]);
                        if (format) {
                            char result_str[1024] = {0};
                            int result_pos = 0;
                            
                            // Simple format string processing
                            for (int i = 0; format[i] && (size_t)result_pos < sizeof(result_str) - 1; i++) {
                                if (format[i] == '{' && format[i+1] == '}') {
                                    // Replace {} with next argument
                                    int arg_idx = 1 + (result_pos % (arg_count - 1));
                                    
                                    if (arg_idx < arg_count && args[arg_idx]) {
                                        data_type_t arg_type = ar_data_get_type(args[arg_idx]);
                                        
                                        if (arg_type == DATA_STRING) {
                                            const char *str_val = ar_data_get_string(args[arg_idx]);
                                            if (str_val) {
                                                size_t str_len = strlen(str_val);
                                                int len = (int)str_len;
                                                if ((size_t)(result_pos + len) < sizeof(result_str) - 1) {
                                                    strcpy(result_str + result_pos, str_val);
                                                    result_pos += len;
                                                }
                                            }
                                        } else if (arg_type == DATA_INTEGER) {
                                            char temp[32];
                                            int len = snprintf(temp, sizeof(temp), "%d", ar_data_get_integer(args[arg_idx]));
                                            if ((size_t)(result_pos + len) < sizeof(result_str) - 1) {
                                                strcpy(result_str + result_pos, temp);
                                                result_pos += len;
                                            }
                                        } else if (arg_type == DATA_DOUBLE) {
                                            char temp[32];
                                            int len = snprintf(temp, sizeof(temp), "%f", ar_data_get_double(args[arg_idx]));
                                            if ((size_t)(result_pos + len) < sizeof(result_str) - 1) {
                                                strcpy(result_str + result_pos, temp);
                                                result_pos += len;
                                            }
                                        }
                                    }
                                    
                                    i++; // Skip the closing brace
                                } else {
                                    // Copy character as is
                                    result_str[result_pos++] = format[i];
                                }
                            }
                            
                            result_str[result_pos] = '\0';
                            
                            // Set result to the built string
                            ar_data_destroy(result);
                            result = ar_data_create_string(result_str);
                            if (!result) {
                                result = ar_data_create_integer(0);
                            }
                        }
                    }
                }
                
                // Free argument values
                for (int i = 0; i < arg_count; i++) {
                    ar_data_destroy(args[i]);
                }
            }
        }
    }
    
    // Check for basic arithmetic operations
    if (expr[*offset] == '+' || expr[*offset] == '-' || 
        expr[*offset] == '*' || expr[*offset] == '/') {
        char op = expr[*offset];
        (*offset)++; // Skip operator
        
        // Skip whitespace
        while (expr[*offset] && ar_string_isspace(expr[*offset])) {
            (*offset)++;
        }
        
        // Evaluate right operand
        int right_offset = 0;
        data_t *right_val = ar_expression_evaluate(agent, message, expr + *offset, &right_offset);
        *offset += right_offset;
        
        if (right_val) {
            data_type_t left_type = ar_data_get_type(result);
            data_type_t right_type = ar_data_get_type(right_val);
            
            // Perform operation
            if (left_type == DATA_INTEGER && right_type == DATA_INTEGER) {
                int left_int = ar_data_get_integer(result);
                int right_int = ar_data_get_integer(right_val);
                int new_int_val = 0;
                
                switch (op) {
                    case '+': new_int_val = left_int + right_int; break;
                    case '-': new_int_val = left_int - right_int; break;
                    case '*': new_int_val = left_int * right_int; break;
                    case '/': 
                        if (right_int != 0) {
                            new_int_val = left_int / right_int;
                        }
                        break;
                }
                
                // Create a new result with the calculated value
                data_t *new_result = ar_data_create_integer(new_int_val);
                if (new_result) {
                    ar_data_destroy(result);
                    result = new_result;
                }
            } else if ((left_type == DATA_DOUBLE || right_type == DATA_DOUBLE)) {
                // Convert to double for operation
                double left_val = (left_type == DATA_INTEGER) ? 
                    (double)ar_data_get_integer(result) : ar_data_get_double(result);
                    
                double right_double = (right_type == DATA_INTEGER) ? 
                    (double)ar_data_get_integer(right_val) : ar_data_get_double(right_val);
                
                double new_double_val = 0.0;
                switch (op) {
                    case '+': new_double_val = left_val + right_double; break;
                    case '-': new_double_val = left_val - right_double; break;
                    case '*': new_double_val = left_val * right_double; break;
                    case '/': 
                        if (right_double != 0.0) {
                            new_double_val = left_val / right_double;
                        }
                        break;
                }
                
                // Create a new result with the calculated value
                data_t *new_result = ar_data_create_double(new_double_val);
                if (new_result) {
                    ar_data_destroy(result);
                    result = new_result;
                }
            }
            
            // Clean up the right operand
            ar_data_destroy(right_val);
        }
    }
    
    return result;
}
