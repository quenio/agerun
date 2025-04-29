#include "agerun_instruction.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_expression.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Function prototypes for recursive descent parsing
static bool parse_instruction(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos);
static bool parse_assignment(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos);
static bool parse_function_instruction(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos);
static bool parse_memory_access(const char *ref_instruction, int *mut_pos, char **path);
static bool parse_function_call(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos, data_t **result);
static bool skip_whitespace(const char *ref_instruction, int *mut_pos);
static bool extract_identifier(const char *ref_instruction, int *mut_pos, char *mut_identifier, int max_size);

// Parse and execute a single instruction
bool ar_instruction_run(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction) {
    if (!mut_agent || !ref_instruction) {
        return false;
    }
    
    printf("DEBUG: Running instruction: %s\n", ref_instruction);
    
    int pos = 0;
    bool result = parse_instruction(mut_agent, mut_message, ref_instruction, &pos);
    
    printf("DEBUG: Instruction result: %s\n", result ? "success" : "failure");
    
    return result;
}

// <instruction> ::= <assignment> | <function-instruction>
static bool parse_instruction(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos) {
    skip_whitespace(ref_instruction, mut_pos);
    
    // Check for assignment or function instruction
    // Save the current position to backtrack if needed
    int save_pos = *mut_pos;
    
    // Try to parse as assignment first
    if (parse_assignment(mut_agent, mut_message, ref_instruction, mut_pos)) {
        return true;
    }
    
    // Backtrack and try as function instruction
    *mut_pos = save_pos;
    return parse_function_instruction(mut_agent, mut_message, ref_instruction, mut_pos);
}

