#include "../include/agerun.h"
#include "../include/agerun_interpreter.h"

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
extern bool agerun_send(agent_id_t agent_id, const char *message);
extern agent_id_t agerun_create(const char *method_name, version_t version, void *context);
extern bool agerun_destroy(agent_id_t agent_id);
extern version_t agerun_method(const char *name, const char *instructions, version_t previous_version, bool backward_compatible, bool persist);
extern bool memory_set(void *memory, const char *key, void *value);

// Forward declarations for internal functions
static bool parse_and_execute_instruction(agent_t *agent, const char *message, const char *instruction);
static value_t evaluate_expression(agent_t *agent, const char *message, const char *expr, int *offset);

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

// Free a value
static void free_value(value_t *value) {
    if (!value) return;
    
    if (value->type == VALUE_STRING && value->data.string_value) {
        free(value->data.string_value);
        value->data.string_value = NULL;
    }
}




// Simplified evaluate an expression in the agent's context
static value_t evaluate_expression(agent_t *agent, const char *message, const char *expr, int *offset) {
    (void)agent; // Avoid unused parameter warning
    (void)message; // Avoid unused parameter warning
    (void)expr; // Avoid unused parameter warning
    (void)offset; // Avoid unused parameter warning

    value_t result;
    result.type = VALUE_INT;
    result.data.int_value = 0;

    return result;
}



// Main interpretation function for agent methods
bool interpret_agent_method(agent_t *agent, const char *message, const char *instructions) {
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
        instruction = trim(instruction);
        
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
    char *instr_trimmed = trim(instr_copy);
    bool result = true;
    
    // Check for assignment operation (key := value)
    char *assign_pos = strstr(instr_trimmed, ":=");
    if (assign_pos != NULL) {
        // Extract key and value parts
        *assign_pos = '0';
        char *key = trim(instr_trimmed);
        char *value_expr = trim(assign_pos + 2);
        
        // Direct key access - no need to check for memory["key"] syntax
        // as memory dictionary is implicit per the spec
        
        // Evaluate the value expression
        int offset = 0;
        value_t value = evaluate_expression(agent, message, value_expr, &offset);
        
        // Store in agent's memory
        memory_set(&agent->memory, key, &value);
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

