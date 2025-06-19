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
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, context, message
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar__instruction_evaluator__destroy(evaluator);
    
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
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    
    // Then it should be created successfully (context and message are optional)
    assert(evaluator != NULL);
    
    // Cleanup
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__destroy_null(void) {
    // When destroying a NULL evaluator
    ar__instruction_evaluator__destroy(NULL);
    
    // Then it should handle it gracefully (no crash)
    // If we reach here, the test passed
}

static void test_instruction_evaluator__create_with_null_expr_evaluator(void) {
    // Given memory but no expression evaluator
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    // When creating an instruction evaluator with NULL expression evaluator
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
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
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, NULL, NULL, NULL
    );
    
    // Then it should fail and return NULL (memory is required)
    assert(evaluator == NULL);
    
    // Cleanup
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(dummy_memory);
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
    
    printf("All instruction_evaluator create/destroy tests passed!\n");
    
    return 0;
}
