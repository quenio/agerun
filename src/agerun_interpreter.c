#include "agerun_system.h"
#include "agerun_interpreter.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_queue.h"
#include "agerun_expression.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

/* Memory Dictionary structure is now defined in agerun_data.h */

/* Agent structure is now defined in agerun_agent.h */


// Function declarations are now in the header file


// This function has been moved to agerun_value.c


// ar_expression_evaluate has been moved to agerun_expression.c



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
            if (!ar_instruction_run(agent, message, instruction)) {
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
bool ar_instruction_run(agent_t *agent, const char *message, const char *instruction) {
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
        data_t value = ar_expression_evaluate(agent, message, value_expr, &offset);
        
        // Store in agent's memory
        ar_dict_set(&agent->memory, key, &value);
    }
    // Parse function call or other expression
    else {
        int offset = 0;
        data_t result_val = ar_expression_evaluate(agent, message, instr_trimmed, &offset);
        ar_data_free(&result_val); // Discard the result
    }
    
    free(instr_copy);
    return result;
}

