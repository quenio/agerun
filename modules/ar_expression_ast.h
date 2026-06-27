#ifndef AGERUN_EXPRESSION_AST_H
#define AGERUN_EXPRESSION_AST_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Expression AST node types.
 * Represents all possible expression types in the AgeRun language.
 */
typedef enum {
    AR_EXPRESSION_AST_TYPE__LITERAL_INT,      /* Integer literal (e.g., 42, -10) */
    AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE,   /* Double literal (e.g., 3.14, -2.5) */
    AR_EXPRESSION_AST_TYPE__LITERAL_STRING,   /* String literal (e.g., "hello") */
    AR_EXPRESSION_AST_TYPE__LITERAL_LIST,     /* List literal (e.g., [1, 2]) */
    AR_EXPRESSION_AST_TYPE__LITERAL_MAP,      /* Map literal (e.g., {name: "Ada"}) */
    AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS,    /* Memory/message/context/local access (e.g., memory.x, .name) */
    AR_EXPRESSION_AST_TYPE__BINARY_OP,        /* Binary operation (arithmetic or comparison) */
    AR_EXPRESSION_AST_TYPE__CALL              /* Pure function call (e.g., parse("x={x}", input)) */
} ar_expression_ast_type_t;

/**
 * Binary operator types.
 * Covers both arithmetic and comparison operators.
 */
typedef enum {
    /* Arithmetic operators */
    AR_BINARY_OPERATOR__ADD,        /* + */
    AR_BINARY_OPERATOR__SUBTRACT,   /* - */
    AR_BINARY_OPERATOR__MULTIPLY,   /* * */
    AR_BINARY_OPERATOR__DIVIDE,     /* / */
    
    /* Comparison operators */
    AR_BINARY_OPERATOR__EQUAL,      /* = */
    AR_BINARY_OPERATOR__NOT_EQUAL,  /* <> */
    AR_BINARY_OPERATOR__LESS,       /* < */
    AR_BINARY_OPERATOR__LESS_EQ,    /* <= */
    AR_BINARY_OPERATOR__GREATER,    /* > */
    AR_BINARY_OPERATOR__GREATER_EQ  /* >= */
} ar_binary_operator_t;

/**
 * Opaque AST node structure.
 * Internal structure is hidden to maintain encapsulation.
 */
typedef struct ar_expression_ast_s ar_expression_ast_t;

/**
 * Get the type of an AST node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The type of the node
 */
ar_expression_ast_type_t ar_expression_ast__get_type(const ar_expression_ast_t *ref_node);

/**
 * Create an integer literal AST node.
 * 
 * @param value The integer value
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_int(int value);

/**
 * Create a double literal AST node.
 * 
 * @param value The double value
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_double(double value);

/**
 * Create a string literal AST node.
 * 
 * @param ref_value The string value (borrowed reference)
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function makes a copy of the string parameter.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_string(const char *ref_value);

/**
 * Create a list literal AST node.
 *
 * @param own_items Array of item AST nodes (ownership transferred to node, can be NULL if item_count is 0)
 * @param item_count Number of item AST nodes
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       Takes ownership of all item nodes. The array itself remains owned by the caller.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_list(
    ar_expression_ast_t **own_items,
    size_t item_count
);

/**
 * Create a map literal AST node.
 *
 * @param ref_keys Array of map keys (borrowed references)
 * @param own_values Array of value AST nodes (ownership transferred to node)
 * @param entry_count Number of key/value entries
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       Takes ownership of all value nodes and copies all keys.
 *       The arrays themselves remain owned by the caller.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_map(
    const char **ref_keys,
    ar_expression_ast_t **own_values,
    size_t entry_count
);

/**
 * Enable or disable block-local access for a map literal AST node.
 *
 * @param mut_node The AST node (mutable reference)
 * @param enabled true to evaluate map entries with `.key` access to earlier entries
 * @return true if successful, false if not a map literal
 */
bool ar_expression_ast__set_map_local_access_enabled(
    ar_expression_ast_t *mut_node,
    bool enabled
);

/**
 * Create a memory access AST node.
 * 
 * @param ref_base The base accessor ("memory", "message", "context", or ".") (borrowed reference)
 * @param ref_path Array of path components after the base (borrowed reference, can be NULL)
 * @param path_count Number of path components
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function makes copies of all string parameters.
 */
