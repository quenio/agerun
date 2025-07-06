#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "ar_assignment_instruction_evaluator.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_log.h"
#include "ar_event.h"

static void test_assignment_instruction_evaluator__create_destroy(void) {
    // Given expression evaluator, memory, and log
    ar_data_t *own_memory = ar_data__create_map();
    assert(own_memory != NULL);
    
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_log, own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating an assignment instruction evaluator
    ar_assignment_instruction_evaluator_t *own_evaluator = ar_assignment_instruction_evaluator__create(own_log, own_expr_eval, own_memory);
    
    // Then it should be created successfully
    assert(own_evaluator != NULL);
    
    // When destroying the evaluator
    ar_assignment_instruction_evaluator__destroy(own_evaluator);
    
    // Then cleanup other resources
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
}

static void test_assignment_instruction_evaluator__evaluate_with_instance(void) {
    // Given an assignment instruction evaluator
    ar_data_t *own_memory = ar_data__create_map();
    assert(own_memory != NULL);
    
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_log, own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    ar_assignment_instruction_evaluator_t *own_evaluator = ar_assignment_instruction_evaluator__create(own_log, own_expr_eval, own_memory);
    assert(own_evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.count := 42
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_assignment("memory.count", "42");
    assert(own_ast != NULL);
    
    // Create and attach the expression AST
    ar_expression_ast_t *own_expr_ast = ar_expression_ast__create_literal_int(42);
    assert(own_expr_ast != NULL);
    bool ast_set = ar_instruction_ast__set_assignment_expression_ast(own_ast, own_expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(own_evaluator, own_ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    int count = ar_data__get_map_integer(own_memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_assignment_instruction_evaluator__destroy(own_evaluator);
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
}

static void test_assignment_instruction_evaluator__evaluate_integer(void) {
    // Given an assignment evaluator with empty memory
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.count := 42
    ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment("memory.count", "42");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_literal_int(42);
    assert(expr_ast != NULL);
    bool ast_set = ar_instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    int count = ar_data__get_map_integer(memory, "count");
    assert(count == 42);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_assignment_instruction_evaluator__evaluate_string(void) {
    // Given an evaluator with empty memory
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment instruction: memory.name := "Alice"
    ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment("memory.name", "\"Alice\"");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_literal_string("Alice");
    assert(expr_ast != NULL);
    bool ast_set = ar_instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in memory
    const char *name = ar_data__get_map_string(memory, "name");
    assert(name != NULL);
    assert(strcmp(name, "Alice") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_assignment_instruction_evaluator__evaluate_nested_path(void) {
    // Given an evaluator with memory containing a nested map
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_data_t *user = ar_data__create_map();
    assert(user != NULL);
    ar_data__set_map_data(memory, "user", user);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment to a nested path: memory.user.age := 25
    ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment("memory.user.age", "25");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_literal_int(25);
    assert(expr_ast != NULL);
    bool ast_set = ar_instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the value should be stored in the nested map
    ar_data_t *stored_user = ar_data__get_map_data(memory, "user");
    assert(stored_user != NULL);
    int age = ar_data__get_map_integer(stored_user, "age");
    assert(age == 25);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_assignment_instruction_evaluator__evaluate_expression(void) {
    // Given an evaluator with memory containing initial values
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    ar_data__set_map_integer(memory, "x", 10);
    ar_data__set_map_integer(memory, "y", 5);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment with an expression: memory.sum := memory.x + memory.y
    ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment("memory.sum", "memory.x + memory.y");
    assert(ast != NULL);
    
    // Create and attach the expression AST for memory.x + memory.y
    const char *x_path[] = {"x"};
    ar_expression_ast_t *left_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    assert(left_ast != NULL);
    
    const char *y_path[] = {"y"};
    ar_expression_ast_t *right_ast = ar_expression_ast__create_memory_access("memory", y_path, 1);
    assert(right_ast != NULL);
    
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_binary_op(AR_OP__ADD, left_ast, right_ast);
    assert(expr_ast != NULL);
    bool ast_set = ar_instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the computed value should be stored in memory
    int sum = ar_data__get_map_integer(memory, "sum");
    assert(sum == 15);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_assignment_instruction_evaluator__evaluate_invalid_path(void) {
    // Given an evaluator with empty memory
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_assignment_instruction_evaluator_t *evaluator = ar_assignment_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an assignment with invalid path: invalid.path := 42
    ar_instruction_ast_t *ast = ar_instruction_ast__create_assignment("invalid.path", "42");
    assert(ast != NULL);
    
    // Create and attach the expression AST
    ar_expression_ast_t *expr_ast = ar_expression_ast__create_literal_int(42);
    assert(expr_ast != NULL);
    bool ast_set = ar_instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_assignment_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should return false
    assert(result == false);
    
    // And error should be logged
    ar_event_t *error_event = ar_log__get_last_error(log);
    assert(error_event != NULL);
    const char *error_msg = ar_event__get_message(error_event);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "Assignment target must start with 'memory.'") != NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_assignment_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
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

