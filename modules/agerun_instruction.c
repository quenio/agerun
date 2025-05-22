#include "agerun_instruction.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_expression.h"
#include "agerun_map.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_agent.h" // Required only for agent_id_t and ar_agent_send
#include "agerun_assert.h" // Include for ownership assertions
#include "agerun_heap.h" // Include for memory allocation macros

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Instruction context structure definition
struct instruction_context_s {
    data_t *mut_memory;        // Mutable reference to memory, not owned
    const data_t *ref_context; // Borrowed reference to context, not owned
    const data_t *ref_message; // Borrowed reference to message, not owned
};

// Function prototypes for recursive descent parsing
static bool parse_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static bool parse_assignment(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static bool parse_function_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos);
static bool parse_memory_access(const char *ref_instruction, int *mut_pos, char **path);
static bool parse_function_call(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos, data_t **result);
static bool skip_whitespace(const char *ref_instruction, int *mut_pos);
static bool extract_identifier(const char *ref_instruction, int *mut_pos, char *mut_identifier, int max_size);

// Create a new instruction context
instruction_context_t* ar_instruction_create_context(data_t *mut_memory, const data_t *ref_context, const data_t *ref_message) {
    // Allocate memory for the context
    instruction_context_t *own_ctx = (instruction_context_t*)AR_HEAP_MALLOC(sizeof(instruction_context_t), "Instruction context");
    if (!own_ctx) {
        return NULL;
    }
    
    // Initialize the context
    own_ctx->mut_memory = mut_memory;
    own_ctx->ref_context = ref_context;
    own_ctx->ref_message = ref_message;
    
    // Return the newly created context
    return own_ctx;
}

// Destroy an instruction context
void ar_instruction_destroy_context(instruction_context_t *own_ctx) {
    if (own_ctx) {
        // The context doesn't own memory, context, or message, so we just free the structure
        AR_HEAP_FREE(own_ctx);
    }
}

// Get the memory from the instruction context
data_t* ar_instruction_get_memory(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->mut_memory;
}

// Get the context data from the instruction context
const data_t* ar_instruction_get_context(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->ref_context;
}

// Get the message from the instruction context
const data_t* ar_instruction_get_message(const instruction_context_t *ref_ctx) {
    if (!ref_ctx) {
        return NULL;
    }
    return ref_ctx->ref_message;
}

// Send a message to another agent
bool ar_instruction_send_message(agent_id_t target_id, data_t *own_message) {
    if (target_id == 0) {
        // Special case: agent_id 0 is a no-op that always returns true
        ar_data_destroy(own_message);
        return true;
    }
    
    // Send message (ownership of own_message is transferred to ar_agent_send)
    return ar_agent_send(target_id, own_message);
}

// Parse and execute a single instruction
bool ar_instruction_run(instruction_context_t *mut_ctx, const char *ref_instruction) {
    if (!mut_ctx || !ref_instruction) {
        return false;
    }
    
    printf("DEBUG: Running instruction: %s\n", ref_instruction);
    
    int pos = 0;
    bool result = parse_instruction(mut_ctx, ref_instruction, &pos);
    
    printf("DEBUG: Instruction result: %s\n", result ? "success" : "failure");
    
    return result;
}

// <instruction> ::= <assignment> | <function-instruction>
static bool parse_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
    skip_whitespace(ref_instruction, mut_pos);
    
    // Check for assignment or function instruction
    // Save the current position to backtrack if needed
    int save_pos = *mut_pos;
    
    // Try to parse as assignment first
    if (parse_assignment(mut_ctx, ref_instruction, mut_pos)) {
        return true;
    }
    
    // Backtrack and try as function instruction
    *mut_pos = save_pos;
    return parse_function_instruction(mut_ctx, ref_instruction, mut_pos);
}

