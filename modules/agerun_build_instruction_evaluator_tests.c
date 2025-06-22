#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "agerun_build_instruction_evaluator.h"
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_expression_ast.h"
#include "agerun_data.h"
#include "agerun_list.h"

static void test_build_instruction_evaluator__create_destroy(void) {
    // Given memory and expression evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating a build instruction evaluator
    ar_build_instruction_evaluator_t *evaluator = ar_build_instruction_evaluator__create(
        expr_eval, memory
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_build_instruction_evaluator__destroy(evaluator);
    
    // Then cleanup dependencies
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_build_instruction_evaluator__evaluate_with_instance(void) {
    // Given memory with a map of values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *values = ar__data__create_map();
    assert(values != NULL);
    assert(ar__data__set_map_data(values, "name", ar__data__create_string("Alice")));
    assert(ar__data__set_map_data(memory, "data", values));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating a build instruction evaluator instance
    ar_build_instruction_evaluator_t *evaluator = ar_build_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a build AST node
    const char *args[] = {"\"Hello {name}!\"", "memory.data"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "Hello {name}!"
    expression_ast_t *template_ast = ar__expression_ast__create_literal_string("Hello {name}!");
    ar__list__add_last(arg_asts, template_ast);
    
    // Values: memory.data
    const char *data_path[] = {"data"};
    expression_ast_t *values_ast = ar__expression_ast__create_memory_access("memory", data_path, 1);
    ar__list__add_last(arg_asts, values_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance
    bool result = ar_build_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed and build the string
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "Hello Alice!") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_build_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_build_instruction_evaluator__evaluate_legacy(void) {
    // Given memory with a map of values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *values = ar__data__create_map();
    assert(values != NULL);
    assert(ar__data__set_map_data(values, "greeting", ar__data__create_string("Hi")));
    assert(ar__data__set_map_data(memory, "vars", values));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // Create an evaluator instance
    ar_build_instruction_evaluator_t *evaluator = ar_build_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a build AST node
    const char *args[] = {"\"{greeting} there!\"", "memory.vars"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args, 2, "memory.message"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "{greeting} there!"
    expression_ast_t *template_ast = ar__expression_ast__create_literal_string("{greeting} there!");
    ar__list__add_last(arg_asts, template_ast);
    
    // Values: memory.vars
    const char *vars_path[] = {"vars"};
    expression_ast_t *values_ast = ar__expression_ast__create_memory_access("memory", vars_path, 1);
    ar__list__add_last(arg_asts, values_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance-based interface
    bool result = ar_build_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed and build the string
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "message");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "Hi there!") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_build_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_build_simple(void) {
    // Given an evaluator with memory containing a map
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    // Create a map with values to use in building
    data_t *values = ar__data__create_map();
    assert(values != NULL);
    assert(ar__data__set_map_data(values, "name", ar__data__create_string("Alice")));
    assert(ar__data__set_map_data(memory, "data", values));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a build AST node with simple template
    const char *args[] = {"\"Hello {name}!\"", "memory.data"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "Hello {name}!"
    expression_ast_t *template_ast = ar__expression_ast__create_literal_string("Hello {name}!");
    ar__list__add_last(arg_asts, template_ast);
    
    // Values: memory.data
    const char *data_path[] = {"data"};
    expression_ast_t *values_ast = ar__expression_ast__create_memory_access("memory", data_path, 1);
    ar__list__add_last(arg_asts, values_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the build instruction
    bool result = ar_instruction_evaluator__evaluate_build(evaluator, ast);
    
    // Then it should succeed and build the string
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "Hello Alice!") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    
    ar__data__destroy(memory);
}
static void test_instruction_evaluator__evaluate_build_multiple_variables(void) {
    // Given an evaluator with memory containing a map
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    // Create a map with multiple values
    data_t *values = ar__data__create_map();
    assert(values != NULL);
    assert(ar__data__set_map_data(values, "firstName", ar__data__create_string("Bob")));
    assert(ar__data__set_map_data(values, "lastName", ar__data__create_string("Smith")));
    assert(ar__data__set_map_data(values, "role", ar__data__create_string("Admin")));
    assert(ar__data__set_map_data(memory, "user", values));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a build AST node with multiple variables
    const char *args[] = {"\"User: {firstName} {lastName}, Role: {role}\"", "memory.user"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "User: {firstName} {lastName}, Role: {role}"
    expression_ast_t *template_ast = ar__expression_ast__create_literal_string("User: {firstName} {lastName}, Role: {role}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Values: memory.user
    const char *user_path[] = {"user"};
    expression_ast_t *values_ast = ar__expression_ast__create_memory_access("memory", user_path, 1);
    ar__list__add_last(arg_asts, values_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the build instruction
    bool result = ar_instruction_evaluator__evaluate_build(evaluator, ast);
    
    // Then it should succeed and build the string with all values
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "User: Bob Smith, Role: Admin") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_build_with_types(void) {
    // Given an evaluator with memory containing a map with different types
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    // Create a map with values of different types
    data_t *values = ar__data__create_map();
    assert(values != NULL);
    assert(ar__data__set_map_data(values, "name", ar__data__create_string("Charlie")));
    assert(ar__data__set_map_data(values, "age", ar__data__create_integer(30)));
    assert(ar__data__set_map_data(values, "score", ar__data__create_double(95.5)));
    assert(ar__data__set_map_data(memory, "stats", values));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a build AST node with different value types
    const char *args[] = {"\"Name: {name}, Age: {age}, Score: {score}\"", "memory.stats"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "Name: {name}, Age: {age}, Score: {score}"
    expression_ast_t *template_ast = ar__expression_ast__create_literal_string("Name: {name}, Age: {age}, Score: {score}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Values: memory.stats
    const char *stats_path[] = {"stats"};
    expression_ast_t *values_ast = ar__expression_ast__create_memory_access("memory", stats_path, 1);
    ar__list__add_last(arg_asts, values_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the build instruction
    bool result = ar_instruction_evaluator__evaluate_build(evaluator, ast);
    
    // Then it should succeed and convert all types to strings
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "Name: Charlie, Age: 30, Score: 95.5") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_build_missing_values(void) {
    // Given an evaluator with memory containing a map with some missing values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    // Create a map with only some values
    data_t *values = ar__data__create_map();
    assert(values != NULL);
    assert(ar__data__set_map_data(values, "firstName", ar__data__create_string("David")));
    // Note: lastName is missing
    assert(ar__data__set_map_data(memory, "person", values));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a build AST node with a missing variable
    const char *args[] = {"\"Name: {firstName} {lastName}\"", "memory.person"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Template: "Name: {firstName} {lastName}"
    expression_ast_t *template_ast = ar__expression_ast__create_literal_string("Name: {firstName} {lastName}");
    ar__list__add_last(arg_asts, template_ast);
    
    // Values: memory.person
    const char *person_path[] = {"person"};
    expression_ast_t *values_ast = ar__expression_ast__create_memory_access("memory", person_path, 1);
    ar__list__add_last(arg_asts, values_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the build instruction
    bool result = ar_instruction_evaluator__evaluate_build(evaluator, ast);
    
    // Then it should succeed but preserve the placeholder for missing value
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "Name: David {lastName}") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_build_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (1 instead of 2)
    const char *args1[] = {"\"template {value}\""};
    instruction_ast_t *ast1 = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args1, 1, NULL
    );
    assert(ast1 != NULL);
    
    // Create and attach expression ASTs - only one argument (should fail)
    list_t *arg_asts1 = ar__list__create();
    assert(arg_asts1 != NULL);
    
    expression_ast_t *template_ast1 = ar__expression_ast__create_literal_string("template {value}");
    ar__list__add_last(arg_asts1, template_ast1);
    
    bool ast_set1 = ar__instruction_ast__set_function_arg_asts(ast1, arg_asts1);
    assert(ast_set1 == true);
    
    bool result1 = ar_instruction_evaluator__evaluate_build(evaluator, ast1);
    assert(result1 == false);
    
    ar__instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string template argument
    data_t *dummy_map = ar__data__create_map();
    assert(ar__data__set_map_data(memory, "dummy", dummy_map));
    
    const char *args2[] = {"123", "memory.dummy"};
    instruction_ast_t *ast2 = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args2, 2, NULL
    );
    assert(ast2 != NULL);
    
    // Create and attach expression ASTs - first is integer, second is map
    list_t *arg_asts2 = ar__list__create();
    assert(arg_asts2 != NULL);
    
    expression_ast_t *template_ast2 = ar__expression_ast__create_literal_int(123);
    ar__list__add_last(arg_asts2, template_ast2);
    
    const char *dummy_path[] = {"dummy"};
    expression_ast_t *values_ast2 = ar__expression_ast__create_memory_access("memory", dummy_path, 1);
    ar__list__add_last(arg_asts2, values_ast2);
    
    bool ast_set2 = ar__instruction_ast__set_function_arg_asts(ast2, arg_asts2);
    assert(ast_set2 == true);
    
    bool result2 = ar_instruction_evaluator__evaluate_build(evaluator, ast2);
    assert(result2 == false);
    
    ar__instruction_ast__destroy(ast2);
    
    // Test case 3: Non-map values argument
    const char *args3[] = {"\"template {value}\"", "\"not a map\""};
    instruction_ast_t *ast3 = ar__instruction_ast__create_function_call(
        INST_AST_BUILD, "build", args3, 2, NULL
    );
    assert(ast3 != NULL);
    
    // Create and attach expression ASTs - first is string, second is string
    list_t *arg_asts3 = ar__list__create();
    assert(arg_asts3 != NULL);
    
    expression_ast_t *template_ast3 = ar__expression_ast__create_literal_string("template {value}");
    ar__list__add_last(arg_asts3, template_ast3);
    
    expression_ast_t *values_ast3 = ar__expression_ast__create_literal_string("not a map");
    ar__list__add_last(arg_asts3, values_ast3);
    
    bool ast_set3 = ar__instruction_ast__set_function_arg_asts(ast3, arg_asts3);
    assert(ast_set3 == true);
    
    bool result3 = ar_instruction_evaluator__evaluate_build(evaluator, ast3);
    assert(result3 == false);
    
    ar__instruction_ast__destroy(ast3);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting build instruction_evaluator tests...\n");
    
    test_build_instruction_evaluator__create_destroy();
    printf("test_build_instruction_evaluator__create_destroy passed!\n");
    
    test_build_instruction_evaluator__evaluate_with_instance();
    printf("test_build_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_build_instruction_evaluator__evaluate_legacy();
    printf("test_build_instruction_evaluator__evaluate_legacy passed!\n");
    
    test_instruction_evaluator__evaluate_build_simple();
    printf("test_instruction_evaluator__evaluate_build_simple passed!\n");
    
    test_instruction_evaluator__evaluate_build_multiple_variables();
    printf("test_instruction_evaluator__evaluate_build_multiple_variables passed!\n");
    
    test_instruction_evaluator__evaluate_build_with_types();
    printf("test_instruction_evaluator__evaluate_build_with_types passed!\n");
    
    test_instruction_evaluator__evaluate_build_missing_values();
    printf("test_instruction_evaluator__evaluate_build_missing_values passed!\n");
    
    test_instruction_evaluator__evaluate_build_invalid_args();
    printf("test_instruction_evaluator__evaluate_build_invalid_args passed!\n");
    
    printf("All build instruction_evaluator tests passed!\n");
    
    return 0;
}