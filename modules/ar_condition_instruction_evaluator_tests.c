#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_condition_instruction_evaluator.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"

static void test_condition_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_create_destroy");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    // When creating a condition instruction evaluator
    ar_condition_instruction_evaluator_t *own_evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    
    // Then it should create successfully
    assert(own_evaluator != NULL);
    
    // When destroying the evaluator
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    
    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_condition_instruction_evaluator__evaluate_with_instance(void) {
    // Given a test fixture with memory containing a condition value
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_with_instance");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(10)));
    
    ar_condition_instruction_evaluator_t *own_evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(own_evaluator != NULL);
    
    // When creating an if AST node with true condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 5
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *five_ast = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, five_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: 100
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: 200
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_int(200);
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating using the instance with frame
    bool result = ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    
    // Then it should succeed and store the true value
    assert(result == true);
    ar_data_t *ref_result_value = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar_data__get_type(ref_result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_result_value) == 100);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_condition_instruction_evaluator__evaluate_without_legacy(void) {
    // Given a test fixture with memory containing a flag value
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_without_legacy");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "flag", ar_data__create_integer(0)));
    
    // When creating a condition instruction evaluator instance
    ar_condition_instruction_evaluator_t *own_evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(own_evaluator != NULL);
    
    // When creating an if AST node with false condition
    const char *args[] = {"memory.flag", "\"yes\"", "\"no\""};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.flag
    const char *flag_path[] = {"flag"};
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_memory_access("memory", flag_path, 1);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: "yes"
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_string("yes");
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: "no"
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_string("no");
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating using the instance-based interface with frame
    bool result = ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    
    // Then it should succeed and store the false value
    assert(result == true);
    ar_data_t *ref_result_value = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar_data__get_type(ref_result_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_result_value), "no") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_true_condition(void) {
    // Given a test fixture with memory containing a condition
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_true_condition");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(10)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with true condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 5
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *five_ast = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, five_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: 100
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: 200
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_int(200);
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and store the true value
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 100);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_false_condition(void) {
    // Given a test fixture with memory containing a condition
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_false_condition");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(3)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with false condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 5 (will be false since x=3)
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *five_ast = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, five_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: 100
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: 200
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_int(200);
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and store the false value
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 200);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_with_expressions(void) {
    // Given a test fixture with memory containing values
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_with_expressions");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "a", ar_data__create_integer(10)));
    assert(ar_data__set_map_data(mut_memory, "b", ar_data__create_integer(20)));
    assert(ar_data__set_map_data(mut_memory, "flag", ar_data__create_integer(1)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with expression arguments
    const char *args[] = {"memory.flag", "memory.a + memory.b", "memory.a - memory.b"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.flag
    const char *flag_path[] = {"flag"};
    ar_expression_ast_t *flag_ast = ar_expression_ast__create_memory_access("memory", flag_path, 1);
    ar_list__add_last(arg_asts, flag_ast);
    
    // True value: memory.a + memory.b
    const char *a_path[] = {"a"};
    const char *b_path[] = {"b"};
    ar_expression_ast_t *a_ast = ar_expression_ast__create_memory_access("memory", a_path, 1);
    ar_expression_ast_t *b_ast = ar_expression_ast__create_memory_access("memory", b_path, 1);
    ar_expression_ast_t *add_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__ADD, a_ast, b_ast);
    ar_list__add_last(arg_asts, add_ast);
    
    // False value: memory.a - memory.b
    const char *a2_path[] = {"a"};
    const char *b2_path[] = {"b"};
    ar_expression_ast_t *a2_ast = ar_expression_ast__create_memory_access("memory", a2_path, 1);
    ar_expression_ast_t *b2_ast = ar_expression_ast__create_memory_access("memory", b2_path, 1);
    ar_expression_ast_t *sub_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__SUBTRACT, a2_ast, b2_ast);
    ar_list__add_last(arg_asts, sub_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and evaluate the true expression
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 30);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_nested(void) {
    // Given a test fixture with memory
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_nested");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(15)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a nested if expression (simulated as string)
    // if(x > 10, if(x > 20, "large", "medium"), "small")
    // Since we can't nest function calls, we'll test with simple return value
    const char *args[] = {"memory.x > 10", "\"medium\"", "\"small\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 10
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *ten_ast = ar_expression_ast__create_literal_int(10);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, ten_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: "medium"
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_string("medium");
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: "small"
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_string("small");
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and return the correct string
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(result_value), "medium") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_invalid_args(void) {
    // Given a test fixture with memory
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_invalid_args");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (2 instead of 3)
    const char *args1[] = {"1", "100"};
    ar_instruction_ast_t *ast1 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts1 = ar_list__create();
    assert(arg_asts1 != NULL);
    
    // Only 2 arguments instead of required 3
    ar_expression_ast_t *one_ast = ar_expression_ast__create_literal_int(1);
    ar_list__add_last(arg_asts1, one_ast);
    
    ar_expression_ast_t *hundred_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts1, hundred_ast);
    
    bool ast_set1 = ar_instruction_ast__set_function_arg_asts(ast1, arg_asts1);
    assert(ast_set1 == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    bool result1 = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast1);
    assert(result1 == false);
    
    ar_instruction_ast__destroy(ast1);
    
    // Test case 2: Invalid condition expression (NULL AST)
    const char *args2[] = {"invalid expression", "100", "200"};
    ar_instruction_ast_t *ast2 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    // Don't attach any ASTs to simulate parsing failure
    
    bool result2 = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast2);
    assert(result2 == false);
    
    ar_instruction_ast__destroy(ast2);
    
    // Cleanup
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    printf("Starting condition instruction_evaluator tests...\n");
    
    test_condition_instruction_evaluator__create_destroy();
    printf("test_condition_instruction_evaluator__create_destroy passed!\n");
    
    test_condition_instruction_evaluator__evaluate_with_instance();
    printf("test_condition_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_condition_instruction_evaluator__evaluate_without_legacy();
    printf("test_condition_instruction_evaluator__evaluate_without_legacy passed!\n");
    
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