// <assignment> ::= <memory-access> ':=' <expression>
static bool parse_assignment(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
    char *path = NULL;
    
    // Parse memory access (left side)
    if (!parse_memory_access(ref_instruction, mut_pos, &path)) {
        return false;
    }
    
    skip_whitespace(ref_instruction, mut_pos);
    
    // Check for ':=' operator
    if (ref_instruction[*mut_pos] != ':' || ref_instruction[*mut_pos + 1] != '=') {
        AR_HEAP_FREE(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    *mut_pos += 2; // Skip ':='
    skip_whitespace(ref_instruction, mut_pos);
    
    // Evaluate the expression (right side)
    // Create a context that we'll reuse for all expressions in this function
    expression_context_t *own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                                                mut_ctx->ref_context, 
                                                                mut_ctx->ref_message, 
                                                                ref_instruction + *mut_pos);
    if (!own_context) {
        AR_HEAP_FREE(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    // Evaluate the expression
    data_t *own_value = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
    *mut_pos += ar_expression_offset(own_context);
    
    // Clean up context immediately after we're done with it
    ar_expression_destroy_context(own_context);
    own_context = NULL; // Mark as destroyed
    
    if (!own_value) {
        // If evaluation or take_ownership returned NULL, we can't use the value
        AR_HEAP_FREE(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    // Store result in agent's memory (transfers ownership of value)
    bool success = ar_data_set_map_data(mut_ctx->mut_memory, path, own_value);
    if (!success) {
        ar_data_destroy(own_value);
    }
    own_value = NULL; // Mark as transferred
    
    AR_HEAP_FREE(path);
    path = NULL; // Mark as freed
    return true;
}

// <function-instruction> ::= [<memory-access> ':='] <function-call>
static bool parse_function_instruction(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos) {
    char *path = NULL;
    data_t *own_result = NULL;
    bool has_assignment = false;
    
    // Check if there's an assignment part
    int save_pos = *mut_pos;
    if (parse_memory_access(ref_instruction, mut_pos, &path)) {
        skip_whitespace(ref_instruction, mut_pos);
        
        if (ref_instruction[*mut_pos] == ':' && ref_instruction[*mut_pos + 1] == '=') {
            has_assignment = true;
            *mut_pos += 2; // Skip ':='
            skip_whitespace(ref_instruction, mut_pos);
        } else {
            // Not an assignment, backtrack
            *mut_pos = save_pos;
            AR_HEAP_FREE(path);
            path = NULL; // Mark as freed
        }
    } else {
        // Not a memory access, backtrack
        *mut_pos = save_pos;
    }
    
    // Parse function call
    if (!parse_function_call(mut_ctx, ref_instruction, mut_pos, &own_result)) {
        AR_HEAP_FREE(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    // Store result in memory if assignment was present
    if (has_assignment && path && own_result) {
        // We're taking ownership of the result
        bool success = ar_data_set_map_data(mut_ctx->mut_memory, path, own_result);
        if (!success) {
            ar_data_destroy(own_result);
        }
        own_result = NULL; // Mark as transferred
    } else if (own_result) {
        // No assignment, discard the result
        ar_data_destroy(own_result);
        own_result = NULL; // Mark as destroyed
    }
    
    AR_HEAP_FREE(path);
    path = NULL; // Mark as freed
    return true;
}

// <memory-access> ::= 'memory' {'.' <identifier>}
// Note: According to the spec, in assignments, only 'memory' paths can be used on the left side
static bool parse_memory_access(const char *ref_instruction, int *mut_pos, char **path) {
    skip_whitespace(ref_instruction, mut_pos);
    
    // Check for 'memory' identifier
    const char *memory_str = "memory";
    size_t memory_len = strlen(memory_str);
    
    if (strncmp(ref_instruction + *mut_pos, memory_str, memory_len) != 0) {
        return false;
    }
    
    // Make sure the 'memory' is a complete token
    if (isalnum((unsigned char)ref_instruction[*mut_pos + (int)memory_len]) || 
        ref_instruction[*mut_pos + (int)memory_len] == '_') {
        return false;
    }
    
    *mut_pos += (int)memory_len;
    
    // Start building the path
    char buffer[1024] = "";
    int buffer_pos = 0;
    
    // Check for dot-separated identifiers
    while (ref_instruction[*mut_pos] == '.') {
        (*mut_pos)++; // Skip '.'
        
        char identifier[256];
        if (!extract_identifier(ref_instruction, mut_pos, identifier, sizeof(identifier))) {
            return false;
        }
        
        // Add to buffer
        if (buffer_pos > 0) {
            buffer[buffer_pos++] = '.';
        }
        // Use safe string copy with bounds checking
        size_t id_len = strlen(identifier);
        size_t remaining_space = sizeof(buffer) - (size_t)buffer_pos;
        
        if (id_len < remaining_space) {
            strncpy(buffer + buffer_pos, identifier, remaining_space - 1);
            size_t index = (size_t)buffer_pos + id_len;
            buffer[index] = '\0';
            buffer_pos += (int)id_len;
        } else {
            // Not enough space - truncate safely
            strncpy(buffer + buffer_pos, identifier, remaining_space - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            buffer_pos = (int)sizeof(buffer) - 1;
        }
    }
    
    // Allocate and return the path
    *path = AR_HEAP_STRDUP(buffer, "Memory path");
    return *path != NULL;
}

// Parse function call and execute it
// <function-call> ::= <send-function> | <parse-function> | <build-function> | <method-function> |
//                     <agent-function> | <destroy-function> | <if-function>
static bool parse_function_call(instruction_context_t *mut_ctx, const char *ref_instruction, int *mut_pos, data_t **own_result) {
    // Extract function name
    char function_name[32];
    if (!extract_identifier(ref_instruction, mut_pos, function_name, sizeof(function_name))) {
        return false;
    }
    
    skip_whitespace(ref_instruction, mut_pos);
    
    // Expect opening parenthesis
    if (ref_instruction[*mut_pos] != '(') {
        return false;
    }
    (*mut_pos)++; // Skip '('
    
    // Initialize result
    *own_result = NULL;
    
    // Handle different function types
    if (strcmp(function_name, "send") == 0) {
        // send(agent_id, message)
        skip_whitespace(ref_instruction, mut_pos);
        
        // Create a single context for all expressions
        expression_context_t *own_context = NULL;
        
        // Parse agent_id expression
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        data_t *own_agent_id = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_agent_id) {
            return false;
        }
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(own_agent_id);
            own_agent_id = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse message expression - reusing the context variable
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            ar_data_destroy(own_agent_id);
            own_agent_id = NULL; // Mark as destroyed
            return false;
        }
        data_t *own_msg = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_msg) {
            ar_data_destroy(own_agent_id);
            own_agent_id = NULL; // Mark as destroyed
            return false;
        }
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar_data_destroy(own_agent_id);
            own_agent_id = NULL; // Mark as destroyed
            ar_data_destroy(own_msg);
            own_msg = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Extract agent_id
        agent_id_t target_id = 0;
        if (ar_data_get_type(own_agent_id) == DATA_INTEGER) {
            target_id = (agent_id_t)ar_data_get_integer(own_agent_id);
        }
        
        // Send message
        bool success = false;
        if (target_id == 0) {
            // Special case: agent_id 0 is a no-op that always returns true
            success = true;
            
            // Only destroy the message data if it's not the original message
            // This prevents trying to free the shared message
            if (own_msg != mut_ctx->ref_message) {
                ar_data_destroy(own_msg);
                own_msg = NULL; // Mark as destroyed
            }
        } else {
            // Ownership of own_msg is transferred to ar_agent_send
            success = ar_instruction_send_message(target_id, own_msg);
            own_msg = NULL; // Mark as transferred
        }
        
        ar_data_destroy(own_agent_id);
        own_agent_id = NULL; // Mark as destroyed
        
        // Create a new result (we own it from creation)
        *own_result = ar_data_create_integer(success ? 1 : 0);
        return true;
    }
    else if (strcmp(function_name, "if") == 0) {
        // if(condition, true_value, false_value)
        skip_whitespace(ref_instruction, mut_pos);
        
        // Create a single context for all expressions
        expression_context_t *own_context = NULL;
        
        // Parse condition expression
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        data_t *own_cond = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_cond) {
            return false;
        }
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse true_value expression - reusing the context variable
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            ar_data_destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            return false;
        }
        data_t *own_true = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_true) {
            ar_data_destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            return false;
        }
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            ar_data_destroy(own_true);
            own_true = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse false_value expression - reusing the context variable
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            ar_data_destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            ar_data_destroy(own_true);
            own_true = NULL; // Mark as destroyed
            return false;
        }
        data_t *own_false = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_false) {
            ar_data_destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            ar_data_destroy(own_true);
            own_true = NULL; // Mark as destroyed
            return false;
        }
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar_data_destroy(own_cond);
            own_cond = NULL; // Mark as destroyed
            ar_data_destroy(own_true);
            own_true = NULL; // Mark as destroyed
            ar_data_destroy(own_false);
            own_false = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Evaluate condition
        bool condition = false;
        data_type_t cond_type = ar_data_get_type(own_cond);
        
        if (cond_type == DATA_INTEGER) {
            condition = (ar_data_get_integer(own_cond) != 0);
        } else if (cond_type == DATA_DOUBLE) {
            condition = (ar_data_get_double(own_cond) != 0.0);
        } else if (cond_type == DATA_STRING) {
            const char *str = ar_data_get_string(own_cond);
            condition = (str && *str); // True if non-empty string
        }
        
        // Select the result based on condition
        if (condition) {
            // We're taking ownership of own_true
            *own_result = own_true;
            ar_data_destroy(own_false);
            own_false = NULL; // Mark as destroyed
        } else {
            // We're taking ownership of own_false
            *own_result = own_false;
            ar_data_destroy(own_true);
            own_true = NULL; // Mark as destroyed
        }
        
        ar_data_destroy(own_cond);
        own_cond = NULL; // Mark as destroyed
        return true;
    }
    else if (strcmp(function_name, "parse") == 0) {
        // parse(template, input)
        skip_whitespace(ref_instruction, mut_pos);
        
        // Create a single context for all expressions
        expression_context_t *own_context = NULL;
        
        // Parse template expression
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        data_t *own_template = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_template) {
            return false;
        }
        
        // Ensure template is a string
        if (ar_data_get_type(own_template) != DATA_STRING) {
            ar_data_destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        const char *template_str = ar_data_get_string(own_template);
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse input expression - reusing the context variable
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            ar_data_destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        data_t *own_input = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_input) {
            ar_data_destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        
        // Ensure input is a string
        if (ar_data_get_type(own_input) != DATA_STRING) {
            ar_data_destroy(own_input);
            own_input = NULL; // Mark as destroyed
            ar_data_destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        const char *input_str = ar_data_get_string(own_input);
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar_data_destroy(own_input);
            own_input = NULL; // Mark as destroyed
            ar_data_destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Create result map (owned by us)
        *own_result = ar_data_create_map();
        if (!*own_result) {
            ar_data_destroy(own_input);
            own_input = NULL; // Mark as destroyed
            ar_data_destroy(own_template);
            own_template = NULL; // Mark as destroyed
            return false;
        }
        
        // Parse the template and input to extract values
        // Template format: "key1={var1}, key2={var2}"
        // Input format: "key1=value1, key2=value2"
        
        // Simple implementation: look for {variable} patterns in template
        const char *template_ptr = template_str;
        const char *input_ptr = input_str;
        
        while (*template_ptr && *input_ptr) {
            // Look for placeholder start
            const char *placeholder_start = strchr(template_ptr, '{');
            if (!placeholder_start) {
                // No more placeholders, check if remaining template matches input
                if (strcmp(template_ptr, input_ptr) != 0) {
                    // Mismatch - parsing failed, return empty map
                    ar_data_destroy(*own_result);
                    *own_result = ar_data_create_map();
                }
                break;
            }
            
            // Check if the literal part before placeholder matches
            size_t literal_len = (size_t)(placeholder_start - template_ptr);
            if (strncmp(template_ptr, input_ptr, literal_len) != 0) {
                // Mismatch - parsing failed, return empty map
                ar_data_destroy(*own_result);
                *own_result = ar_data_create_map();
                break;
            }
            
            // Move past the literal part
            template_ptr = placeholder_start + 1; // Skip '{'
            input_ptr += literal_len;
            
            // Find the end of the placeholder
            const char *placeholder_end = strchr(template_ptr, '}');
            if (!placeholder_end) {
                // Invalid template - no closing brace
                ar_data_destroy(*own_result);
                *own_result = ar_data_create_map();
                break;
            }
            
            // Extract variable name
            size_t var_len = (size_t)(placeholder_end - template_ptr);
            char *var_name = (char*)AR_HEAP_MALLOC(var_len + 1, "Parse variable name");
            if (!var_name) {
                ar_data_destroy(*own_result);
                *own_result = ar_data_create_map();
                break;
            }
            strncpy(var_name, template_ptr, var_len);
            var_name[var_len] = '\0';
            
            // Move past the placeholder
            template_ptr = placeholder_end + 1; // Skip '}'
            
            // Find the next literal part or end of template
            const char *next_literal_start = template_ptr;
            const char *next_placeholder = strchr(template_ptr, '{');
            size_t next_literal_len = next_placeholder ? 
                (size_t)(next_placeholder - next_literal_start) : strlen(next_literal_start);
            
            // Extract the value from input
            const char *value_end = NULL;
            if (next_literal_len > 0) {
                // Look for the next literal part in the input
                value_end = strstr(input_ptr, next_literal_start);
                if (!value_end || (next_placeholder && value_end > input_ptr + strlen(input_ptr) - next_literal_len)) {
                    value_end = NULL;
                }
            } else {
                // No more literals, take the rest of the input
                value_end = input_ptr + strlen(input_ptr);
            }
            
            if (value_end) {
                // Extract the value
                size_t value_len = (size_t)(value_end - input_ptr);
                char *value_str = (char*)AR_HEAP_MALLOC(value_len + 1, "Parse value string");
                if (value_str) {
                    strncpy(value_str, input_ptr, value_len);
                    value_str[value_len] = '\0';
                    
                    // Try to parse as integer first, then double, then string
                    data_t *own_value = NULL;
                    char *endptr;
                    
                    // Try integer
                    long int_val = strtol(value_str, &endptr, 10);
                    if (*endptr == '\0' && value_str[0] != '\0') {
                        own_value = ar_data_create_integer((int)int_val);
                    } else {
                        // Try double
                        double double_val = strtod(value_str, &endptr);
                        if (*endptr == '\0' && value_str[0] != '\0' && strchr(value_str, '.')) {
                            own_value = ar_data_create_double(double_val);
                        } else {
                            // Use as string
                            own_value = ar_data_create_string(value_str);
                        }
                    }
                    
                    // Store in result map
                    if (own_value) {
                        ar_data_set_map_data(*own_result, var_name, own_value);
                        // Ownership of own_value is transferred
                    }
                    
                    AR_HEAP_FREE(value_str);
                }
                
                // Move input pointer past the value
                input_ptr = value_end;
            } else {
                // Could not find the next literal - parsing failed
                ar_data_destroy(*own_result);
                *own_result = ar_data_create_map();
                AR_HEAP_FREE(var_name);
                break;
            }
            
            AR_HEAP_FREE(var_name);
        }
        
        // Clean up input data
        ar_data_destroy(own_input);
        own_input = NULL; // Mark as destroyed
        ar_data_destroy(own_template);
        own_template = NULL; // Mark as destroyed
        
        return true;
    }
    else if (strcmp(function_name, "method") == 0) {
        // method(name, instructions, version)
        skip_whitespace(ref_instruction, mut_pos);
        
        // Create a single context for all expressions
        expression_context_t *own_context = NULL;
        
        // Parse method name expression
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            return false;
        }
        data_t *own_name = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_name) {
            return false;
        }
        
        // Ensure name is a string
        if (ar_data_get_type(own_name) != DATA_STRING) {
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        const char *method_name = ar_data_get_string(own_name);
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse instructions expression - reusing the context variable
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        data_t *own_instr = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_instr) {
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        
        // Ensure instructions are a string
        if (ar_data_get_type(own_instr) != DATA_STRING) {
            ar_data_destroy(own_instr);
            own_instr = NULL; // Mark as destroyed
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        const char *instructions = ar_data_get_string(own_instr);
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(own_instr);
            own_instr = NULL; // Mark as destroyed
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse version expression - reusing the context variable
        own_context = ar_expression_create_context(mut_ctx->mut_memory, 
                                              mut_ctx->ref_context, 
                                              mut_ctx->ref_message, 
                                              ref_instruction + *mut_pos);
        if (!own_context) {
            ar_data_destroy(own_instr);
            own_instr = NULL; // Mark as destroyed
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        data_t *own_version = ar_expression_take_ownership(own_context, ar_expression_evaluate(own_context));
        *mut_pos += ar_expression_offset(own_context);
        
        // Clean up context immediately
        ar_expression_destroy_context(own_context);
        own_context = NULL; // Mark as destroyed
        
        if (!own_version) {
            ar_data_destroy(own_instr);
            own_instr = NULL; // Mark as destroyed
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar_data_destroy(own_version);
            own_version = NULL; // Mark as destroyed
            ar_data_destroy(own_instr);
            own_instr = NULL; // Mark as destroyed
            ar_data_destroy(own_name);
            own_name = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Extract version string (default to "1.0.0" if not a valid string)
        const char *version_str = "1.0.0";
        if (ar_data_get_type(own_version) == DATA_STRING) {
            version_str = ar_data_get_string(own_version);
        } else if (ar_data_get_type(own_version) == DATA_INTEGER) {
            // If version is provided as a number, convert it to a string "X.0.0"
            static char version_buffer[16]; // Buffer for conversion
            snprintf(version_buffer, sizeof(version_buffer), "%d.0.0", ar_data_get_integer(own_version));
            version_str = version_buffer;
        }
        
        // Call methodology module directly to create method with just 3 parameters:
        // name, instructions, version
        bool success = ar_methodology_create_method(method_name, instructions, version_str);
        
        // Clean up input data now that we're done with it
        ar_data_destroy(own_version);
        own_version = NULL; // Mark as destroyed
        ar_data_destroy(own_instr);
        own_instr = NULL; // Mark as destroyed
        ar_data_destroy(own_name);
        own_name = NULL; // Mark as destroyed
        
        // Return success indicator
        *own_result = ar_data_create_integer(success ? 1 : 0);
        return true;
    }
    else {
        // For all other functions (build, agent, destroy),
        // just return a default result for now
        // Skip to closing parenthesis
        int nesting = 1;
        while (ref_instruction[*mut_pos] && nesting > 0) {
            if (ref_instruction[*mut_pos] == '(') {
                nesting++;
            } else if (ref_instruction[*mut_pos] == ')') {
                nesting--;
            }
            (*mut_pos)++;
        }
        
        // Create a default result (we own it from creation)
        *own_result = ar_data_create_integer(0);
        return true;
    }
    
    return false; // Should not reach here
}

// Utility functions

static bool skip_whitespace(const char *ref_instruction, int *mut_pos) {
    while (ref_instruction[*mut_pos] && isspace((unsigned char)ref_instruction[*mut_pos])) {
        (*mut_pos)++;
    }
    return true;
}

static bool extract_identifier(const char *ref_instruction, int *mut_pos, char *mut_identifier, int max_size) {
    skip_whitespace(ref_instruction, mut_pos);
    
    int i = 0;
    // First character must be a letter
    if (!isalpha((unsigned char)ref_instruction[*mut_pos])) {
        return false;
    }
    
    // Extract the identifier
    while (i < max_size - 1 && 
           (isalnum((unsigned char)ref_instruction[*mut_pos]) || ref_instruction[*mut_pos] == '_')) {
        mut_identifier[i++] = ref_instruction[*mut_pos];
        (*mut_pos)++;
    }
    
    mut_identifier[i] = '\0';
    return i > 0;
}
