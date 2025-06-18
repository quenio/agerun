/**
 * @file agerun_instruction_evaluator.c
 * @brief Implementation of the instruction evaluator module
 */

#include "agerun_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_agency.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

/**
 * Internal structure for instruction evaluator
 */
struct instruction_evaluator_s {
    expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
    data_t *ref_context;                         /* Context map (borrowed reference, can be NULL) */
    data_t *ref_message;                         /* Message data (borrowed reference, can be NULL) */
};

/**
 * Creates a new instruction evaluator
 */
instruction_evaluator_t* ar__instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
) {
    // Validate required parameters
    if (ref_expr_evaluator == NULL || mut_memory == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(instruction_evaluator_t), "instruction_evaluator");
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    evaluator->mut_memory = mut_memory;
    evaluator->ref_context = ref_context;
    evaluator->ref_message = ref_message;
    
    return evaluator;
}

/**
 * Destroys an instruction evaluator
 */
void ar__instruction_evaluator__destroy(instruction_evaluator_t *own_evaluator) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}

/* Helper function to evaluate an expression AST node using the expression evaluator */
static data_t* _evaluate_expression_ast(instruction_evaluator_t *mut_evaluator, expression_ast_t *ref_ast) {
    if (!ref_ast) {
        return NULL;
    }
    
    expression_ast_type_t type = ar__expression_ast__get_type(ref_ast);
    
    switch (type) {
        case EXPR_AST_LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        case EXPR_AST_MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy
            {
                data_t *ref_value = ar__expression_evaluator__evaluate_memory_access(mut_evaluator->ref_expr_evaluator, ref_ast);
                if (!ref_value) return NULL;
                
                // Create a copy based on type
                switch (ar__data__get_type(ref_value)) {
                    case DATA_INTEGER:
                        return ar__data__create_integer(ar__data__get_integer(ref_value));
                    case DATA_DOUBLE:
                        return ar__data__create_double(ar__data__get_double(ref_value));
                    case DATA_STRING:
                        return ar__data__create_string(ar__data__get_string(ref_value));
                    default:
                        // For unsupported types (like maps), return NULL
                        return NULL;
                }
            }
            
        case EXPR_AST_BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_evaluator->ref_expr_evaluator, ref_ast);
            
        default:
            return NULL;
    }
}

bool ar__instruction_evaluator__evaluate_assignment(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Verify this is an assignment AST node
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_ASSIGNMENT) {
        return false;
    }
    
    // Get assignment details
    const char *ref_path = ar__instruction_ast__get_assignment_path(ref_ast);
    const char *ref_expression = ar__instruction_ast__get_assignment_expression(ref_ast);
    
    if (!ref_path || !ref_expression) {
        return false;
    }
    
    // Check that path starts with "memory."
    if (strncmp(ref_path, "memory.", 7) != 0) {
        return false;
    }
    
    // Strip "memory." prefix to get the actual key path
    const char *key_path = ref_path + 7;
    
    // Parse the expression to get an AST
    expression_parser_t *parser = ar__expression_parser__create(ref_expression);
    if (!parser) {
        return false;
    }
    
    expression_ast_t *expr_ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!expr_ast) {
        return false;
    }
    
    // Evaluate the expression AST
    data_t *own_value = _evaluate_expression_ast(mut_evaluator, expr_ast);
    ar__expression_ast__destroy(expr_ast);
    
    if (!own_value) {
        return false;
    }
    
    // Store the value in memory (transfers ownership)
    bool success = ar__data__set_map_data(mut_evaluator->mut_memory, key_path, own_value);
    if (!success) {
        ar__data__destroy(own_value);
    }
    
    return success;
}

