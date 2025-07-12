#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_parse_instruction_evaluator.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_instruction_evaluator_fixture.h"
#include "ar_frame.h"

static void test_parse_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_parse_instruction_evaluator__create_destroy"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    // When creating a parse instruction evaluator
    ar_parse_instruction_evaluator_t *own_evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    
    // Then it should create successfully
    assert(own_evaluator != NULL);
    
    // When destroying the evaluator
    ar_parse_instruction_evaluator__destroy(own_evaluator);
    
    // Cleanup
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

static void test_parse_instruction_evaluator__evaluate_with_instance(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_parse_instruction_evaluator__evaluate_with_instance"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_instruction_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);
    
    ar_parse_instruction_evaluator_t *own_evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(own_evaluator != NULL);
    
    // When creating a parse AST node
    const char *args[] = {"\"name={name}\"", "\"name=John\""};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args, 2, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Template: "name={name}"
    ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("name={name}");
    ar_list__add_last(arg_asts, template_ast);
    
    // Input: "name=John"
    ar_expression_ast_t *input_ast = ar_expression_ast__create_literal_string("name=John");
    ar_list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance
    bool result = ar_parse_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    
    // Then it should succeed and store the parsed value
    assert(result == true);
    ar_data_t *ref_result_value = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar_data__get_type(ref_result_value) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_name_value = ar_data__get_map_data(ref_result_value, "name");
    assert(ref_name_value != NULL);
    assert(ar_data__get_type(ref_name_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_name_value), "John") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_parse_instruction_evaluator__destroy(own_evaluator);
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

static void test_parse_instruction_evaluator__evaluate_without_legacy(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_parse_instruction_evaluator__evaluate_without_legacy"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_instruction_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);
    
    // When creating a parse instruction evaluator instance
    ar_parse_instruction_evaluator_t *own_evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(own_evaluator != NULL);
    
    // When creating a parse AST node
    const char *args[] = {"\"user={username}, role={role}\"", "\"user=alice, role=admin\""};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args, 2, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Template: "user={username}, role={role}"
    ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("user={username}, role={role}");
    ar_list__add_last(arg_asts, template_ast);
    
    // Input: "user=alice, role=admin"
    ar_expression_ast_t *input_ast = ar_expression_ast__create_literal_string("user=alice, role=admin");
    ar_list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance-based interface
    bool result = ar_parse_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    
    // Then it should succeed and store the parsed values
    assert(result == true);
    ar_data_t *ref_result_value = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar_data__get_type(ref_result_value) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_username_value = ar_data__get_map_data(ref_result_value, "username");
    assert(ref_username_value != NULL);
    assert(ar_data__get_type(ref_username_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_username_value), "alice") == 0);
    
    ar_data_t *ref_role_value = ar_data__get_map_data(ref_result_value, "role");
    assert(ref_role_value != NULL);
    assert(ar_data__get_type(ref_role_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_role_value), "admin") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_parse_instruction_evaluator__destroy(own_evaluator);
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_parse_simple(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_instruction_evaluator__evaluate_parse_simple"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_instruction_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with simple template
    const char *args[] = {"\"name={name}\"", "\"name=John\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Template: "name={name}"
    ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("name={name}");
    ar_list__add_last(arg_asts, template_ast);
    
    // Input: "name=John"
    ar_expression_ast_t *input_ast = ar_expression_ast__create_literal_string("name=John");
    ar_list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and store a map with the parsed value
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__MAP);
    
    // Check the extracted value
    ar_data_t *name_value = ar_data__get_map_data(result_value, "name");
    assert(name_value != NULL);
    assert(ar_data__get_type(name_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(name_value), "John") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_parse_multiple_variables(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_instruction_evaluator__evaluate_parse_multiple_variables"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_instruction_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with multiple variables
    const char *args[] = {"\"user={username}, role={role}\"", "\"user=alice, role=admin\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Template: "user={username}, role={role}"
    ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("user={username}, role={role}");
    ar_list__add_last(arg_asts, template_ast);
    
    // Input: "user=alice, role=admin"
    ar_expression_ast_t *input_ast = ar_expression_ast__create_literal_string("user=alice, role=admin");
    ar_list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and store a map with the parsed values
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__MAP);
    
    // Check the extracted values
    ar_data_t *username_value = ar_data__get_map_data(result_value, "username");
    assert(username_value != NULL);
    assert(ar_data__get_type(username_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(username_value), "alice") == 0);
    
    ar_data_t *role_value = ar_data__get_map_data(result_value, "role");
    assert(role_value != NULL);
    assert(ar_data__get_type(role_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(role_value), "admin") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_parse_with_types(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_instruction_evaluator__evaluate_parse_with_types"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_instruction_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with values that should be parsed as different types
    const char *args[] = {"\"age={age}, score={score}, name={name}\"", "\"age=25, score=98.5, name=Bob\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Template: "age={age}, score={score}, name={name}"
    ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("age={age}, score={score}, name={name}");
    ar_list__add_last(arg_asts, template_ast);
    
    // Input: "age=25, score=98.5, name=Bob"
    ar_expression_ast_t *input_ast = ar_expression_ast__create_literal_string("age=25, score=98.5, name=Bob");
    ar_list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and store a map with the parsed values of correct types
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__MAP);
    
    // Check the extracted values have correct types
    ar_data_t *age_value = ar_data__get_map_data(result_value, "age");
    assert(age_value != NULL);
    assert(ar_data__get_type(age_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(age_value) == 25);
    
    ar_data_t *score_value = ar_data__get_map_data(result_value, "score");
    assert(score_value != NULL);
    assert(ar_data__get_type(score_value) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(score_value) == 98.5);
    
    ar_data_t *name_value = ar_data__get_map_data(result_value, "name");
    assert(name_value != NULL);
    assert(ar_data__get_type(name_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(name_value), "Bob") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_parse_no_match(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_instruction_evaluator__evaluate_parse_no_match"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_instruction_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node where template doesn't match input
    const char *args[] = {"\"name={name}, age={age}\"", "\"username=John, level=5\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Template: "name={name}, age={age}"
    ar_expression_ast_t *template_ast = ar_expression_ast__create_literal_string("name={name}, age={age}");
    ar_list__add_last(arg_asts, template_ast);
    
    // Input: "username=John, level=5"
    ar_expression_ast_t *input_ast = ar_expression_ast__create_literal_string("username=John, level=5");
    ar_list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed but store an empty map
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__MAP);
    
    // Check the map is empty (no extracted values)
    // Since we don't have a way to check map size, we'll check that expected keys don't exist
    assert(ar_data__get_map_data(result_value, "name") == NULL);
    assert(ar_data__get_map_data(result_value, "age") == NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_parse_invalid_args(void) {
    // Given a test fixture
    ar_instruction_evaluator_fixture_t *own_fixture = ar_instruction_evaluator_fixture__create(
        "test_instruction_evaluator__evaluate_parse_invalid_args"
    );
    assert(own_fixture != NULL);
    
    // Get references from fixture
    ar_log_t *ref_log = ar_instruction_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_instruction_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_instruction_evaluator_fixture__create_frame(own_fixture);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (1 instead of 2)
    const char *args1[] = {"\"template={value}\""};
    ar_instruction_ast_t *ast1 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args1, 1, NULL
    );
    assert(ast1 != NULL);
    
    // Create and attach expression ASTs - only one argument (should fail)
    ar_list_t *arg_asts1 = ar_list__create();
    assert(arg_asts1 != NULL);
    
    ar_expression_ast_t *template_ast1 = ar_expression_ast__create_literal_string("template={value}");
    ar_list__add_last(arg_asts1, template_ast1);
    
    bool ast_set1 = ar_instruction_ast__set_function_arg_asts(ast1, arg_asts1);
    assert(ast_set1 == true);
    
    bool result1 = ar_parse_instruction_evaluator__evaluate(evaluator, ref_frame, ast1);
    assert(result1 == false);
    
    ar_instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string template argument
    const char *args2[] = {"123", "\"input=value\""};
    ar_instruction_ast_t *ast2 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args2, 2, NULL
    );
    assert(ast2 != NULL);
    
    // Create and attach expression ASTs - first is integer, second is string
    ar_list_t *arg_asts2 = ar_list__create();
    assert(arg_asts2 != NULL);
    
    ar_expression_ast_t *template_ast2 = ar_expression_ast__create_literal_int(123);
    ar_list__add_last(arg_asts2, template_ast2);
    
    ar_expression_ast_t *input_ast2 = ar_expression_ast__create_literal_string("input=value");
    ar_list__add_last(arg_asts2, input_ast2);
    
    bool ast_set2 = ar_instruction_ast__set_function_arg_asts(ast2, arg_asts2);
    assert(ast_set2 == true);
    
    bool result2 = ar_parse_instruction_evaluator__evaluate(evaluator, ref_frame, ast2);
    assert(result2 == false);
    
    ar_instruction_ast__destroy(ast2);
    
    // Test case 3: Non-string input argument
    const char *args3[] = {"\"template={value}\"", "456"};
    ar_instruction_ast_t *ast3 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__PARSE, "parse", args3, 2, NULL
    );
    assert(ast3 != NULL);
    
    // Create and attach expression ASTs - first is string, second is integer
    ar_list_t *arg_asts3 = ar_list__create();
    assert(arg_asts3 != NULL);
    
    ar_expression_ast_t *template_ast3 = ar_expression_ast__create_literal_string("template={value}");
    ar_list__add_last(arg_asts3, template_ast3);
    
    ar_expression_ast_t *input_ast3 = ar_expression_ast__create_literal_int(456);
    ar_list__add_last(arg_asts3, input_ast3);
    
    bool ast_set3 = ar_instruction_ast__set_function_arg_asts(ast3, arg_asts3);
    assert(ast_set3 == true);
    
    bool result3 = ar_parse_instruction_evaluator__evaluate(evaluator, ref_frame, ast3);
    assert(result3 == false);
    
    ar_instruction_ast__destroy(ast3);
    
    // Cleanup
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar_instruction_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    printf("Starting parse instruction_evaluator tests...\n");
    
    test_parse_instruction_evaluator__create_destroy();
    printf("test_parse_instruction_evaluator__create_destroy passed!\n");
    
    test_parse_instruction_evaluator__evaluate_with_instance();
    printf("test_parse_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_parse_instruction_evaluator__evaluate_without_legacy();
    printf("test_parse_instruction_evaluator__evaluate_without_legacy passed!\n");
    
    test_instruction_evaluator__evaluate_parse_simple();
    printf("test_instruction_evaluator__evaluate_parse_simple passed!\n");
    
    test_instruction_evaluator__evaluate_parse_multiple_variables();
    printf("test_instruction_evaluator__evaluate_parse_multiple_variables passed!\n");
    
    test_instruction_evaluator__evaluate_parse_with_types();
    printf("test_instruction_evaluator__evaluate_parse_with_types passed!\n");
    
    test_instruction_evaluator__evaluate_parse_no_match();
    printf("test_instruction_evaluator__evaluate_parse_no_match passed!\n");
    
    test_instruction_evaluator__evaluate_parse_invalid_args();
    printf("test_instruction_evaluator__evaluate_parse_invalid_args passed!\n");
    
    printf("All parse instruction_evaluator tests passed!\n");
    
    return 0;
}
