#include "../include/agerun.h"
#include "../include/agerun_interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Forward declarations for types defined in agerun.c
typedef struct agent_s agent_t;
typedef struct memory_dict_s memory_dict_t;
typedef struct value_s value_t;

// External function declarations
extern bool agerun_send(agent_id_t agent_id, const char *message);
extern agent_id_t agerun_create(const char *method_name, version_t version, void *context);
extern bool agerun_destroy(agent_id_t agent_id);
extern version_t agerun_method(const char *name, const char *instructions, version_t previous_version, bool backward_compatible, bool persist);

// Forward declarations for internal functions
static bool parse_and_execute_instruction(agent_t *agent, const char *message, const char *instruction);
static char* parse_string_literal(const char *str, int *offset);
static char* parse_identifier(const char *str, int *offset);
static value_t evaluate_expression(agent_t *agent, const char *message, const char *expr, int *offset);
static value_t call_function(agent_t *agent, const char *message, const char *func_name, value_t *args, int arg_count);
static void skip_whitespace(const char *str, int *offset);
static bool is_operator(char c);
static int get_operator_precedence(char op);
static value_t apply_operator(char op, value_t left, value_t right);
static bool values_equal(value_t a, value_t b);
static bool value_to_bool(value_t val);
static value_t* memory_get(memory_dict_t *memory, const char *key);
static bool memory_set(memory_dict_t *memory, const char *key, value_t value);
static void free_value(value_t *value);
static value_t copy_value(value_t src);
static void value_to_string(value_t val, char *buffer, size_t buffer_size);

// Helper function to check if a string starts with a prefix
static bool starts_with(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Helper function to check if a string ends with a suffix
static bool ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return false;
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str)
        return false;
    return strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0;
}

// Helper function to trim whitespace from a string
static char* trim(char *str) {
    if (!str) return NULL;
    
    // Trim leading space
    char *start = str;
    while(isspace((unsigned char)*start)) start++;
    
    if(*start == 0) // All spaces
        return start;
    
    // Trim trailing space
    char *end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    return start;
}

// Main interpretation function for agent methods
bool interpret_agent_method(agent_t *agent, const char *message, const char *instructions) {
    // Make a copy of the instructions for tokenization
    char *instructions_copy = strdup(instructions);
    if (!instructions_copy) {
        return false;
    }
    
    // Split instructions by newlines
    char *instruction = strtok(instructions_copy, "\n");
    bool result = true;
    
    while (instruction != NULL) {
        instruction = trim(instruction);
        
        // Skip empty lines and comments
        if (strlen(instruction) > 0 && instruction[0] != '#') {
            if (!parse_and_execute_instruction(agent, message, instruction)) {
                result = false;
                break;
            }
        }
        
        instruction = strtok(NULL, "\n");
    }
    
    free(instructions_copy);
    return result;
}

