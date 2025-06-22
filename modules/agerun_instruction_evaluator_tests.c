#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_expression_ast.h"
#include "agerun_data.h"
#include "agerun_methodology.h"
#include "agerun_agency.h"
#include "agerun_system.h"
#include "agerun_assignment_instruction_evaluator.h"
#include "agerun_send_instruction_evaluator.h"
#include "agerun_condition_instruction_evaluator.h"
#include "agerun_parse_instruction_evaluator.h"
#include "agerun_build_instruction_evaluator.h"
#include "agerun_method_instruction_evaluator.h"
#include "agerun_agent_instruction_evaluator.h"
#include "agerun_destroy_agent_instruction_evaluator.h"
#include "agerun_destroy_method_instruction_evaluator.h"

static void test_instruction_evaluator__create_destroy(void) {
    // Given an expression evaluator and memory/context/message data
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *context = ar__data__create_map();
    assert(context != NULL);
    
    data_t *message = ar__data__create_string("test message");
    assert(message != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, context);
    assert(expr_eval != NULL);
    
    // When creating an instruction evaluator
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, context, message
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_instruction_evaluator__destroy(evaluator);
    
    // Then cleanup other resources
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(message);
    ar__data__destroy(context);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__create_with_null_context(void) {
    // Given an expression evaluator and memory, but no context or message
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating an instruction evaluator with NULL context and message
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    
    // Then it should be created successfully (context and message are optional)
    assert(evaluator != NULL);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__destroy_null(void) {
    // When destroying a NULL evaluator
    ar_instruction_evaluator__destroy(NULL);
    
    // Then it should handle it gracefully (no crash)
    // If we reach here, the test passed
}

static void test_instruction_evaluator__create_with_null_expr_evaluator(void) {
    // Given memory but no expression evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    // When creating an instruction evaluator with NULL expression evaluator
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        NULL, memory, NULL, NULL
    );
    
    // Then it should fail and return NULL
    assert(evaluator == NULL);
    
    // Cleanup
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__create_with_null_memory(void) {
    // Given an expression evaluator created with dummy memory
    data_t *dummy_memory = ar__data__create_map();
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(dummy_memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating an instruction evaluator with NULL memory
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, NULL, NULL, NULL
    );
    
    // Then it should fail and return NULL (memory is required)
    assert(evaluator == NULL);
    
    // Cleanup
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(dummy_memory);
}

static void test_instruction_evaluator__stores_assignment_evaluator_instance(void) {
    // Given an instruction evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating an instruction evaluator
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Then it should have created and stored an assignment evaluator instance
    // This test will fail until we add the instance storage
    assignment_instruction_evaluator_t *assignment_eval = ar_instruction_evaluator__get_assignment_evaluator(evaluator);
    assert(assignment_eval != NULL);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__stores_all_evaluator_instances(void) {
    // Given an instruction evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    // When creating an instruction evaluator
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Then it should have created and stored all evaluator instances
    assert(ar_instruction_evaluator__get_assignment_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_send_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_condition_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_parse_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_build_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_method_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_agent_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_destroy_agent_evaluator(evaluator) != NULL);
    assert(ar_instruction_evaluator__get_destroy_method_evaluator(evaluator) != NULL);
    
    // Cleanup
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_uses_instance(void) {
    // Given an instruction evaluator with assignment evaluator instance
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Verify the assignment evaluator instance exists
    assignment_instruction_evaluator_t *assignment_eval = ar_instruction_evaluator__get_assignment_evaluator(evaluator);
    assert(assignment_eval != NULL);
    
    // When evaluating an assignment instruction: memory.test := 123
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.test", "123");
    assert(ast != NULL);
    
    // Create and attach the expression AST for the assignment
    expression_ast_t *expr_ast = ar__expression_ast__create_literal_int(123);
    assert(expr_ast != NULL);
    bool ast_set = ar__instruction_ast__set_assignment_expression_ast(ast, expr_ast);
    assert(ast_set == true);
    
    bool result = ar_instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should succeed and use the instance (not the legacy wrapper)
    assert(result == true);
    
    // And the value should be stored in memory
    int value = ar__data__get_map_integer(memory, "test");
    assert(value == 123);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting instruction_evaluator create/destroy tests...\n");
    
    test_instruction_evaluator__create_destroy();
    printf("test_instruction_evaluator__create_destroy passed!\n");
    
    test_instruction_evaluator__create_with_null_context();
    printf("test_instruction_evaluator__create_with_null_context passed!\n");
    
    test_instruction_evaluator__destroy_null();
    printf("test_instruction_evaluator__destroy_null passed!\n");
    
    test_instruction_evaluator__create_with_null_expr_evaluator();
    printf("test_instruction_evaluator__create_with_null_expr_evaluator passed!\n");
    
    test_instruction_evaluator__create_with_null_memory();
    printf("test_instruction_evaluator__create_with_null_memory passed!\n");
    
    test_instruction_evaluator__stores_assignment_evaluator_instance();
    printf("test_instruction_evaluator__stores_assignment_evaluator_instance passed!\n");
    
    test_instruction_evaluator__stores_all_evaluator_instances();
    printf("test_instruction_evaluator__stores_all_evaluator_instances passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_uses_instance();
    printf("test_instruction_evaluator__evaluate_assignment_uses_instance passed!\n");
    
    printf("All instruction_evaluator create/destroy tests passed!\n");
    
    return 0;
}
