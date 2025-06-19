#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"

static void test_instruction_evaluator__evaluate_assignment_integer(void) {
    // Given an evaluator with empty memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.count := 42
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.count", "42");
    assert(ast != NULL);
    
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    int count = ar__data__get_map_integer(memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_string(void) {
    // Given an evaluator with empty memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.name := "Alice"
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.name", "\"Alice\"");
    assert(ast != NULL);
    
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    const char *name = ar__data__get_map_string(memory, "name");
    assert(name != NULL);
    assert(strcmp(name, "Alice") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_nested_path(void) {
    // Given an evaluator with memory containing a nested map
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *user = ar__data__create_map();
    assert(user != NULL);
    ar__data__set_map_data(memory, "user", user);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment to a nested path: memory.user.age := 25
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.user.age", "25");
    assert(ast != NULL);
    
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in the nested map
    data_t *stored_user = ar__data__get_map_data(memory, "user");
    assert(stored_user != NULL);
    int age = ar__data__get_map_integer(stored_user, "age");
    assert(age == 25);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_expression(void) {
    // Given an evaluator with memory containing initial values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    ar__data__set_map_integer(memory, "x", 10);
    ar__data__set_map_integer(memory, "y", 5);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment with an expression: memory.sum := memory.x + memory.y
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.sum", "memory.x + memory.y");
    assert(ast != NULL);
    
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the computed value should be stored in memory
    int sum = ar__data__get_map_integer(memory, "sum");
    assert(sum == 15);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_invalid_path(void) {
    // Given an evaluator with empty memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment with invalid path: invalid.path := 42
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("invalid.path", "42");
    assert(ast != NULL);
    
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should return false
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting assignment instruction_evaluator tests...\n");
    
    test_instruction_evaluator__evaluate_assignment_integer();
    printf("test_instruction_evaluator__evaluate_assignment_integer passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_string();
    printf("test_instruction_evaluator__evaluate_assignment_string passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_nested_path();
    printf("test_instruction_evaluator__evaluate_assignment_nested_path passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_expression();
    printf("test_instruction_evaluator__evaluate_assignment_expression passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_invalid_path();
    printf("test_instruction_evaluator__evaluate_assignment_invalid_path passed!\n");
    
    printf("All assignment instruction_evaluator tests passed!\n");
    
    return 0;
}