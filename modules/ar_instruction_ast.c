#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_list.h"
#include <string.h>
#include <stdlib.h>

/**
 * Internal structure for instruction AST nodes.
 */
struct instruction_ast_s {
    ar_instruction_ast_type_t type;
    
    /* For assignment instructions */
    char *own_memory_path;       /* Owned: e.g., "memory.x.y" */
    char *own_expression;        /* Owned: the expression to evaluate (legacy) */
    ar_expression_ast_t *own_expression_ast; /* Owned: the expression as AST (new) */
    
    /* For function call instructions */
    char *own_function_name;     /* Owned: function name */
    char **own_args;             /* Owned: array of owned argument strings (legacy) */
    ar_list_t *own_arg_asts;        /* Owned: list of owned expression ASTs (new) */
    size_t arg_count;            /* Number of arguments */
    char *own_result_path;       /* Owned: optional result assignment path (may be NULL) */
};

/**
 * Get the type of an AST node.
 */
ar_instruction_ast_type_t ar_instruction_ast__get_type(const ar_instruction_ast_t *ref_node) {
    if (!ref_node) {
        return AR_INST__ASSIGNMENT; /* Default type */
    }
    return ref_node->type;
}

/**
 * Create an assignment instruction AST node.
 */
ar_instruction_ast_t* ar_instruction_ast__create_assignment(
    const char *ref_memory_path,
    const char *ref_expression
) {
    if (!ref_memory_path || !ref_expression) {
        return NULL;
    }
    
    /* Allocate the node */
    ar_instruction_ast_t *own_node = AR__HEAP__MALLOC(sizeof(ar_instruction_ast_t), "instruction AST node");
    if (!own_node) {
        return NULL;
    }
    
    /* Initialize the node */
    own_node->type = AR_INST__ASSIGNMENT;
    own_node->own_memory_path = AR__HEAP__STRDUP(ref_memory_path, "assignment memory path");
    own_node->own_expression = AR__HEAP__STRDUP(ref_expression, "assignment expression");
    own_node->own_expression_ast = NULL; /* Will be set by parser in Phase 2 */
    
    /* Initialize unused fields */
    own_node->own_function_name = NULL;
    own_node->own_args = NULL;
    own_node->own_arg_asts = NULL;
    own_node->arg_count = 0;
    own_node->own_result_path = NULL;
    
    /* Check for allocation failures */
    if (!own_node->own_memory_path || !own_node->own_expression) {
        ar_instruction_ast__destroy(own_node);
        return NULL;
    }
    
    return own_node;
}

/**
 * Create a function call instruction AST node.
 */
