#include "agerun_instruction_ast.h"
#include "agerun_heap.h"
#include "agerun_list.h"
#include <string.h>
#include <stdlib.h>

/**
 * Internal structure for instruction AST nodes.
 */
struct instruction_ast_s {
    instruction_ast_type_t type;
    
    /* For assignment instructions */
    char *own_memory_path;       /* Owned: e.g., "memory.x.y" */
    char *own_expression;        /* Owned: the expression to evaluate */
    
    /* For function call instructions */
    char *own_function_name;     /* Owned: function name */
    char **own_args;             /* Owned: array of owned argument strings */
    size_t arg_count;            /* Number of arguments */
    char *own_result_path;       /* Owned: optional result assignment path (may be NULL) */
};

/**
 * Get the type of an AST node.
 */
instruction_ast_type_t ar__instruction_ast__get_type(const instruction_ast_t *ref_node) {
    if (!ref_node) {
        return INST_AST_ASSIGNMENT; /* Default type */
    }
    return ref_node->type;
}

/**
 * Create an assignment instruction AST node.
 */
instruction_ast_t* ar__instruction_ast__create_assignment(
    const char *ref_memory_path,
    const char *ref_expression
) {
    if (!ref_memory_path || !ref_expression) {
        return NULL;
    }
    
    /* Allocate the node */
    instruction_ast_t *own_node = AR__HEAP__MALLOC(sizeof(instruction_ast_t), "instruction AST node");
    if (!own_node) {
        return NULL;
    }
    
    /* Initialize the node */
    own_node->type = INST_AST_ASSIGNMENT;
    own_node->own_memory_path = AR__HEAP__STRDUP(ref_memory_path, "assignment memory path");
    own_node->own_expression = AR__HEAP__STRDUP(ref_expression, "assignment expression");
    
    /* Initialize unused fields */
    own_node->own_function_name = NULL;
    own_node->own_args = NULL;
    own_node->arg_count = 0;
    own_node->own_result_path = NULL;
    
    /* Check for allocation failures */
    if (!own_node->own_memory_path || !own_node->own_expression) {
        ar__instruction_ast__destroy(own_node);
        return NULL;
    }
    
    return own_node;
}

/**
 * Create a function call instruction AST node.
 */
instruction_ast_t* ar__instruction_ast__create_function_call(
    instruction_ast_type_t type,
    const char *ref_function_name,
    const char **ref_args,
    size_t arg_count,
    const char *ref_result_path
) {
    if (!ref_function_name || type == INST_AST_ASSIGNMENT) {
        return NULL;
    }
    
    /* Allocate the node */
    instruction_ast_t *own_node = AR__HEAP__MALLOC(sizeof(instruction_ast_t), "instruction AST node");
    if (!own_node) {
        return NULL;
    }
    
    /* Initialize the node */
    own_node->type = type;
    own_node->own_memory_path = NULL;
    own_node->own_expression = NULL;
    own_node->own_function_name = NULL;
    own_node->own_args = NULL;
    own_node->arg_count = 0;
    own_node->own_result_path = NULL;
    
    /* Copy function name */
    own_node->own_function_name = AR__HEAP__STRDUP(ref_function_name, "function name");
    if (!own_node->own_function_name) {
        ar__instruction_ast__destroy(own_node);
        return NULL;
    }
    
    /* Copy result path if provided */
    own_node->own_result_path = ref_result_path ? AR__HEAP__STRDUP(ref_result_path, "result path") : NULL;
    if (ref_result_path && !own_node->own_result_path) {
        ar__instruction_ast__destroy(own_node);
        return NULL;
    }
    
    /* Copy arguments if provided */
    if (arg_count > 0 && ref_args) {
        own_node->own_args = AR__HEAP__MALLOC(sizeof(char*) * arg_count, "function arguments array");
        if (!own_node->own_args) {
            ar__instruction_ast__destroy(own_node);
            return NULL;
        }
        
        own_node->arg_count = arg_count;
        
        /* Copy each argument */
        for (size_t i = 0; i < arg_count; i++) {
            own_node->own_args[i] = AR__HEAP__STRDUP(ref_args[i], "function argument");
            if (!own_node->own_args[i]) {
                /* Clean up partially allocated args */
                for (size_t j = 0; j < i; j++) {
                    AR__HEAP__FREE(own_node->own_args[j]);
                }
                AR__HEAP__FREE(own_node->own_args);
                own_node->own_args = NULL;
                own_node->arg_count = 0;
                ar__instruction_ast__destroy(own_node);
                return NULL;
            }
        }
    } else {
        own_node->own_args = NULL;
        own_node->arg_count = 0;
    }
    
    return own_node;
}

/**
 * Destroy an AST node and all its components.
 */
void ar__instruction_ast__destroy(instruction_ast_t *own_node) {
    if (!own_node) {
        return;
    }
    
    /* Free assignment fields */
    if (own_node->own_memory_path) {
        AR__HEAP__FREE(own_node->own_memory_path);
    }
    if (own_node->own_expression) {
        AR__HEAP__FREE(own_node->own_expression);
    }
    
    /* Free function call fields */
    if (own_node->own_function_name) {
        AR__HEAP__FREE(own_node->own_function_name);
    }
    if (own_node->own_result_path) {
        AR__HEAP__FREE(own_node->own_result_path);
    }
    
    /* Free arguments array */
    if (own_node->own_args) {
        for (size_t i = 0; i < own_node->arg_count; i++) {
            if (own_node->own_args[i]) {
                AR__HEAP__FREE(own_node->own_args[i]);
            }
        }
        AR__HEAP__FREE(own_node->own_args);
    }
    
    AR__HEAP__FREE(own_node);
}

/**
 * Get memory path from an assignment node.
 */
const char* ar__instruction_ast__get_assignment_path(const instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type != INST_AST_ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_memory_path;
}

/**
 * Get expression from an assignment node.
 */
const char* ar__instruction_ast__get_assignment_expression(const instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type != INST_AST_ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_expression;
}

/**
 * Get function name from a function call node.
 */
const char* ar__instruction_ast__get_function_name(const instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == INST_AST_ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_function_name;
}

/**
 * Get arguments from a function call node.
 */
list_t* ar__instruction_ast__get_function_args(const instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == INST_AST_ASSIGNMENT || ref_node->arg_count == 0) {
        return NULL;
    }
    
    /* Create a new list */
    list_t *own_list = ar__list__create();
    if (!own_list) {
        return NULL;
    }
    
    /* Add each argument to the list (as borrowed references) */
    for (size_t i = 0; i < ref_node->arg_count; i++) {
        if (!ar__list__add_last(own_list, ref_node->own_args[i])) {
            ar__list__destroy(own_list);
            return NULL;
        }
    }
    
    return own_list;
}

/**
 * Get result path from a function call node.
 */
const char* ar__instruction_ast__get_function_result_path(const instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == INST_AST_ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_result_path;
}

/**
 * Check if a function call has a result assignment.
 */
bool ar__instruction_ast__has_result_assignment(const instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == INST_AST_ASSIGNMENT) {
        return false;
    }
    return ref_node->own_result_path != NULL;
}