bool ar__instruction_evaluator__evaluate_send(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Verify this is a send AST node
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_SEND) {
        return false;
    }
    
    // Get function arguments
    list_t *own_args = ar__instruction_ast__get_function_args(ref_ast);
    if (!own_args) {
        return false;
    }
    
    // Send requires exactly 2 arguments
    if (ar__list__count(own_args) != 2) {
        ar__list__destroy(own_args);
        return false;
    }
    
    // Get argument strings (borrowed references from the list)
    void **items = ar__list__items(own_args);
    if (!items) {
        ar__list__destroy(own_args);
        return false;
    }
    
    const char *ref_agent_id_expr = (const char*)items[0];
    const char *ref_message_expr = (const char*)items[1];
    
    if (!ref_agent_id_expr || !ref_message_expr) {
        AR__HEAP__FREE(items);
        ar__list__destroy(own_args);
        return false;
    }
    
    // Parse and evaluate agent ID expression
    expression_parser_t *parser = ar__expression_parser__create(ref_agent_id_expr);
    if (!parser) {
        AR__HEAP__FREE(items);
        ar__list__destroy(own_args);
        return false;
    }
    
    expression_ast_t *agent_id_ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!agent_id_ast) {
        AR__HEAP__FREE(items);
        ar__list__destroy(own_args);
        return false;
    }
    
    data_t *own_agent_id_data = _evaluate_expression_ast(mut_evaluator, agent_id_ast);
    ar__expression_ast__destroy(agent_id_ast);
    
    if (!own_agent_id_data) {
        AR__HEAP__FREE(items);
        ar__list__destroy(own_args);
        return false;
    }
    
    // Extract agent ID as integer
    int64_t agent_id = 0;
    if (ar__data__get_type(own_agent_id_data) == DATA_INTEGER) {
        agent_id = ar__data__get_integer(own_agent_id_data);
    }
    ar__data__destroy(own_agent_id_data);
    
    // Parse and evaluate message expression
    parser = ar__expression_parser__create(ref_message_expr);
    if (!parser) {
        AR__HEAP__FREE(items);
        ar__list__destroy(own_args);
        return false;
    }
    
    expression_ast_t *message_ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!message_ast) {
        AR__HEAP__FREE(items);
        ar__list__destroy(own_args);
        return false;
    }
    
    data_t *own_message = _evaluate_expression_ast(mut_evaluator, message_ast);
    ar__expression_ast__destroy(message_ast);
    
    if (!own_message) {
        AR__HEAP__FREE(items);
        ar__list__destroy(own_args);
        return false;
    }
    
    // Clean up items array and args list (we're done with them)
    AR__HEAP__FREE(items);
    ar__list__destroy(own_args);
    
    // Send the message
    bool send_result;
    if (agent_id == 0) {
        // Special case: agent_id 0 is a no-op that always returns true
        ar__data__destroy(own_message);
        send_result = true;
    } else {
        // Send message (ownership transferred to ar__agency__send_to_agent)
        send_result = ar__agency__send_to_agent(agent_id, own_message);
    }
    
    // Handle result assignment if present
    const char *ref_result_path = ar__instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path) {
        // Check that path starts with "memory."
        if (strncmp(ref_result_path, "memory.", 7) != 0) {
            return false;
        }
        
        // Strip "memory." prefix
        const char *key_path = ref_result_path + 7;
        
        // Create result value (true = 1, false = 0)
        data_t *own_result = ar__data__create_integer(send_result ? 1 : 0);
        bool store_success = ar__data__set_map_data(mut_evaluator->mut_memory, key_path, own_result);
        if (!store_success) {
            ar__data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    }
    
    return send_result;
}

bool ar__instruction_evaluator__evaluate_if(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    (void)mut_evaluator;
    (void)ref_ast;
    assert(false && "Not implemented yet");
    return false;
}

bool ar__instruction_evaluator__evaluate_parse(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    (void)mut_evaluator;
    (void)ref_ast;
    assert(false && "Not implemented yet");
    return false;
}

bool ar__instruction_evaluator__evaluate_build(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    (void)mut_evaluator;
    (void)ref_ast;
    assert(false && "Not implemented yet");
    return false;
}

bool ar__instruction_evaluator__evaluate_method(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    (void)mut_evaluator;
    (void)ref_ast;
    assert(false && "Not implemented yet");
    return false;
}

bool ar__instruction_evaluator__evaluate_agent(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    (void)mut_evaluator;
    (void)ref_ast;
    assert(false && "Not implemented yet");
    return false;
}

bool ar__instruction_evaluator__evaluate_destroy(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    (void)mut_evaluator;
    (void)ref_ast;
    assert(false && "Not implemented yet");
    return false;
}