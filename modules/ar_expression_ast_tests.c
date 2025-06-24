#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "ar_expression_ast.h"
#include "ar_heap.h"

static void test_create_integer_literal(void) {
    printf("Testing integer literal creation...\n");
    
    // Given an integer value
    int value = 42;
    
    // When creating an integer literal AST node
    ar_expression_ast_t *own_node = ar__expression_ast__create_literal_int(value);
    
    // Then the node should be created successfully with correct type and value
    assert(own_node != NULL);
    assert(ar__expression_ast__get_type(own_node) == AR_EXPR__LITERAL_INT);
    assert(ar__expression_ast__get_int_value(own_node) == 42);
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_negative_integer_literal(void) {
    printf("Testing negative integer literal creation...\n");
    
    // Given a negative integer value
    int value = -100;
    
    // When creating an integer literal AST node
    ar_expression_ast_t *own_node = ar__expression_ast__create_literal_int(value);
    
    // Then the node should be created with the negative value
    assert(own_node != NULL);
    assert(ar__expression_ast__get_int_value(own_node) == -100);
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_double_literal(void) {
    printf("Testing double literal creation...\n");
    
    // Given a double value
    double value = 3.14;
    
    // When creating a double literal AST node
    ar_expression_ast_t *own_node = ar__expression_ast__create_literal_double(value);
    
    // Then the node should be created successfully with correct type and value
    assert(own_node != NULL);
    assert(ar__expression_ast__get_type(own_node) == AR_EXPR__LITERAL_DOUBLE);
    assert(fabs(ar__expression_ast__get_double_value(own_node) - 3.14) < 0.001);
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_string_literal(void) {
    printf("Testing string literal creation...\n");
    
    // Given a string value
    const char *value = "hello world";
    
    // When creating a string literal AST node
    ar_expression_ast_t *own_node = ar__expression_ast__create_literal_string(value);
    
    // Then the node should be created successfully with a copy of the string
    assert(own_node != NULL);
    assert(ar__expression_ast__get_type(own_node) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(own_node), "hello world") == 0);
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_string_literal_with_null(void) {
    printf("Testing string literal creation with NULL...\n");
    
    // Given a NULL string value
    const char *value = NULL;
    
    // When creating a string literal AST node with NULL
    ar_expression_ast_t *own_node = ar__expression_ast__create_literal_string(value);
    
    // Then the creation should fail
    assert(own_node == NULL);
}

static void test_create_simple_memory_access(void) {
    printf("Testing simple memory access creation...\n");
    
    // Given a base accessor with no path
    const char *base = "memory";
    
    // When creating a memory access AST node
    ar_expression_ast_t *own_node = ar__expression_ast__create_memory_access(base, NULL, 0);
    
    // Then the node should be created with just the base
    assert(own_node != NULL);
    assert(ar__expression_ast__get_type(own_node) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar__expression_ast__get_memory_base(own_node), "memory") == 0);
    
    size_t path_count = 0;
    char **own_path = ar__expression_ast__get_memory_path(own_node, &path_count);
    assert(path_count == 0);
    assert(own_path == NULL);
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_memory_access_with_path(void) {
    printf("Testing memory access with path creation...\n");
    
    // Given a base accessor with a path
    const char *base = "message";
    const char *path_components[] = {"user", "name"};
    
    // When creating a memory access AST node with path
    ar_expression_ast_t *own_node = ar__expression_ast__create_memory_access(base, path_components, 2);
    
    // Then the node should be created with the base and path components
    assert(own_node != NULL);
    assert(strcmp(ar__expression_ast__get_memory_base(own_node), "message") == 0);
    
    size_t path_count = 0;
    char **own_path = ar__expression_ast__get_memory_path(own_node, &path_count);
    assert(path_count == 2);
    assert(own_path != NULL);
    assert(strcmp(own_path[0], "user") == 0);
    assert(strcmp(own_path[1], "name") == 0);
    AR__HEAP__FREE(own_path);  // Free the array after use
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_binary_addition(void) {
    printf("Testing binary addition creation...\n");
    
    // Given two integer literal nodes
    ar_expression_ast_t *own_left = ar__expression_ast__create_literal_int(5);
    ar_expression_ast_t *own_right = ar__expression_ast__create_literal_int(3);
    
    // When creating a binary addition node
    ar_expression_ast_t *own_node = ar__expression_ast__create_binary_op(AR_OP__ADD, own_left, own_right);
    
    // Then the node should be created with correct operator and operands
    assert(own_node != NULL);
    assert(ar__expression_ast__get_type(own_node) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_node) == AR_OP__ADD);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_node);
    const ar_expression_ast_t *ref_right = ar__expression_ast__get_right(own_node);
    assert(ar__expression_ast__get_int_value(ref_left) == 5);
    assert(ar__expression_ast__get_int_value(ref_right) == 3);
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_binary_comparison(void) {
    printf("Testing binary comparison creation...\n");
    
    // Given two string literal nodes
    ar_expression_ast_t *own_left = ar__expression_ast__create_literal_string("hello");
    ar_expression_ast_t *own_right = ar__expression_ast__create_literal_string("world");
    
    // When creating a binary equality comparison node
    ar_expression_ast_t *own_node = ar__expression_ast__create_binary_op(AR_OP__EQUAL, own_left, own_right);
    
    // Then the node should be created with correct operator and operands
    assert(own_node != NULL);
    assert(ar__expression_ast__get_operator(own_node) == AR_OP__EQUAL);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_node);
    const ar_expression_ast_t *ref_right = ar__expression_ast__get_right(own_node);
    assert(strcmp(ar__expression_ast__get_string_value(ref_left), "hello") == 0);
    assert(strcmp(ar__expression_ast__get_string_value(ref_right), "world") == 0);
    
    ar__expression_ast__destroy(own_node);
}

static void test_create_nested_expression(void) {
    printf("Testing nested expression creation...\n");
    
    // Given a nested expression: (1 + 2) * 3
    ar_expression_ast_t *own_one = ar__expression_ast__create_literal_int(1);
    ar_expression_ast_t *own_two = ar__expression_ast__create_literal_int(2);
    ar_expression_ast_t *own_add = ar__expression_ast__create_binary_op(AR_OP__ADD, own_one, own_two);
    ar_expression_ast_t *own_three = ar__expression_ast__create_literal_int(3);
    
    // When creating the multiplication node
    ar_expression_ast_t *own_multiply = ar__expression_ast__create_binary_op(AR_OP__MULTIPLY, own_add, own_three);
    
    // Then the nested structure should be preserved
    assert(own_multiply != NULL);
    assert(ar__expression_ast__get_operator(own_multiply) == AR_OP__MULTIPLY);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_multiply);
    assert(ar__expression_ast__get_type(ref_left) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(ref_left) == AR_OP__ADD);
    
    ar__expression_ast__destroy(own_multiply);
}

static void test_binary_op_with_null_operands(void) {
    printf("Testing binary operation with NULL operands...\n");
    
    // Given one valid operand and one NULL
    ar_expression_ast_t *own_left = ar__expression_ast__create_literal_int(1);
    
    // When creating a binary operation with NULL right operand
    ar_expression_ast_t *own_node = ar__expression_ast__create_binary_op(AR_OP__ADD, own_left, NULL);
    
    // Then the creation should fail and clean up the valid operand
    assert(own_node == NULL);
    
    // Given one NULL and one valid operand
    ar_expression_ast_t *own_right = ar__expression_ast__create_literal_int(2);
    
    // When creating a binary operation with NULL left operand
    own_node = ar__expression_ast__create_binary_op(AR_OP__ADD, NULL, own_right);
    
    // Then the creation should fail and clean up the valid operand
    assert(own_node == NULL);
}

static void test_accessor_type_safety(void) {
    printf("Testing accessor type safety...\n");
    
    // Given an integer literal node
    ar_expression_ast_t *own_int = ar__expression_ast__create_literal_int(42);
    
    // When accessing with wrong type accessors
    // Then they should return default values
    assert(ar__expression_ast__get_double_value(own_int) == 0.0);
    assert(ar__expression_ast__get_string_value(own_int) == NULL);
    assert(ar__expression_ast__get_memory_base(own_int) == NULL);
    assert(ar__expression_ast__get_left(own_int) == NULL);
    assert(ar__expression_ast__get_right(own_int) == NULL);
    
    ar__expression_ast__destroy(own_int);
}

static void test_complex_expression_tree(void) {
    printf("Testing complex expression tree...\n");
    
    // Given a complex expression: (memory.x + 5) > (message.count * 2)
    
    // Build left side: memory.x + 5
    const char *path_x[] = {"x"};
    ar_expression_ast_t *own_memory_x = ar__expression_ast__create_memory_access("memory", path_x, 1);
    ar_expression_ast_t *own_five = ar__expression_ast__create_literal_int(5);
    ar_expression_ast_t *own_add = ar__expression_ast__create_binary_op(AR_OP__ADD, own_memory_x, own_five);
    
    // Build right side: message.count * 2
    const char *path_count[] = {"count"};
    ar_expression_ast_t *own_msg_count = ar__expression_ast__create_memory_access("message", path_count, 1);
    ar_expression_ast_t *own_two = ar__expression_ast__create_literal_int(2);
    ar_expression_ast_t *own_multiply = ar__expression_ast__create_binary_op(AR_OP__MULTIPLY, own_msg_count, own_two);
    
    // When creating the comparison
    ar_expression_ast_t *own_compare = ar__expression_ast__create_binary_op(AR_OP__GREATER, own_add, own_multiply);
    
    // Then the entire tree structure should be correct
    assert(own_compare != NULL);
    assert(ar__expression_ast__get_type(own_compare) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_compare) == AR_OP__GREATER);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_compare);
    const ar_expression_ast_t *ref_right = ar__expression_ast__get_right(own_compare);
    assert(ar__expression_ast__get_type(ref_left) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(ref_left) == AR_OP__ADD);
    assert(ar__expression_ast__get_type(ref_right) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(ref_right) == AR_OP__MULTIPLY);
    
    ar__expression_ast__destroy(own_compare);
}

