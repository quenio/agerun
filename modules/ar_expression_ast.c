#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_assert.h"
#include "ar_list.h"
#include <string.h>
#include <stdlib.h>

/**
 * Internal structure for integer literal nodes.
 */
typedef struct {
    int value;
} literal_int_data_t;

/**
 * Internal structure for double literal nodes.
 */
typedef struct {
    double value;
} literal_double_data_t;

/**
 * Internal structure for string literal nodes.
 */
typedef struct {
    char *own_value;
} literal_string_data_t;

/**
 * Internal structure for memory access nodes.
 */
typedef struct {
    char *own_base;        /* "memory", "message", or "context" */
    ar_list_t *own_path;      /* List of path components (strings) */
} memory_access_data_t;

/**
 * Internal structure for binary operation nodes.
 */
typedef struct {
    ar_binary_operator_t op;
    ar_expression_ast_t *own_left;
    ar_expression_ast_t *own_right;
} binary_op_data_t;

/**
 * Full definition of the AST node structure.
 * This is only visible in the implementation file.
 */
struct expression_ast_s {
    ar_expression_ast_type_t type;
    union {
        literal_int_data_t literal_int;
        literal_double_data_t literal_double;
        literal_string_data_t literal_string;
        memory_access_data_t memory_access;
        binary_op_data_t binary_op;
    } data;
};

/**
 * Get the type of an AST node.
 */
ar_expression_ast_type_t ar_expression_ast__get_type(const ar_expression_ast_t *ref_node) {
    if (!ref_node) {
        return AR_EXPR__LITERAL_INT; // Default value
    }
    return ref_node->type;
}

/**
 * Create an integer literal AST node.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_int(int value) {
    ar_expression_ast_t *own_node = AR__HEAP__MALLOC(sizeof(ar_expression_ast_t), "Expression AST node (int)");
    if (!own_node) {
        return NULL;
    }
    
    own_node->type = AR_EXPR__LITERAL_INT;
    own_node->data.literal_int.value = value;
    
    AR_ASSERT_OWNERSHIP(own_node);
    return own_node; // Ownership transferred to caller
}

/**
 * Create a double literal AST node.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_double(double value) {
    ar_expression_ast_t *own_node = AR__HEAP__MALLOC(sizeof(ar_expression_ast_t), "Expression AST node (double)");
    if (!own_node) {
        return NULL;
    }
    
    own_node->type = AR_EXPR__LITERAL_DOUBLE;
    own_node->data.literal_double.value = value;
    
    AR_ASSERT_OWNERSHIP(own_node);
    return own_node; // Ownership transferred to caller
}

/**
 * Create a string literal AST node.
 */
ar_expression_ast_t* ar_expression_ast__create_literal_string(const char *ref_value) {
    if (!ref_value) {
        return NULL;
    }
    
    ar_expression_ast_t *own_node = AR__HEAP__MALLOC(sizeof(ar_expression_ast_t), "Expression AST node (string)");
    if (!own_node) {
        return NULL;
    }
    
    own_node->type = AR_EXPR__LITERAL_STRING;
    own_node->data.literal_string.own_value = AR__HEAP__STRDUP(ref_value, "String literal value");
    if (!own_node->data.literal_string.own_value) {
        AR__HEAP__FREE(own_node);
        return NULL;
    }
    
    AR_ASSERT_OWNERSHIP(own_node);
    return own_node; // Ownership transferred to caller
}

/**
 * Create a memory access AST node.
 */
ar_expression_ast_t* ar_expression_ast__create_memory_access(
    const char *ref_base,
    const char **ref_path,
    size_t path_count
) {
    if (!ref_base) {
        return NULL;
    }
    
    ar_expression_ast_t *own_node = AR__HEAP__MALLOC(sizeof(ar_expression_ast_t), "Expression AST node (memory)");
    if (!own_node) {
        return NULL;
    }
    
    own_node->type = AR_EXPR__MEMORY_ACCESS;
    own_node->data.memory_access.own_base = AR__HEAP__STRDUP(ref_base, "Memory access base");
    if (!own_node->data.memory_access.own_base) {
        AR__HEAP__FREE(own_node);
        return NULL;
    }
    
    // Create list for path components
    own_node->data.memory_access.own_path = ar_list__create();
    if (!own_node->data.memory_access.own_path) {
        AR__HEAP__FREE(own_node->data.memory_access.own_base);
        AR__HEAP__FREE(own_node);
        return NULL;
    }
    
    // Copy path components
    if (ref_path && path_count > 0) {
        for (size_t i = 0; i < path_count; i++) {
            if (ref_path[i]) {
                char *own_component = AR__HEAP__STRDUP(ref_path[i], "Memory path component");
                if (!own_component) {
                    // Clean up on failure
                    ar_expression_ast__destroy(own_node);
                    return NULL;
                }
                ar_list__add_last(own_node->data.memory_access.own_path, own_component);
            }
        }
    }
    
    AR_ASSERT_OWNERSHIP(own_node);
    return own_node; // Ownership transferred to caller
}

