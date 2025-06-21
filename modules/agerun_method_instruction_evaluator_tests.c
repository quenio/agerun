#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "agerun_method_instruction_evaluator.h"
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"
#include "agerun_methodology.h"
#include "agerun_agency.h"
#include "agerun_system.h"

static void test_method_instruction_evaluator__create_destroy(void) {
    // Given memory and expression evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating a method instruction evaluator
    ar_method_instruction_evaluator_t *evaluator = ar_method_instruction_evaluator__create(
        expr_eval, memory
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_method_instruction_evaluator__destroy(evaluator);
    
    // Then cleanup dependencies
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_method_instruction_evaluator__evaluate_with_instance(void) {
    // Given memory and expression evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating a method instruction evaluator instance
    ar_method_instruction_evaluator_t *evaluator = ar_method_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a method AST node
    const char *args[] = {"\"test_method\"", "\"send(0, 42)\"", "\"1.0.0\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args, 3, NULL
    );
    assert(ast != NULL);
    
    // When evaluating using the instance
    bool result = ar_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_method_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up the method we registered
    ar__methodology__cleanup();
}

static void test_method_instruction_evaluator__evaluate_legacy(void) {
    // Given memory and expression evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // Create an evaluator instance
    ar_method_instruction_evaluator_t *evaluator = ar_method_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a method AST node with result assignment
    const char *args[] = {"\"legacy_test\"", "\"send(0, 99)\"", "\"2.0.0\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating using the instance-based interface
    bool result = ar_method_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be stored in memory
    int created = ar__data__get_map_integer(memory, "result");
    assert(created == 1);  // true is stored as 1
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_method_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up the method we registered
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_simple(void) {
    // Given an instruction evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating a method instruction: method("counter", "send(message.sender, memory.count + 1)", "1.0.0")
    const char *args[] = {"\"counter\"", "\"send(message.sender, memory.count + 1)\"", "\"1.0.0\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args, 3, NULL
    );
    assert(ast != NULL);
    
    bool result = ar_instruction_evaluator__evaluate_method(evaluator, ast);
    
    // Then it should return true (method creation successful)
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up the method we registered
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_with_result(void) {
    // Given an instruction evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating a method instruction with result assignment: memory.created := method("echo", "send(message.sender, message.content)", "2.0.0")
    const char *args[] = {"\"echo\"", "\"send(message.sender, message.content)\"", "\"2.0.0\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args, 3, "memory.created"
    );
    assert(ast != NULL);
    
    bool result = ar_instruction_evaluator__evaluate_method(evaluator, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the result should be stored in memory
    int created = ar__data__get_map_integer(memory, "created");
    assert(created == 1);  // true is stored as 1
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up the method we registered
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_invalid_instructions(void) {
    // Given an instruction evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When evaluating a method instruction with invalid instructions: method("bad", "invalid syntax here", "1.0.0")
    const char *args[] = {"\"bad\"", "\"invalid syntax here\"", "\"1.0.0\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args, 3, NULL
    );
    assert(ast != NULL);
    
    bool result = ar_instruction_evaluator__evaluate_method(evaluator, ast);
    
    // Then it should return true (method creation succeeds even with invalid instructions)
    // The validation happens when the method is actually executed
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up the method we registered
    ar__methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_invalid_args(void) {
    // Given an instruction evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments
    const char *args1[] = {"\"test\"", "\"send(0, 42)\""};  // Missing version
    instruction_ast_t *ast1 = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    bool result1 = ar_instruction_evaluator__evaluate_method(evaluator, ast1);
    assert(result1 == false);
    
    ar__instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string method name
    const char *args2[] = {"42", "\"send(0, 42)\"", "\"1.0.0\""};
    instruction_ast_t *ast2 = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    bool result2 = ar_instruction_evaluator__evaluate_method(evaluator, ast2);
    assert(result2 == false);
    
    ar__instruction_ast__destroy(ast2);
    
    // Test case 3: Non-string instructions
    const char *args3[] = {"\"test\"", "42", "\"1.0.0\""};
    instruction_ast_t *ast3 = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args3, 3, NULL
    );
    assert(ast3 != NULL);
    
    bool result3 = ar_instruction_evaluator__evaluate_method(evaluator, ast3);
    assert(result3 == false);
    
    ar__instruction_ast__destroy(ast3);
    
    // Test case 4: Non-string version
    const char *args4[] = {"\"test\"", "\"send(0, 42)\"", "1.0"};
    instruction_ast_t *ast4 = ar__instruction_ast__create_function_call(
        INST_AST_METHOD, "method", args4, 3, NULL
    );
    assert(ast4 != NULL);
    
    bool result4 = ar_instruction_evaluator__evaluate_method(evaluator, ast4);
    assert(result4 == false);
    
    ar__instruction_ast__destroy(ast4);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
    
    // Clean up any methods that might have been registered
    ar__methodology__cleanup();
}

int main(void) {
    printf("Starting method instruction evaluator tests...\n");
    
    // Check if running from bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd bin && ./agerun_method_instruction_evaluator_tests\n");
            return 1;
        }
    }
    
    // Clean up any existing state at the start
    ar__system__shutdown();
    ar__methodology__cleanup();
    ar__agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_method_instruction_evaluator__create_destroy();
    printf("test_method_instruction_evaluator__create_destroy passed!\n");
    
    test_method_instruction_evaluator__evaluate_with_instance();
    printf("test_method_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_method_instruction_evaluator__evaluate_legacy();
    printf("test_method_instruction_evaluator__evaluate_legacy passed!\n");
    
    test_instruction_evaluator__evaluate_method_simple();
    printf("test_instruction_evaluator__evaluate_method_simple passed!\n");
    
    test_instruction_evaluator__evaluate_method_with_result();
    printf("test_instruction_evaluator__evaluate_method_with_result passed!\n");
    
    test_instruction_evaluator__evaluate_method_invalid_instructions();
    printf("test_instruction_evaluator__evaluate_method_invalid_instructions passed!\n");
    
    test_instruction_evaluator__evaluate_method_invalid_args();
    printf("test_instruction_evaluator__evaluate_method_invalid_args passed!\n");
    
    printf("All method instruction evaluator tests passed!\n");
    
    // Clean up after tests
    ar__methodology__cleanup();
    
    return 0;
}