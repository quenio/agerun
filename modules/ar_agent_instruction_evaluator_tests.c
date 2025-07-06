#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "ar_agent_instruction_evaluator.h"
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

static void test_agent_instruction_evaluator__evaluate_with_context(void) {
    // Initialize system for agent creation
    ar_system__init(NULL, NULL);
    
    // Given an agent instruction evaluator with memory and a registered method
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_data_t *context = ar_data__create_map();
    assert(context != NULL);
    ar_data__set_map_string(context, "config", "production");
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, context);
    assert(expr_eval != NULL);
    
    ar_agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Register a method to create agents with
    method_t *method = ar_method__create("worker", "send(0, context.config)", "2.0.0");
    assert(method != NULL);
    ar_methodology__register_method(method);
    
    // When evaluating an agent instruction with context: agent("worker", "2.0.0", memory)
    const char *args[] = {"\"worker\"", "\"2.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "worker"
    ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_string("worker");
    ar_list__add_last(arg_asts, method_ast);
    
    // Version: "2.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("2.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    // Context: context
    ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access("context", NULL, 0);
    ar_list__add_last(arg_asts, context_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_agent_instruction_evaluator__evaluate(evaluator, context, ast);
    
    // Then it should return true
    assert(result == true);
    
    // Process wake message to avoid leak
    ar_system__process_next_message();
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_agent_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(context);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    ar_system__shutdown();
    
    // Clean up methodology after each test to prevent accumulation
    ar_methodology__cleanup();
}

static void test_agent_instruction_evaluator__evaluate_with_result(void) {
    // Initialize system for agent creation
    ar_system__init(NULL, NULL);
    // Given an instruction evaluator with memory and a registered method
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Register a method to create agents with
    method_t *method = ar_method__create("counter", "memory.count := memory.count + 1", "1.0.0");
    assert(method != NULL);
    ar_methodology__register_method(method);
    
    // When evaluating an agent instruction with result assignment: memory.agent_id := agent("counter", "1.0.0", memory)
    const char *args[] = {"\"counter\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args, 3, "memory.agent_id"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
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
    
    bool result = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the agent ID should be stored in memory
    int agent_id = ar_data__get_map_integer(memory, "agent_id");
    assert(agent_id > 0);  // Agent IDs start from 1
    
    // Process wake message to avoid leak
    ar_system__process_next_message();
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_agent_instruction_evaluator__destroy(evaluator);
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

static void test_agent_instruction_evaluator__evaluate_invalid_method(void) {
    // Initialize system for agent creation
    ar_system__init(NULL, NULL);
    // Given an instruction evaluator with memory (no methods registered)
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When evaluating an agent instruction with non-existent method: agent("missing", "1.0.0", memory)
    const char *args[] = {"\"missing\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args, 3, NULL
    );
    assert(ast != NULL);
    
    bool result = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast);
    
    // Then it should return false (method not found)
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_agent_instruction_evaluator__destroy(evaluator);
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

static void test_agent_instruction_evaluator__evaluate_invalid_args(void) {
    // Initialize system for agent creation
    ar_system__init(NULL, NULL);
    // Given an instruction evaluator with memory
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments
    const char *args1[] = {"\"test\"", "\"1.0.0\""};  // Missing context
    ar_instruction_ast_t *ast1 = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    bool result1 = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast1);
    assert(result1 == false);
    
    ar_instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string method name
    const char *args2[] = {"42", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast2 = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    bool result2 = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast2);
    assert(result2 == false);
    
    ar_instruction_ast__destroy(ast2);
    
    // Test case 3: Non-string version
    const char *args3[] = {"\"test\"", "1.0", "memory"};
    ar_instruction_ast_t *ast3 = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args3, 3, NULL
    );
    assert(ast3 != NULL);
    
    bool result3 = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast3);
    assert(result3 == false);
    
    ar_instruction_ast__destroy(ast3);
    
    // Test case 4: Invalid context type (not map)
    const char *args4[] = {"\"test\"", "\"1.0.0\"", "42"};
    ar_instruction_ast_t *ast4 = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args4, 3, NULL
    );
    assert(ast4 != NULL);
    
    bool result4 = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast4);
    assert(result4 == false);
    
    ar_instruction_ast__destroy(ast4);
    
    // Cleanup
    ar_agent_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Shutdown system
    ar_system__shutdown();
}

