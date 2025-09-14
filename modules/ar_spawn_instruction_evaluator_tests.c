#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "ar_assert.h"
#include "ar_evaluator_fixture.h"
#include "ar_spawn_instruction_evaluator.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_methodology.h"
#include "ar_agency.h"
#include "ar_system.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_frame.h"

static void test_spawn_instruction_evaluator__evaluate_with_context(void) {
    // Initialize system for agent creation
    
    // Given a test fixture and frame-based create evaluator
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_with_context");
    assert(fixture != NULL);
    
    // Set up config in memory (since memory becomes the agent's context)
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data__set_map_string(memory, "config", "production");
    
    // Create frame using fixture
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
        log, expr_eval, mut_agency
    );
    assert(evaluator != NULL);
    
    // Register a method to create agents with using instance APIs
    ar_method_t *method = ar_method__create("worker", "send(0, context.config)", "2.0.0");
    assert(method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, method);
    
    // When evaluating a create instruction with context: spawn("worker", "2.0.0", memory)
    const char *args[] = {"\"worker\"", "\"2.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "worker"
    ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_string("worker");
    ar_list__add_last(arg_asts, method_ast);
    
    // Version: "2.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("2.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    // Context: memory
    ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access("memory", NULL, 0);
    ar_list__add_last(arg_asts, context_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Agency cleanup handled by fixture destroy
    
    // Shutdown system
    
    // Clean up methodology after each test to prevent accumulation
    /* ar_methodology__cleanup() removed - fixture handles cleanup */
}

static void test_spawn_instruction_evaluator__evaluate_with_result(void) {
    // Initialize system for agent creation
    
    // Given a test fixture and frame-based create evaluator
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_with_result");
    assert(fixture != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
        log, expr_eval, mut_agency
    );
    assert(evaluator != NULL);
    
    // Register a method to create agents with using instance APIs
    ar_method_t *method = ar_method__create("counter", "memory.count := memory.count + 1", "1.0.0");
    assert(method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, method);
    
    // When evaluating a create instruction with result assignment: memory.agent_id := spawn("counter", "1.0.0", memory)
    const char *args[] = {"\"counter\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args, 3, "memory.agent_id"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "counter"
    ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_string("counter");
    ar_list__add_last(arg_asts, method_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    // Context: memory
    ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access("memory", NULL, 0);
    ar_list__add_last(arg_asts, context_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the agent ID should be stored in memory
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    int agent_id = ar_data__get_map_integer(memory, "agent_id");
    assert(agent_id > 0);  // Agent IDs start from 1
    
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Agency cleanup handled by fixture destroy
    
    // Shutdown system
    
    // Clean up methodology after each test to prevent accumulation
    /* ar_methodology__cleanup() removed - fixture handles cleanup */
}

static void test_spawn_instruction_evaluator__evaluate_invalid_method(void) {
    // Initialize system for agent creation
    
    // Given a test fixture and frame-based create evaluator (no methods registered)
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_invalid_method");
    assert(fixture != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
        log, expr_eval, mut_agency
    );
    assert(evaluator != NULL);
    
    // When evaluating a create instruction with non-existent method: spawn("missing", "1.0.0", memory)
    const char *args[] = {"\"missing\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args, 3, NULL
    );
    assert(ast != NULL);
    
    bool result = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return false (method not found)
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Agency cleanup handled by fixture destroy
    
    // Shutdown system
    
    // Clean up methodology after each test to prevent accumulation
    /* ar_methodology__cleanup() removed - fixture handles cleanup */
}

static void test_spawn_instruction_evaluator__evaluate_invalid_args(void) {
    // Initialize system for agent creation
    
    // Given a test fixture and frame-based create evaluator
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_invalid_args");
    assert(fixture != NULL);
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
        log, expr_eval, mut_agency
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments
    const char *args1[] = {"\"test\"", "\"1.0.0\""};  // Missing context
    ar_instruction_ast_t *ast1 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    bool result1 = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast1);
    assert(result1 == false);
    
    ar_instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string method name
    const char *args2[] = {"42", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast2 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    bool result2 = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast2);
    assert(result2 == false);
    
    ar_instruction_ast__destroy(ast2);
    
    // Test case 3: Non-string version
    const char *args3[] = {"\"test\"", "1.0", "memory"};
    ar_instruction_ast_t *ast3 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args3, 3, NULL
    );
    assert(ast3 != NULL);
    
    bool result3 = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast3);
    assert(result3 == false);
    
    ar_instruction_ast__destroy(ast3);
    
    // Test case 4: Invalid context type (not map)
    const char *args4[] = {"\"test\"", "\"1.0.0\"", "42"};
    ar_instruction_ast_t *ast4 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args4, 3, NULL
    );
    assert(ast4 != NULL);
    
    bool result4 = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast4);
    assert(result4 == false);
    
    ar_instruction_ast__destroy(ast4);
    
    // Cleanup
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Shutdown system
}

