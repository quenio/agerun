/**
 * @file agerun_instruction_evaluator.c
 * @brief Implementation of the instruction evaluator module
 */

#include "agerun_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_assignment_instruction_evaluator.h"
#include "agerun_send_instruction_evaluator.h"
#include "agerun_condition_instruction_evaluator.h"
#include "agerun_parse_instruction_evaluator.h"
#include "agerun_build_instruction_evaluator.h"
#include "agerun_method_instruction_evaluator.h"
#include "agerun_agent_instruction_evaluator.h"
#include "agerun_destroy_agent_instruction_evaluator.h"
#include "agerun_destroy_method_instruction_evaluator.h"
#include "agerun_list.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


/**
 * Internal structure for instruction evaluator
 */
struct instruction_evaluator_s {
    expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
    data_t *ref_context;                         /* Context map (borrowed reference, can be NULL) */
    data_t *ref_message;                         /* Message data (borrowed reference, can be NULL) */
    assignment_instruction_evaluator_t *own_assignment_evaluator;  /* Assignment evaluator instance (owned) */
    send_instruction_evaluator_t *own_send_evaluator;  /* Send evaluator instance (owned) */
    condition_instruction_evaluator_t *own_condition_evaluator;  /* Condition evaluator instance (owned) */
    parse_instruction_evaluator_t *own_parse_evaluator;  /* Parse evaluator instance (owned) */
    ar_build_instruction_evaluator_t *own_build_evaluator;  /* Build evaluator instance (owned) */
    ar_method_instruction_evaluator_t *own_method_evaluator;  /* Method evaluator instance (owned) */
    ar_agent_instruction_evaluator_t *own_agent_evaluator;  /* Agent evaluator instance (owned) */
    ar_destroy_agent_instruction_evaluator_t *own_destroy_agent_evaluator;  /* Destroy agent evaluator instance (owned) */
    ar_destroy_method_instruction_evaluator_t *own_destroy_method_evaluator;  /* Destroy method evaluator instance (owned) */
};

/**
 * Creates a new instruction evaluator
 */
instruction_evaluator_t* ar_instruction_evaluator__create(
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
    
    // Create assignment evaluator instance
    evaluator->own_assignment_evaluator = ar_assignment_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_assignment_evaluator == NULL) {
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create send evaluator instance
    evaluator->own_send_evaluator = ar_send_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_send_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create condition evaluator instance
    evaluator->own_condition_evaluator = ar_condition_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_condition_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create parse evaluator instance
    evaluator->own_parse_evaluator = ar_parse_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_parse_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create build evaluator instance
    evaluator->own_build_evaluator = ar_build_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_build_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create method evaluator instance
    evaluator->own_method_evaluator = ar_method_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_method_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create agent evaluator instance
    evaluator->own_agent_evaluator = ar_agent_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_agent_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        ar_method_instruction_evaluator__destroy(evaluator->own_method_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create destroy agent evaluator instance
    evaluator->own_destroy_agent_evaluator = ar_destroy_agent_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_destroy_agent_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        ar_method_instruction_evaluator__destroy(evaluator->own_method_evaluator);
        ar_agent_instruction_evaluator__destroy(evaluator->own_agent_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create destroy method evaluator instance
    evaluator->own_destroy_method_evaluator = ar_destroy_method_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_destroy_method_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        ar_method_instruction_evaluator__destroy(evaluator->own_method_evaluator);
        ar_agent_instruction_evaluator__destroy(evaluator->own_agent_evaluator);
        ar_destroy_agent_instruction_evaluator__destroy(evaluator->own_destroy_agent_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    return evaluator;
}

/**
 * Destroys an instruction evaluator
 */
void ar_instruction_evaluator__destroy(instruction_evaluator_t *own_evaluator) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Destroy owned evaluator instances
    if (own_evaluator->own_assignment_evaluator != NULL) {
        ar_assignment_instruction_evaluator__destroy(own_evaluator->own_assignment_evaluator);
    }
    if (own_evaluator->own_send_evaluator != NULL) {
        ar_send_instruction_evaluator__destroy(own_evaluator->own_send_evaluator);
    }
    if (own_evaluator->own_condition_evaluator != NULL) {
        ar_condition_instruction_evaluator__destroy(own_evaluator->own_condition_evaluator);
    }
    if (own_evaluator->own_parse_evaluator != NULL) {
        ar_parse_instruction_evaluator__destroy(own_evaluator->own_parse_evaluator);
    }
    if (own_evaluator->own_build_evaluator != NULL) {
        ar_build_instruction_evaluator__destroy(own_evaluator->own_build_evaluator);
    }
    if (own_evaluator->own_method_evaluator != NULL) {
        ar_method_instruction_evaluator__destroy(own_evaluator->own_method_evaluator);
    }
    if (own_evaluator->own_agent_evaluator != NULL) {
        ar_agent_instruction_evaluator__destroy(own_evaluator->own_agent_evaluator);
    }
    if (own_evaluator->own_destroy_agent_evaluator != NULL) {
        ar_destroy_agent_instruction_evaluator__destroy(own_evaluator->own_destroy_agent_evaluator);
    }
    if (own_evaluator->own_destroy_method_evaluator != NULL) {
        ar_destroy_method_instruction_evaluator__destroy(own_evaluator->own_destroy_method_evaluator);
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Gets the assignment evaluator instance
 */
assignment_instruction_evaluator_t* ar_instruction_evaluator__get_assignment_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_assignment_evaluator;
}

/**
 * Gets the send evaluator instance
 */
send_instruction_evaluator_t* ar_instruction_evaluator__get_send_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_send_evaluator;
}

/**
 * Gets the condition evaluator instance
 */
condition_instruction_evaluator_t* ar_instruction_evaluator__get_condition_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_condition_evaluator;
}

/**
 * Gets the parse evaluator instance
 */
parse_instruction_evaluator_t* ar_instruction_evaluator__get_parse_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_parse_evaluator;
}

