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
static bool parse_instruction(agent_t *agent, data_t *message, const char *instruction, int *pos);
static bool parse_assignment(agent_t *agent, data_t *message, const char *instruction, int *pos);
static bool parse_function_instruction(agent_t *agent, data_t *message, const char *instruction, int *pos);
static bool parse_memory_access(const char *instruction, int *pos, char **path);
static bool parse_function_call(agent_t *agent, data_t *message, const char *instruction, int *pos, data_t **result);
static bool skip_whitespace(const char *instruction, int *pos);
static bool extract_identifier(const char *instruction, int *pos, char *identifier, int max_size);

// Parse and execute a single instruction
bool ar_instruction_run(agent_t *agent, data_t *message, const char *instruction) {
    if (!agent || !instruction) {
        return false;
    }
    
    printf("DEBUG: Running instruction: %s\n", instruction);
    
    int pos = 0;
    bool result = parse_instruction(agent, message, instruction, &pos);
    
    printf("DEBUG: Instruction result: %s\n", result ? "success" : "failure");
    
    return result;
}

// <instruction> ::= <assignment> | <function-instruction>
static bool parse_instruction(agent_t *agent, data_t *message, const char *instruction, int *pos) {
    skip_whitespace(instruction, pos);
    
    // Check for assignment or function instruction
    // Save the current position to backtrack if needed
    int save_pos = *pos;
    
    // Try to parse as assignment first
    if (parse_assignment(agent, message, instruction, pos)) {
        return true;
    }
    
    // Backtrack and try as function instruction
    *pos = save_pos;
    return parse_function_instruction(agent, message, instruction, pos);
}

// <assignment> ::= <memory-access> ':=' <expression>
static bool parse_assignment(agent_t *agent, data_t *message, const char *instruction, int *pos) {
    char *path = NULL;
    
    // Parse memory access (left side)
    if (!parse_memory_access(instruction, pos, &path)) {
        return false;
    }
    
    skip_whitespace(instruction, pos);
    
    // Check for ':=' operator
    if (instruction[*pos] != ':' || instruction[*pos + 1] != '=') {
        free(path);
        return false;
    }
    
    *pos += 2; // Skip ':='
    skip_whitespace(instruction, pos);
    
    // Evaluate the expression (right side)
    expression_context_t *ctx = ar_expression_create_context(agent, message, instruction + *pos);
    if (!ctx) {
        free(path);
        return false;
    }
    data_t *value = ar_expression_evaluate(ctx);
    *pos += ar_expression_offset(ctx);
    ar_expression_destroy_context(ctx);
    
    if (!value) {
        free(path);
        return false;
    }
    
    // Store result in agent's memory
    bool success = ar_data_set_map_data(agent->memory, path, value);
    if (!success) {
        ar_data_destroy(value);
    }
    
    free(path);
    return true;
}

// <function-instruction> ::= [<memory-access> ':='] <function-call>
static bool parse_function_instruction(agent_t *agent, data_t *message, const char *instruction, int *pos) {
    char *path = NULL;
    data_t *result = NULL;
    bool has_assignment = false;
    
    // Check if there's an assignment part
    int save_pos = *pos;
    if (parse_memory_access(instruction, pos, &path)) {
        skip_whitespace(instruction, pos);
        
        if (instruction[*pos] == ':' && instruction[*pos + 1] == '=') {
            has_assignment = true;
            *pos += 2; // Skip ':='
            skip_whitespace(instruction, pos);
        } else {
            // Not an assignment, backtrack
            *pos = save_pos;
            free(path);
            path = NULL;
        }
    } else {
        // Not a memory access, backtrack
        *pos = save_pos;
    }
    
    // Parse function call
    if (!parse_function_call(agent, message, instruction, pos, &result)) {
        free(path);
        return false;
    }
    
    // Store result in memory if assignment was present
    if (has_assignment && path && result) {
        bool success = ar_data_set_map_data(agent->memory, path, result);
        if (!success) {
            ar_data_destroy(result);
        }
    } else if (result) {
        // No assignment, discard the result
        ar_data_destroy(result);
    }
    
    free(path);
    return true;
}

