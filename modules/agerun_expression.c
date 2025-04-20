#include "agerun_system.h"
#include "agerun_expression.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_queue.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Evaluate an expression in the agent's context
data_t* ar_expression_evaluate(agent_t *agent, const char *message, const char *expr, int *offset) {
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
        result = ar_data_create_string(message ? message : "");
        if (!result) {
            // If string creation failed, return a default integer
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
            snprintf(temp, sizeof(temp), "%lld", ar_data_get_integer(key_val));
            key = strdup(temp);
        } else if (key_type == DATA_DOUBLE) {
            char temp[32];
            snprintf(temp, sizeof(temp), "%f", ar_data_get_double(key_val));
            key = strdup(temp);
        }
        
        // Destroy the key value as we've extracted what we need
        ar_data_destroy(key_val);
        
        if (key) {
            // Look up value in memory using the map interface
            const data_t *value = (const data_t *)ar_map_get(agent->memory, key);
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
        
        int64_t value = 0;
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
            int64_t final_value = is_negative ? -value : value;
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
                        char *send_message = NULL;
                        
                        // Get target agent ID
                        data_type_t arg0_type = ar_data_get_type(args[0]);
                        if (arg0_type == DATA_INTEGER) {
                            target_id = ar_data_get_integer(args[0]);
                        } else if (arg0_type == DATA_STRING) {
                            const char *id_str = ar_data_get_string(args[0]);
                            if (id_str) {
                                target_id = atoll(id_str);
                            }
                        }
                        
                        // Get message content
                        data_type_t arg1_type = ar_data_get_type(args[1]);
                        if (arg1_type == DATA_STRING) {
                            const char *msg_str = ar_data_get_string(args[1]);
                            if (msg_str) {
                                send_message = strdup(msg_str);
                            }
                        } else if (arg1_type == DATA_INTEGER) {
                            char temp[32];
                            snprintf(temp, sizeof(temp), "%lld", ar_data_get_integer(args[1]));
                            send_message = strdup(temp);
                        } else if (arg1_type == DATA_DOUBLE) {
                            char temp[32];
                            snprintf(temp, sizeof(temp), "%f", ar_data_get_double(args[1]));
                            send_message = strdup(temp);
                        }
                        
                        // Send the message
                        if (target_id > 0 && send_message) {
                            ar_agent_send(target_id, send_message);
                            
                            // Set result to success
                            ar_data_destroy(result);
                            result = ar_data_create_integer(1);
                            if (!result) {
                                result = ar_data_create_integer(0);
                            }
                        }
                        
                        // Free temporary message string
                        free(send_message);
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
                                            int len = snprintf(temp, sizeof(temp), "%lld", ar_data_get_integer(args[arg_idx]));
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
                int64_t left_int = ar_data_get_integer(result);
                int64_t right_int = ar_data_get_integer(right_val);
                int64_t new_int_val = 0;
                
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
