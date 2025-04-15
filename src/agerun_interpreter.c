#include "../include/agerun_system.h"
#include "../include/agerun_interpreter.h"
#include "../include/agerun_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Value Type Definitions
typedef enum {
    VALUE_INT,
    VALUE_DOUBLE,
    VALUE_STRING
} value_type_t;

typedef struct value_s {
    value_type_t type;
    union {
        int64_t int_value;
        double double_value;
        char *string_value;
    } data;
} value_t;

/* Memory Dictionary for Agent State */
typedef struct memory_entry_s {
    char *key;
    value_t value;
    bool is_used;
} memory_entry_t;

typedef struct memory_dict_s {
    memory_entry_t entries[256]; // Use same size as defined in agerun.c
    int count;
} memory_dict_t;

typedef struct agent_s {
    agent_id_t id;
    char method_name[64]; // Use same size as defined in agerun.c
    version_t method_version;
    bool is_active;
    bool is_persistent;
    void *queue;
    memory_dict_t memory;
    memory_dict_t *context;
} agent_t;

// External function declarations
extern bool ar_send(agent_id_t agent_id, const char *message);
extern agent_id_t ar_create(const char *method_name, version_t version, void *context);
extern bool ar_destroy(agent_id_t agent_id);
extern version_t ar_method(const char *name, const char *instructions, version_t previous_version, bool backward_compatible, bool persist);
extern bool ar_memory_set(void *memory, const char *key, void *value);

// Forward declarations for internal functions
static bool parse_and_execute_instruction(agent_t *agent, const char *message, const char *instruction);
static value_t evaluate_expression(agent_t *agent, const char *message, const char *expr, int *offset);


// Free a value
static void free_value(value_t *value) {
    if (!value) return;
    
    if (value->type == VALUE_STRING && value->data.string_value) {
        free(value->data.string_value);
        value->data.string_value = NULL;
    }
}




