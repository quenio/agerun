/**
 * @file ar_method_evaluator_tests.c
 * @brief Tests for the method evaluator module
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "ar_method_evaluator.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_instruction_evaluator.h"
#include "ar_expression_evaluator.h"
#include "ar_data.h"
#include "ar_frame.h"
#include "ar_method_ast.h"
#include "ar_expression_ast.h"
#include "ar_instruction_ast.h"

static void test_method_evaluator__create_destroy(void) {
    printf("Testing method evaluator create/destroy...\n");
    
    // Given a log and instruction evaluator
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    
    // Create expression evaluator first (needed by instruction evaluator)
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_log, own_memory, own_context);
    assert(own_expr_eval != NULL);
    
    // Create instruction evaluator
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_instruction_evaluator_t *own_instr_eval = ar_instruction_evaluator__create(
        own_log, own_expr_eval, own_memory, own_context, own_message
    );
    assert(own_instr_eval != NULL);
    
    // When creating a method evaluator
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, own_instr_eval);
    
    // Then it should be created successfully
    assert(own_evaluator != NULL);
    
    // Cleanup
    ar_method_evaluator__destroy(own_evaluator);
    ar_instruction_evaluator__destroy(own_instr_eval);
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    
    printf("  ✓ Method evaluator created and destroyed successfully\n");
}

static void test_method_evaluator__evaluate_empty_method(void) {
    printf("Testing method evaluator with empty method...\n");
    
    // Given a method evaluator and frame
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_log, own_memory, own_context);
    ar_instruction_evaluator_t *own_instr_eval = ar_instruction_evaluator__create(
        own_log, own_expr_eval, own_memory, own_context, own_message
    );
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, own_instr_eval);
    
    // Create a frame
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    assert(own_frame != NULL);
    
    // Create an empty method AST
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    // Verify the method is actually empty
    assert(ar_method_ast__get_instruction_count(own_ast) == 0);
    
    // When evaluating the empty method
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should succeed (empty method has no instructions to fail)
    assert(result == true);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_instruction_evaluator__destroy(own_instr_eval);
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    
    printf("  ✓ Empty method evaluated successfully\n");
}

static void test_method_evaluator__evaluate_single_instruction_method(void) {
    printf("Testing method evaluator with single instruction...\n");
    
    // Given a method evaluator
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_log, own_memory, own_context);
    ar_instruction_evaluator_t *own_instr_eval = ar_instruction_evaluator__create(
        own_log, own_expr_eval, own_memory, own_context, own_message
    );
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, own_instr_eval);
    
    // Create a frame
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Create a method AST with one instruction
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // Create a proper assignment instruction with expression AST
    ar_instruction_ast_t *own_instr = ar_instruction_ast__create_assignment("memory.x", "42");
    ar_expression_ast_t *own_expr = ar_expression_ast__create_literal_int(42);
    ar_instruction_ast__set_assignment_expression_ast(own_instr, own_expr);
    ar_method_ast__add_instruction(own_ast, own_instr);
    
    // Verify the method has one instruction
    assert(ar_method_ast__get_instruction_count(own_ast) == 1);
    
    // When evaluating the method with one instruction
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the value should be stored in memory
    assert(ar_data__get_map_integer(own_memory, "x") == 42);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_instruction_evaluator__destroy(own_instr_eval);
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    
    printf("  ✓ Single instruction method evaluated successfully\n");
}

static void test_method_evaluator__evaluate_multiple_instructions(void) {
    printf("Testing method evaluator with multiple instructions...\n");
    
    // Given a method evaluator and frame
    ar_log_t *own_log = ar_log__create();
    ar_data_t *own_memory = ar_data__create_map();
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_string("test message");
    ar_expression_evaluator_t *own_expr_eval = ar_expression_evaluator__create(own_log, own_memory, own_context);
    ar_instruction_evaluator_t *own_instr_eval = ar_instruction_evaluator__create(
        own_log, own_expr_eval, own_memory, own_context, own_message
    );
    ar_method_evaluator_t *own_evaluator = ar_method_evaluator__create(own_log, own_instr_eval);
    
    // Create a frame
    ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
    
    // Create a method AST with three instructions
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // First instruction: memory.x := 10
    ar_instruction_ast_t *own_instr1 = ar_instruction_ast__create_assignment("memory.x", "10");
    ar_expression_ast_t *own_expr1 = ar_expression_ast__create_literal_int(10);
    ar_instruction_ast__set_assignment_expression_ast(own_instr1, own_expr1);
    ar_method_ast__add_instruction(own_ast, own_instr1);
    
    // Second instruction: memory.y := 20
    ar_instruction_ast_t *own_instr2 = ar_instruction_ast__create_assignment("memory.y", "20");
    ar_expression_ast_t *own_expr2 = ar_expression_ast__create_literal_int(20);
    ar_instruction_ast__set_assignment_expression_ast(own_instr2, own_expr2);
    ar_method_ast__add_instruction(own_ast, own_instr2);
    
    // Third instruction: memory.z := 30
    ar_instruction_ast_t *own_instr3 = ar_instruction_ast__create_assignment("memory.z", "30");
    ar_expression_ast_t *own_expr3 = ar_expression_ast__create_literal_int(30);
    ar_instruction_ast__set_assignment_expression_ast(own_instr3, own_expr3);
    ar_method_ast__add_instruction(own_ast, own_instr3);
    
    // Verify the method has three instructions
    assert(ar_method_ast__get_instruction_count(own_ast) == 3);
    
    // When evaluating the method with multiple instructions
    bool result = ar_method_evaluator__evaluate(own_evaluator, own_frame, own_ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And all values should be stored in memory
    assert(ar_data__get_map_integer(own_memory, "x") == 10);
    assert(ar_data__get_map_integer(own_memory, "y") == 20);
    assert(ar_data__get_map_integer(own_memory, "z") == 30);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_method_evaluator__destroy(own_evaluator);
    ar_instruction_evaluator__destroy(own_instr_eval);
    ar_expression_evaluator__destroy(own_expr_eval);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_data__destroy(own_memory);
    ar_log__destroy(own_log);
    
    printf("  ✓ Multiple instruction method evaluated successfully\n");
}

int main(void) {
    // Directory check - must be run from bin/*-tests/
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        const char *bin_pos = strstr(cwd, "/bin/");
        if (!bin_pos || !strstr(bin_pos + 5, "-tests")) {
            fprintf(stderr, "ERROR: Tests must be run from a bin/*-tests directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Tests are automatically run by make from the correct directory.\n");
            return 1;
        }
    }
    
    printf("Running method evaluator tests...\n");
    
    test_method_evaluator__create_destroy();
    test_method_evaluator__evaluate_empty_method();
    test_method_evaluator__evaluate_single_instruction_method();
    test_method_evaluator__evaluate_multiple_instructions();
    
    // Check for memory leaks
    ar_heap__memory_report();
    
    printf("All 4 tests passed!\n");
    return 0;
}