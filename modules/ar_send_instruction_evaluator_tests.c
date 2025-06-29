#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ar_send_instruction_evaluator.h"
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"

static void test_send_instruction_evaluator__create_destroy(void) {
    // Given memory and expression evaluator
    data_t *own_memory = ar_data__create_map();
    assert(own_memory != NULL);
    
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating a send instruction evaluator
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_instruction_evaluator__create(
        own_expr_eval, own_memory
    );
    
    // Then it should succeed
    assert(own_evaluator != NULL);
    
    // Cleanup
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_memory);
}

static void test_send_instruction_evaluator__evaluate_with_instance(void) {
    // Given memory and expression evaluator
    data_t *own_memory = ar_data__create_map();
    assert(own_memory != NULL);
    
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_memory, NULL);
    assert(own_expr_eval != NULL);
    
    // When creating a send instruction evaluator
    ar_send_instruction_evaluator_t *own_evaluator = ar_send_instruction_evaluator__create(
        own_expr_eval, own_memory
    );
    assert(own_evaluator != NULL);
    
    // When creating a send AST node for "send(0, 42)"
    const char *args[] = {"0", "42"};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INST__SEND, "send", args, 2, NULL
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *own_arg_asts = ar_list__create();
    assert(own_arg_asts != NULL);
    
    ar_expression_ast_t *own_agent_ast = ar_expression_ast__create_literal_int(0);
    assert(own_agent_ast != NULL);
    ar_list__add_last(own_arg_asts, own_agent_ast);
    
    ar_expression_ast_t *own_msg_ast = ar_expression_ast__create_literal_int(42);
    assert(own_msg_ast != NULL);
    ar_list__add_last(own_arg_asts, own_msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(own_evaluator, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_send_instruction_evaluator__destroy(own_evaluator);
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_memory);
}

static void test_instruction_evaluator__evaluate_send_integer_message(void) {
    // Given an evaluator with memory and agency mock capability
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, 42)"
    const char *args[] = {"0", "42"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_int(42);
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed (send to agent 0 is a no-op that returns true)
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_string_message(void) {
    // Given an evaluator with memory
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, \"hello\")"
    const char *args[] = {"0", "\"hello\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("hello");
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_with_result(void) {
    // Given an evaluator with memory
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "memory.result := send(0, \"test\")"
    const char *args[] = {"0", "\"test\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__SEND, "send", args, 2, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_literal_string("test");
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be stored in memory (send returns true for agent 0)
    data_t *result_value = ar_data__get_map_data(memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == DATA_INTEGER);
    assert(ar_data__get_integer(result_value) == 1); // true as integer
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_memory_reference(void) {
    // Given an evaluator with memory containing a message
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    data_t *msg_value = ar_data__create_string("Hello from memory");
    ar_data__set_map_data(memory, "msg", msg_value);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node for "send(0, memory.msg)"
    const char *args[] = {"0", "memory.msg"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__SEND, "send", args, 2, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    const char *msg_path[] = {"msg"};
    ar_expression_ast_t *msg_ast = ar_expression_ast__create_memory_access("memory", msg_path, 1);
    assert(msg_ast != NULL);
    ar_list__add_last(arg_asts, msg_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
}

static void test_instruction_evaluator__evaluate_send_invalid_args(void) {
    // Given an evaluator with memory
    data_t *memory = ar_data__create_map();
    assert(memory != NULL);
    
    ar_expression_evaluator_t *expr_eval = ar_expression_evaluator__create(memory, NULL);
    assert(expr_eval != NULL);
    
    ar_send_instruction_evaluator_t *evaluator = ar_send_instruction_evaluator__create(
        expr_eval, memory
    );
    assert(evaluator != NULL);
    
    // When creating a send AST node with only one argument
    const char *args[] = {"0"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INST__SEND, "send", args, 1, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments (only one arg)
    list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    ar_expression_ast_t *agent_ast = ar_expression_ast__create_literal_int(0);
    assert(agent_ast != NULL);
    ar_list__add_last(arg_asts, agent_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating the send
    bool result = ar_send_instruction_evaluator__evaluate(evaluator, ast);
    
    // Then it should fail (send requires 2 arguments)
    assert(result == false);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_send_instruction_evaluator__destroy(evaluator);
    ar_expression_evaluator__destroy(expr_eval);
    ar_data__destroy(memory);
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