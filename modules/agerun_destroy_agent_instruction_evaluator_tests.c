#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "agerun_destroy_agent_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_expression_ast.h"
#include "agerun_data.h"
#include "agerun_list.h"
#include "agerun_methodology.h"
#include "agerun_agency.h"
#include "agerun_system.h"
#include "agerun_heap.h"

// Test create/destroy lifecycle
static void test_destroy_agent_instruction_evaluator__create_destroy(void) {
    // Given dependencies
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating a destroy agent evaluator
    ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
        expr_eval, memory
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_destroy_agent_instruction_evaluator__destroy(evaluator);
    
    // Then no memory leaks should occur (verified by test framework)
    
    // Cleanup
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

// Test evaluate with instance
static void test_destroy_agent_instruction_evaluator__evaluate_with_instance(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Clean up any existing methodology state
    ar__methodology__cleanup();
    
    // Initialize system for agent operations
    ar__system__init(NULL, NULL);
    
    // Given an evaluator instance with an existing agent
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create a test method and agent
    ar__methodology__create_method("test_method", "memory.x := 1", "1.0.0");
    int64_t agent_id = ar__agency__create_agent("test_method", "1.0.0", NULL);
    assert(agent_id > 0);
    
    // Process wake message to avoid leak
    ar__system__process_next_message();
    
    // Set the agent ID in memory for evaluation
    ar__data__set_map_integer(memory, "agent_id", (int)agent_id);
    
    // Create destroy AST with agent ID
    const char *args[] = {"memory.agent_id"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 1, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression AST for the argument
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Agent ID: memory.agent_id
    const char *agent_id_path[] = {"agent_id"};
    expression_ast_t *agent_id_ast = ar__expression_ast__create_memory_access("memory", agent_id_path, 1);
    ar__list__add_last(arg_asts, agent_id_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call using instance
    bool result = ar_destroy_agent_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the agent should be destroyed (not exist anymore)
    assert(ar__agency__agent_exists(agent_id) == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_destroy_agent_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

// Test legacy function
static void test_destroy_agent_instruction_evaluator__evaluate_legacy(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for agent operations
    ar__system__init(NULL, NULL);
    
    // Given dependencies
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // Create an evaluator instance
    ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create a test method and agent
    ar__methodology__create_method("test_method", "memory.x := 1", "1.0.0");
    int64_t agent_id = ar__agency__create_agent("test_method", "1.0.0", NULL);
    assert(agent_id > 0);
    
    // Process wake message to avoid leak
    ar__system__process_next_message();
    
    // Create destroy AST with agent ID
    char agent_id_str[32];
    snprintf(agent_id_str, sizeof(agent_id_str), "%" PRId64, agent_id);
    const char *args[] = {agent_id_str};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 1, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression AST for the argument
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Agent ID: literal integer
    expression_ast_t *agent_id_ast = ar__expression_ast__create_literal_int((int)agent_id);
    ar__list__add_last(arg_asts, agent_id_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using instance-based interface
    bool result = ar_destroy_agent_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the agent should be destroyed
    assert(ar__agency__agent_exists(agent_id) == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_destroy_agent_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

// Test destroy agent with result assignment
static void test_destroy_agent_instruction_evaluator__evaluate_with_result(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for agent operations
    ar__system__init(NULL, NULL);
    
    // Given an evaluator instance with an existing agent
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create a test method and agent
    ar__methodology__create_method("test_method", "memory.x := 1", "1.0.0");
    int64_t agent_id = ar__agency__create_agent("test_method", "1.0.0", NULL);
    assert(agent_id > 0);
    
    // Process wake message to avoid leak
    ar__system__process_next_message();
    
    // Create destroy AST with result assignment
    char agent_id_str[32];
    snprintf(agent_id_str, sizeof(agent_id_str), "%" PRId64, agent_id);
    const char *args[] = {agent_id_str};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 1, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression AST for the argument
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Agent ID: literal integer
    expression_ast_t *agent_id_ast = ar__expression_ast__create_literal_int((int)agent_id);
    ar__list__add_last(arg_asts, agent_id_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_agent_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be true (1)
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 1);
    
    // And the agent should be destroyed
    assert(ar__agency__agent_exists(agent_id) == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_destroy_agent_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

// Test destroy nonexistent agent
static void test_destroy_agent_instruction_evaluator__evaluate_nonexistent(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for agent operations
    ar__system__init(NULL, NULL);
    
    // Given an evaluator instance with no agents
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-existent agent ID
    const char *args[] = {"999"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 1, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression AST for the argument
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Agent ID: 999 (nonexistent)
    expression_ast_t *agent_id_ast = ar__expression_ast__create_literal_int(999);
    ar__list__add_last(arg_asts, agent_id_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_agent_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed (no error)
    assert(result == true);
    
    // But the result should be false (0) since agent doesn't exist
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_destroy_agent_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

// Test destroy with invalid agent ID type
static void test_destroy_agent_instruction_evaluator__evaluate_invalid_type(void) {
    // Given an evaluator instance
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-integer agent ID (string)
    const char *args[] = {"\"not_a_number\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 1, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression AST for the argument
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Agent ID: "not_a_number" (string, not integer)
    expression_ast_t *agent_id_ast = ar__expression_ast__create_literal_string("not_a_number");
    ar__list__add_last(arg_asts, agent_id_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_agent_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should fail due to invalid argument type
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_destroy_agent_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

// Test destroy with wrong number of arguments
static void test_destroy_agent_instruction_evaluator__evaluate_wrong_arg_count(void) {
    // Given an evaluator instance
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_destroy_agent_instruction_evaluator_t *evaluator = ar_destroy_agent_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with 2 args (should be 1 for agent)
    const char *args[] = {"1", "\"extra\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for the arguments
    list_t *arg_asts = ar__list__create();
    assert(arg_asts != NULL);
    
    // Agent ID: 1
    expression_ast_t *agent_id_ast = ar__expression_ast__create_literal_int(1);
    ar__list__add_last(arg_asts, agent_id_ast);
    
    // Extra argument: "extra" (should cause error due to wrong arg count)
    expression_ast_t *extra_ast = ar__expression_ast__create_literal_string("extra");
    ar__list__add_last(arg_asts, extra_ast);
    
    bool ast_set = ar__instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the destroy call
    bool result = ar_destroy_agent_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should fail due to wrong argument count
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_destroy_agent_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting destroy agent instruction evaluator tests...\n");
    
    // Check if running from bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd bin && ./agerun_destroy_agent_instruction_evaluator_tests\n");
            return 1;
        }
    }
    
    // Clean up any existing state at the start
    ar__system__shutdown();
    ar__methodology__cleanup();
    ar__agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_destroy_agent_instruction_evaluator__create_destroy();
    printf("test_destroy_agent_instruction_evaluator__create_destroy passed!\n");
    
    test_destroy_agent_instruction_evaluator__evaluate_with_instance();
    printf("test_destroy_agent_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_destroy_agent_instruction_evaluator__evaluate_legacy();
    printf("test_destroy_agent_instruction_evaluator__evaluate_legacy passed!\n");
    
    test_destroy_agent_instruction_evaluator__evaluate_with_result();
    printf("test_destroy_agent_instruction_evaluator__evaluate_with_result passed!\n");
    
    test_destroy_agent_instruction_evaluator__evaluate_nonexistent();
    printf("test_destroy_agent_instruction_evaluator__evaluate_nonexistent passed!\n");
    
    test_destroy_agent_instruction_evaluator__evaluate_invalid_type();
    printf("test_destroy_agent_instruction_evaluator__evaluate_invalid_type passed!\n");
    
    test_destroy_agent_instruction_evaluator__evaluate_wrong_arg_count();
    printf("test_destroy_agent_instruction_evaluator__evaluate_wrong_arg_count passed!\n");
    
    printf("All destroy agent instruction evaluator tests passed!\n");
    
    // Clean up after tests
    ar__methodology__cleanup();
    ar__agency__reset();
    
    return 0;
}