// <memory-access> ::= 'memory' {'.' <identifier>}
// Note: According to the spec, in assignments, only 'memory' paths can be used on the left side
static bool parse_memory_access(const char *instruction, int *pos, char **path) {
    skip_whitespace(instruction, pos);
    
    // Check for 'memory' identifier
    const char *memory_str = "memory";
    size_t memory_len = strlen(memory_str);
    
    if (strncmp(instruction + *pos, memory_str, memory_len) != 0) {
        return false;
    }
    
    // Make sure the 'memory' is a complete token
    if (isalnum((unsigned char)instruction[*pos + (int)memory_len]) || 
        instruction[*pos + (int)memory_len] == '_') {
        return false;
    }
    
    *pos += (int)memory_len;
    
    // Start building the path
    char buffer[1024] = "";
    int buffer_pos = 0;
    
    // Check for dot-separated identifiers
    while (instruction[*pos] == '.') {
        (*pos)++; // Skip '.'
        
        char identifier[256];
        if (!extract_identifier(instruction, pos, identifier, sizeof(identifier))) {
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
static bool parse_function_call(agent_t *agent, data_t *message, const char *instruction, int *pos, data_t **result) {
    // Extract function name
    char function_name[32];
    if (!extract_identifier(instruction, pos, function_name, sizeof(function_name))) {
        return false;
    }
    
    skip_whitespace(instruction, pos);
    
    // Expect opening parenthesis
    if (instruction[*pos] != '(') {
        return false;
    }
    (*pos)++; // Skip '('
    
    // Initialize result
    *result = NULL;
    
    // Handle different function types
    if (strcmp(function_name, "send") == 0) {
        // send(agent_id, message)
        skip_whitespace(instruction, pos);
        
        // Parse agent_id expression
        expression_context_t *agent_id_ctx = ar_expression_create_context(agent, message, instruction + *pos);
        if (!agent_id_ctx) {
            return false;
        }
        data_t *agent_id_data = ar_expression_evaluate(agent_id_ctx);
        *pos += ar_expression_offset(agent_id_ctx);
        ar_expression_destroy_context(agent_id_ctx);
        
        if (!agent_id_data) {
            return false;
        }
        
        skip_whitespace(instruction, pos);
        
        // Expect comma
        if (instruction[*pos] != ',') {
            ar_data_destroy(agent_id_data);
            return false;
        }
        (*pos)++; // Skip ','
        skip_whitespace(instruction, pos);
        
        // Parse message expression
        expression_context_t *msg_ctx = ar_expression_create_context(agent, message, instruction + *pos);
        if (!msg_ctx) {
            ar_data_destroy(agent_id_data);
            return false;
        }
        data_t *msg_data = ar_expression_evaluate(msg_ctx);
        *pos += ar_expression_offset(msg_ctx);
        ar_expression_destroy_context(msg_ctx);
        
        if (!msg_data) {
            ar_data_destroy(agent_id_data);
            return false;
        }
        
        skip_whitespace(instruction, pos);
        
        // Expect closing parenthesis
        if (instruction[*pos] != ')') {
            ar_data_destroy(agent_id_data);
            ar_data_destroy(msg_data);
            return false;
        }
        (*pos)++; // Skip ')'
        
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
            if (msg_data != message) {
                ar_data_destroy(msg_data);
            }
        } else {
            // Ownership of msg_data is transferred to ar_agent_send
            success = ar_agent_send(target_id, msg_data);
        }
        
        ar_data_destroy(agent_id_data);
        *result = ar_data_create_integer(success ? 1 : 0);
        return true;
    }
    else if (strcmp(function_name, "if") == 0) {
        // if(condition, true_value, false_value)
        skip_whitespace(instruction, pos);
        
        // Parse condition expression
        expression_context_t *cond_ctx = ar_expression_create_context(agent, message, instruction + *pos);
        if (!cond_ctx) {
            return false;
        }
        data_t *cond_data = ar_expression_evaluate(cond_ctx);
        *pos += ar_expression_offset(cond_ctx);
        ar_expression_destroy_context(cond_ctx);
        
        if (!cond_data) {
            return false;
        }
        
        skip_whitespace(instruction, pos);
        
        // Expect comma
        if (instruction[*pos] != ',') {
            ar_data_destroy(cond_data);
            return false;
        }
        (*pos)++; // Skip ','
        skip_whitespace(instruction, pos);
        
        // Parse true_value expression
        expression_context_t *true_ctx = ar_expression_create_context(agent, message, instruction + *pos);
        if (!true_ctx) {
            ar_data_destroy(cond_data);
            return false;
        }
        data_t *true_data = ar_expression_evaluate(true_ctx);
        *pos += ar_expression_offset(true_ctx);
        ar_expression_destroy_context(true_ctx);
        
        if (!true_data) {
            ar_data_destroy(cond_data);
            return false;
        }
        
        skip_whitespace(instruction, pos);
        
        // Expect comma
        if (instruction[*pos] != ',') {
            ar_data_destroy(cond_data);
            ar_data_destroy(true_data);
            return false;
        }
        (*pos)++; // Skip ','
        skip_whitespace(instruction, pos);
        
        // Parse false_value expression
        expression_context_t *false_ctx = ar_expression_create_context(agent, message, instruction + *pos);
        if (!false_ctx) {
            ar_data_destroy(cond_data);
            ar_data_destroy(true_data);
            return false;
        }
        data_t *false_data = ar_expression_evaluate(false_ctx);
        *pos += ar_expression_offset(false_ctx);
        ar_expression_destroy_context(false_ctx);
        
        if (!false_data) {
            ar_data_destroy(cond_data);
            ar_data_destroy(true_data);
            return false;
        }
        
        skip_whitespace(instruction, pos);
        
        // Expect closing parenthesis
        if (instruction[*pos] != ')') {
            ar_data_destroy(cond_data);
            ar_data_destroy(true_data);
            ar_data_destroy(false_data);
            return false;
        }
        (*pos)++; // Skip ')'
        
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
            *result = true_data;
            ar_data_destroy(false_data);
        } else {
            *result = false_data;
            ar_data_destroy(true_data);
        }
        
        ar_data_destroy(cond_data);
        return true;
    }
    else {
        // For all other functions (parse, build, method, agent, destroy),
        // just return a default result for now
        // Skip to closing parenthesis
        int nesting = 1;
        while (instruction[*pos] && nesting > 0) {
            if (instruction[*pos] == '(') {
                nesting++;
            } else if (instruction[*pos] == ')') {
                nesting--;
            }
            (*pos)++;
        }
        
        // Create a default result
        *result = ar_data_create_integer(0);
        return true;
    }
    
    return false; // Should not reach here
}

// Utility functions

static bool skip_whitespace(const char *instruction, int *pos) {
    while (instruction[*pos] && isspace((unsigned char)instruction[*pos])) {
        (*pos)++;
    }
    return true;
}

static bool extract_identifier(const char *instruction, int *pos, char *identifier, int max_size) {
    skip_whitespace(instruction, pos);
    
    int i = 0;
    // First character must be a letter
    if (!isalpha((unsigned char)instruction[*pos])) {
        return false;
    }
    
    // Extract the identifier
    while (i < max_size - 1 && 
           (isalnum((unsigned char)instruction[*pos]) || instruction[*pos] == '_')) {
        identifier[i++] = instruction[*pos];
        (*pos)++;
    }
    
    identifier[i] = '\0';
    return i > 0;
}