static void test_spawn_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_spawn_destroy");
    assert(fixture != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    
    // When creating a create instruction evaluator instance
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(log, expr_eval, mut_agency);
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_spawn_instruction_evaluator__destroy(evaluator);
    
    // Then it should not crash (no assertion needed)
    
    // Cleanup dependencies
    ar_evaluator_fixture__destroy(fixture);
}

static void test_spawn_instruction_evaluator__evaluate_with_instance(void) {
    // Initialize system for agent creation
    
    // Given a test fixture and frame-based create evaluator with config data
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_evaluate_with_instance");
    assert(fixture != NULL);
    
    // Set up config in memory (since memory becomes the agent's context)
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data__set_map_string(memory, "config", "test");
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(log, expr_eval, mut_agency);
    assert(evaluator != NULL);
    
    // Register a method to create agents with
    ar_method_t *method = ar_method__create("tester", "send(0, memory.config)", "1.0.0");
    assert(method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, method);
    
    // When evaluating an agent instruction with the instance: agent("tester", "1.0.0", memory)
    const char *args[] = {"\"tester\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "tester"
    ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_string("tester");
    ar_list__add_last(arg_asts, method_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    // Context: memory
    ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access("memory", NULL, 0);
    ar_list__add_last(arg_asts, context_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Agency cleanup handled by fixture destroy
    
    // Shutdown system
    
    // Clean up methodology after test
    /* ar_methodology__cleanup() removed - fixture handles cleanup */
}

static void test_spawn_instruction_evaluator__legacy_evaluate_function(void) {
    // Initialize system for agent creation
    
    // Given a test fixture and frame-based create evaluator with status data
    ar_evaluator_fixture_t *fixture = 
        ar_evaluator_fixture__create("test_legacy_evaluate_function");
    assert(fixture != NULL);
    
    // Set up status in memory (since memory becomes the agent's context)
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    ar_data__set_map_string(memory, "status", "legacy");
    
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    assert(frame != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_methodology_t *mut_methodology = ar_evaluator_fixture__get_methodology(fixture);
    
    // Create an evaluator instance
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
        log, expr_eval, mut_agency
    );
    assert(evaluator != NULL);
    
    // Register a method to create agents with
    ar_method_t *method = ar_method__create("legacy_worker", "send(0, memory.status)", "1.0.0");
    assert(method != NULL);
    ar_methodology__register_method_with_instance(mut_methodology, method);
    
    // When calling the instance-based evaluate function: agent("legacy_worker", "1.0.0", memory)
    const char *args[] = {"\"legacy_worker\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "legacy_worker"
    ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_string("legacy_worker");
    ar_list__add_last(arg_asts, method_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    // Context: memory
    ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access("memory", NULL, 0);
    ar_list__add_last(arg_asts, context_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    // Agency cleanup handled by fixture destroy
    
    // Shutdown system
    
    // Clean up methodology after test
    /* ar_methodology__cleanup() removed - fixture handles cleanup */
}

static void test_spawn_instruction_evaluator__noop_with_integer_zero(void) {
    printf("Testing spawn with method_name = 0 (no-op case)...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create("test_spawn_noop_integer");
    printf("Fixture created: %s\n", fixture ? "yes" : "no");
    AR_ASSERT(fixture != NULL, "Test fixture should be created");
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    
    // Create spawn evaluator
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
        log, expr_eval, mut_agency
    );
    assert(evaluator != NULL);
    
    // Create spawn AST with result assignment: spawn(0, "1.0.0", context)
    const char *args[] = {"0", "\"1.0.0\"", "context"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args, 3, "memory.agent_id"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: literal integer 0
    ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_int(0);
    ar_list__add_last(arg_asts, method_ast);
    
    // Version: literal string "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    // Context: context variable
    ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access("context", NULL, 0);
    ar_list__add_last(arg_asts, context_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the spawn with method_name = 0
    bool result = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed (no-op returns true)
    printf("Result: %s\n", result ? "true" : "false");
    AR_ASSERT(result == true, "spawn(0, version, context) should return true as a no-op");
    
    // And the agent_id should be 0
    const ar_data_t *agent_id = ar_data__get_map_data(memory, "agent_id");
    AR_ASSERT(agent_id != NULL, "agent_id should be set in memory");
    AR_ASSERT(ar_data__get_type(agent_id) == AR_DATA_TYPE__INTEGER, "agent_id should be an integer");
    AR_ASSERT(ar_data__get_integer(agent_id) == 0, "agent_id should be 0 for no-op spawn");
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    printf("PASS\n");
}

static void test_spawn_instruction_evaluator__noop_with_empty_string(void) {
    printf("Testing spawn with method_name = \"\" (no-op case)...\n");
    
    // Given a test fixture
    ar_evaluator_fixture_t *fixture = ar_evaluator_fixture__create("test_spawn_noop_string");
    assert(fixture != NULL);
    
    ar_log_t *log = ar_evaluator_fixture__get_log(fixture);
    ar_expression_evaluator_t *expr_eval = ar_evaluator_fixture__get_expression_evaluator(fixture);
    ar_agency_t *mut_agency = ar_evaluator_fixture__get_agency(fixture);
    ar_frame_t *frame = ar_evaluator_fixture__create_frame(fixture);
    ar_data_t *memory = ar_evaluator_fixture__get_memory(fixture);
    
    // Create spawn evaluator
    ar_spawn_instruction_evaluator_t *evaluator = ar_spawn_instruction_evaluator__create(
        log, expr_eval, mut_agency
    );
    assert(evaluator != NULL);
    
    // Create spawn AST with result assignment: spawn("", "1.0.0", context)
    const char *args[] = {"\"\"", "\"1.0.0\"", "context"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__SPAWN, "spawn", args, 3, "memory.agent_id"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: literal empty string ""
    ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_string("");
    ar_list__add_last(arg_asts, method_ast);
    
    // Version: literal string "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    // Context: context variable
    ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access("context", NULL, 0);
    ar_list__add_last(arg_asts, context_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the spawn with method_name = ""
    bool result = ar_spawn_instruction_evaluator__evaluate(evaluator, frame, ast);
    
    // Then it should succeed (no-op returns true)
    assert(result == true);
    
    // And the agent_id should be 0
    const ar_data_t *agent_id = ar_data__get_map_data(memory, "agent_id");
    AR_ASSERT(agent_id != NULL, "agent_id should be set in memory");
    AR_ASSERT(ar_data__get_type(agent_id) == AR_DATA_TYPE__INTEGER, "agent_id should be an integer");
    AR_ASSERT(ar_data__get_integer(agent_id) == 0, "agent_id should be 0 for no-op spawn");
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_spawn_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(fixture);
    
    printf("PASS\n");
}

int main(void) {
    printf("Starting create instruction evaluator tests...\n");
    
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
    /* ar_methodology__cleanup() removed - fixture handles cleanup */
    // ar_agency__reset(); // Global API removed - fixtures handle their own agencies
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_spawn_instruction_evaluator__evaluate_with_context();
    printf("test_spawn_instruction_evaluator__evaluate_with_context passed!\n");
    
    test_spawn_instruction_evaluator__evaluate_with_result();
    printf("test_spawn_instruction_evaluator__evaluate_with_result passed!\n");
    
    test_spawn_instruction_evaluator__evaluate_invalid_method();
    printf("test_spawn_instruction_evaluator__evaluate_invalid_method passed!\n");
    
    test_spawn_instruction_evaluator__evaluate_invalid_args();
    printf("test_spawn_instruction_evaluator__evaluate_invalid_args passed!\n");
    
    test_spawn_instruction_evaluator__create_destroy();
    printf("test_spawn_instruction_evaluator__create_destroy passed!\n");
    
    test_spawn_instruction_evaluator__evaluate_with_instance();
    printf("test_spawn_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_spawn_instruction_evaluator__legacy_evaluate_function();
    printf("test_spawn_instruction_evaluator__legacy_evaluate_function passed!\n");
    
    test_spawn_instruction_evaluator__noop_with_integer_zero();
    test_spawn_instruction_evaluator__noop_with_empty_string();
    
    printf("All create instruction evaluator tests passed!\n");
    
    // Final cleanup to ensure no agents are left running
    /* ar_methodology__cleanup() removed - fixture handles cleanup */
    // ar_agency__reset(); // Global API removed - fixtures handle their own agencies
    remove("methodology.agerun");
    remove("agency.agerun");
    
    return 0;
}

