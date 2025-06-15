#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "agerun_instruction_ast.h"
#include "agerun_list.h"
#include "agerun_heap.h"

static void test_create_assignment_instruction(void) {
    printf("Testing assignment instruction creation...\n");
    
    // Given a memory path and expression
    const char *memory_path = "memory.x";
    const char *expression = "42";
    
    // When creating an assignment instruction AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_assignment(memory_path, expression);
    
    // Then the node should be created successfully with correct type and values
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_node), "memory.x") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_node), "42") == 0);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_nested_assignment_instruction(void) {
    printf("Testing nested assignment instruction creation...\n");
    
    // Given a nested memory path and complex expression
    const char *memory_path = "memory.user.name";
    const char *expression = "\"John Doe\"";
    
    // When creating an assignment instruction AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_assignment(memory_path, expression);
    
    // Then the node should handle nested paths correctly
    assert(own_node != NULL);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_node), "memory.user.name") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_node), "\"John Doe\"") == 0);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_send_function_without_assignment(void) {
    printf("Testing send function without assignment...\n");
    
    // Given send function arguments
    const char *function_name = "send";
    const char *args[] = {"0", "\"Hello\""};
    size_t arg_count = 2;
    
    // When creating a send function call AST node without result assignment
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_SEND, function_name, args, arg_count, NULL
    );
    
    // Then the node should be created successfully
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_SEND);
    assert(strcmp(ar__instruction_ast__get_function_name(own_node), "send") == 0);
    assert(ar__instruction_ast__has_result_assignment(own_node) == false);
    assert(ar__instruction_ast__get_function_result_path(own_node) == NULL);
    
    // Verify arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    void **own_items = ar__list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "0") == 0);
    assert(strcmp((const char*)own_items[1], "\"Hello\"") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_send_function_with_assignment(void) {
    printf("Testing send function with assignment...\n");
    
    // Given send function with result assignment
    const char *function_name = "send";
    const char *args[] = {"memory.target", "memory.message"};
    size_t arg_count = 2;
    const char *result_path = "memory.result";
    
    // When creating a send function call AST node with result assignment
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_SEND, function_name, args, arg_count, result_path
    );
    
    // Then the node should include the result assignment
    assert(own_node != NULL);
    assert(ar__instruction_ast__has_result_assignment(own_node) == true);
    assert(strcmp(ar__instruction_ast__get_function_result_path(own_node), "memory.result") == 0);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_if_function(void) {
    printf("Testing if function creation...\n");
    
    // Given if function arguments
    const char *function_name = "if";
    const char *args[] = {"memory.count > 5", "\"High\"", "\"Low\""};
    size_t arg_count = 3;
    const char *result_path = "memory.level";
    
    // When creating an if function call AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_IF, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created with correct type and arguments
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_IF);
    assert(strcmp(ar__instruction_ast__get_function_name(own_node), "if") == 0);
    
    // Verify arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 3);
    void **own_items = ar__list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "memory.count > 5") == 0);
    assert(strcmp((const char*)own_items[1], "\"High\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"Low\"") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_method_function(void) {
    printf("Testing method function creation...\n");
    
    // Given method function arguments
    const char *function_name = "method";
    const char *args[] = {"\"greet\"", "\"memory.msg := \\\"Hello\\\"\"", "\"1.0.0\""};
    size_t arg_count = 3;
    const char *result_path = "memory.method_created";
    
    // When creating a method function call AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_METHOD);
    assert(ar__instruction_ast__has_result_assignment(own_node) == true);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_agent_function(void) {
    printf("Testing agent function creation...\n");
    
    // Given agent function arguments
    const char *function_name = "agent";
    const char *args[] = {"\"echo\"", "\"1.0.0\"", "memory.context"};
    size_t arg_count = 3;
    const char *result_path = "memory.agent_id";
    
    // When creating an agent function call AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_AGENT, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_AGENT);
    assert(strcmp(ar__instruction_ast__get_function_name(own_node), "agent") == 0);
    assert(strcmp(ar__instruction_ast__get_function_result_path(own_node), "memory.agent_id") == 0);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_destroy_function_one_arg(void) {
    printf("Testing destroy function with one argument...\n");
    
    // Given destroy function for agent
    const char *function_name = "destroy";
    const char *args[] = {"memory.agent_id"};
    size_t arg_count = 1;
    const char *result_path = "memory.destroyed";
    
    // When creating a destroy function call AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_DESTROY);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 1);
    void **own_items = ar__list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "memory.agent_id") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_destroy_function_two_args(void) {
    printf("Testing destroy function with two arguments...\n");
    
    // Given destroy function for method
    const char *function_name = "destroy";
    const char *args[] = {"\"calculator\"", "\"1.0.0\""};
    size_t arg_count = 2;
    
    // When creating a destroy function call AST node without assignment
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, function_name, args, arg_count, NULL
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_DESTROY);
    assert(ar__instruction_ast__has_result_assignment(own_node) == false);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_node);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_parse_function(void) {
    printf("Testing parse function creation...\n");
    
    // Given parse function arguments
    const char *function_name = "parse";
    const char *args[] = {"\"name={name}\"", "\"name=John\""};
    size_t arg_count = 2;
    const char *result_path = "memory.parsed";
    
    // When creating a parse function call AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_PARSE);
    assert(strcmp(ar__instruction_ast__get_function_name(own_node), "parse") == 0);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_create_build_function(void) {
    printf("Testing build function creation...\n");
    
    // Given build function arguments
    const char *function_name = "build";
    const char *args[] = {"\"Hello {name}!\"", "memory.data"};
    size_t arg_count = 2;
    const char *result_path = "memory.greeting";
    
    // When creating a build function call AST node
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, function_name, args, arg_count, result_path
    );
    
    // Then the node should be created correctly
    assert(own_node != NULL);
    assert(ar__instruction_ast__get_type(own_node) == INST_AST_BUILD);
    assert(strcmp(ar__instruction_ast__get_function_name(own_node), "build") == 0);
    assert(strcmp(ar__instruction_ast__get_function_result_path(own_node), "memory.greeting") == 0);
    
    ar__instruction_ast__destroy(own_node);
}

static void test_null_handling(void) {
    printf("Testing NULL handling...\n");
    
    // Test destroying NULL node
    ar__instruction_ast__destroy(NULL); // Should not crash
    
    // Test accessors with NULL node
    assert(ar__instruction_ast__get_type(NULL) == INST_AST_ASSIGNMENT); // Default type
    assert(ar__instruction_ast__get_assignment_path(NULL) == NULL);
    assert(ar__instruction_ast__get_assignment_expression(NULL) == NULL);
    assert(ar__instruction_ast__get_function_name(NULL) == NULL);
    assert(ar__instruction_ast__has_result_assignment(NULL) == false);
    
    assert(ar__instruction_ast__get_function_args(NULL) == NULL);
}

static void test_empty_arguments(void) {
    printf("Testing empty arguments...\n");
    
    // Given a function with no arguments (hypothetical)
    const char *function_name = "test";
    
    // When creating a function call with no arguments
    instruction_ast_t *own_node = ar__instruction_ast__create_function_call(
        INST_AST_SEND, function_name, NULL, 0, NULL
    );
    
    // Then the node should handle empty arguments correctly
    assert(own_node != NULL);
    
    list_t *own_args = ar__instruction_ast__get_function_args(own_node);
    assert(own_args == NULL);
    
    ar__instruction_ast__destroy(own_node);
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
    test_create_destroy_function_one_arg();
    test_create_destroy_function_two_args();
    test_create_parse_function();
    test_create_build_function();
    
    // Edge case tests
    test_null_handling();
    test_empty_arguments();
    
    printf("\nAll instruction_ast tests passed!\n");
    return 0;
}