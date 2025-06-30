#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "ar_destroy_method_instruction_evaluator.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_event.h"

// Test create/destroy lifecycle
static void test_destroy_method_instruction_evaluator__create_destroy(void) {
    // Given dependencies
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    assert(expr_eval != NULL);
    
    // When creating a destroy method evaluator
    ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
        log, expr_eval, memory
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_destroy_method_instruction_evaluator__destroy(evaluator);
    
    // Then no memory leaks should occur (verified by test framework)
    
    // Cleanup
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

// Test evaluate with instance
static void test_destroy_method_instruction_evaluator__evaluate_with_instance(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    ar_system__init(NULL, NULL);
    
    // Given an evaluator instance with a registered method
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create a test method
    ar_methodology__create_method("test_destroyer", "memory.x := 1", "1.0.0");
    
    // Verify method exists
    method_t *method = ar_methodology__get_method("test_destroyer", "1.0.0");
    assert(method != NULL);
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__DESTROY_METHOD, "destroy", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "test_destroyer"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_destroyer");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call using instance
    bool result = ar_destroy_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the method should be destroyed (not exist anymore)
    method = ar_methodology__get_method("test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_destroy_method_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    ar_system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

// Test legacy function
static void test_destroy_method_instruction_evaluator__evaluate_legacy(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    ar_system__init(NULL, NULL);
    
    // Given dependencies
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    assert(expr_eval != NULL);
    
    // Create an evaluator instance
    ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create a test method
    ar_methodology__create_method("test_destroyer", "memory.x := 1", "1.0.0");
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__DESTROY_METHOD, "destroy", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "test_destroyer"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_destroyer");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using instance-based interface
    bool result = ar_destroy_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the method should be destroyed
    method_t *method = ar_methodology__get_method("test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_destroy_method_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    ar_system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

// Test destroy method with agents using it
static void test_destroy_method_instruction_evaluator__evaluate_with_agents(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    ar_system__init(NULL, NULL);
    
    // Given an evaluator instance with a method and agents using it
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create a test method and agents using it
    ar_methodology__create_method("test_destroyer", "memory.x := 1", "1.0.0");
    int64_t agent1 = ar_agency__create_agent("test_destroyer", "1.0.0", NULL);
    int64_t agent2 = ar_agency__create_agent("test_destroyer", "1.0.0", NULL);
    assert(agent1 > 0);
    assert(agent2 > 0);
    
    // Process wake messages to avoid leaks
    ar_system__process_next_message();
    ar_system__process_next_message();
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__DESTROY_METHOD, "destroy", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "test_destroyer"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_destroyer");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be true (1)
    data_t *result_value = ar_data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == DATA_INTEGER);
    assert(ar_data__get_integer(result_value) == 1);
    
    // The agents should already be destroyed
    // (In the current implementation, destroy() handles agent destruction immediately)
    assert(ar_agency__agent_exists(agent1) == false);
    assert(ar_agency__agent_exists(agent2) == false);
    
    // And the method should be destroyed
    method_t *method = ar_methodology__get_method("test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_destroy_method_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    ar_system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

// Test destroy nonexistent method
static void test_destroy_method_instruction_evaluator__evaluate_nonexistent(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    ar_system__init(NULL, NULL);
    
    // Given an evaluator instance with no methods
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-existent method
    const char *args[] = {"\"nonexistent\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__DESTROY_METHOD, "destroy", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "nonexistent"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("nonexistent");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed (no error)
    assert(result == true);
    
    // But the result should be false (0) since method doesn't exist
    data_t *result_value = ar_data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == DATA_INTEGER);
    assert(ar_data__get_integer(result_value) == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_destroy_method_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    ar_system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

// Test destroy with invalid method name type
static void test_destroy_method_instruction_evaluator__evaluate_invalid_name_type(void) {
    // Given an evaluator instance
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-string method name (integer)
    const char *args[] = {"123", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__DESTROY_METHOD, "destroy", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: 123 (integer, not string)
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_int(123);
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should fail due to invalid argument type
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_destroy_method_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

// Test destroy with wrong number of arguments
static void test_destroy_method_instruction_evaluator__evaluate_wrong_arg_count(void) {
    // Given an evaluator instance
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_method_instruction_evaluator_t *evaluator = ar_destroy_method_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with 1 arg (should be 2 for method)
    const char *args[] = {"\"method_name\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__DESTROY_METHOD, "destroy", args, 1, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression AST for the single argument
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name only (missing version - should cause error)
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("method_name");
    ar_list__add_last(arg_asts, name_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should fail due to wrong argument count
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_destroy_method_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

int main(void) {
    printf("Starting destroy method instruction evaluator tests...\n");
    
    // Check if running from bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd bin && ./agerun_destroy_ar_method_instruction_evaluator_tests\n");
            return 1;
        }
    }
    
    // Clean up any existing state at the start
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_destroy_method_instruction_evaluator__create_destroy();
    printf("test_destroy_method_instruction_evaluator__create_destroy passed!\n");
    
    test_destroy_method_instruction_evaluator__evaluate_with_instance();
    printf("test_destroy_method_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_destroy_method_instruction_evaluator__evaluate_legacy();
    printf("test_destroy_method_instruction_evaluator__evaluate_legacy passed!\n");
    
    test_destroy_method_instruction_evaluator__evaluate_with_agents();
    printf("test_destroy_method_instruction_evaluator__evaluate_with_agents passed!\n");
    
    test_destroy_method_instruction_evaluator__evaluate_nonexistent();
    printf("test_destroy_method_instruction_evaluator__evaluate_nonexistent passed!\n");
    
    test_destroy_method_instruction_evaluator__evaluate_invalid_name_type();
    printf("test_destroy_method_instruction_evaluator__evaluate_invalid_name_type passed!\n");
    
    test_destroy_method_instruction_evaluator__evaluate_wrong_arg_count();
    printf("test_destroy_method_instruction_evaluator__evaluate_wrong_arg_count passed!\n");
    
    printf("All destroy method instruction evaluator tests passed!\n");
    
    // Clean up after tests
    ar_methodology__cleanup();
    ar_agency__reset();
    
    return 0;
}