/**
 * Gets the build evaluator instance
 */
ar_build_instruction_evaluator_t* ar_instruction_evaluator__get_build_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_build_evaluator;
}

/**
 * Gets the method evaluator instance
 */
ar_method_instruction_evaluator_t* ar_instruction_evaluator__get_method_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_method_evaluator;
}

/**
 * Gets the agent evaluator instance
 */
ar_agent_instruction_evaluator_t* ar_instruction_evaluator__get_agent_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_agent_evaluator;
}

/**
 * Gets the destroy agent evaluator instance
 */
ar_destroy_agent_instruction_evaluator_t* ar_instruction_evaluator__get_destroy_agent_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_destroy_agent_evaluator;
}

/**
 * Gets the destroy method evaluator instance
 */
ar_destroy_method_instruction_evaluator_t* ar_instruction_evaluator__get_destroy_method_evaluator(
    const instruction_evaluator_t *ref_evaluator
) {
    if (ref_evaluator == NULL) {
        return NULL;
    }
    return ref_evaluator->own_destroy_method_evaluator;
}


bool ar_instruction_evaluator__evaluate_assignment(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the assignment instruction evaluator instance
    return ar_assignment_instruction_evaluator__evaluate(
        mut_evaluator->own_assignment_evaluator,
        ref_ast
    );
}

bool ar_instruction_evaluator__evaluate_send(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the send instruction evaluator instance
    return ar_send_instruction_evaluator__evaluate(
        mut_evaluator->own_send_evaluator,
        ref_ast
    );
}

bool ar_instruction_evaluator__evaluate_if(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the condition instruction evaluator instance
    return ar_condition_instruction_evaluator__evaluate(
        mut_evaluator->own_condition_evaluator,
        ref_ast
    );
}


bool ar_instruction_evaluator__evaluate_parse(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the parse instruction evaluator instance
    return ar_parse_instruction_evaluator__evaluate(
        mut_evaluator->own_parse_evaluator,
        ref_ast
    );
}


bool ar_instruction_evaluator__evaluate_build(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the build instruction evaluator instance
    return ar_build_instruction_evaluator__evaluate(
        mut_evaluator->own_build_evaluator,
        ref_ast
    );
}

bool ar_instruction_evaluator__evaluate_method(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the method instruction evaluator instance
    return ar_method_instruction_evaluator__evaluate(
        mut_evaluator->own_method_evaluator,
        ref_ast
    );
}

bool ar_instruction_evaluator__evaluate_agent(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the agent instruction evaluator instance
    return ar_agent_instruction_evaluator__evaluate(
        mut_evaluator->own_agent_evaluator,
        mut_evaluator->ref_context,
        ref_ast
    );
}

bool ar_instruction_evaluator__evaluate_destroy(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Validate AST type
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_DESTROY) {
        return false;
    }
    
    // Get function arguments to determine which evaluator to use
    list_t *own_args = ar__instruction_ast__get_function_args(ref_ast);
    if (!own_args) {
        return false;
    }
    
    size_t arg_count = ar__list__count(own_args);
    ar__list__destroy(own_args);
    
    if (arg_count == 1) {
        // destroy(agent_id) - dispatch to destroy agent evaluator instance
        return ar_destroy_agent_instruction_evaluator__evaluate(
            mut_evaluator->own_destroy_agent_evaluator,
            ref_ast
        );
    } else if (arg_count == 2) {
        // destroy(method_name, method_version) - dispatch to destroy method evaluator instance
        return ar_destroy_method_instruction_evaluator__evaluate(
            mut_evaluator->own_destroy_method_evaluator,
            ref_ast
        );
    } else {
        // Invalid argument count
        return false;
    }
}