// Evaluate an expression in the agent's context
static value_t evaluate_expression(agent_t *agent, const char *message, const char *expr, int *offset) {
    value_t result;
    result.type = VALUE_INT;
    result.data.int_value = 0;
    
    if (!expr || !offset) {
        return result;
    }
    
    // Skip leading whitespace
    while (expr[*offset] && isspace((unsigned char)expr[*offset])) {
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
        
        // Allocate and copy the string
        result.type = VALUE_STRING;
        result.data.string_value = malloc((size_t)len + 1);
        if (result.data.string_value) {
            strncpy(result.data.string_value, expr + start, (size_t)len);
            result.data.string_value[len] = '\0';
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
        
        result.type = VALUE_STRING;
        result.data.string_value = strdup(message ? message : "");
        return result;
    }
    
    // Check for memory access (memory["key"] or memory[key])
    if (strncmp(expr + *offset, "memory", 6) == 0 && expr[*offset + 6] == '[') {
        *offset += 7; // Skip "memory["
        
        // Evaluate the key expression
        int key_offset = 0;
        value_t key_val = evaluate_expression(agent, message, expr + *offset, &key_offset);
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
        if (key_val.type == VALUE_STRING) {
            key = key_val.data.string_value;
        } else if (key_val.type == VALUE_INT) {
            char temp[32];
            snprintf(temp, sizeof(temp), "%lld", key_val.data.int_value);
            key = strdup(temp);
            free_value(&key_val);
        } else if (key_val.type == VALUE_DOUBLE) {
            char temp[32];
            snprintf(temp, sizeof(temp), "%f", key_val.data.double_value);
            key = strdup(temp);
            free_value(&key_val);
        }
        
        if (key) {
            // Look up value in memory
            for (int i = 0; i < agent->memory.count; i++) {
                if (agent->memory.entries[i].is_used && 
                    strcmp(agent->memory.entries[i].key, key) == 0) {
                    
                    // Copy the value
                    if (agent->memory.entries[i].value.type == VALUE_STRING && 
                        agent->memory.entries[i].value.data.string_value) {
                        result.type = VALUE_STRING;
                        result.data.string_value = strdup(agent->memory.entries[i].value.data.string_value);
                    } else if (agent->memory.entries[i].value.type == VALUE_INT) {
                        result.type = VALUE_INT;
                        result.data.int_value = agent->memory.entries[i].value.data.int_value;
                    } else if (agent->memory.entries[i].value.type == VALUE_DOUBLE) {
                        result.type = VALUE_DOUBLE;
                        result.data.double_value = agent->memory.entries[i].value.data.double_value;
                    }
                    
                    if (key_val.type != VALUE_STRING) {
                        free(key);
                    }
                    return result;
                }
            }
            
            if (key_val.type != VALUE_STRING) {
                free(key);
            } else {
                free(key_val.data.string_value);
            }
        }
        
        // Key not found in memory, return default value
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
        
        if (is_double) {
            result.type = VALUE_DOUBLE;
            result.data.double_value = is_negative ? -double_value : double_value;
        } else {
            result.type = VALUE_INT;
            result.data.int_value = is_negative ? -value : value;
        }
        
        return result;
    }
    
    // Check for function calls (function(arg1, arg2, ...))
    // For now, we'll support a few basic functions like send() and build()
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
            while (expr[*offset] && isspace((unsigned char)expr[*offset])) {
                (*offset)++;
            }
            
            // Check for opening parenthesis
            if (expr[*offset] == '(') {
                (*offset)++; // Skip opening parenthesis
                
                // Parse arguments
                value_t args[10]; // Support up to 10 arguments
                int arg_count = 0;
                
                // Skip whitespace
                while (expr[*offset] && isspace((unsigned char)expr[*offset])) {
                    (*offset)++;
                }
                
                // Check for empty argument list
                if (expr[*offset] != ')') {
                    do {
                        // Skip whitespace and comma
                        while (expr[*offset] && (isspace((unsigned char)expr[*offset]) || expr[*offset] == ',')) {
                            (*offset)++;
                        }
                        
                        // Evaluate argument expression
                        if (arg_count < 10 && expr[*offset] && expr[*offset] != ')') {
                            int arg_offset = 0;
                            args[arg_count] = evaluate_expression(agent, message, expr + *offset, &arg_offset);
                            *offset += arg_offset;
                            arg_count++;
                        }
                        
                        // Skip whitespace
                        while (expr[*offset] && isspace((unsigned char)expr[*offset])) {
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
                    if (arg_count >= 2) {
                        agent_id_t target_id = 0;
                        char *send_message = NULL;
                        
                        // Get target agent ID
                        if (args[0].type == VALUE_INT) {
                            target_id = args[0].data.int_value;
                        } else if (args[0].type == VALUE_STRING && args[0].data.string_value) {
                            target_id = atoll(args[0].data.string_value);
                        }
                        
                        // Get message content
                        if (args[1].type == VALUE_STRING && args[1].data.string_value) {
                            send_message = args[1].data.string_value;
                        } else if (args[1].type == VALUE_INT) {
                            char temp[32];
                            snprintf(temp, sizeof(temp), "%lld", args[1].data.int_value);
                            send_message = strdup(temp);
                        } else if (args[1].type == VALUE_DOUBLE) {
                            char temp[32];
                            snprintf(temp, sizeof(temp), "%f", args[1].data.double_value);
                            send_message = strdup(temp);
                        }
                        
                        // Send the message
                        if (target_id > 0 && send_message) {
                            ar_send(target_id, send_message);
                            
                            // Set result to success
                            result.type = VALUE_INT;
                            result.data.int_value = 1;
                        }
                        
                        // Free temporary message string
                        if (args[1].type != VALUE_STRING && send_message) {
                            free(send_message);
                        }
                    }
                } else if (strcmp(func_name, "build") == 0) {
                    // build(format, arg1, arg2, ...)
                    if (arg_count >= 1 && args[0].type == VALUE_STRING && args[0].data.string_value) {
                        char *format = args[0].data.string_value;
                        char result_str[1024] = {0};
                        int result_pos = 0;
                        
                        // Simple format string processing
                        for (int i = 0; format[i] && (size_t)result_pos < sizeof(result_str) - 1; i++) {
                            if (format[i] == '{' && format[i+1] == '}') {
                                // Replace {} with next argument
                                int arg_idx = 1 + (result_pos % (arg_count - 1));
                                
                                if (arg_idx < arg_count) {
                                    if (args[arg_idx].type == VALUE_STRING && args[arg_idx].data.string_value) {
                                        size_t str_len = strlen(args[arg_idx].data.string_value);
                                        int len = (int)str_len;
                                        if ((size_t)(result_pos + len) < sizeof(result_str) - 1) {
                                            strcpy(result_str + result_pos, args[arg_idx].data.string_value);
                                            result_pos += len;
                                        }
                                    } else if (args[arg_idx].type == VALUE_INT) {
                                        char temp[32];
                                        int len = snprintf(temp, sizeof(temp), "%lld", args[arg_idx].data.int_value);
                                        if ((size_t)(result_pos + len) < sizeof(result_str) - 1) {
                                            strcpy(result_str + result_pos, temp);
                                            result_pos += len;
                                        }
                                    } else if (args[arg_idx].type == VALUE_DOUBLE) {
                                        char temp[32];
                                        int len = snprintf(temp, sizeof(temp), "%f", args[arg_idx].data.double_value);
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
                        result.type = VALUE_STRING;
                        result.data.string_value = strdup(result_str);
                    }
                }
                
                // Free argument values
                for (int i = 0; i < arg_count; i++) {
                    free_value(&args[i]);
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
        while (expr[*offset] && isspace((unsigned char)expr[*offset])) {
            (*offset)++;
        }
        
        // Evaluate right operand
        int right_offset = 0;
        value_t right_val = evaluate_expression(agent, message, expr + *offset, &right_offset);
        *offset += right_offset;
        
        // Perform operation
        if (result.type == VALUE_INT && right_val.type == VALUE_INT) {
            switch (op) {
                case '+': result.data.int_value += right_val.data.int_value; break;
                case '-': result.data.int_value -= right_val.data.int_value; break;
                case '*': result.data.int_value *= right_val.data.int_value; break;
                case '/': 
                    if (right_val.data.int_value != 0) {
                        result.data.int_value /= right_val.data.int_value;
                    }
                    break;
            }
        } else if ((result.type == VALUE_DOUBLE || right_val.type == VALUE_DOUBLE)) {
            // Convert to double for operation
            double left_val = (result.type == VALUE_INT) ? 
                (double)result.data.int_value : result.data.double_value;
                
            double right_double = (right_val.type == VALUE_INT) ? 
                (double)right_val.data.int_value : right_val.data.double_value;
                
            result.type = VALUE_DOUBLE;
            
            switch (op) {
                case '+': result.data.double_value = left_val + right_double; break;
                case '-': result.data.double_value = left_val - right_double; break;
                case '*': result.data.double_value = left_val * right_double; break;
                case '/': 
                    if (right_double != 0.0) {
                        result.data.double_value = left_val / right_double;
                    }
                    break;
            }
        }
        
        free_value(&right_val);
    }
    
    return result;
}



// Main interpretation function for agent methods
bool ar_interpret_agent_method(agent_t *agent, const char *message, const char *instructions) {
    (void)agent; // Avoid unused parameter warning
    (void)message; // Avoid unused parameter warning
    
    // Make a copy of the instructions for tokenization
    char *instructions_copy = strdup(instructions);
    if (!instructions_copy) {
        return false;
    }
    
    // Split instructions by newlines
    char *instruction = strtok(instructions_copy, "n");
    bool result = true;
    
    while (instruction != NULL) {
        instruction = ar_trim(instruction);
        
        // Skip empty lines and comments
        if (strlen(instruction) > 0 && instruction[0] != '#') {
            if (!parse_and_execute_instruction(agent, message, instruction)) {
                result = false;
                break;
            }
        }
        
        instruction = strtok(NULL, "n");
    }
    
    free(instructions_copy);
    return result;
}

// Parse and execute a single instruction
static bool parse_and_execute_instruction(agent_t *agent, const char *message, const char *instruction) {
    char *instr_copy = strdup(instruction);
    char *instr_trimmed = ar_trim(instr_copy);
    bool result = true;
    
    // Check for assignment operation (key := value)
    char *assign_pos = strstr(instr_trimmed, ":=");
    if (assign_pos != NULL) {
        // Extract key and value parts
        *assign_pos = '0';
        char *key = ar_trim(instr_trimmed);
        char *value_expr = ar_trim(assign_pos + 2);
        
        // Direct key access - no need to check for memory["key"] syntax
        // as memory dictionary is implicit per the spec
        
        // Evaluate the value expression
        int offset = 0;
        value_t value = evaluate_expression(agent, message, value_expr, &offset);
        
        // Store in agent's memory
        ar_memory_set(&agent->memory, key, &value);
    }
    // Parse function call or other expression
    else {
        int offset = 0;
        value_t result_val = evaluate_expression(agent, message, instr_trimmed, &offset);
        free_value(&result_val); // Discard the result
    }
    
    free(instr_copy);
    return result;
}