// Parse and execute a single instruction
static bool parse_and_execute_instruction(agent_t *agent, const char *message, const char *instruction) {
    char *instr_copy = strdup(instruction);
    char *instr_trimmed = trim(instr_copy);
    bool result = true;
    
    // Check for assignment operation (key := value)
    char *assign_pos = strstr(instr_trimmed, ":=");
    if (assign_pos != NULL) {
        // Extract key and value parts
        *assign_pos = '\0';
        char *key = trim(instr_trimmed);
        char *value_expr = trim(assign_pos + 2);
        
        // Parse key - it can be either a direct identifier or memory["key"]
        char *actual_key = key;
        if (starts_with(key, "memory[\"") && ends_with(key, "\"]")) {
            actual_key = key + 8; // Skip "memory[\"
            actual_key[strlen(actual_key) - 2] = '\0'; // Remove trailing \"]
        }
        
        // Evaluate the value expression
        int offset = 0;
        value_t value = evaluate_expression(agent, message, value_expr, &offset);
        
        // Store in agent's memory
        memory_set(&agent->memory, actual_key, value);
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

// Parse a string literal from the input
static char* parse_string_literal(const char *str, int *offset) {
    if (str[*offset] != '"') {
        return NULL;
    }
    
    (*offset)++; // Skip opening quote
    int start = *offset;
    int len = 0;
    
    // Find the closing quote, handling escapes
    bool escaped = false;
    while (str[*offset] != '\0') {
        if (escaped) {
            escaped = false;
        } else if (str[*offset] == '\\') {
            escaped = true;
        } else if (str[*offset] == '"') {
            break;
        }
        
        (*offset)++;
        len++;
    }
    
    if (str[*offset] != '"') {
        // Unterminated string
        return NULL;
    }
    
    char *result = (char *)malloc(len + 1);
    if (!result) {
        return NULL;
    }
    
    // Copy the string, processing escapes
    int j = 0;
    escaped = false;
    for (int i = 0; i < len; i++) {
        if (escaped) {
            switch (str[start + i]) {
                case 'n': result[j++] = '\n'; break;
                case 't': result[j++] = '\t'; break;
                case 'r': result[j++] = '\r'; break;
                case '\\': result[j++] = '\\'; break;
                case '"': result[j++] = '"'; break;
                default: result[j++] = str[start + i]; break;
            }
            escaped = false;
        } else if (str[start + i] == '\\') {
            escaped = true;
        } else {
            result[j++] = str[start + i];
        }
    }
    
    result[j] = '\0';
    (*offset)++; // Skip closing quote
    
    return result;
}

// Parse an identifier from the input
static char* parse_identifier(const char *str, int *offset) {
    if (!isalpha(str[*offset]) && str[*offset] != '_') {
        return NULL;
    }
    
    int start = *offset;
    int len = 0;
    
    while (isalnum(str[*offset]) || str[*offset] == '_') {
        (*offset)++;
        len++;
    }
    
    char *result = (char *)malloc(len + 1);
    if (!result) {
        return NULL;
    }
    
    strncpy(result, str + start, len);
    result[len] = '\0';
    
    return result;
}

// Skip whitespace in the input
static void skip_whitespace(const char *str, int *offset) {
    while (isspace(str[*offset])) {
        (*offset)++;
    }
}

// Check if a character is an operator
static bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '==' || c == '!=' || 
           c == '<' || c == '>' || c == '<=' || c == '>=';
}

// Get the precedence of an operator
static int get_operator_precedence(char op) {
    switch (op) {
        case '*':
        case '/': return 2;
        case '+':
        case '-': return 1;
        case '=':
        case '!':
        case '<':
        case '>': return 0;
        default: return -1;
    }
}

// Apply an operator to two values
static value_t apply_operator(char op, value_t left, value_t right) {
    value_t result;
    
    // For now, a very simple implementation
    if (left.type == VALUE_INT && right.type == VALUE_INT) {
        result.type = VALUE_INT;
        switch (op) {
            case '+': result.data.int_value = left.data.int_value + right.data.int_value; break;
            case '-': result.data.int_value = left.data.int_value - right.data.int_value; break;
            case '*': result.data.int_value = left.data.int_value * right.data.int_value; break;
            case '/': 
                if (right.data.int_value == 0) {
                    result.data.int_value = 0; // Handle division by zero
                } else {
                    result.data.int_value = left.data.int_value / right.data.int_value;
                }
                break;
            default: result.data.int_value = 0; break;
        }
    } else if (left.type == VALUE_DOUBLE || right.type == VALUE_DOUBLE) {
        result.type = VALUE_DOUBLE;
        double left_val = (left.type == VALUE_INT) ? (double)left.data.int_value : left.data.double_value;
        double right_val = (right.type == VALUE_INT) ? (double)right.data.int_value : right.data.double_value;
        
        switch (op) {
            case '+': result.data.double_value = left_val + right_val; break;
            case '-': result.data.double_value = left_val - right_val; break;
            case '*': result.data.double_value = left_val * right_val; break;
            case '/':
                if (right_val == 0.0) {
                    result.data.double_value = 0.0; // Handle division by zero
                } else {
                    result.data.double_value = left_val / right_val;
                }
                break;
            default: result.data.double_value = 0.0; break;
        }
    } else if (left.type == VALUE_STRING && op == '+' && right.type == VALUE_STRING) {
        // String concatenation
        result.type = VALUE_STRING;
        size_t len1 = strlen(left.data.string_value);
        size_t len2 = strlen(right.data.string_value);
        result.data.string_value = (char *)malloc(len1 + len2 + 1);
        
        if (result.data.string_value) {
            strcpy(result.data.string_value, left.data.string_value);
            strcat(result.data.string_value, right.data.string_value);
        } else {
            // Memory allocation failed
            result.type = VALUE_STRING;
            result.data.string_value = strdup("");
        }
    } else {
        // Unsupported operation
        result.type = VALUE_INT;
        result.data.int_value = 0;
    }
    
    return result;
}

// Check if two values are equal
static bool values_equal(value_t a, value_t b) {
    if (a.type != b.type) {
        return false;
    }
    
    switch (a.type) {
        case VALUE_INT:
            return a.data.int_value == b.data.int_value;
        case VALUE_DOUBLE:
            return a.data.double_value == b.data.double_value;
        case VALUE_STRING:
            return strcmp(a.data.string_value, b.data.string_value) == 0;
        default:
            return false;
    }
}

// Convert a value to a boolean
static bool value_to_bool(value_t val) {
    switch (val.type) {
        case VALUE_INT:
            return val.data.int_value != 0;
        case VALUE_DOUBLE:
            return val.data.double_value != 0.0;
        case VALUE_STRING:
            return val.data.string_value != NULL && strlen(val.data.string_value) > 0;
        default:
            return false;
    }
}

// Make a copy of a value
static value_t copy_value(value_t src) {
    value_t dest;
    dest.type = src.type;
    
    switch (src.type) {
        case VALUE_INT:
            dest.data.int_value = src.data.int_value;
            break;
        case VALUE_DOUBLE:
            dest.data.double_value = src.data.double_value;
            break;
        case VALUE_STRING:
            if (src.data.string_value) {
                dest.data.string_value = strdup(src.data.string_value);
            } else {
                dest.data.string_value = NULL;
            }
            break;
    }
    
    return dest;
}

// Convert a value to a string representation
static void value_to_string(value_t val, char *buffer, size_t buffer_size) {
    switch (val.type) {
        case VALUE_INT:
            snprintf(buffer, buffer_size, "%ld", val.data.int_value);
            break;
        case VALUE_DOUBLE:
            snprintf(buffer, buffer_size, "%f", val.data.double_value);
            break;
        case VALUE_STRING:
            if (val.data.string_value) {
                strncpy(buffer, val.data.string_value, buffer_size - 1);
                buffer[buffer_size - 1] = '\0';
            } else {
                strncpy(buffer, "", buffer_size);
            }
            break;
    }
}

// Evaluate an expression in the agent's context
static value_t evaluate_expression(agent_t *agent, const char *message, const char *expr, int *offset) {
    value_t result;
    result.type = VALUE_INT;
    result.data.int_value = 0;
    
    skip_whitespace(expr, offset);
    
    // Check if expression is a function call
    char *func_name = parse_identifier(expr, offset);
    if (func_name != NULL) {
        skip_whitespace(expr, offset);
        
        if (expr[*offset] == '(') {
            (*offset)++; // Skip opening parenthesis
            
            // Parse arguments
            value_t args[10]; // Maximum 10 arguments
            int arg_count = 0;
            
            while (expr[*offset] != ')' && expr[*offset] != '\0' && arg_count < 10) {
                skip_whitespace(expr, offset);
                
                if (expr[*offset] == ',') {
                    (*offset)++; // Skip comma
                    continue;
                }
                
                args[arg_count] = evaluate_expression(agent, message, expr, offset);
                arg_count++;
                
                skip_whitespace(expr, offset);
            }
            
            if (expr[*offset] == ')') {
                (*offset)++; // Skip closing parenthesis
                
                // Call the function
                result = call_function(agent, message, func_name, args, arg_count);
                
                // Free argument values
                for (int i = 0; i < arg_count; i++) {
                    free_value(&args[i]);
                }
            }
        }
        // Check if it's a variable reference
        else if (strcmp(func_name, "message") == 0) {
            // Special case for the message parameter
            result.type = VALUE_STRING;
            result.data.string_value = strdup(message);
        } else {
            // Look up in memory
            value_t *memory_val = memory_get(&agent->memory, func_name);
            if (memory_val != NULL) {
                result = copy_value(*memory_val);
            } else {
                // Check if it's in context
                memory_val = memory_get(agent->context, func_name);
                if (memory_val != NULL) {
                    result = copy_value(*memory_val);
                } else {
                    // Not found, return empty value
                    result.type = VALUE_INT;
                    result.data.int_value = 0;
                }
            }
        }
        
        free(func_name);
    }
    // Check if it's a string literal
    else if (expr[*offset] == '"') {
        char *str_val = parse_string_literal(expr, offset);
        if (str_val != NULL) {
            result.type = VALUE_STRING;
            result.data.string_value = str_val;
        }
    }
    // Check if it's a number
    else if (isdigit(expr[*offset]) || (expr[*offset] == '-' && isdigit(expr[*offset + 1]))) {
        bool is_negative = expr[*offset] == '-';
        if (is_negative) {
            (*offset)++;
        }
        
        int start = *offset;
        bool has_decimal = false;
        
        while (isdigit(expr[*offset]) || expr[*offset] == '.') {
            if (expr[*offset] == '.') {
                if (has_decimal) {
                    break; // Multiple decimal points not allowed
                }
                has_decimal = true;
            }
            (*offset)++;
        }
        
        int len = *offset - start;
        if (len > 0) {
            char *num_str = (char *)malloc(len + 1);
            if (num_str != NULL) {
                strncpy(num_str, expr + start, len);
                num_str[len] = '\0';
                
                if (has_decimal) {
                    result.type = VALUE_DOUBLE;
                    result.data.double_value = strtod(num_str, NULL);
                    if (is_negative) {
                        result.data.double_value = -result.data.double_value;
                    }
                } else {
                    result.type = VALUE_INT;
                    result.data.int_value = strtol(num_str, NULL, 10);
                    if (is_negative) {
                        result.data.int_value = -result.data.int_value;
                    }
                }
                
                free(num_str);
            }
        }
    }
    // Other cases - parenthesized expressions, etc.
    else if (expr[*offset] == '(') {
        (*offset)++; // Skip opening parenthesis
        result = evaluate_expression(agent, message, expr, offset);
        
        skip_whitespace(expr, offset);
        if (expr[*offset] == ')') {
            (*offset)++; // Skip closing parenthesis
        }
    }
    
    return result;
}

// Call a built-in function
static value_t call_function(agent_t *agent, const char *message, const char *func_name, value_t *args, int arg_count) {
    value_t result;
    result.type = VALUE_INT;
    result.data.int_value = 0;
    
    // Function: send(agent_id, message)
    if (strcmp(func_name, "send") == 0) {
        if (arg_count >= 2 && args[0].type == VALUE_INT) {
            char buffer[1024];
            value_to_string(args[1], buffer, sizeof(buffer));
            
            agent_id_t target_id = args[0].data.int_value;
            bool success = agerun_send(target_id, buffer);
            
            result.type = VALUE_INT;
            result.data.int_value = success ? 1 : 0;
        }
    }
    // Function: parse(template, input)
    else if (strcmp(func_name, "parse") == 0) {
        // Stub implementation for now
        result.type = VALUE_INT;
        result.data.int_value = 0;
    }
    // Function: build(template, values)
    else if (strcmp(func_name, "build") == 0) {
        // Simple implementation - replace {} with string representation of second arg
        if (arg_count >= 2 && args[0].type == VALUE_STRING) {
            char *template = args[0].data.string_value;
            char *placeholder = strstr(template, "{}");
            
            if (placeholder != NULL) {
                char buffer[1024];
                value_to_string(args[1], buffer, sizeof(buffer));
                
                size_t pre_len = placeholder - template;
                size_t total_len = strlen(template) - 2 + strlen(buffer) + 1;
                
                char *new_str = (char *)malloc(total_len);
                if (new_str != NULL) {
                    strncpy(new_str, template, pre_len);
                    new_str[pre_len] = '\0';
                    strcat(new_str, buffer);
                    strcat(new_str, placeholder + 2);
                    
                    result.type = VALUE_STRING;
                    result.data.string_value = new_str;
                }
            } else {
                // No placeholder found, return template as is
                result.type = VALUE_STRING;
                result.data.string_value = strdup(template);
            }
        }
    }
    // Function: if(condition, true_value, false_value)
    else if (strcmp(func_name, "if") == 0) {
        if (arg_count >= 3) {
            bool condition = value_to_bool(args[0]);
            if (condition) {
                result = copy_value(args[1]);
            } else {
                result = copy_value(args[2]);
            }
        }
    }
    // Function: create(method_name, version, context)
    else if (strcmp(func_name, "create") == 0) {
        if (arg_count >= 1 && args[0].type == VALUE_STRING) {
            char *method_name = args[0].data.string_value;
            version_t version = 0;
            
            if (arg_count >= 2 && args[1].type == VALUE_INT) {
                version = args[1].data.int_value;
            }
            
            // For now, just pass NULL as context - will be enhanced later
            agent_id_t new_agent_id = agerun_create(method_name, version, NULL);
            
            result.type = VALUE_INT;
            result.data.int_value = new_agent_id;
        }
    }
    // Function: destroy(agent_id)
    else if (strcmp(func_name, "destroy") == 0) {
        if (arg_count >= 1 && args[0].type == VALUE_INT) {
            agent_id_t agent_id = args[0].data.int_value;
            bool success = agerun_destroy(agent_id);
            
            result.type = VALUE_INT;
            result.data.int_value = success ? 1 : 0;
        }
    }
    // Function: method(name, instructions, previous_version, backward_compatible, persist)
    else if (strcmp(func_name, "method") == 0) {
        if (arg_count >= 2 && args[0].type == VALUE_STRING && args[1].type == VALUE_STRING) {
            char *name = args[0].data.string_value;
            char *instructions = args[1].data.string_value;
            version_t previous_version = 0;
            bool backward_compatible = true;
            bool persist = false;
            
            if (arg_count >= 3 && args[2].type == VALUE_INT) {
                previous_version = args[2].data.int_value;
            }
            
            if (arg_count >= 4) {
                backward_compatible = value_to_bool(args[3]);
            }
            
            if (arg_count >= 5) {
                persist = value_to_bool(args[4]);
            }
            
            version_t new_version = agerun_method(name, instructions, previous_version, backward_compatible, persist);
            
            result.type = VALUE_INT;
            result.data.int_value = new_version;
        }
    }
    
    return result;
}