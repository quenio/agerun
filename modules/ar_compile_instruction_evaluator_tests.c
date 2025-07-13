#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "ar_compile_instruction_evaluator.h"
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
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"

static void test_compile_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_compile_instruction_evaluator__create_destroy");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    // When creating a compile instruction evaluator
    ar_compile_instruction_evaluator_t *evaluator = ar_compile_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    
    // Then it should be created successfully
    assert(evaluator != NULL);
    
    // When destroying the evaluator
    ar_compile_instruction_evaluator__destroy(evaluator);
    
    // Then cleanup fixture
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_compile_instruction_evaluator__evaluate_with_instance(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_compile_instruction_evaluator__evaluate_with_instance");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When creating a compile instruction evaluator instance
    ar_compile_instruction_evaluator_t *evaluator = ar_compile_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a method AST node
    const char *args[] = {"\"test_method\"", "\"send(0, 42)\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "test_method"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("test_method");
    ar_list__add_last(arg_asts, name_ast);
    
    // Instructions: "send(0, 42)"
    ar_expression_ast_t *instructions_ast = ar_expression_ast__create_literal_string("send(0, 42)");
    ar_list__add_last(arg_asts, instructions_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance
    bool result = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_compile_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
    
    // Clean up the method we registered
    ar_methodology__cleanup();
}

static void test_compile_instruction_evaluator__evaluate_legacy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_compile_instruction_evaluator__evaluate_legacy");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // Create an evaluator instance
    ar_compile_instruction_evaluator_t *evaluator = ar_compile_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a method AST node with result assignment
    const char *args[] = {"\"legacy_test\"", "\"send(0, 99)\"", "\"2.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "legacy_test"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("legacy_test");
    ar_list__add_last(arg_asts, name_ast);
    
    // Instructions: "send(0, 99)"
    ar_expression_ast_t *instructions_ast = ar_expression_ast__create_literal_string("send(0, 99)");
    ar_list__add_last(arg_asts, instructions_ast);
    
    // Version: "2.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("2.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // When evaluating using the instance-based interface
    bool result = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed
    assert(result == true);
    
    // And the result should be stored in memory
    int created = ar_data__get_map_integer(mut_memory, "result");
    assert(created == 1);  // true is stored as 1
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_compile_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
    
    // Clean up the method we registered
    ar_methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_simple(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_instruction_evaluator__evaluate_method_simple");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    ar_compile_instruction_evaluator_t *evaluator = ar_compile_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When evaluating a compile instruction: compile("counter", "send(message.sender, memory.count + 1)", "1.0.0")
    const char *args[] = {"\"counter\"", "\"send(message.sender, memory.count + 1)\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "counter"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("counter");
    ar_list__add_last(arg_asts, name_ast);
    
    // Instructions: "send(message.sender, memory.count + 1)"
    ar_expression_ast_t *instructions_ast = ar_expression_ast__create_literal_string("send(message.sender, memory.count + 1)");
    ar_list__add_last(arg_asts, instructions_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should return true (method creation successful)
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_compile_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
    
    // Clean up the method we registered
    ar_methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_with_result(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_instruction_evaluator__evaluate_method_with_result");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    ar_compile_instruction_evaluator_t *evaluator = ar_compile_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When evaluating a compile instruction with result assignment: memory.created := compile("echo", "send(message.sender, message.content)", "2.0.0")
    const char *args[] = {"\"echo\"", "\"send(message.sender, message.content)\"", "\"2.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args, 3, "memory.created"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "echo"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("echo");
    ar_list__add_last(arg_asts, name_ast);
    
    // Instructions: "send(message.sender, message.content)"
    ar_expression_ast_t *instructions_ast = ar_expression_ast__create_literal_string("send(message.sender, message.content)");
    ar_list__add_last(arg_asts, instructions_ast);
    
    // Version: "2.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("2.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should return true
    assert(result == true);
    
    // And the result should be stored in memory
    int created = ar_data__get_map_integer(mut_memory, "created");
    assert(created == 1);  // true is stored as 1
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_compile_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
    
    // Clean up the method we registered
    ar_methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_invalid_instructions(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_instruction_evaluator__evaluate_method_invalid_instructions");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    ar_compile_instruction_evaluator_t *evaluator = ar_compile_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When evaluating a compile instruction with invalid instructions: compile("bad", "invalid syntax here", "1.0.0")
    const char *args[] = {"\"bad\"", "\"invalid syntax here\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args, 3, NULL
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Method name: "bad"
    ar_expression_ast_t *name_ast = ar_expression_ast__create_literal_string("bad");
    ar_list__add_last(arg_asts, name_ast);
    
    // Instructions: "invalid syntax here"
    ar_expression_ast_t *instructions_ast = ar_expression_ast__create_literal_string("invalid syntax here");
    ar_list__add_last(arg_asts, instructions_ast);
    
    // Version: "1.0.0"
    ar_expression_ast_t *version_ast = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts, version_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    bool result = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should return true (method creation succeeds even with invalid instructions)
    // The validation happens when the method is actually executed
    assert(result == true);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_compile_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
    
    // Clean up the method we registered
    ar_methodology__cleanup();
}

static void test_instruction_evaluator__evaluate_method_invalid_args(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_instruction_evaluator__evaluate_method_invalid_args");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    ar_compile_instruction_evaluator_t *evaluator = ar_compile_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments
    const char *args1[] = {"\"test\"", "\"send(0, 42)\""};  // Missing version
    ar_instruction_ast_t *ast1 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    // Create and attach expression ASTs - only 2 arguments (should fail)
    ar_list_t *arg_asts1 = ar_list__create();
    assert(arg_asts1 != NULL);
    
    ar_expression_ast_t *name_ast1 = ar_expression_ast__create_literal_string("test");
    ar_list__add_last(arg_asts1, name_ast1);
    
    ar_expression_ast_t *instructions_ast1 = ar_expression_ast__create_literal_string("send(0, 42)");
    ar_list__add_last(arg_asts1, instructions_ast1);
    
    bool ast_set1 = ar_instruction_ast__set_function_arg_asts(ast1, arg_asts1);
    assert(ast_set1 == true);
    
    bool result1 = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast1);
    assert(result1 == false);
    
    ar_instruction_ast__destroy(ast1);
    
    // Test case 2: Non-string method name
    const char *args2[] = {"42", "\"send(0, 42)\"", "\"1.0.0\""};
    ar_instruction_ast_t *ast2 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    // Create and attach expression ASTs - first is integer, others are strings
    ar_list_t *arg_asts2 = ar_list__create();
    assert(arg_asts2 != NULL);
    
    ar_expression_ast_t *name_ast2 = ar_expression_ast__create_literal_int(42);
    ar_list__add_last(arg_asts2, name_ast2);
    
    ar_expression_ast_t *instructions_ast2 = ar_expression_ast__create_literal_string("send(0, 42)");
    ar_list__add_last(arg_asts2, instructions_ast2);
    
    ar_expression_ast_t *version_ast2 = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts2, version_ast2);
    
    bool ast_set2 = ar_instruction_ast__set_function_arg_asts(ast2, arg_asts2);
    assert(ast_set2 == true);
    
    bool result2 = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast2);
    assert(result2 == false);
    
    ar_instruction_ast__destroy(ast2);
    
    // Test case 3: Non-string instructions
    const char *args3[] = {"\"test\"", "42", "\"1.0.0\""};
    ar_instruction_ast_t *ast3 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args3, 3, NULL
    );
    assert(ast3 != NULL);
    
    // Create and attach expression ASTs - second is integer, others are strings
    ar_list_t *arg_asts3 = ar_list__create();
    assert(arg_asts3 != NULL);
    
    ar_expression_ast_t *name_ast3 = ar_expression_ast__create_literal_string("test");
    ar_list__add_last(arg_asts3, name_ast3);
    
    ar_expression_ast_t *instructions_ast3 = ar_expression_ast__create_literal_int(42);
    ar_list__add_last(arg_asts3, instructions_ast3);
    
    ar_expression_ast_t *version_ast3 = ar_expression_ast__create_literal_string("1.0.0");
    ar_list__add_last(arg_asts3, version_ast3);
    
    bool ast_set3 = ar_instruction_ast__set_function_arg_asts(ast3, arg_asts3);
    assert(ast_set3 == true);
    
    bool result3 = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast3);
    assert(result3 == false);
    
    ar_instruction_ast__destroy(ast3);
    
    // Test case 4: Non-string version
    const char *args4[] = {"\"test\"", "\"send(0, 42)\"", "1.0"};
    ar_instruction_ast_t *ast4 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__COMPILE, "method", args4, 3, NULL
    );
    assert(ast4 != NULL);
    
    // Create and attach expression ASTs - third is double, others are strings
    ar_list_t *arg_asts4 = ar_list__create();
    assert(arg_asts4 != NULL);
    
    ar_expression_ast_t *name_ast4 = ar_expression_ast__create_literal_string("test");
    ar_list__add_last(arg_asts4, name_ast4);
    
    ar_expression_ast_t *instructions_ast4 = ar_expression_ast__create_literal_string("send(0, 42)");
    ar_list__add_last(arg_asts4, instructions_ast4);
    
    ar_expression_ast_t *version_ast4 = ar_expression_ast__create_literal_double(1.0);
    ar_list__add_last(arg_asts4, version_ast4);
    
    bool ast_set4 = ar_instruction_ast__set_function_arg_asts(ast4, arg_asts4);
    assert(ast_set4 == true);
    
    bool result4 = ar_compile_instruction_evaluator__evaluate(evaluator, ref_frame, ast4);
    assert(result4 == false);
    
    ar_instruction_ast__destroy(ast4);
    
    // Cleanup
    ar_compile_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
    
    // Clean up any methods that might have been registered
    ar_methodology__cleanup();
}

int main(void) {
    printf("Starting compile instruction evaluator tests...\n");
    
    // Check if running from bin directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run from a bin directory\n");
            return 1;
        }
    }
    
    // Clean up any existing state at the start
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    test_compile_instruction_evaluator__create_destroy();
    printf("test_compile_instruction_evaluator__create_destroy passed!\n");
    
    test_compile_instruction_evaluator__evaluate_with_instance();
    printf("test_compile_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_compile_instruction_evaluator__evaluate_legacy();
    printf("test_compile_instruction_evaluator__evaluate_legacy passed!\n");
    
    test_instruction_evaluator__evaluate_method_simple();
    printf("test_instruction_evaluator__evaluate_method_simple passed!\n");
    
    test_instruction_evaluator__evaluate_method_with_result();
    printf("test_instruction_evaluator__evaluate_method_with_result passed!\n");
    
    test_instruction_evaluator__evaluate_method_invalid_instructions();
    printf("test_instruction_evaluator__evaluate_method_invalid_instructions passed!\n");
    
    test_instruction_evaluator__evaluate_method_invalid_args();
    printf("test_instruction_evaluator__evaluate_method_invalid_args passed!\n");
    
    printf("All compile instruction evaluator tests passed!\n");
    
    // Clean up after tests
    ar_methodology__cleanup();
    
    return 0;
}
