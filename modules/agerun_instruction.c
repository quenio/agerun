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
static bool parse_instruction(agent_t *agent, const data_t *message, const char *instruction, int *pos);
static bool parse_assignment(agent_t *agent, const data_t *message, const char *instruction, int *pos);
static bool parse_function_instruction(agent_t *agent, const data_t *message, const char *instruction, int *pos);
static bool parse_memory_access(const char *instruction, int *pos, char **path);
static bool parse_function_call(agent_t *agent, const data_t *message, const char *instruction, int *pos, data_t **result);
static bool skip_whitespace(const char *instruction, int *pos);
static bool extract_identifier(const char *instruction, int *pos, char *identifier, int max_size);

// Parse and execute a single instruction
bool ar_instruction_run(agent_t *agent, const data_t *message, const char *instruction) {
    if (!agent || !instruction) {
        return false;
    }
    
    int pos = 0;
    return parse_instruction(agent, message, instruction, &pos);
}

// <instruction> ::= <assignment> | <function-instruction>
static bool parse_instruction(agent_t *agent, const data_t *message, const char *instruction, int *pos) {
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
static bool parse_assignment(agent_t *agent, const data_t *message, const char *instruction, int *pos) {
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
    int expr_pos = *pos;
    data_t *value = ar_expression_evaluate(agent, message, instruction + expr_pos, &expr_pos);
    *pos += expr_pos;
    
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
static bool parse_function_instruction(agent_t *agent, const data_t *message, const char *instruction, int *pos) {
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
static bool parse_function_call(agent_t *agent, const data_t *message, const char *instruction, int *pos, data_t **result) {
    // For the recursive descent implementation, we'll rely on ar_expression_evaluate
    // to handle the function call parsing and execution, since the expressions already
    // include function calls according to the grammar.
    
    int expr_pos = *pos;
    *result = ar_expression_evaluate(agent, message, instruction + expr_pos, &expr_pos);
    *pos += expr_pos;
    
    return *result != NULL;
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
