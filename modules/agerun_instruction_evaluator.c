/**
 * @file agerun_instruction_evaluator.c
 * @brief Implementation of the instruction evaluator module
 */

#include "agerun_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include <assert.h>
#include <string.h>

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
            return ar__expression_evaluator__evaluate_memory_access(mut_evaluator->ref_expr_evaluator, ref_ast);
            
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
    (void)mut_evaluator;
    (void)ref_ast;
    assert(false && "Not implemented yet");
    return false;
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