ar_expression_ast_t* ar_expression_ast__create_memory_access(
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
ar_expression_ast_t* ar_expression_ast__create_binary_op(
    ar_binary_operator_t op,
    ar_expression_ast_t *own_left,
    ar_expression_ast_t *own_right
);

/**
 * Create a pure function call AST node.
 *
 * @param ref_function_name Function name (borrowed reference)
 * @param own_args Array of argument AST nodes (ownership transferred to node)
 * @param arg_count Number of argument AST nodes
 * @return Newly created AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       Takes ownership of all argument nodes and copies the function name.
 *       The array itself remains owned by the caller.
 */
ar_expression_ast_t* ar_expression_ast__create_function_call(
    const char *ref_function_name,
    ar_expression_ast_t **own_args,
    size_t arg_count
);

/**
 * Destroy an AST node and all its children.
 * 
 * @param own_node The AST node to destroy (ownership transferred to function)
 * @note Ownership: Takes ownership of the node and destroys it recursively.
 */
void ar_expression_ast__destroy(ar_expression_ast_t *own_node);

/* Accessor functions for node data */

/**
 * Get integer value from an integer literal node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The integer value, or 0 if not an integer literal
 */
int ar_expression_ast__get_int_value(const ar_expression_ast_t *ref_node);

/**
 * Get double value from a double literal node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The double value, or 0.0 if not a double literal
 */
double ar_expression_ast__get_double_value(const ar_expression_ast_t *ref_node);

/**
 * Get string value from a string literal node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The string value (borrowed reference), or NULL if not a string literal
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar_expression_ast__get_string_value(const ar_expression_ast_t *ref_node);

/**
 * Get the number of items from a list literal node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @return The number of list items, or 0 if not a list literal
 */
size_t ar_expression_ast__get_list_item_count(const ar_expression_ast_t *ref_node);

/**
 * Get a list item AST node from a list literal node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @param index Zero-based list item index
 * @return The item AST node (borrowed reference), or NULL if unavailable
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const ar_expression_ast_t* ar_expression_ast__get_list_item(
    const ar_expression_ast_t *ref_node,
    size_t index
);

/**
 * Get the number of entries from a map literal node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @return The number of map entries, or 0 if not a map literal
 */
size_t ar_expression_ast__get_map_entry_count(const ar_expression_ast_t *ref_node);

/**
 * Get a map key from a map literal node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @param index Zero-based map entry index
 * @return The map key (borrowed reference), or NULL if unavailable
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar_expression_ast__get_map_key(
    const ar_expression_ast_t *ref_node,
    size_t index
);

/**
 * Get a map value AST node from a map literal node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @param index Zero-based map entry index
 * @return The value AST node (borrowed reference), or NULL if unavailable
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const ar_expression_ast_t* ar_expression_ast__get_map_value(
    const ar_expression_ast_t *ref_node,
    size_t index
);

/**
 * Check whether a map literal evaluates entries with block-local `.key` access.
 *
 * @param ref_node The AST node (borrowed reference)
 * @return true if the node is a map literal with block-local access enabled
 */
bool ar_expression_ast__is_map_local_access_enabled(const ar_expression_ast_t *ref_node);

/**
 * Get base accessor from a memory access node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The base accessor string (borrowed reference), or NULL if not a memory access
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar_expression_ast__get_memory_base(const ar_expression_ast_t *ref_node);

/**
 * Get path components from a memory access node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @param out_count Output parameter for the number of path components
 * @return Array of path components (owned by caller), or NULL if not a memory access
 * @note Ownership: Returns an owned array that caller must free with AR__HEAP__FREE.
 *       The strings in the array are borrowed references and should not be freed.
 */
char** ar_expression_ast__get_memory_path(
    const ar_expression_ast_t *ref_node,
    size_t *out_count
);

/**
 * Get the number of path components from a memory access node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @return Number of path components, or 0 if not a memory access
 */
size_t ar_expression_ast__get_memory_path_count(const ar_expression_ast_t *ref_node);

/**
 * Get one path component from a memory access node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @param index Zero-based path component index
 * @return The path component (borrowed reference), or NULL if unavailable
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar_expression_ast__get_memory_path_component(
    const ar_expression_ast_t *ref_node,
    size_t index
);

/**
 * Get operator from a binary operation node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The binary operator, or AR_BINARY_OPERATOR__ADD if not a binary operation
 */
ar_binary_operator_t ar_expression_ast__get_operator(const ar_expression_ast_t *ref_node);

/**
 * Get left operand from a binary operation node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The left operand (borrowed reference), or NULL if not a binary operation
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const ar_expression_ast_t* ar_expression_ast__get_left(const ar_expression_ast_t *ref_node);

/**
 * Get right operand from a binary operation node.
 * 
 * @param ref_node The AST node (borrowed reference)
 * @return The right operand (borrowed reference), or NULL if not a binary operation
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const ar_expression_ast_t* ar_expression_ast__get_right(const ar_expression_ast_t *ref_node);

/**
 * Get the function name from a pure function call node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @return The function name (borrowed reference), or NULL if not a function call
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar_expression_ast__get_function_name(const ar_expression_ast_t *ref_node);

/**
 * Get the number of arguments from a pure function call node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @return The number of arguments, or 0 if not a function call
 */
size_t ar_expression_ast__get_function_arg_count(const ar_expression_ast_t *ref_node);

/**
 * Get one argument AST node from a pure function call node.
 *
 * @param ref_node The AST node (borrowed reference)
 * @param index Zero-based argument index
 * @return The argument AST node (borrowed reference), or NULL if unavailable
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const ar_expression_ast_t* ar_expression_ast__get_function_arg(
    const ar_expression_ast_t *ref_node,
    size_t index
);

/**
 * Formats an expression AST as a human-readable path string
 * @param ref_ast The expression AST to format
 * @return Newly allocated string representation of the expression
 * @note Ownership: Caller owns the returned string and must free it
 * @note Examples: "memory.x", "\"hello\"", "42", "<expression>"
 */
char* ar_expression_ast__format_path(const ar_expression_ast_t *ref_ast);

#endif /* AGERUN_EXPRESSION_AST_H */
