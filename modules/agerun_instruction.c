#include "agerun_instruction.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_expression.h"
#include "agerun_map.h"
#include "agerun_method.h"
#include "agerun_methodology.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Function prototypes for recursive descent parsing
static bool parse_instruction(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos);
static bool parse_assignment(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos);
static bool parse_function_instruction(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos);
static bool parse_memory_access(const char *ref_instruction, int *mut_pos, char **path);
static bool parse_function_call(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos, data_t **result);
static bool skip_whitespace(const char *ref_instruction, int *mut_pos);
static bool extract_identifier(const char *ref_instruction, int *mut_pos, char *mut_identifier, int max_size);

// Parse and execute a single instruction
bool ar_instruction_run(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction) {
    if (!mut_agent || !ref_instruction) {
        return false;
    }
    
    printf("DEBUG: Running instruction: %s\n", ref_instruction);
    
    int pos = 0;
    bool result = parse_instruction(mut_agent, ref_message, ref_instruction, &pos);
    
    printf("DEBUG: Instruction result: %s\n", result ? "success" : "failure");
    
    return result;
}

// <instruction> ::= <assignment> | <function-instruction>
static bool parse_instruction(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos) {
    skip_whitespace(ref_instruction, mut_pos);
    
    // Check for assignment or function instruction
    // Save the current position to backtrack if needed
    int save_pos = *mut_pos;
    
    // Try to parse as assignment first
    if (parse_assignment(mut_agent, ref_message, ref_instruction, mut_pos)) {
        return true;
    }
    
    // Backtrack and try as function instruction
    *mut_pos = save_pos;
    return parse_function_instruction(mut_agent, ref_message, ref_instruction, mut_pos);
}

// <assignment> ::= <memory-access> ':=' <expression>
static bool parse_assignment(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos) {
    char *path = NULL;
    
    // Parse memory access (left side)
    if (!parse_memory_access(ref_instruction, mut_pos, &path)) {
        return false;
    }
    
    skip_whitespace(ref_instruction, mut_pos);
    
    // Check for ':=' operator
    if (ref_instruction[*mut_pos] != ':' || ref_instruction[*mut_pos + 1] != '=') {
        free(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    *mut_pos += 2; // Skip ':='
    skip_whitespace(ref_instruction, mut_pos);
    
    // Evaluate the expression (right side)
    // Create a context that we'll reuse for all expressions in this function
    expression_context_t *own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
    if (!own_context) {
        free(path);
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
        free(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    // Store result in agent's memory (transfers ownership of value)
    bool success = ar_data_set_map_data(mut_agent->own_memory, path, own_value);
    if (!success) {
        ar_data_destroy(own_value);
    }
    own_value = NULL; // Mark as transferred
    
    free(path);
    path = NULL; // Mark as freed
    return true;
}

// <function-instruction> ::= [<memory-access> ':='] <function-call>
static bool parse_function_instruction(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos) {
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
            free(path);
            path = NULL; // Mark as freed
        }
    } else {
        // Not a memory access, backtrack
        *mut_pos = save_pos;
    }
    
    // Parse function call
    if (!parse_function_call(mut_agent, ref_message, ref_instruction, mut_pos, &own_result)) {
        free(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    // Store result in memory if assignment was present
    if (has_assignment && path && own_result) {
        // We're taking ownership of the result
        bool success = ar_data_set_map_data(mut_agent->own_memory, path, own_result);
        if (!success) {
            ar_data_destroy(own_result);
        }
        own_result = NULL; // Mark as transferred
    } else if (own_result) {
        // No assignment, discard the result
        ar_data_destroy(own_result);
        own_result = NULL; // Mark as destroyed
    }
    
    free(path);
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
        strcpy(buffer + buffer_pos, identifier);
        buffer_pos += (int)strlen(identifier);
    }
    
    // Allocate and return the path
    *path = strdup(buffer);
    return *path != NULL;
}

// Parse function call and execute it
// <function-call> ::= <send-function> | <parse-function> | <build-function> | <method-function> |
//                     <agent-function> | <destroy-function> | <if-function>
static bool parse_function_call(agent_t *mut_agent, const data_t *ref_message, const char *ref_instruction, int *mut_pos, data_t **own_result) {
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
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
            if (own_msg != ref_message) {
                ar_data_destroy(own_msg);
                own_msg = NULL; // Mark as destroyed
            }
        } else {
            // Ownership of own_msg is transferred to ar_agent_send
            success = ar_agent_send(target_id, own_msg);
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
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
    else if (strcmp(function_name, "method") == 0) {
        // method(name, instructions, version)
        skip_whitespace(ref_instruction, mut_pos);
        
        // Create a single context for all expressions
        expression_context_t *own_context = NULL;
        
        // Parse method name expression
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
        own_context = ar_expression_create_context(mut_agent->own_memory, mut_agent->ref_context, ref_message, ref_instruction + *mut_pos);
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
        
        // Extract version information (default to version 1 if not a valid number)
        version_t version = 1;
        if (ar_data_get_type(own_version) == DATA_INTEGER) {
            version = (version_t)ar_data_get_integer(own_version);
        } else if (ar_data_get_type(own_version) == DATA_STRING) {
            // Try to parse version string if provided as string
            // For simplicity, we'll just use the first number found in the string
            const char *ver_str = ar_data_get_string(own_version);
            if (ver_str) {
                version = (version_t)atoi(ver_str);
            }
        }
        
        // Set default values for backward compatibility and persistence
        // According to the spec, we're not taking these as parameters anymore
        version_t previous_version = 0;  // Default to 0 for first version
        bool backward_compatible = true; // Default to true for backward compatibility
        bool persist = false;            // Default to false for persistence
        
        // Create method with specified version (not auto-versioning)
        method_t *own_method = ar_method_create(method_name, instructions, version, previous_version, 
                                          backward_compatible, persist);
        
        // Clean up input data
        ar_data_destroy(own_version);
        own_version = NULL; // Mark as destroyed
        ar_data_destroy(own_instr);
        own_instr = NULL; // Mark as destroyed
        ar_data_destroy(own_name);
        own_name = NULL; // Mark as destroyed
        
        // Check if method creation was successful
        if (!own_method) {
            // Method creation failed
            *own_result = ar_data_create_integer(0);
            return true;
        }
        
        // Register the method with methodology
        extern void ar_methodology_register_method(method_t *own_method);
        ar_methodology_register_method(own_method);
        own_method = NULL; // Mark as transferred
        
        // Return success indicator
        *own_result = ar_data_create_integer(1);
        return true;
    }
    else {
        // For all other functions (parse, build, agent, destroy),
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
