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

// Parse and execute a single instruction
bool ar_instruction_run(agent_t *agent, const char *message, const char *instruction) {
    char *instr_copy = strdup(instruction);
    char *instr_trimmed = ar_string_trim(instr_copy);
    bool result = true;
    
    // Check for assignment operation (key := value)
    char *assign_pos = strstr(instr_trimmed, ":=");
    if (assign_pos != NULL) {
        // Extract key and value parts
        *assign_pos = '0';
        char *key = ar_string_trim(instr_trimmed);
        char *value_expr = ar_string_trim(assign_pos + 2);
        
        // Direct key access - no need to check for memory["key"] syntax
        // as memory map is implicit per the spec
        
        // Evaluate the value expression
        int offset = 0;
        data_t value = ar_expression_evaluate(agent, message, value_expr, &offset);
        
        // Store in agent's memory
        ar_map_set(&agent->memory, key, &value);
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
