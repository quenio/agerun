#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"

static void test_instruction_evaluator__evaluate_parse_simple(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with simple template
    const char *args[] = {"\"name={name}\"", "\"name=John\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the parse instruction
    bool result = ar__instruction_evaluator__evaluate_parse(evaluator, ast);
    
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
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_multiple_variables(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with multiple variables
    const char *args[] = {"\"user={username}, role={role}\"", "\"user=alice, role=admin\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the parse instruction
    bool result = ar__instruction_evaluator__evaluate_parse(evaluator, ast);
    
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
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_with_types(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node with values that should be parsed as different types
    const char *args[] = {"\"age={age}, score={score}, name={name}\"", "\"age=25, score=98.5, name=Bob\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the parse instruction
    bool result = ar__instruction_evaluator__evaluate_parse(evaluator, ast);
    
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
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_no_match(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a parse AST node where template doesn't match input
    const char *args[] = {"\"name={name}, age={age}\"", "\"username=John, level=5\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the parse instruction
    bool result = ar__instruction_evaluator__evaluate_parse(evaluator, ast);
    
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
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_parse_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (1 instead of 2)
    const char *args1[] = {"\"template={value}\""};
    instruction_ast_t *ast1 = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", args1, 1, NULL
    );
    assert(ast1 != NULL);
    
    bool result1 = ar__instruction_evaluator__evaluate_parse(evaluator, ast1);
    assert(result1 == false);
    
    ar__instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string template argument
    const char *args2[] = {"123", "\"input=value\""};
    instruction_ast_t *ast2 = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", args2, 2, NULL
    );
    assert(ast2 != NULL);
    
    bool result2 = ar__instruction_evaluator__evaluate_parse(evaluator, ast2);
    assert(result2 == false);
    
    ar__instruction_ast__destroy(ast2);
    
    // Test case 3: Non-string input argument
    const char *args3[] = {"\"template={value}\"", "456"};
    instruction_ast_t *ast3 = ar__instruction_ast__create_function_call(
        INST_AST_PARSE, "parse", args3, 2, NULL
    );
    assert(ast3 != NULL);
    
    bool result3 = ar__instruction_evaluator__evaluate_parse(evaluator, ast3);
    assert(result3 == false);
    
    ar__instruction_ast__destroy(ast3);
    
    // Cleanup
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting parse instruction_evaluator tests...\n");
    
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