static void test_all_operator_types(void) {
    printf("Testing all operator types...\n");
    
    // Given all supported operator types
    ar_binary_operator_t operators[] = {
        AR_OP__ADD, AR_OP__SUBTRACT, AR_OP__MULTIPLY, AR_OP__DIVIDE,
        AR_OP__EQUAL, AR_OP__NOT_EQUAL, AR_OP__LESS, AR_OP__LESS_EQ, AR_OP__GREATER, AR_OP__GREATER_EQ
    };
    
    // When creating binary operations with each operator
    for (int i = 0; i < 10; i++) {
        ar_expression_ast_t *own_left = ar__expression_ast__create_literal_int(10);
        ar_expression_ast_t *own_right = ar__expression_ast__create_literal_int(5);
        ar_expression_ast_t *own_node = ar__expression_ast__create_binary_op(operators[i], own_left, own_right);
        
        // Then each should be created with the correct operator
        assert(own_node != NULL);
        assert(ar__expression_ast__get_operator(own_node) == operators[i]);
        
        ar__expression_ast__destroy(own_node);
    }
}

static void test_null_node_safety(void) {
    printf("Testing NULL node safety...\n");
    
    // Given NULL node pointers
    // When calling accessor functions with NULL
    // Then they should return safe default values
    assert(ar__expression_ast__get_type(NULL) == AR_EXPR__LITERAL_INT);
    assert(ar__expression_ast__get_int_value(NULL) == 0);
    assert(ar__expression_ast__get_double_value(NULL) == 0.0);
    assert(ar__expression_ast__get_string_value(NULL) == NULL);
    assert(ar__expression_ast__get_memory_base(NULL) == NULL);
    assert(ar__expression_ast__get_operator(NULL) == AR_OP__ADD);
    assert(ar__expression_ast__get_left(NULL) == NULL);
    assert(ar__expression_ast__get_right(NULL) == NULL);
    
    size_t count = 999;
    char **own_path = ar__expression_ast__get_memory_path(NULL, &count);
    assert(own_path == NULL);
    assert(count == 0);
    
    // When destroying NULL
    // Then it should not crash
    ar__expression_ast__destroy(NULL);
}

int main(void) {
    printf("Running expression AST node tests...\n\n");
    
    test_create_integer_literal();
    test_create_negative_integer_literal();
    test_create_double_literal();
    test_create_string_literal();
    test_create_string_literal_with_null();
    test_create_simple_memory_access();
    test_create_memory_access_with_path();
    test_create_binary_addition();
    test_create_binary_comparison();
    test_create_nested_expression();
    test_binary_op_with_null_operands();
    test_accessor_type_safety();
    test_complex_expression_tree();
    test_all_operator_types();
    test_null_node_safety();
    
    printf("\nAll expression AST node tests passed!\n");
    return 0;
}