/**
 * Create a binary operation AST node.
 */
ar_expression_ast_t* ar_expression_ast__create_binary_op(
    ar_binary_operator_t op,
    ar_expression_ast_t *own_left,
    ar_expression_ast_t *own_right
) {
    if (!own_left || !own_right) {
        // Clean up any provided nodes
        if (own_left) ar_expression_ast__destroy(own_left);
        if (own_right) ar_expression_ast__destroy(own_right);
        return NULL;
    }
    
    ar_expression_ast_t *own_node = AR__HEAP__MALLOC(sizeof(ar_expression_ast_t), "Expression AST node (binary)");
    if (!own_node) {
        // Clean up provided nodes on failure
        ar_expression_ast__destroy(own_left);
        ar_expression_ast__destroy(own_right);
        return NULL;
    }
    
    own_node->type = AR_EXPR__BINARY_OP;
    own_node->data.binary_op.op = op;
    own_node->data.binary_op.own_left = own_left;   // Ownership transferred
    own_node->data.binary_op.own_right = own_right; // Ownership transferred
    
    AR_ASSERT_OWNERSHIP(own_node);
    
    return own_node; // Ownership transferred to caller
}

/**
 * Destroy an AST node and all its children.
 */
void ar_expression_ast__destroy(ar_expression_ast_t *own_node) {
    if (!own_node) {
        return;
    }
    
    switch (own_node->type) {
        case AR_EXPR__LITERAL_INT:
        case AR_EXPR__LITERAL_DOUBLE:
            // No dynamic memory to free
            break;
            
        case AR_EXPR__LITERAL_STRING:
            if (own_node->data.literal_string.own_value) {
                AR__HEAP__FREE(own_node->data.literal_string.own_value);
            }
            break;
            
        case AR_EXPR__MEMORY_ACCESS:
            if (own_node->data.memory_access.own_base) {
                AR__HEAP__FREE(own_node->data.memory_access.own_base);
            }
            if (own_node->data.memory_access.own_path) {
                // Free all path components
                size_t count = ar_list__count(own_node->data.memory_access.own_path);
                if (count > 0) {
                    void **own_items = ar_list__items(own_node->data.memory_access.own_path);
                    if (own_items) {
                        for (size_t i = 0; i < count; i++) {
                            if (own_items[i]) {
                                AR__HEAP__FREE(own_items[i]);
                            }
                        }
                        AR__HEAP__FREE(own_items);
                    }
                }
                ar_list__destroy(own_node->data.memory_access.own_path);
            }
            break;
            
        case AR_EXPR__BINARY_OP:
            if (own_node->data.binary_op.own_left) {
                ar_expression_ast__destroy(own_node->data.binary_op.own_left);
            }
            if (own_node->data.binary_op.own_right) {
                ar_expression_ast__destroy(own_node->data.binary_op.own_right);
            }
            break;
    }
    
    AR__HEAP__FREE(own_node);
}

/* Accessor function implementations */

int ar_expression_ast__get_int_value(const ar_expression_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_EXPR__LITERAL_INT) {
        return 0;
    }
    return ref_node->data.literal_int.value;
}

double ar_expression_ast__get_double_value(const ar_expression_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_EXPR__LITERAL_DOUBLE) {
        return 0.0;
    }
    return ref_node->data.literal_double.value;
}

const char* ar_expression_ast__get_string_value(const ar_expression_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_EXPR__LITERAL_STRING) {
        return NULL;
    }
    return ref_node->data.literal_string.own_value;
}

const char* ar_expression_ast__get_memory_base(const ar_expression_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_EXPR__MEMORY_ACCESS) {
        return NULL;
    }
    return ref_node->data.memory_access.own_base;
}

char** ar_expression_ast__get_memory_path(
    const ar_expression_ast_t *ref_node,
    size_t *out_count
) {
    if (!ref_node || ref_node->type != AR_EXPR__MEMORY_ACCESS || !out_count) {
        if (out_count) *out_count = 0;
        return NULL;
    }
    
    *out_count = ar_list__count(ref_node->data.memory_access.own_path);
    if (*out_count == 0) {
        return NULL;
    }
    
    // Transfer ownership of the array to caller - they must free it
    return (char**)ar_list__items(ref_node->data.memory_access.own_path);
}

ar_binary_operator_t ar_expression_ast__get_operator(const ar_expression_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_EXPR__BINARY_OP) {
        return AR_OP__ADD; // Default value
    }
    return ref_node->data.binary_op.op;
}

const ar_expression_ast_t* ar_expression_ast__get_left(const ar_expression_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_EXPR__BINARY_OP) {
        return NULL;
    }
    return ref_node->data.binary_op.own_left;
}

const ar_expression_ast_t* ar_expression_ast__get_right(const ar_expression_ast_t *ref_node) {
    if (!ref_node || ref_node->type != AR_EXPR__BINARY_OP) {
        return NULL;
    }
    return ref_node->data.binary_op.own_right;
}