ar_instruction_ast_t* ar_instruction_ast__create_function_call(
    ar_instruction_ast_type_t type,
    const char *ref_function_name,
    const char **ref_args,
    size_t arg_count,
    const char *ref_result_path
) {
    if (!ref_function_name || type == AR_INST__ASSIGNMENT) {
        return NULL;
    }
    
    /* Allocate the node */
    ar_instruction_ast_t *own_node = AR__HEAP__MALLOC(sizeof(ar_instruction_ast_t), "instruction AST node");
    if (!own_node) {
        return NULL;
    }
    
    /* Initialize the node */
    own_node->type = type;
    own_node->own_memory_path = NULL;
    own_node->own_expression = NULL;
    own_node->own_expression_ast = NULL;
    own_node->own_function_name = NULL;
    own_node->own_args = NULL;
    own_node->own_arg_asts = NULL;
    own_node->arg_count = 0;
    own_node->own_result_path = NULL;
    
    /* Copy function name */
    own_node->own_function_name = AR__HEAP__STRDUP(ref_function_name, "function name");
    if (!own_node->own_function_name) {
        ar_instruction_ast__destroy(own_node);
        return NULL;
    }
    
    /* Copy result path if provided */
    own_node->own_result_path = ref_result_path ? AR__HEAP__STRDUP(ref_result_path, "result path") : NULL;
    if (ref_result_path && !own_node->own_result_path) {
        ar_instruction_ast__destroy(own_node);
        return NULL;
    }
    
    /* Copy arguments if provided */
    if (arg_count > 0 && ref_args) {
        own_node->own_args = AR__HEAP__MALLOC(sizeof(char*) * arg_count, "function arguments array");
        if (!own_node->own_args) {
            ar_instruction_ast__destroy(own_node);
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
                ar_instruction_ast__destroy(own_node);
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
void ar_instruction_ast__destroy(ar_instruction_ast_t *own_node) {
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
    if (own_node->own_expression_ast) {
        ar_expression_ast__destroy(own_node->own_expression_ast);
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
    
    /* Free argument ASTs list */
    if (own_node->own_arg_asts) {
        /* Destroy all expression ASTs in the list */
        void **items = ar_list__items(own_node->own_arg_asts);
        if (items) {
            size_t list_count = ar_list__count(own_node->own_arg_asts);
            for (size_t i = 0; i < list_count; i++) {
                ar_expression_ast_t *own_ast = (ar_expression_ast_t*)items[i];
                if (own_ast) {
                    ar_expression_ast__destroy(own_ast);
                }
            }
            AR__HEAP__FREE(items);
        }
        ar_list__destroy(own_node->own_arg_asts);
    }
    
    AR__HEAP__FREE(own_node);
}

/**
 * Get memory path from an assignment node.
 */
const char* ar_instruction_ast__get_assignment_path(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_INST__ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_memory_path;
}

/**
 * Get expression from an assignment node.
 */
const char* ar_instruction_ast__get_assignment_expression(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_INST__ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_expression;
}

/**
 * Get expression AST from an assignment node.
 */
const ar_expression_ast_t* ar_instruction_ast__get_assignment_expression_ast(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_INST__ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_expression_ast;
}

/**
 * Set expression AST for an assignment node.
 */
bool ar_instruction_ast__set_assignment_expression_ast(ar_instruction_ast_t *mut_node, ar_expression_ast_t *own_expression_ast) {
    if (!mut_node || mut_node->type != AR_INST__ASSIGNMENT) {
        return false;
    }
    
    /* Destroy any existing expression AST */
    if (mut_node->own_expression_ast) {
        ar_expression_ast__destroy(mut_node->own_expression_ast);
    }
    
    /* Set the new expression AST */
    mut_node->own_expression_ast = own_expression_ast;
    return true;
}

/**
 * Get function name from a function call node.
 */
const char* ar_instruction_ast__get_function_name(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == AR_INST__ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_function_name;
}

/**
 * Get arguments from a function call node.
 */
ar_list_t* ar_instruction_ast__get_function_args(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == AR_INST__ASSIGNMENT || ref_node->arg_count == 0) {
        return NULL;
    }
    
    /* Create a new list */
    ar_list_t *own_list = ar_list__create();
    if (!own_list) {
        return NULL;
    }
    
    /* Add each argument to the list (as borrowed references) */
    for (size_t i = 0; i < ref_node->arg_count; i++) {
        if (!ar_list__add_last(own_list, ref_node->own_args[i])) {
            ar_list__destroy(own_list);
            return NULL;
        }
    }
    
    return own_list;
}

/**
 * Get argument ASTs from a function call node.
 */
const ar_list_t* ar_instruction_ast__get_function_arg_asts(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == AR_INST__ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_arg_asts;
}

/**
 * Set argument ASTs for a function call node.
 */
bool ar_instruction_ast__set_function_arg_asts(
    ar_instruction_ast_t *mut_node, 
    ar_list_t *own_arg_asts
) {
    if (!mut_node || mut_node->type == AR_INST__ASSIGNMENT) {
        return false;
    }
    
    /* Destroy any existing argument ASTs */
    if (mut_node->own_arg_asts) {
        void **items = ar_list__items(mut_node->own_arg_asts);
        if (items) {
            size_t list_count = ar_list__count(mut_node->own_arg_asts);
            for (size_t i = 0; i < list_count; i++) {
                ar_expression_ast_t *own_ast = (ar_expression_ast_t*)items[i];
                if (own_ast) {
                    ar_expression_ast__destroy(own_ast);
                }
            }
            AR__HEAP__FREE(items);
        }
        ar_list__destroy(mut_node->own_arg_asts);
    }
    
    /* Set the new argument ASTs list */
    mut_node->own_arg_asts = own_arg_asts;
    return true;
}

/**
 * Get result path from a function call node.
 */
const char* ar_instruction_ast__get_function_result_path(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == AR_INST__ASSIGNMENT) {
        return NULL;
    }
    return ref_node->own_result_path;
}

/**
 * Check if a function call has a result assignment.
 */
bool ar_instruction_ast__has_result_assignment(const ar_instruction_ast_t *ref_node) {
    if (!ref_node || ref_node->type == AR_INST__ASSIGNMENT) {
        return false;
    }
    return ref_node->own_result_path != NULL;
}
