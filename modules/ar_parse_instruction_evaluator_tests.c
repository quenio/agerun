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

static void test_parse_instruction_evaluator__create_destroy(void) {
    // Given memory and expression evaluator
    data_t *own_memory = ar__data__create_map();
    assert(own_memory != NULL);
    ar_expression_evaluator_t *own_expr_eval = ar__expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating a parse instruction evaluator
    ar_parse_instruction_evaluator_t *own_evaluator = ar_parse_instruction_evaluator__create(
        own_expr_eval, own_memory
    );
    
    // Then it should create successfully
    assert(own_evaluator != NULL);
    
    // When destroying the evaluator
    ar_parse_instruction_evaluator__destroy(own_evaluator);
    
    // Cleanup
    ar__expression_evaluator__destroy(own_expr_eval);
    ar__data__destroy(own_memory);
}

static void test_parse_instruction_evaluator__evaluate_with_instance(void) {
    // Given memory and an evaluator instance
    data_t *own_memory = ar__data__create_map();
    assert(own_memory != NULL);
    
    ar_expression_evaluator_t *own_expr_eval = ar__expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    ar_parse_instruction_evaluator_t *own_evaluator = ar_parse_instruction_evaluator__create(
        own_expr_eval, own_memory
    );
    assert(own_evaluator != NULL);
    
    // When creating a parse AST node
    const char *args[] = {"\"name={name}\"", "\"name=John\""};
    ar_instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args, 2, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "name={name}"
    ar_expression_ast_t *template_ast = ar__expression_ast__create_literal_string("name={name}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Input: "name=John"
    ar_expression_ast_t *input_ast = ar__expression_ast__create_literal_string("name=John");
    ar__list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance
    bool result = ar_parse_instruction_evaluator__evaluate(own_evaluator, own_ast);
    
    // Then it should succeed and store the parsed value
    assert(result == true);
    data_t *ref_result_value = ar__data__get_map_data(own_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar__data__get_type(ref_result_value) == DATA_MAP);
    
    data_t *ref_name_value = ar__data__get_map_data(ref_result_value, "name");
    assert(ref_name_value != NULL);
    assert(ar__data__get_type(ref_name_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(ref_name_value), "John") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(own_ast);
    ar_parse_instruction_evaluator__destroy(own_evaluator);
    ar__expression_evaluator__destroy(own_expr_eval);
    ar__data__destroy(own_memory);
}

static void test_parse_instruction_evaluator__evaluate_without_legacy(void) {
    // Given memory and expression evaluator
    data_t *own_memory = ar__data__create_map();
    assert(own_memory != NULL);
    
    ar_expression_evaluator_t *own_expr_eval = ar__expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating a parse instruction evaluator instance
    ar_parse_instruction_evaluator_t *own_evaluator = ar_parse_instruction_evaluator__create(
        own_expr_eval, own_memory
    );
    assert(own_evaluator != NULL);
    
    // When creating a parse AST node
    const char *args[] = {"\"user={username}, role={role}\"", "\"user=alice, role=admin\""};
    ar_instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args, 2, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "user={username}, role={role}"
    ar_expression_ast_t *template_ast = ar__expression_ast__create_literal_string("user={username}, role={role}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Input: "user=alice, role=admin"
    ar_expression_ast_t *input_ast = ar__expression_ast__create_literal_string("user=alice, role=admin");
    ar__list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance-based interface
    bool result = ar_parse_instruction_evaluator__evaluate(own_evaluator, own_ast);
    
    // Then it should succeed and store the parsed values
    assert(result == true);
    data_t *ref_result_value = ar__data__get_map_data(own_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar__data__get_type(ref_result_value) == DATA_MAP);
    
    data_t *ref_username_value = ar__data__get_map_data(ref_result_value, "username");
    assert(ref_username_value != NULL);
    assert(ar__data__get_type(ref_username_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(ref_username_value), "alice") == 0);
    
    data_t *ref_role_value = ar__data__get_map_data(ref_result_value, "role");
    assert(ref_role_value != NULL);
    assert(ar__data__get_type(ref_role_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(ref_role_value), "admin") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(own_ast);
    ar_parse_instruction_evaluator__destroy(own_evaluator);
    ar__expression_evaluator__destroy(own_expr_eval);
    ar__data__destroy(own_memory);
}

static void test_instruction_evaluator__evaluate_parse_simple(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with simple template
    const char *args[] = {"\"name={name}\"", "\"name=John\""};
    ar_instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "name={name}"
    ar_expression_ast_t *template_ast = ar__expression_ast__create_literal_string("name={name}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Input: "name=John"
    ar_expression_ast_t *input_ast = ar__expression_ast__create_literal_string("name=John");
    ar__list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed and store a map with the parsed value
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_MAP);
    
    // Check the extracted value
    data_t *name_value = ar__data__get_map_data(result_value, "name");
    assert(name_value != NULL);
    assert(ar__data__get_type(name_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(name_value), "John") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_multiple_variables(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with multiple variables
    const char *args[] = {"\"user={username}, role={role}\"", "\"user=alice, role=admin\""};
    ar_instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "user={username}, role={role}"
    ar_expression_ast_t *template_ast = ar__expression_ast__create_literal_string("user={username}, role={role}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Input: "user=alice, role=admin"
    ar_expression_ast_t *input_ast = ar__expression_ast__create_literal_string("user=alice, role=admin");
    ar__list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed and store a map with the parsed values
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_MAP);
    
    // Check the extracted values
    data_t *username_value = ar__data__get_map_data(result_value, "username");
    assert(username_value != NULL);
    assert(ar__data__get_type(username_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(username_value), "alice") == 0);
    
    data_t *role_value = ar__data__get_map_data(result_value, "role");
    assert(role_value != NULL);
    assert(ar__data__get_type(role_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(role_value), "admin") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_with_types(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with values that should be parsed as different types
    const char *args[] = {"\"age={age}, score={score}, name={name}\"", "\"age=25, score=98.5, name=Bob\""};
    ar_instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "age={age}, score={score}, name={name}"
    ar_expression_ast_t *template_ast = ar__expression_ast__create_literal_string("age={age}, score={score}, name={name}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Input: "age=25, score=98.5, name=Bob"
    ar_expression_ast_t *input_ast = ar__expression_ast__create_literal_string("age=25, score=98.5, name=Bob");
    ar__list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed and store a map with the parsed values of correct types
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_MAP);
    
    // Check the extracted values have correct types
    data_t *age_value = ar__data__get_map_data(result_value, "age");
    assert(age_value != NULL);
    assert(ar__data__get_type(age_value) == DATA_INTEGER);
    assert(ar__data__get_integer(age_value) == 25);
    
    data_t *score_value = ar__data__get_map_data(result_value, "score");
    assert(score_value != NULL);
    assert(ar__data__get_type(score_value) == DATA_DOUBLE);
    assert(ar__data__get_double(score_value) == 98.5);
    
    data_t *name_value = ar__data__get_map_data(result_value, "name");
    assert(name_value != NULL);
    assert(ar__data__get_type(name_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(name_value), "Bob") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_no_match(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node where template doesn't match input
    const char *args[] = {"\"name={name}, age={age}\"", "\"username=John, level=5\""};
    ar_instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "name={name}, age={age}"
    ar_expression_ast_t *template_ast = ar__expression_ast__create_literal_string("name={name}, age={age}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Input: "username=John, level=5"
    ar_expression_ast_t *input_ast = ar__expression_ast__create_literal_string("username=John, level=5");
    ar__list__add_last(arg_asts, input_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the parse instruction
    bool result = ar_parse_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed but store an empty map
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_MAP);
    
    // Check the map is empty (no extracted values)
    // Since we don't have a way to check map size, we'll check that expected keys don't exist
    assert(ar__data__get_map_data(result_value, "name") == NULL);
    assert(ar__data__get_map_data(result_value, "age") == NULL);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_parse_instruction_evaluator_t *evaluator = ar_parse_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (1 instead of 2)
    const char *args1[] = {"\"template={value}\""};
    ar_instruction_ast_t *ast1 = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args1, 1, NULL
    );
    assert(ast1 != NULL);
    
    // Create and attach expression ASTs - only one argument (should fail)
    list_t *arg_asts1 = ar__list__create();
    assert(arg_asts1 != NULL);
    
    ar_expression_ast_t *template_ast1 = ar__expression_ast__create_literal_string("template={value}");
    ar__list__add_last(arg_asts1, template_ast1);
    
    bool ast_set1 = ar__instruction_ast__set_function_arg_asts(ast1, arg_asts1);
    assert(ast_set1 == true);
    
    bool result1 = ar_parse_instruction_evaluator__evaluate(evaluator, ast1);
    assert(result1 == false);
    
    ar__instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string template argument
    const char *args2[] = {"123", "\"input=value\""};
    ar_instruction_ast_t *ast2 = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args2, 2, NULL
    );
    assert(ast2 != NULL);
    
    // Create and attach expression ASTs - first is integer, second is string
    list_t *arg_asts2 = ar__list__create();
    assert(arg_asts2 != NULL);
    
    ar_expression_ast_t *template_ast2 = ar__expression_ast__create_literal_int(123);
    ar__list__add_last(arg_asts2, template_ast2);
    
    ar_expression_ast_t *input_ast2 = ar__expression_ast__create_literal_string("input=value");
    ar__list__add_last(arg_asts2, input_ast2);
    
    bool ast_set2 = ar__instruction_ast__set_function_arg_asts(ast2, arg_asts2);
    assert(ast_set2 == true);
    
    bool result2 = ar_parse_instruction_evaluator__evaluate(evaluator, ast2);
    assert(result2 == false);
    
    ar__instruction_ast__destroy(ast2);
    
    // Test case 3: Non-string input argument
    const char *args3[] = {"\"template={value}\"", "456"};
    ar_instruction_ast_t *ast3 = ar__instruction_ast__create_function_call(
        AR_INST__PARSE, "parse", args3, 2, NULL
    );
    assert(ast3 != NULL);
    
    // Create and attach expression ASTs - first is string, second is integer
    list_t *arg_asts3 = ar__list__create();
    assert(arg_asts3 != NULL);
    
    ar_expression_ast_t *template_ast3 = ar__expression_ast__create_literal_string("template={value}");
    ar__list__add_last(arg_asts3, template_ast3);
    
    ar_expression_ast_t *input_ast3 = ar__expression_ast__create_literal_int(456);
    ar__list__add_last(arg_asts3, input_ast3);
    
    bool ast_set3 = ar__instruction_ast__set_function_arg_asts(ast3, arg_asts3);
    assert(ast_set3 == true);
    
    bool result3 = ar_parse_instruction_evaluator__evaluate(evaluator, ast3);
    assert(result3 == false);
    
    ar__instruction_ast__destroy(ast3);
    
    // Cleanup
    ar_parse_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
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