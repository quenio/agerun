#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "agerun_send_instruction_evaluator.h"
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"

static void test_send_instruction_evaluator__create_destroy(void) {
    // Given memory and expression evaluator
    data_t *own_memory = ar__data__create_map();
    assert(own_memory != NULL);
    
    expression_evaluator_t *own_expr_eval = ar__expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating a send instruction evaluator
    send_instruction_evaluator_t *own_evaluator = ar_send_instruction_evaluator__create(
        own_expr_eval, own_memory
    );
    
    // Then it should succeed
    assert(own_evaluator != NULL);
    
    // Cleanup
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar__expression_evaluator__destroy(own_expr_eval);
    ar__data__destroy(own_memory);
}

static void test_send_instruction_evaluator__evaluate_with_instance(void) {
    // Given memory and expression evaluator
    data_t *own_memory = ar__data__create_map();
    assert(own_memory != NULL);
    
    expression_evaluator_t *own_expr_eval = ar__expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating a send instruction evaluator
    send_instruction_evaluator_t *own_evaluator = ar_send_instruction_evaluator__create(
        own_expr_eval, own_memory
    );
    assert(own_evaluator != NULL);
    
    // When creating a send AST node for "send(0, 42)"
    const char *args[] = {"0", "42"};
    instruction_ast_t *own_ast = ar__instruction_ast__create_function_call(
        INST_AST_SEND, "send", args, 2, NULL
    );
    assert(own_ast != NULL);
    
    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(own_evaluator, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(own_ast);
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar__expression_evaluator__destroy(own_expr_eval);
    ar__data__destroy(own_memory);
}

static void test_instruction_evaluator__evaluate_send_integer_message(void) {
    // Given an evaluator with memory and agency mock capability
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, 42)"
    const char *args[] = {"0", "42"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // When evaluating the send
    bool result = ar_instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed (send to agent 0 is a no-op that returns true)
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_string_message(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, \"hello\")"
    const char *args[] = {"0", "\"hello\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // When evaluating the send
    bool result = ar_instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_with_result(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "memory.result := send(0, \"test\")"
    const char *args[] = {"0", "\"test\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_SEND, "send", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the send
    bool result = ar_instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be stored in memory (send returns true for agent 0)
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 1); // true as integer
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_memory_reference(void) {
    // Given an evaluator with memory containing a message
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *msg_value = ar__data__create_string("Hello from memory");
    ar__data__set_map_data(memory, "msg", msg_value);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, memory.msg)"
    const char *args[] = {"0", "memory.msg"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // When evaluating the send
    bool result = ar_instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node with only one argument
    const char *args[] = {"0"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_SEND, "send", args, 1, NULL
    );
    assert(ast != NULL);
    
    // When evaluating the send
    bool result = ar_instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should fail (send requires 2 arguments)
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar_instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

int main(void) {
    printf("Starting send instruction_evaluator tests...\n");
    
    test_send_instruction_evaluator__create_destroy();
    printf("test_send_instruction_evaluator__create_destroy passed!\n");
    
    test_send_instruction_evaluator__evaluate_with_instance();
    printf("test_send_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_instruction_evaluator__evaluate_send_integer_message();
    printf("test_instruction_evaluator__evaluate_send_integer_message passed!\n");
    
    test_instruction_evaluator__evaluate_send_string_message();
    printf("test_instruction_evaluator__evaluate_send_string_message passed!\n");
    
    test_instruction_evaluator__evaluate_send_with_result();
    printf("test_instruction_evaluator__evaluate_send_with_result passed!\n");
    
    test_instruction_evaluator__evaluate_send_memory_reference();
    printf("test_instruction_evaluator__evaluate_send_memory_reference passed!\n");
    
    test_instruction_evaluator__evaluate_send_invalid_args();
    printf("test_instruction_evaluator__evaluate_send_invalid_args passed!\n");
    
    printf("All send instruction_evaluator tests passed!\n");
    
    return 0;
}