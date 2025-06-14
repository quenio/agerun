#ifndef AGERUN_EXPRESSION_AST_H
#define AGERUN_EXPRESSION_AST_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Expression AST node types.
 * Represents all possible expression types in the AgeRun language.
 */
typedef enum {
    EXPR_AST_LITERAL_INT,      /* Integer literal (e.g., 42, -10) */
    EXPR_AST_LITERAL_DOUBLE,   /* Double literal (e.g., 3.14, -2.5) */
    EXPR_AST_LITERAL_STRING,   /* String literal (e.g., "hello") */
    EXPR_AST_MEMORY_ACCESS,    /* Memory/message/context access (e.g., memory.x, message.content) */
    EXPR_AST_BINARY_OP         /* Binary operation (arithmetic or comparison) */
} expression_ast_type_t;

/**
 * Binary operator types.
 * Covers both arithmetic and comparison operators.
 */
typedef enum {
    /* Arithmetic operators */
    OP_ADD,        /* + */
    OP_SUBTRACT,   /* - */
    OP_MULTIPLY,   /* * */
    OP_DIVIDE,     /* / */
    
    /* Comparison operators */
    OP_EQUAL,      /* = */
    OP_NOT_EQUAL,  /* <> */
    OP_LESS,       /* < */
    OP_LESS_EQ,    /* <= */
    OP_GREATER,    /* > */
    OP_GREATER_EQ  /* >= */
} binary_operator_t;

/**
 * Opaque AST node structure.
 * Internal structure is hidden to maintain encapsulation.
 */
typedef struct expression_ast_s expression_ast_t;

/**
 * Get the type of an AST node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The type of the node
 */
expression_ast_type_t ar__expression_ast__get_type(const expression_ast_t *ref_node);

/**
 * Create an integer literal AST node.
 * 
 * @param value The integer value
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
expression_ast_t* ar__expression_ast__create_literal_int(int value);

/**
 * Create a double literal AST node.
 * 
 * @param value The double value
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
expression_ast_t* ar__expression_ast__create_literal_double(double value);

/**
 * Create a string literal AST node.
 * 
 * @param ref_value The string value (borrowed reference)
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function makes a copy of the string parameter.
 */
expression_ast_t* ar__expression_ast__create_literal_string(const char *ref_value);

/**
 * Create a memory access AST node.
 * 
 * @param ref_base The base accessor ("memory", "message", or "context") (borrowed reference)
 * @param ref_path Array of path components after the base (borrowed reference, can be NULL)
 * @param path_count Number of path components
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function makes copies of all string parameters.
 */
expression_ast_t* ar__expression_ast__create_memory_access(
    const char *ref_base,
    const char **ref_path,
    size_t path_count
);

/**
 * Create a binary operation AST node.
 * 
 * @param op The binary operator
 * @param own_left The left operand (ownership transferred to node)
 * @param own_right The right operand (ownership transferred to node)
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       Takes ownership of both operand nodes.
 */
expression_ast_t* ar__expression_ast__create_binary_op(
    binary_operator_t op,
    expression_ast_t *own_left,
    expression_ast_t *own_right
);

/**
 * Destroy an AST node and all its children.
 * 
 * @param own_node The AST node to destroy (ownership transferred to function)
 * @note Ownership: Takes ownership of the node and destroys it recursively.
 */
void ar__expression_ast__destroy(expression_ast_t *own_node);

/* Accessor functions for node data */

/**
 * Get integer value from an integer literal node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The integer value, or 0 if not an integer literal
 */
int ar__expression_ast__get_int_value(const expression_ast_t *ref_node);

/**
 * Get double value from a double literal node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The double value, or 0.0 if not a double literal
 */
double ar__expression_ast__get_double_value(const expression_ast_t *ref_node);

/**
 * Get string value from a string literal node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The string value (borrowed reference), or NULL if not a string literal
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__expression_ast__get_string_value(const expression_ast_t *ref_node);

/**
 * Get base accessor from a memory access node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The base accessor string (borrowed reference), or NULL if not a memory access
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__expression_ast__get_memory_base(const expression_ast_t *ref_node);

/**
 * Get path components from a memory access node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @param out_count Output parameter for the number of path components
 * @return Array of path components (owned by caller), or NULL if not a memory access
 * @note Ownership: Returns an owned array that caller must free with AR__HEAP__FREE.
 *       The strings in the array are borrowed references and should not be freed.
 */
char** ar__expression_ast__get_memory_path(
    const expression_ast_t *ref_node,
    size_t *out_count
);

/**
 * Get operator from a binary operation node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The binary operator, or OP_ADD if not a binary operation
 */
binary_operator_t ar__expression_ast__get_operator(const expression_ast_t *ref_node);

/**
 * Get left operand from a binary operation node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The left operand (borrowed reference), or NULL if not a binary operation
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const expression_ast_t* ar__expression_ast__get_left(const expression_ast_t *ref_node);

/**
 * Get right operand from a binary operation node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The right operand (borrowed reference), or NULL if not a binary operation
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const expression_ast_t* ar__expression_ast__get_right(const expression_ast_t *ref_node);

#endif /* AGERUN_EXPRESSION_AST_H */