// <assignment> ::= <memory-access> ':=' <expression>
static bool parse_assignment(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos) {
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
    expression_context_t *ctx = ar_expression_create_context(mut_agent->memory, mut_agent->context, mut_message, ref_instruction + *mut_pos);
    if (!ctx) {
        free(path);
        path = NULL; // Mark as freed
        return false;
    }
    data_t *value = ar_expression_evaluate(ctx);
    *mut_pos += ar_expression_offset(ctx);
    
    if (!value) {
        ar_expression_destroy_context(ctx);
        ctx = NULL; // Mark as destroyed
        free(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    // Take ownership of the value from the expression context before destroying it
    ar_expression_take_ownership(ctx, value);
    ar_expression_destroy_context(ctx);
    ctx = NULL; // Mark as destroyed
    
    // Store result in agent's memory (transfers ownership of value)
    bool success = ar_data_set_map_data(mut_agent->memory, path, value);
    if (!success) {
        ar_data_destroy(value);
    }
    value = NULL; // Mark as transferred
    
    free(path);
    path = NULL; // Mark as freed
    return true;
}

// <function-instruction> ::= [<memory-access> ':='] <function-call>
static bool parse_function_instruction(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos) {
    char *path = NULL;
    data_t *result = NULL;
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
    if (!parse_function_call(mut_agent, mut_message, ref_instruction, mut_pos, &result)) {
        free(path);
        path = NULL; // Mark as freed
        return false;
    }
    
    // Store result in memory if assignment was present
    if (has_assignment && path && result) {
        // We're taking ownership of the result
        bool success = ar_data_set_map_data(mut_agent->memory, path, result);
        if (!success) {
            ar_data_destroy(result);
        }
        result = NULL; // Mark as transferred
    } else if (result) {
        // No assignment, discard the result
        ar_data_destroy(result);
        result = NULL; // Mark as destroyed
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
static bool parse_function_call(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction, int *mut_pos, data_t **result) {
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
    *result = NULL;
    
    // Handle different function types
    if (strcmp(function_name, "send") == 0) {
        // send(agent_id, message)
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse agent_id expression
        expression_context_t *agent_id_ctx = ar_expression_create_context(mut_agent->memory, mut_agent->context, mut_message, ref_instruction + *mut_pos);
        if (!agent_id_ctx) {
            return false;
        }
        data_t *agent_id_data = ar_expression_evaluate(agent_id_ctx);
        *mut_pos += ar_expression_offset(agent_id_ctx);
        
        if (!agent_id_data) {
            ar_expression_destroy_context(agent_id_ctx);
            agent_id_ctx = NULL; // Mark as destroyed
            return false;
        }
        
        // Take ownership of agent_id_data before destroying the context
        ar_expression_take_ownership(agent_id_ctx, agent_id_data);
        ar_expression_destroy_context(agent_id_ctx);
        agent_id_ctx = NULL; // Mark as destroyed
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(agent_id_data);
            agent_id_data = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse message expression
        expression_context_t *msg_ctx = ar_expression_create_context(mut_agent->memory, mut_agent->context, mut_message, ref_instruction + *mut_pos);
        if (!msg_ctx) {
            ar_data_destroy(agent_id_data);
            agent_id_data = NULL; // Mark as destroyed
            return false;
        }
        data_t *msg_data = ar_expression_evaluate(msg_ctx);
        *mut_pos += ar_expression_offset(msg_ctx);
        
        if (!msg_data) {
            ar_expression_destroy_context(msg_ctx);
            msg_ctx = NULL; // Mark as destroyed
            ar_data_destroy(agent_id_data);
            agent_id_data = NULL; // Mark as destroyed
            return false;
        }
        
        // Take ownership of msg_data before destroying the context
        ar_expression_take_ownership(msg_ctx, msg_data);
        ar_expression_destroy_context(msg_ctx);
        msg_ctx = NULL; // Mark as destroyed
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar_data_destroy(agent_id_data);
            agent_id_data = NULL; // Mark as destroyed
            ar_data_destroy(msg_data);
            msg_data = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Extract agent_id
        agent_id_t target_id = 0;
        if (ar_data_get_type(agent_id_data) == DATA_INTEGER) {
            target_id = (agent_id_t)ar_data_get_integer(agent_id_data);
        }
        
        // Send message
        bool success = false;
        if (target_id == 0) {
            // Special case: agent_id 0 is a no-op that always returns true
            success = true;
            
            // Only destroy the message data if it's not the original message
            // This prevents trying to free the shared message
            if (msg_data != mut_message) {
                ar_data_destroy(msg_data);
                msg_data = NULL; // Mark as destroyed
            }
        } else {
            // Ownership of msg_data is transferred to ar_agent_send
            success = ar_agent_send(target_id, msg_data);
            msg_data = NULL; // Mark as transferred
        }
        
        ar_data_destroy(agent_id_data);
        agent_id_data = NULL; // Mark as destroyed
        
        // Create a new result (we own it from creation)
        *result = ar_data_create_integer(success ? 1 : 0);
        return true;
    }
    else if (strcmp(function_name, "if") == 0) {
        // if(condition, true_value, false_value)
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse condition expression
        expression_context_t *cond_ctx = ar_expression_create_context(mut_agent->memory, mut_agent->context, mut_message, ref_instruction + *mut_pos);
        if (!cond_ctx) {
            return false;
        }
        data_t *cond_data = ar_expression_evaluate(cond_ctx);
        *mut_pos += ar_expression_offset(cond_ctx);
        
        if (!cond_data) {
            ar_expression_destroy_context(cond_ctx);
            cond_ctx = NULL; // Mark as destroyed
            return false;
        }
        
        // Take ownership of the condition data
        ar_expression_take_ownership(cond_ctx, cond_data);
        ar_expression_destroy_context(cond_ctx);
        cond_ctx = NULL; // Mark as destroyed
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(cond_data);
            cond_data = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse true_value expression
        expression_context_t *true_ctx = ar_expression_create_context(mut_agent->memory, mut_agent->context, mut_message, ref_instruction + *mut_pos);
        if (!true_ctx) {
            ar_data_destroy(cond_data);
            cond_data = NULL; // Mark as destroyed
            return false;
        }
        data_t *true_data = ar_expression_evaluate(true_ctx);
        *mut_pos += ar_expression_offset(true_ctx);
        
        if (!true_data) {
            ar_expression_destroy_context(true_ctx);
            true_ctx = NULL; // Mark as destroyed
            ar_data_destroy(cond_data);
            cond_data = NULL; // Mark as destroyed
            return false;
        }
        
        // Take ownership of the true data
        ar_expression_take_ownership(true_ctx, true_data);
        ar_expression_destroy_context(true_ctx);
        true_ctx = NULL; // Mark as destroyed
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect comma
        if (ref_instruction[*mut_pos] != ',') {
            ar_data_destroy(cond_data);
            cond_data = NULL; // Mark as destroyed
            ar_data_destroy(true_data);
            true_data = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ','
        skip_whitespace(ref_instruction, mut_pos);
        
        // Parse false_value expression
        expression_context_t *false_ctx = ar_expression_create_context(mut_agent->memory, mut_agent->context, mut_message, ref_instruction + *mut_pos);
        if (!false_ctx) {
            ar_data_destroy(cond_data);
            cond_data = NULL; // Mark as destroyed
            ar_data_destroy(true_data);
            true_data = NULL; // Mark as destroyed
            return false;
        }
        data_t *false_data = ar_expression_evaluate(false_ctx);
        *mut_pos += ar_expression_offset(false_ctx);
        
        if (!false_data) {
            ar_expression_destroy_context(false_ctx);
            false_ctx = NULL; // Mark as destroyed
            ar_data_destroy(cond_data);
            cond_data = NULL; // Mark as destroyed
            ar_data_destroy(true_data);
            true_data = NULL; // Mark as destroyed
            return false;
        }
        
        // Take ownership of the false data
        ar_expression_take_ownership(false_ctx, false_data);
        ar_expression_destroy_context(false_ctx);
        false_ctx = NULL; // Mark as destroyed
        
        skip_whitespace(ref_instruction, mut_pos);
        
        // Expect closing parenthesis
        if (ref_instruction[*mut_pos] != ')') {
            ar_data_destroy(cond_data);
            cond_data = NULL; // Mark as destroyed
            ar_data_destroy(true_data);
            true_data = NULL; // Mark as destroyed
            ar_data_destroy(false_data);
            false_data = NULL; // Mark as destroyed
            return false;
        }
        (*mut_pos)++; // Skip ')'
        
        // Evaluate condition
        bool condition = false;
        data_type_t cond_type = ar_data_get_type(cond_data);
        
        if (cond_type == DATA_INTEGER) {
            condition = (ar_data_get_integer(cond_data) != 0);
        } else if (cond_type == DATA_DOUBLE) {
            condition = (ar_data_get_double(cond_data) != 0.0);
        } else if (cond_type == DATA_STRING) {
            const char *str = ar_data_get_string(cond_data);
            condition = (str && *str); // True if non-empty string
        }
        
        // Select the result based on condition
        if (condition) {
            // We're taking ownership of true_data
            *result = true_data;
            ar_data_destroy(false_data);
            false_data = NULL; // Mark as destroyed
        } else {
            // We're taking ownership of false_data
            *result = false_data;
            ar_data_destroy(true_data);
            true_data = NULL; // Mark as destroyed
        }
        
        ar_data_destroy(cond_data);
        cond_data = NULL; // Mark as destroyed
        return true;
    }
    else {
        // For all other functions (parse, build, method, agent, destroy),
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
        *result = ar_data_create_integer(0);
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
