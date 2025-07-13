#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_expression_parser.h"
#include "ar_list.h"
#include "ar_heap.h"
#include "ar_log.h"

static void test_create_assignment_instruction(void) {
    printf("Testing assignment instruction creation...\n");
    
    // Given a memory path and expression
    const char *memory_path = "memory.x";
    const char *expression = "42";
    
    // When creating an assignment instruction AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_assignment(memory_path, expression);
    
    // Then the node should be created successfully with correct type and values
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_node), "memory.x") == 0);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_node), "42") == 0);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_nested_assignment_instruction(void) {
    printf("Testing nested assignment instruction creation...\n");
    
    // Given a nested memory path and complex expression
    const char *memory_path = "memory.user.name";
    const char *expression = "\"John Doe\"";
    
    // When creating an assignment instruction AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_assignment(memory_path, expression);
    
    // Then the node should handle nested paths correctly
    assert(own_node != NULL);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_node), "memory.user.name") == 0);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_node), "\"John Doe\"") == 0);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_send_function_without_assignment(void) {
    printf("Testing send function without assignment...\n");
    
    // Given send function arguments
    const char *function_name = "send";
    const char *args[] = {"0", "\"Hello\""};
    size_t arg_count = 2;
    
    // When creating a send function call AST node without result assignment
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, function_name, args, arg_count, NULL
    );
    
    // Then the node should be created successfully
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__SEND);
    assert(strcmp(ar_instruction_ast__get_function_name(own_node), "send") == 0);
    assert(ar_instruction_ast__has_result_assignment(own_node) == false);
    assert(ar_instruction_ast__get_function_result_path(own_node) == NULL);
    
    // Verify arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "0") == 0);
    assert(strcmp((const char*)own_items[1], "\"Hello\"") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_send_function_with_assignment(void) {
    printf("Testing send function with assignment...\n");
    
    // Given send function with result assignment
    const char *function_name = "send";
    const char *args[] = {"memory.target", "memory.message"};
    size_t arg_count = 2;
    const char *result_path = "memory.result";
    
    // When creating a send function call AST node with result assignment
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, function_name, args, arg_count, result_path
    );
    
    // Then the node should include the result assignment
    assert(own_node != NULL);
    assert(ar_instruction_ast__has_result_assignment(own_node) == true);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_node), "memory.result") == 0);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_if_function(void) {
    printf("Testing if function creation...\n");
    
    // Given if function arguments
    const char *function_name = "if";
    const char *args[] = {"memory.count > 5", "\"High\"", "\"Low\""};
    size_t arg_count = 3;
    const char *result_path = "memory.level";
    
    // When creating an if function call AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created with correct type and arguments
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__IF);
    assert(strcmp(ar_instruction_ast__get_function_name(own_node), "if") == 0);
    
    // Verify arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 3);
    void **own_items = ar_list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "memory.count > 5") == 0);
    assert(strcmp((const char*)own_items[1], "\"High\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"Low\"") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_method_function(void) {
    printf("Testing method function creation...\n");
    
    // Given method function arguments
    const char *function_name = "method";
    const char *args[] = {"\"greet\"", "\"memory.msg := \\\"Hello\\\"\"", "\"1.0.0\""};
    size_t arg_count = 3;
    const char *result_path = "memory.method_created";
    
    // When creating a compile function call AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__COMPILE);
    assert(ar_instruction_ast__has_result_assignment(own_node) == true);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_agent_function(void) {
    printf("Testing create function creation...\n");
    
    // Given create function arguments
    const char *function_name = "create";
    const char *args[] = {"\"echo\"", "\"1.0.0\"", "memory.context"};
    size_t arg_count = 3;
    const char *result_path = "memory.agent_id";
    
    // When creating a create function call AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__CREATE, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__CREATE);
    assert(strcmp(ar_instruction_ast__get_function_name(own_node), "create") == 0);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_node), "memory.agent_id") == 0);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_destroy_agent_function(void) {
    printf("Testing destroy agent function...\n");
    
    // Given destroy function for agent
    const char *function_name = "destroy";
    const char *args[] = {"memory.agent_id"};
    size_t arg_count = 1;
    const char *result_path = "memory.destroyed";
    
    // When creating a destroy agent function call AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DESTROY, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__DESTROY);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 1);
    void **own_items = ar_list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "memory.agent_id") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_destroy_method_function(void) {
    printf("Testing destroy method function...\n");
    
    // Given destroy function for method
    const char *function_name = "destroy";
    const char *args[] = {"\"calculator\"", "\"1.0.0\""};
    size_t arg_count = 2;
    
    // When creating a destroy method function call AST node without assignment
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DEPRECATE, function_name, args, arg_count, NULL
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__DEPRECATE);
    assert(ar_instruction_ast__has_result_assignment(own_node) == false);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_parse_function(void) {
    printf("Testing parse function creation...\n");
    
    // Given parse function arguments
    const char *function_name = "parse";
    const char *args[] = {"\"name={name}\"", "\"name=John\""};
    size_t arg_count = 2;
    const char *result_path = "memory.parsed";
    
    // When creating a parse function call AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__PARSE);
    assert(strcmp(ar_instruction_ast__get_function_name(own_node), "parse") == 0);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_create_build_function(void) {
    printf("Testing build function creation...\n");
    
    // Given build function arguments
    const char *function_name = "build";
    const char *args[] = {"\"Hello {name}!\"", "memory.data"};
    size_t arg_count = 2;
    const char *result_path = "memory.greeting";
    
    // When creating a build function call AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__BUILD, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar_instruction_ast__get_type(own_node) == AR_INSTRUCTION_AST_TYPE__BUILD);
    assert(strcmp(ar_instruction_ast__get_function_name(own_node), "build") == 0);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_node), "memory.greeting") == 0);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_null_handling(void) {
    printf("Testing NULL handling...\n");
    
    // Test destroying NULL node
    ar_instruction_ast__destroy(NULL); // Should not crash
    
    // Test accessors with NULL node
    assert(ar_instruction_ast__get_type(NULL) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT); // Default type
    assert(ar_instruction_ast__get_assignment_path(NULL) == NULL);
    assert(ar_instruction_ast__get_assignment_expression(NULL) == NULL);
    assert(ar_instruction_ast__get_function_name(NULL) == NULL);
    assert(ar_instruction_ast__has_result_assignment(NULL) == false);
    
    assert(ar_instruction_ast__get_function_args(NULL) == NULL);
}

