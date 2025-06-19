#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"
#include "agerun_methodology.h"
#include "agerun_agency.h"
#include "agerun_system.h"

static void test_instruction_evaluator__evaluate_destroy_agent(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Clean up any existing methodology state
    ar__methodology__cleanup();
    
    // Initialize system for agent operations
    ar__system__init(NULL, NULL);
    
    // Given an instruction evaluator with an existing agent
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
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
    
    // When evaluating the destroy call
    bool result = ar__instruction_evaluator__evaluate_destroy(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the agent should be destroyed (not exist anymore)
    assert(ar__agency__agent_exists(agent_id) == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_destroy_agent_with_result(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for agent operations
    ar__system__init(NULL, NULL);
    
    // Given an instruction evaluator with an existing agent
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
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
    
    // When evaluating the destroy call
    bool result = ar__instruction_evaluator__evaluate_destroy(evaluator, ast);
    
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
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_destroy_nonexistent_agent(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for agent operations
    ar__system__init(NULL, NULL);
    
    // Given an instruction evaluator with no agents
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-existent agent ID
    const char *args[] = {"999"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 1, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the destroy call
    bool result = ar__instruction_evaluator__evaluate_destroy(evaluator, ast);
    
    // Then it should succeed (no error)
    assert(result == true);
    
    // But the result should be false (0) since agent doesn't exist
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_destroy_method(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    ar__system__init(NULL, NULL);
    
    // Given an instruction evaluator with a registered method
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Create a test method
    ar__methodology__create_method("test_destroyer", "memory.x := 1", "1.0.0");
    
    // Verify method exists
    method_t *method = ar__methodology__get_method("test_destroyer", "1.0.0");
    assert(method != NULL);
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 2, NULL
    );
    assert(ast != NULL);
    
    // When evaluating the destroy call
    bool result = ar__instruction_evaluator__evaluate_destroy(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the method should be destroyed (not exist anymore)
    method = ar__methodology__get_method("test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_destroy_method_with_agents(void) {
    // Clean up any existing persistence files
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system for method operations
    ar__system__init(NULL, NULL);
    
    // Given an instruction evaluator with a method and agents using it
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Create a test method and agents using it
    ar__methodology__create_method("test_destroyer", "memory.x := 1", "1.0.0");
    int64_t agent1 = ar__agency__create_agent("test_destroyer", "1.0.0", NULL);
    int64_t agent2 = ar__agency__create_agent("test_destroyer", "1.0.0", NULL);
    assert(agent1 > 0);
    assert(agent2 > 0);
    
    // Process wake messages to avoid leaks
    ar__system__process_next_message();
    ar__system__process_next_message();
    
    // Create destroy AST with method name and version
    const char *args[] = {"\"test_destroyer\"", "\"1.0.0\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the destroy call
    bool result = ar__instruction_evaluator__evaluate_destroy(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be true (1)
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 1);
    
    // The agents should already be destroyed
    // (In the current implementation, destroy() handles agent destruction immediately)
    assert(ar__agency__agent_exists(agent1) == false);
    assert(ar__agency__agent_exists(agent2) == false);
    
    // And the method should be destroyed
    method_t *method = ar__methodology__get_method("test_destroyer", "1.0.0");
    assert(method == NULL);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up agency before shutting down
    ar__agency__reset();
    
    // Shutdown system
    ar__system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_destroy_invalid_args(void) {
    // Given an instruction evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Create destroy AST with non-integer agent ID (string)
    const char *args[] = {"\"not_a_number\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_DESTROY, "destroy", args, 1, NULL
    );
    assert(ast != NULL);
    
    // When evaluating the destroy call
    bool result = ar__instruction_evaluator__evaluate_destroy(evaluator, ast);
    
    // Then it should fail due to invalid argument type
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting destroy instruction evaluator tests...\n");
    
    // Check if running from bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd bin && ./agerun_destroy_instruction_evaluator_tests\n");
            return 1;
        }
    }
    
    // Clean up any existing state at the start
    ar__system__shutdown();
    ar__methodology__cleanup();
    ar__agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_instruction_evaluator__evaluate_destroy_agent();
    printf("test_instruction_evaluator__evaluate_destroy_agent passed!\n");
    
    test_instruction_evaluator__evaluate_destroy_agent_with_result();
    printf("test_instruction_evaluator__evaluate_destroy_agent_with_result passed!\n");
    
    test_instruction_evaluator__evaluate_destroy_nonexistent_agent();
    printf("test_instruction_evaluator__evaluate_destroy_nonexistent_agent passed!\n");
    
    test_instruction_evaluator__evaluate_destroy_method();
    printf("test_instruction_evaluator__evaluate_destroy_method passed!\n");
    
    test_instruction_evaluator__evaluate_destroy_method_with_agents();
    printf("test_instruction_evaluator__evaluate_destroy_method_with_agents passed!\n");
    
    test_instruction_evaluator__evaluate_destroy_invalid_args();
    printf("test_instruction_evaluator__evaluate_destroy_invalid_args passed!\n");
    
    printf("All destroy instruction evaluator tests passed!\n");
    
    // Clean up after tests
    ar__methodology__cleanup();
    ar__agency__reset();
    
    return 0;
}