#ifndef AGERUN_INSTRUCTION_AST_H
#define AGERUN_INSTRUCTION_AST_H

#include <stdbool.h>
#include <stddef.h>
#include "agerun_list.h"
#include "agerun_expression_ast.h"

/**
 * Instruction AST node types.
 * Represents all possible instruction types in the AgeRun language.
 */
typedef enum {
    INST_AST_ASSIGNMENT,      /* memory.x := expression */
    INST_AST_SEND,           /* send(target, message) or result := send(...) */
    INST_AST_IF,             /* if(condition, true_val, false_val) */
    INST_AST_METHOD,         /* method(name, instructions, version) */
    INST_AST_AGENT,          /* agent(method, version, context) */
    INST_AST_DESTROY,        /* destroy(agent_id) or destroy(method, version) */
    INST_AST_DESTROY_AGENT,  /* destroy(agent_id) - specialized parser */
    INST_AST_DESTROY_METHOD, /* destroy(method, version) - specialized parser */
    INST_AST_PARSE,          /* parse(template, input) */
    INST_AST_BUILD           /* build(template, values) */
} instruction_ast_type_t;

/**
 * Opaque AST node structure.
 * Internal structure is hidden to maintain encapsulation.
 */
typedef struct instruction_ast_s instruction_ast_t;

/**
 * Get the type of an AST node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The type of the node, or INST_AST_ASSIGNMENT if node is NULL
 */
instruction_ast_type_t ar__instruction_ast__get_type(const instruction_ast_t *ref_node);

/**
 * Create an assignment instruction AST node.
 * 
 * @param ref_memory_path The memory path (e.g., "memory.x.y") (borrowed reference)
 * @param ref_expression The expression to assign (borrowed reference)
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function makes copies of the string parameters.
 */
instruction_ast_t* ar__instruction_ast__create_assignment(
    const char *ref_memory_path,
    const char *ref_expression
);

/**
 * Create a function call instruction AST node.
 * 
 * @param type The function type (must be one of the function instruction types)
 * @param ref_function_name The function name (borrowed reference)
 * @param ref_args Array of argument expressions (borrowed references, can be NULL)
 * @param arg_count Number of arguments
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function makes copies of all string parameters.
 */
instruction_ast_t* ar__instruction_ast__create_function_call(
    instruction_ast_type_t type,
    const char *ref_function_name,
    const char **ref_args,
    size_t arg_count,
    const char *ref_result_path
);

/**
 * Destroy an AST node and all its components.
 * 
 * @param own_node The AST node to destroy (ownership transferred to function)
 * @note Ownership: Takes ownership of the node and destroys it.
 *       Safe to call with NULL.
 */
void ar__instruction_ast__destroy(instruction_ast_t *own_node);

/* Accessor functions for assignment nodes */

/**
 * Get memory path from an assignment node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The memory path (borrowed reference), or NULL if not an assignment
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__instruction_ast__get_assignment_path(const instruction_ast_t *ref_node);

/**
 * Get expression from an assignment node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The expression string (borrowed reference), or NULL if not an assignment
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__instruction_ast__get_assignment_expression(const instruction_ast_t *ref_node);

/**
 * Get expression AST from an assignment node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The expression AST (borrowed reference), or NULL if not an assignment or no AST
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const expression_ast_t* ar__instruction_ast__get_assignment_expression_ast(const instruction_ast_t *ref_node);

/**
 * Set expression AST for an assignment node.
 * 
 * @param mut_node The AST node (mutable reference)
 * @param own_expression_ast The expression AST to set (ownership transferred)
 * @return true if successful, false if not an assignment node
 * @note Ownership: Takes ownership of the expression AST. Any existing AST is destroyed.
 */
bool ar__instruction_ast__set_assignment_expression_ast(instruction_ast_t *mut_node, expression_ast_t *own_expression_ast);

/* Accessor functions for function call nodes */

/**
 * Get function name from a function call node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The function name (borrowed reference), or NULL if not a function call
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__instruction_ast__get_function_name(const instruction_ast_t *ref_node);

/**
 * Get arguments from a function call node as a new list.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return A newly created list containing argument strings, or NULL if not a function call
 * @note Ownership: IMPORTANT - This function creates and returns a NEW list that the
 *       caller MUST destroy with ar__list__destroy() to avoid memory leaks.
 *       The strings in the list are borrowed references and should not be freed individually.
 * @note Despite the "get" in the name, this function allocates memory.
 */
list_t* ar__instruction_ast__get_function_args(const instruction_ast_t *ref_node);

/**
 * Get argument ASTs from a function call node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return List of expression ASTs (borrowed reference), or NULL if not a function call or no ASTs
 * @note Ownership: Returns a borrowed reference to the list. Do not free.
 *       Use ar__list__get() to access individual ASTs as borrowed references.
 */
const list_t* ar__instruction_ast__get_function_arg_asts(const instruction_ast_t *ref_node);

/**
 * Set argument ASTs for a function call node.
 * 
 * @param mut_node The AST node (mutable reference)
 * @param own_arg_asts List of expression ASTs (ownership transferred)
 * @return true if successful, false if not a function call node
 * @note Ownership: Takes ownership of the list and all ASTs in it. Any existing ASTs are destroyed.
 */
bool ar__instruction_ast__set_function_arg_asts(
    instruction_ast_t *mut_node, 
    list_t *own_arg_asts
);

/**
 * Get result path from a function call node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The result assignment path (borrowed reference), or NULL if no assignment or not a function call
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__instruction_ast__get_function_result_path(const instruction_ast_t *ref_node);

/**
 * Check if a function call has a result assignment.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return true if the function call assigns to a result path, false otherwise
 */
bool ar__instruction_ast__has_result_assignment(const instruction_ast_t *ref_node);

#endif /* AGERUN_INSTRUCTION_AST_H */