static void test_agent_instruction_evaluator__create_destroy(void) {
    // Given expression evaluator and memory dependencies
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating an agent instruction evaluator instance
    ar_agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(log, expr_eval, memory);
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_agent_instruction_evaluator__destroy(evaluator);
    
    // Then it should not crash (no assertion needed)
    
    // Cleanup dependencies
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
}

static void test_agent_instruction_evaluator__evaluate_with_instance(void) {
    // Initialize system for agent creation
    ar_system__init(NULL, NULL);
    
    // Given an agent instruction evaluator instance with dependencies
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_data__set_map_string(memory, "config", "test");
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    ar_agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(log, expr_eval, memory);
    assert(evaluator != NULL);
    
    // Register a method to create agents with
    method_t *method = ar_method__create("tester", "send(0, memory.config)", "1.0.0");
    assert(method != NULL);
    ar_methodology__register_method(method);
    
    // When evaluating an agent instruction with the instance: agent("tester", "1.0.0", memory)
    const char *args[] = {"\"tester\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
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
    
    bool result = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast);
    
    // Then it should return true
    assert(result == true);
    
    // Process wake message to avoid leak
    ar_system__process_next_message();
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_agent_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    ar_system__shutdown();
    
    // Clean up methodology after test
    ar_methodology__cleanup();
}

static void test_agent_instruction_evaluator__legacy_evaluate_function(void) {
    // Initialize system for agent creation
    ar_system__init(NULL, NULL);
    
    // Given legacy-style usage of evaluate function with explicit parameters
    ar_data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_data__set_map_string(memory, "status", "legacy");
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(log, memory, NULL);
    assert(expr_eval != NULL);
    
    // Create an evaluator instance
    ar_agent_instruction_evaluator_t *evaluator = ar_agent_instruction_evaluator__create(
        log, expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // Register a method to create agents with
    method_t *method = ar_method__create("legacy_worker", "send(0, memory.status)", "1.0.0");
    assert(method != NULL);
    ar_methodology__register_method(method);
    
    // When calling the instance-based evaluate function: agent("legacy_worker", "1.0.0", memory)
    const char *args[] = {"\"legacy_worker\"", "\"1.0.0\"", "memory"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__AGENT, "agent", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
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
    
    bool result = ar_agent_instruction_evaluator__evaluate(evaluator, NULL, ast);
    
    // Then it should return true
    assert(result == true);
    
    // Process wake message to avoid leak
    ar_system__process_next_message();
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_agent_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
    ar_log__destroy(log);
    
    // Clean up agency before shutting down
    ar_agency__reset();
    
    // Shutdown system
    ar_system__shutdown();
    
    // Clean up methodology after test
    ar_methodology__cleanup();
}

int main(void) {
    printf("Starting agent instruction evaluator tests...\n");
    
    // Check if running from bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd bin && ./agerun_ar_agent_instruction_evaluator_tests\n");
            return 1;
        }
    }
    
    // Clean up any existing state at the start
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_agent_instruction_evaluator__evaluate_with_context();
    printf("test_agent_instruction_evaluator__evaluate_with_context passed!\n");
    
    test_agent_instruction_evaluator__evaluate_with_result();
    printf("test_agent_instruction_evaluator__evaluate_with_result passed!\n");
    
    test_agent_instruction_evaluator__evaluate_invalid_method();
    printf("test_agent_instruction_evaluator__evaluate_invalid_method passed!\n");
    
    test_agent_instruction_evaluator__evaluate_invalid_args();
    printf("test_agent_instruction_evaluator__evaluate_invalid_args passed!\n");
    
    test_agent_instruction_evaluator__create_destroy();
    printf("test_agent_instruction_evaluator__create_destroy passed!\n");
    
    test_agent_instruction_evaluator__evaluate_with_instance();
    printf("test_agent_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_agent_instruction_evaluator__legacy_evaluate_function();
    printf("test_agent_instruction_evaluator__legacy_evaluate_function passed!\n");
    
    printf("All agent instruction evaluator tests passed!\n");
    
    // Final cleanup to ensure no agents are left running
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    return 0;
}

