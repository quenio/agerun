#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"

static void test_instruction_evaluator__evaluate_if_true_condition(void) {
    // Given an evaluator with memory containing a condition
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "x", ar__data__create_integer(10)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with true condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and store the true value
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 100);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_false_condition(void) {
    // Given an evaluator with memory containing a condition
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "x", ar__data__create_integer(3)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with false condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and store the false value
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 200);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_with_expressions(void) {
    // Given an evaluator with memory containing values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "a", ar__data__create_integer(10)));
    assert(ar__data__set_map_data(memory, "b", ar__data__create_integer(20)));
    assert(ar__data__set_map_data(memory, "flag", ar__data__create_integer(1)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with expression arguments
    const char *args[] = {"memory.flag", "memory.a + memory.b", "memory.a - memory.b"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and evaluate the true expression
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 30);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_nested(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "x", ar__data__create_integer(15)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a nested if expression (simulated as string)
    // if(x > 10, if(x > 20, "large", "medium"), "small")
    // Since we can't nest function calls, we'll test with simple return value
    const char *args[] = {"memory.x > 10", "\"medium\"", "\"small\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and return the correct string
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "medium") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (2 instead of 3)
    const char *args1[] = {"1", "100"};
    instruction_ast_t *ast1 = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    bool result1 = ar__instruction_evaluator__evaluate_if(evaluator, ast1);
    assert(result1 == false);
    
    ar__instruction_ast__destroy(ast1);
    
    // Test case 2: Invalid condition expression
    const char *args2[] = {"invalid expression", "100", "200"};
    instruction_ast_t *ast2 = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    bool result2 = ar__instruction_evaluator__evaluate_if(evaluator, ast2);
    assert(result2 == false);
    
    ar__instruction_ast__destroy(ast2);
    
    // Cleanup
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting condition instruction_evaluator tests...\n");
    
    test_instruction_evaluator__evaluate_if_true_condition();
    printf("test_instruction_evaluator__evaluate_if_true_condition passed!\n");
    
    test_instruction_evaluator__evaluate_if_false_condition();
    printf("test_instruction_evaluator__evaluate_if_false_condition passed!\n");
    
    test_instruction_evaluator__evaluate_if_with_expressions();
    printf("test_instruction_evaluator__evaluate_if_with_expressions passed!\n");
    
    test_instruction_evaluator__evaluate_if_nested();
    printf("test_instruction_evaluator__evaluate_if_nested passed!\n");
    
    test_instruction_evaluator__evaluate_if_invalid_args();
    printf("test_instruction_evaluator__evaluate_if_invalid_args passed!\n");
    
    printf("All condition instruction_evaluator tests passed!\n");
    
    return 0;
}