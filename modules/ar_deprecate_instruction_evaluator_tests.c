#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "ar_deprecate_instruction_evaluator.h"
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_agent_registry.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_event.h"

// Test create/destroy lifecycle
static void test_deprecate_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create(
        "test_deprecate_instruction_evaluator__create_destroy"
    );
    assert(fixture != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    // When creating a destroy method evaluator (frame-based pattern)
    ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
        log, expr_eval, mut_methodology
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_deprecate_instruction_evaluator__destroy(evaluator);
    
    // Then no memory leaks should occur (verified by test framework)
    
    // Cleanup
    ar_evaluator_fixture__destroy(fixture);
}

// Test evaluate with instance
static void test_deprecate_instruction_evaluator__evaluate_with_instance(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    
    // Given a test fixture and evaluator instance
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create(
        "test_deprecate_instruction_evaluator__evaluate_with_instance"
    );
    assert(fixture != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
        log, expr_eval, mut_methodology
    );
    assert(evaluator != NULL);
    
    // Create a test method using instance APIs
    ar_method_t *own_method = ar_method__create("test_destroyer", "memory.x := 1", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    
    // Verify method exists
    ar_method_t *method = ar_methodology__get_method_with_instance(mut_methodology, "test_destroyer", "1.0.0");
    assert(method != NULL);
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "test_destroyer"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_destroyer");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create frame for evaluation
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // When evaluating the destroy call using frame-based pattern
    bool result = ar_deprecate_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the method should be destroyed (not exist anymore)
    method = ar_methodology__get_method_with_instance(mut_methodology, "test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_deprecate_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

// Test frame-based evaluation
static void test_deprecate_instruction_evaluator__evaluate_frame_based(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create(
        "test_deprecate_instruction_evaluator__evaluate_frame_based"
    );
    assert(fixture != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    // Create an evaluator instance
    ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
        log, expr_eval, mut_methodology
    );
    assert(evaluator != NULL);
    
    // Create a test method using instance APIs
    ar_method_t *own_method = ar_method__create("test_destroyer", "memory.x := 1", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "test_destroyer"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_destroyer");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create frame for evaluation
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // When evaluating using frame-based interface
    bool result = ar_deprecate_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the method should be destroyed
    ar_method_t *method = ar_methodology__get_method_with_instance(mut_methodology, "test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_deprecate_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

// Test deprecate method with agents using it (agents should remain active)
static void test_deprecate_instruction_evaluator__evaluate_with_agents(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    
    // Given a test fixture and evaluator instance with a method and agents using it
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create(
        "test_deprecate_instruction_evaluator__evaluate_with_agents"
    );
    assert(fixture != NULL);
    
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    assert(mut_agency != NULL);
    assert(mut_methodology != NULL);
    
    ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
        log, expr_eval, mut_methodology
    );
    assert(evaluator != NULL);
    
    // Create a test method using instance APIs
    ar_method_t *own_method = ar_method__create("test_destroyer", "memory.x := 1", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    
    // Create agents using instance APIs
    int64_t agent1 = ar_agency__create_agent_with_instance(mut_agency, "test_destroyer", "1.0.0", NULL);
    int64_t agent2 = ar_agency__create_agent_with_instance(mut_agency, "test_destroyer", "1.0.0", NULL);
    assert(agent1 > 0);
    assert(agent2 > 0);
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "test_destroyer"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_destroyer");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create frame for evaluation
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // When evaluating the destroy call
    bool result = ar_deprecate_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be true (1)
    ar_data_t *result_value = ar_data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 1);
    
    // The agents should still exist (deprecate no longer destroys agents)
    ar_agent_registry_t *registry = ar_agency__get_registry_with_instance(mut_agency);
    assert(ar_agent_registry__is_registered(registry, agent1) == true);
    assert(ar_agent_registry__is_registered(registry, agent2) == true);
    
    // And the method should be destroyed
    ar_method_t *method = ar_methodology__get_method_with_instance(mut_methodology, "test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_deprecate_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // System cleanup is handled by fixture destroy
}

// Test destroy nonexistent method
static void test_deprecate_instruction_evaluator__evaluate_nonexistent(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    
    // Given a test fixture and evaluator instance with no methods
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create(
        "test_deprecate_instruction_evaluator__evaluate_nonexistent"
    );
    assert(fixture != NULL);
    
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
        log, expr_eval, mut_methodology
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-existent method
    const char *args[] = {"\"nonexistent\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "nonexistent"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("nonexistent");
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create frame for evaluation
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // When evaluating the destroy call
    bool result = ar_deprecate_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed (no error)
    assert(result == true);
    
    // But the result should be false (0) since method doesn't exist
    ar_data_t *result_value = ar_data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_deprecate_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

// Test destroy with invalid method name type
static void test_deprecate_instruction_evaluator__evaluate_invalid_name_type(void) {
    // Given a test fixture and evaluator instance
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create(
        "test_deprecate_instruction_evaluator__evaluate_invalid_name_type"
    );
    assert(fixture != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
        log, expr_eval, mut_methodology
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-string method name (integer)
    const char *args[] = {"123", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: 123 (integer, not string)
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_int(123);
    ar_list__add_last(arg_asts, name_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create frame for evaluation
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // When evaluating the destroy call
    bool result = ar_deprecate_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should fail due to invalid argument type
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_deprecate_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

// Test destroy with wrong number of arguments
static void test_deprecate_instruction_evaluator__evaluate_wrong_arg_count(void) {
    // Given a test fixture and evaluator instance
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create(
        "test_deprecate_instruction_evaluator__evaluate_wrong_arg_count"
    );
    assert(fixture != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    ar_deprecate_instruction_evaluator_t *evaluator = ar_deprecate_instruction_evaluator__create(
        log, expr_eval, mut_methodology
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with 1 arg (should be 2 for method)
    const char *args[] = {"\"method_name\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__DEPRECATE, "deprecate", args, 1, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression AST for the single argument
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name only (missing version - should cause error)
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("method_name");
    ar_list__add_last(arg_asts, name_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create frame for evaluation
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    // When evaluating the destroy call
    bool result = ar_deprecate_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should fail due to wrong argument count
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_deprecate_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
}

int main(void) {
    printf("Starting deprecate instruction evaluator tests...\n");
    
    // Check if running from bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run from a bin directory\n");
            return 1;
        }
    }
    
    // Clean up any existing state at the start
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_deprecate_instruction_evaluator__create_destroy();
    printf("test_deprecate_instruction_evaluator__create_destroy passed!\n");
    
    test_deprecate_instruction_evaluator__evaluate_with_instance();
    printf("test_deprecate_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_deprecate_instruction_evaluator__evaluate_frame_based();
    printf("test_deprecate_instruction_evaluator__evaluate_frame_based passed!\n");
    
    test_deprecate_instruction_evaluator__evaluate_with_agents();
    printf("test_deprecate_instruction_evaluator__evaluate_with_agents passed!\n");
    
    test_deprecate_instruction_evaluator__evaluate_nonexistent();
    printf("test_deprecate_instruction_evaluator__evaluate_nonexistent passed!\n");
    
    test_deprecate_instruction_evaluator__evaluate_invalid_name_type();
    printf("test_deprecate_instruction_evaluator__evaluate_invalid_name_type passed!\n");
    
    test_deprecate_instruction_evaluator__evaluate_wrong_arg_count();
    printf("test_deprecate_instruction_evaluator__evaluate_wrong_arg_count passed!\n");
    
    printf("All deprecate instruction evaluator tests passed!\n");
    
    // Clean up after tests
    ar_methodology__cleanup();
    ar_agency__reset();
    
    return 0;
}