static void test_empty_arguments(void) {
    printf("Testing empty arguments...\n");
    
    // Given a function with no arguments (hypothetical)
    const char *function_name = "test";
    
    // When creating a function call with no arguments
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, function_name, NULL, 0, NULL
    );
    
    // Then the node should handle empty arguments correctly
    assert(own_node != NULL);
    
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_node);
    assert(own_args == NULL);
    
    ar_instruction_ast__destroy(own_node);
}

static void test_instruction_ast__assignment_expression_ast(void) {
    printf("Testing assignment instruction with expression AST...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an assignment instruction
    const char *memory_path = "memory.x";
    const char *expression = "42";
    
    // When creating an assignment instruction AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_assignment(memory_path, expression);
    assert(own_node != NULL);
    
    // Initially, expression AST should be NULL
    assert(ar_instruction_ast__get_assignment_expression_ast(own_node) == NULL);
    
    // When setting an expression AST
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    assert(own_parser != NULL);
    
    ar_expression_ast_t *own_expr_ast = ar_expression_parser__parse_expression(own_parser);
    assert(own_expr_ast != NULL);
    ar_expression_parser__destroy(own_parser);
    
    bool success = ar_instruction_ast__set_assignment_expression_ast(own_node, own_expr_ast);
    assert(success == true);
    
    // Then the expression AST should be retrievable
    const ar_expression_ast_t *ref_stored_ast = ar_instruction_ast__get_assignment_expression_ast(own_node);
    assert(ref_stored_ast != NULL);
    assert(ref_stored_ast == own_expr_ast); // Same pointer
    
    // And the string expression should still be available
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_node), "42") == 0);
    
    // Cleanup (should destroy the embedded expression AST)
    ar_instruction_ast__destroy(own_node);
    ar_log__destroy(log);
}

static void test_instruction_ast__function_arg_asts(void) {
    printf("Testing function instruction with argument ASTs...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a send function with arguments
    const char *function_name = "send";
    const char *args[] = {"1", "\"Hello\""};
    size_t arg_count = 2;
    
    // When creating a send function call AST node
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, function_name, args, arg_count, NULL
    );
    assert(own_node != NULL);
    
    // Initially, argument ASTs should be NULL
    assert(ar_instruction_ast__get_function_arg_asts(own_node) == NULL);
    
    // When creating expression ASTs for arguments
    ar_list_t *own_arg_asts = ar_list__create();
    assert(own_arg_asts != NULL);
    
    for (size_t i = 0; i < arg_count; i++) {
        ar_expression_parser_t *own_parser = ar_expression_parser__create(log, args[i]);
        assert(own_parser != NULL);
        ar_expression_ast_t *own_arg_ast = ar_expression_parser__parse_expression(own_parser);
        assert(own_arg_ast != NULL);
        ar_expression_parser__destroy(own_parser);
        ar_list__add_last(own_arg_asts, own_arg_ast);
    }
    
    // When setting the argument ASTs
    bool success = ar_instruction_ast__set_function_arg_asts(own_node, own_arg_asts);
    assert(success == true);
    
    // Then the argument ASTs should be retrievable
    const ar_list_t *ref_stored_asts = ar_instruction_ast__get_function_arg_asts(own_node);
    assert(ref_stored_asts != NULL);
    assert(ar_list__count(ref_stored_asts) == 2);
    
    // And the string arguments should still be available
    ar_list_t *own_string_args = ar_instruction_ast__get_function_args(own_node);
    assert(own_string_args != NULL);
    assert(ar_list__count(own_string_args) == 2);
    ar_list__destroy(own_string_args);
    
    // Cleanup (should destroy all embedded expression ASTs)
    ar_instruction_ast__destroy(own_node);
    ar_log__destroy(log);
}

