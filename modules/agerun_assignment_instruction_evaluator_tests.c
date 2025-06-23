#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "agerun_assignment_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_expression_ast.h"
#include "agerun_data.h"

static void test_assignment_instruction_evaluator__create_destroy(void) {
    // Given expression evaluator and memory
    data_t *own_memory = ar__data__create_map();
    assert(own_memory != NULL);
    
    expression_evaluator_t *own_expr_eval = ar__expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating an assignment instruction evaluator
    assignment_instruction_evaluator_t *own_evaluator = ar_assignment_instruction_evaluator__create(own_expr_eval, own_memory);
    
    // Then it should be created successfully
    assert(own_evaluator != NULL);
    
    // When destroying the evaluator
    ar_assignment_instruction_evaluator__destroy(own_evaluator);
    
    // Then cleanup other resources
    ar__expression_evaluator__destroy(own_expr_eval);
    ar__data__destroy(own_memory);
}

static void test_assignment_instruction_evaluator__evaluate_with_instance(void) {
    // Given an assignment instruction evaluator
    data_t *own_memory = ar__data__create_map();
    assert(own_memory != NULL);
    
    expression_evaluator_t *own_expr_eval = ar__expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    assignment_instruction_evaluator_t *own_evaluator = ar_assignment_instruction_evaluator__create(own_expr_eval, own_memory);
    assert(own_evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.count := 42
    instruction_ast_t *own_ast = ar__instruction_ast__create_assignment("memory.count", "42");
    assert(own_ast != NULL);
    
    // Create and attach the expression AST
    expression_ast_t *own_expr_ast = ar__expression_ast__create_literal_int(42);
    assert(own_expr_ast != NULL);
    bool ast_set = ar__instruction_ast__set_assignment_expression_ast(own_ast, own_expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(own_evaluator, own_ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    int count = ar__data__get_map_integer(own_memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar__instruction_ast__destroy(own_ast);
    ar_assignment_instruction_evaluator__destroy(own_evaluator);
    ar__expression_evaluator__destroy(own_expr_eval);
    ar__data__destroy(own_memory);
}

static void test_assignment_instruction_evaluator__evaluate_integer(void) {
    // Given an assignment evaluator with empty memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.count := 42
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.count", "42");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    expression_ast_t *expr_ast = ar__expression_ast__create_literal_int(42);
    assert(expr_ast != NULL);
    bool ast_set = ar__instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    int count = ar__data__get_map_integer(memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_assignment_instruction_evaluator__evaluate_string(void) {
    // Given an evaluator with empty memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.name := "Alice"
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.name", "\"Alice\"");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    expression_ast_t *expr_ast = ar__expression_ast__create_literal_string("Alice");
    assert(expr_ast != NULL);
    bool ast_set = ar__instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    const char *name = ar__data__get_map_string(memory, "name");
    assert(name != NULL);
    assert(strcmp(name, "Alice") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_assignment_instruction_evaluator__evaluate_nested_path(void) {
    // Given an evaluator with memory containing a nested map
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *user = ar__data__create_map();
    assert(user != NULL);
    ar__data__set_map_data(memory, "user", user);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment to a nested path: memory.user.age := 25
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.user.age", "25");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    expression_ast_t *expr_ast = ar__expression_ast__create_literal_int(25);
    assert(expr_ast != NULL);
    bool ast_set = ar__instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in the nested map
    data_t *stored_user = ar__data__get_map_data(memory, "user");
    assert(stored_user != NULL);
    int age = ar__data__get_map_integer(stored_user, "age");
    assert(age == 25);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_assignment_instruction_evaluator__evaluate_expression(void) {
    // Given an evaluator with memory containing initial values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    ar__data__set_map_integer(memory, "x", 10);
    ar__data__set_map_integer(memory, "y", 5);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment with an expression: memory.sum := memory.x + memory.y
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.sum", "memory.x + memory.y");
    assert(ast != NULL);
    
    // Create and attach the expression AST for memory.x + memory.y
    const char *x_path[] = {"x"};
    expression_ast_t *left_ast = ar__expression_ast__create_memory_access("memory", x_path, 1);
    assert(left_ast != NULL);
    
    const char *y_path[] = {"y"};
    expression_ast_t *right_ast = ar__expression_ast__create_memory_access("memory", y_path, 1);
    assert(right_ast != NULL);
    
    expression_ast_t *expr_ast = ar__expression_ast__create_binary_op(OP_ADD, left_ast, right_ast);
    assert(expr_ast != NULL);
    bool ast_set = ar__instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the computed value should be stored in memory
    int sum = ar__data__get_map_integer(memory, "sum");
    assert(sum == 15);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_assignment_instruction_evaluator__evaluate_invalid_path(void) {
    // Given an evaluator with empty memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment with invalid path: invalid.path := 42
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("invalid.path", "42");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    expression_ast_t *expr_ast = ar__expression_ast__create_literal_int(42);
    assert(expr_ast != NULL);
    bool ast_set = ar__instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return false
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting assignment instruction_evaluator tests...\n");
    
    test_assignment_instruction_evaluator__create_destroy();
    printf("test_assignment_instruction_evaluator__create_destroy passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_with_instance();
    printf("test_assignment_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_integer();
    printf("test_assignment_instruction_evaluator__evaluate_integer passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_string();
    printf("test_assignment_instruction_evaluator__evaluate_string passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_nested_path();
    printf("test_assignment_instruction_evaluator__evaluate_nested_path passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_expression();
    printf("test_assignment_instruction_evaluator__evaluate_expression passed!\n");
    
    test_assignment_instruction_evaluator__evaluate_invalid_path();
    printf("test_assignment_instruction_evaluator__evaluate_invalid_path passed!\n");
    
    printf("All assignment instruction_evaluator tests passed!\n");
    
    return 0;
}