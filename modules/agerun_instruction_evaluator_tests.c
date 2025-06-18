#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "agerun_instruction_evaluator.h"
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"

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

static void test_instruction_evaluator__evaluate_assignment_integer(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an assignment AST node for "memory.x := 42"
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.x", "42");
    assert(ast != NULL);
    
    // When evaluating the assignment
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the value should be stored in memory
    data_t *value = ar__data__get_map_data(memory, "x");
    assert(value != NULL);
    assert(ar__data__get_type(value) == DATA_INTEGER);
    assert(ar__data__get_integer(value) == 42);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_string(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an assignment AST node for "memory.name := \"Alice\""
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.name", "\"Alice\"");
    assert(ast != NULL);
    
    // When evaluating the assignment
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the value should be stored in memory
    data_t *value = ar__data__get_map_data(memory, "name");
    assert(value != NULL);
    assert(ar__data__get_type(value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(value), "Alice") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_nested_path(void) {
    // Given an evaluator with memory containing a nested map
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *user = ar__data__create_map();
    assert(user != NULL);
    ar__data__set_map_data(memory, "user", user);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an assignment AST node for "memory.user.age := 30"
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.user.age", "30");
    assert(ast != NULL);
    
    // When evaluating the assignment
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the value should be stored in the nested map
    data_t *age = ar__data__get_map_data(user, "age");
    assert(age != NULL);
    assert(ar__data__get_type(age) == DATA_INTEGER);
    assert(ar__data__get_integer(age) == 30);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_expression(void) {
    // Given an evaluator with memory containing initial values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *x_value = ar__data__create_integer(10);
    ar__data__set_map_data(memory, "x", x_value);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an assignment AST node for "memory.result := memory.x + 5"
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("memory.result", "memory.x + 5");
    assert(ast != NULL);
    
    // When evaluating the assignment
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the computed value should be stored
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 15);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_assignment_invalid_path(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an assignment AST node with invalid path (not starting with "memory.")
    instruction_ast_t *ast = ar__instruction_ast__create_assignment("x", "42");
    assert(ast != NULL);
    
    // When evaluating the assignment
    bool result = ar__instruction_evaluator__evaluate_assignment(evaluator, ast);
    
    // Then it should fail
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_integer_message(void) {
    // Given an evaluator with memory and agency mock capability
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
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
    bool result = ar__instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed (send to agent 0 is a no-op that returns true)
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_string_message(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
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
    bool result = ar__instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_with_result(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
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
    bool result = ar__instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be stored in memory (send returns true for agent 0)
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 1); // true as integer
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
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
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
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
    bool result = ar__instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
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
    bool result = ar__instruction_evaluator__evaluate_send(evaluator, ast);
    
    // Then it should fail (send requires 2 arguments)
    assert(result == false);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_true_condition(void) {
    // Given an evaluator with memory containing a condition
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "x", ar__data__create_integer(10)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with true condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and store the true value
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 100);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_false_condition(void) {
    // Given an evaluator with memory containing a condition
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "x", ar__data__create_integer(3)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with false condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and store the false value
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 200);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_with_expressions(void) {
    // Given an evaluator with memory containing values
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "a", ar__data__create_integer(10)));
    assert(ar__data__set_map_data(memory, "b", ar__data__create_integer(20)));
    assert(ar__data__set_map_data(memory, "flag", ar__data__create_integer(1)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with expression arguments
    const char *args[] = {"memory.flag", "memory.a + memory.b", "memory.a - memory.b"};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and evaluate the true expression
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_INTEGER);
    assert(ar__data__get_integer(result_value) == 30);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_nested(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    assert(ar__data__set_map_data(memory, "x", ar__data__create_integer(15)));
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // When creating a nested if expression (simulated as string)
    // if(x > 10, if(x > 20, "large", "medium"), "small")
    // Since we can't nest function calls, we'll test with simple return value
    const char *args[] = {"memory.x > 10", "\"medium\"", "\"small\""};
    instruction_ast_t *ast = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // When evaluating the if instruction
    bool result = ar__instruction_evaluator__evaluate_if(evaluator, ast);
    
    // Then it should succeed and return the correct string
    assert(result == true);
    data_t *result_value = ar__data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar__data__get_type(result_value) == DATA_STRING);
    assert(strcmp(ar__data__get_string(result_value), "medium") == 0);
    
    // Cleanup
    ar__instruction_ast__destroy(ast);
    ar__instruction_evaluator__destroy(evaluator);
    ar__expression_evaluator__destroy(expr_eval);
    ar__data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_if_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    expression_evaluator_t *expr_eval = ar__expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    instruction_evaluator_t *evaluator = ar__instruction_evaluator__create(
        expr_eval, memory, NULL, NULL
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (2 instead of 3)
    const char *args1[] = {"1", "100"};
    instruction_ast_t *ast1 = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    bool result1 = ar__instruction_evaluator__evaluate_if(evaluator, ast1);
    assert(result1 == false);
    
    ar__instruction_ast__destroy(ast1);
    
    // Test case 2: Invalid condition expression
    const char *args2[] = {"invalid expression", "100", "200"};
    instruction_ast_t *ast2 = ar__instruction_ast__create_function_call(
        INST_AST_IF, "if", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    bool result2 = ar__instruction_evaluator__evaluate_if(evaluator, ast2);
    assert(result2 == false);
    
    ar__instruction_ast__destroy(ast2);
    
    // Cleanup
    ar__instruction_evaluator__destroy(evaluator);
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
    
    printf("All instruction_evaluator create/destroy tests passed!\n");
    
    printf("\nStarting instruction_evaluator assignment tests...\n");
    
    test_instruction_evaluator__evaluate_assignment_integer();
    printf("test_instruction_evaluator__evaluate_assignment_integer passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_string();
    printf("test_instruction_evaluator__evaluate_assignment_string passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_nested_path();
    printf("test_instruction_evaluator__evaluate_assignment_nested_path passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_expression();
    printf("test_instruction_evaluator__evaluate_assignment_expression passed!\n");
    
    test_instruction_evaluator__evaluate_assignment_invalid_path();
    printf("test_instruction_evaluator__evaluate_assignment_invalid_path passed!\n");
    
    printf("All instruction_evaluator assignment tests passed!\n");
    
    printf("\nStarting instruction_evaluator send tests...\n");
    
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
    
    printf("All instruction_evaluator send tests passed!\n");
    
    printf("\nStarting instruction_evaluator if tests...\n");
    
    test_instruction_evaluator__evaluate_if_true_condition();
    printf("test_instruction_evaluator__evaluate_if_true_condition passed!\n");
    
    test_instruction_evaluator__evaluate_if_false_condition();
    printf("test_instruction_evaluator__evaluate_if_false_condition passed!\n");
    
    test_instruction_evaluator__evaluate_if_with_expressions();
    printf("test_instruction_evaluator__evaluate_if_with_expressions passed!\n");
    
    test_instruction_evaluator__evaluate_if_nested();
    printf("test_instruction_evaluator__evaluate_if_nested passed!\n");
    
    test_instruction_evaluator__evaluate_if_invalid_args();
    printf("test_instruction_evaluator__evaluate_if_invalid_args passed!\n");
    
    printf("All instruction_evaluator tests passed!\n");
    
    return 0;
}