static void test_instruction_ast__expression_ast_replacement(void) {
    printf("Testing expression AST replacement...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an assignment with an expression AST
    ar_instruction_ast_t *own_node = ar_instruction_ast__create_assignment("memory.x", "42");
    assert(own_node != NULL);
    
    ar_expression_parser_t *own_parser1 = ar_expression_parser__create(log, "42");
    assert(own_parser1 != NULL);
    
    ar_expression_ast_t *own_first_ast = ar_expression_parser__parse_expression(own_parser1);
    assert(own_first_ast != NULL);
    ar_expression_parser__destroy(own_parser1);
    
    ar_instruction_ast__set_assignment_expression_ast(own_node, own_first_ast);
    
    // When replacing the expression AST with a new one
    ar_expression_parser_t *own_parser2 = ar_expression_parser__create(log, "100");
    assert(own_parser2 != NULL);
    
    ar_expression_ast_t *own_second_ast = ar_expression_parser__parse_expression(own_parser2);
    assert(own_second_ast != NULL);
    ar_expression_parser__destroy(own_parser2);
    
    bool success = ar_instruction_ast__set_assignment_expression_ast(own_node, own_second_ast);
    assert(success == true);
    
    // Then the new AST should be stored (old one destroyed internally)
    const ar_expression_ast_t *ref_stored_ast = ar_instruction_ast__get_assignment_expression_ast(own_node);
    assert(ref_stored_ast == own_second_ast);
    
    // Cleanup
    ar_instruction_ast__destroy(own_node);
    ar_log__destroy(log);
}

static void test_instruction_ast__expression_ast_null_handling(void) {
    printf("Testing expression AST null handling...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Test setting expression AST on wrong node type
    ar_instruction_ast_t *own_send_node = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SEND, "send", NULL, 0, NULL
    );
    assert(own_send_node != NULL);
    
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, "42");
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    ar_expression_parser__destroy(own_parser);
    
    // Should fail to set expression AST on non-assignment node
    bool success = ar_instruction_ast__set_assignment_expression_ast(own_send_node, own_ast);
    assert(success == false);
    
    // Clean up the AST since it wasn't taken
    ar_expression_ast__destroy(own_ast);
    
    // Test getting expression AST from wrong node type
    assert(ar_instruction_ast__get_assignment_expression_ast(own_send_node) == NULL);
    
    ar_instruction_ast__destroy(own_send_node);
    
    // Test setting arg ASTs on wrong node type
    ar_instruction_ast_t *own_assign_node = ar_instruction_ast__create_assignment("memory.x", "42");
    assert(own_assign_node != NULL);
    
    ar_list_t *own_arg_asts = ar_list__create();
    success = ar_instruction_ast__set_function_arg_asts(own_assign_node, own_arg_asts);
    assert(success == false);
    
    // Clean up the list since it wasn't taken
    ar_list__destroy(own_arg_asts);
    
    // Test getting arg ASTs from wrong node type  
    assert(ar_instruction_ast__get_function_arg_asts(own_assign_node) == NULL);
    
    ar_instruction_ast__destroy(own_assign_node);
    ar_log__destroy(log);
}

int main(void) {
    printf("Running instruction AST tests...\n\n");
    
    // Assignment tests
    test_create_assignment_instruction();
    test_create_nested_assignment_instruction();
    
    // Function call tests
    test_create_send_function_without_assignment();
    test_create_send_function_with_assignment();
    test_create_if_function();
    test_create_method_function();
    test_create_agent_function();
    test_create_destroy_agent_function();
    test_create_destroy_method_function();
    test_create_parse_function();
    test_create_build_function();
    
    // Edge case tests
    test_null_handling();
    test_empty_arguments();
    
    // Expression AST tests
    test_instruction_ast__assignment_expression_ast();
    test_instruction_ast__function_arg_asts();
    test_instruction_ast__expression_ast_replacement();
    test_instruction_ast__expression_ast_null_handling();
    
    printf("\nAll instruction_ast tests passed!\n");
    return 0;
}
