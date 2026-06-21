#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ar_assert.h"
#include "ar_expression_evaluator.h"
#include "ar_expression_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_condition_instruction_evaluator.h"
#include "ar_log.h"
#include "ar_event.h"
#include "ar_evaluator_fixture.h"
#include "ar_frame.h"

static void test_condition_instruction_evaluator__create_destroy(void) {
    // Given a test fixture
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_create_destroy");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    // When creating a condition instruction evaluator
    ar_condition_instruction_evaluator_t *own_evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    
    // Then it should create successfully
    assert(own_evaluator != NULL);
    
    // When destroying the evaluator
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    
    // Cleanup
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_condition_instruction_evaluator__evaluate_with_instance(void) {
    // Given a test fixture with memory containing a condition value
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_with_instance");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(10)));
    
    ar_condition_instruction_evaluator_t *own_evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(own_evaluator != NULL);
    
    // When creating an if AST node with true condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 5
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *five_ast = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, five_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: 100
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: 200
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_int(200);
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating using the instance with frame
    bool result = ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    
    // Then it should succeed and store the true value
    assert(result == true);
    ar_data_t *ref_result_value = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar_data__get_type(ref_result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_result_value) == 100);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_condition_instruction_evaluator__evaluate_without_legacy(void) {
    // Given a test fixture with memory containing a flag value
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_evaluate_without_legacy");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "flag", ar_data__create_integer(0)));
    
    // When creating a condition instruction evaluator instance
    ar_condition_instruction_evaluator_t *own_evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(own_evaluator != NULL);
    
    // When creating an if AST node with false condition
    const char *args[] = {"memory.flag", "\"yes\"", "\"no\""};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(own_ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.flag
    const char *flag_path[] = {"flag"};
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_memory_access("memory", flag_path, 1);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: "yes"
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_string("yes");
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: "no"
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_string("no");
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(own_ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating using the instance-based interface with frame
    bool result = ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);
    
    // Then it should succeed and store the false value
    assert(result == true);
    ar_data_t *ref_result_value = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result_value != NULL);
    assert(ar_data__get_type(ref_result_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_result_value), "no") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_condition_instruction_evaluator__accepts_empty_list_condition(void) {
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_empty_list_condition");
    assert(own_fixture != NULL);

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_message = ar_data__create_map();
    ar_data_t *own_payloads = ar_data__create_list();
    ar_data_t *own_context = ar_data__create_map();
    assert(own_message != NULL);
    assert(own_payloads != NULL);
    assert(own_context != NULL);
    assert(ar_data__set_map_data(own_message, "payloads", own_payloads));
    own_payloads = NULL;
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    assert(own_frame != NULL);

    ar_condition_instruction_evaluator_t *own_evaluator =
        ar_condition_instruction_evaluator__create(ref_log, ref_expr_eval);
    assert(own_evaluator != NULL);

    const char *args[] = {"message.payloads = []", "1", "0"};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.is_empty"
    );
    assert(own_ast != NULL);

    ar_list_t *own_arg_asts = ar_list__create();
    assert(own_arg_asts != NULL);
    const char *payload_path[] = {"payloads"};
    ar_expression_ast_t *own_left =
        ar_expression_ast__create_memory_access("message", payload_path, 1);
    ar_expression_ast_t *own_right = ar_expression_ast__create_literal_list(NULL, 0);
    ar_expression_ast_t *own_condition =
        ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__EQUAL, own_left, own_right);
    ar_expression_ast_t *own_true = ar_expression_ast__create_literal_int(1);
    ar_expression_ast_t *own_false = ar_expression_ast__create_literal_int(0);
    assert(ar_list__add_last(own_arg_asts, own_condition));
    assert(ar_list__add_last(own_arg_asts, own_true));
    assert(ar_list__add_last(own_arg_asts, own_false));
    assert(ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts));

    bool result =
        ar_condition_instruction_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    assert(result == true);
    assert(ar_data__get_map_integer(mut_memory, "is_empty") == 1);

    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_condition_instruction_evaluator__selected_branch_allows_parse_expression(void) {
    // Given an if instruction whose selected branch is a pure parse() expression
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_if_selected_branch_allows_parse_expression");
    assert(own_fixture != NULL);

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_condition_instruction_evaluator_t *own_evaluator =
        ar_condition_instruction_evaluator__create(ref_log, ref_expr_eval);
    assert(own_evaluator != NULL);

    const char *args[] = {
        "1",
        "parse(\"name={name}\", \"name=Ada\")",
        "parse(\"name={name}\", \"name=Byron\")"
    };
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(own_ast != NULL);

    ar_list_t *own_arg_asts = ar_list__create();
    assert(own_arg_asts != NULL);
    assert(ar_list__add_last(own_arg_asts, ar_expression_ast__create_literal_int(1)));

    ar_expression_parser_t *own_true_parser =
        ar_expression_parser__create(ref_log, "parse(\"name={name}\", \"name=Ada\")");
    assert(own_true_parser != NULL);
    ar_expression_ast_t *own_true_ast =
        ar_expression_parser__parse_expression(own_true_parser);
    assert(own_true_ast != NULL);
    ar_expression_parser__destroy(own_true_parser);
    assert(ar_list__add_last(own_arg_asts, own_true_ast));

    ar_expression_parser_t *own_false_parser =
        ar_expression_parser__create(ref_log, "parse(\"name={name}\", \"name=Byron\")");
    assert(own_false_parser != NULL);
    ar_expression_ast_t *own_false_ast =
        ar_expression_parser__parse_expression(own_false_parser);
    assert(own_false_ast != NULL);
    ar_expression_parser__destroy(own_false_parser);
    assert(ar_list__add_last(own_arg_asts, own_false_ast));

    assert(ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts));

    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    bool result =
        ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    assert(result == true);
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__MAP);
    assert(strcmp(ar_data__get_map_string(ref_result, "name"), "Ada") == 0);

    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_condition_instruction_evaluator__selected_branch_allows_build_expression(void) {
    // Given an if instruction whose selected branch is a pure build() expression
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_if_selected_branch_allows_build_expression");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_condition_instruction_evaluator_t *own_evaluator =
        ar_condition_instruction_evaluator__create(ref_log, ref_expr_eval);
    AR_ASSERT(own_evaluator != NULL, "Condition evaluator creation should succeed");

    const char *ref_args[] = {
        "1",
        "build(\"Hello {name}!\", {name: \"Ada\"})",
        "build(\"Hello {name}!\", {name: \"Byron\"})"
    };
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", ref_args, 3, "memory.result"
    );
    AR_ASSERT(own_ast != NULL, "If instruction AST creation should succeed");

    ar_list_t *own_arg_asts = ar_list__create();
    AR_ASSERT(own_arg_asts != NULL, "Argument AST list creation should succeed");
    AR_ASSERT(ar_list__add_last(own_arg_asts, ar_expression_ast__create_literal_int(1)),
              "Condition AST should be added");

    ar_expression_parser_t *own_true_parser =
        ar_expression_parser__create(ref_log, "build(\"Hello {name}!\", {name: \"Ada\"})");
    AR_ASSERT(own_true_parser != NULL, "True branch parser creation should succeed");
    ar_expression_ast_t *own_true_ast =
        ar_expression_parser__parse_expression(own_true_parser);
    AR_ASSERT(own_true_ast != NULL, "True branch build expression should parse");
    ar_expression_parser__destroy(own_true_parser);
    AR_ASSERT(ar_list__add_last(own_arg_asts, own_true_ast),
              "True branch AST should be added");
    own_true_ast = NULL;

    ar_expression_parser_t *own_false_parser =
        ar_expression_parser__create(ref_log, "build(\"Hello {name}!\", {name: \"Byron\"})");
    AR_ASSERT(own_false_parser != NULL, "False branch parser creation should succeed");
    ar_expression_ast_t *own_false_ast =
        ar_expression_parser__parse_expression(own_false_parser);
    AR_ASSERT(own_false_ast != NULL, "False branch build expression should parse");
    ar_expression_parser__destroy(own_false_parser);
    AR_ASSERT(ar_list__add_last(own_arg_asts, own_false_ast),
              "False branch AST should be added");
    own_false_ast = NULL;

    AR_ASSERT(ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts),
              "If instruction should take ownership of argument ASTs");
    own_arg_asts = NULL;

    // When evaluating the if instruction
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    bool result =
        ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    // Then only the selected build() branch should produce the assigned string
    AR_ASSERT(result == true, "If evaluation should succeed");
    const char *ref_result = ar_data__get_map_string(mut_memory, "result");
    AR_ASSERT(ref_result != NULL, "Selected branch should store a result");
    AR_ASSERT(strcmp(ref_result, "Hello Ada!") == 0,
              "Selected branch should store the true build result");

    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static ar_expression_ast_t *_parse_condition_test_expression(
    ar_log_t *ref_log,
    const char *ref_expression
) {
    ar_expression_parser_t *own_parser =
        ar_expression_parser__create(ref_log, ref_expression);
    AR_ASSERT(own_parser != NULL, "Expression parser creation should succeed");
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    AR_ASSERT(own_ast != NULL, "Expression should parse");
    ar_expression_parser__destroy(own_parser);

    return own_ast; // Ownership transferred to caller
}

static void test_condition_instruction_evaluator__selected_branch_allows_head_tail_expression(void) {
    // Given if instructions whose selected branches are pure head() and tail() expressions
    ar_evaluator_fixture_t *own_fixture =
        ar_evaluator_fixture__create("test_if_selected_branch_allows_head_tail_expression");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    // Given expression evaluation dependencies and writable memory
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval =
        ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    // Given a condition evaluator with a frame
    ar_condition_instruction_evaluator_t *own_evaluator =
        ar_condition_instruction_evaluator__create(ref_log, ref_expr_eval);
    AR_ASSERT(own_evaluator != NULL, "Condition evaluator creation should succeed");
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    // When building an if instruction whose selected branch is head()
    const char *ref_head_args[] = {"1", "head([4, 5])", "0"};
    ar_instruction_ast_t *own_head_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", ref_head_args, 3, "memory.first"
    );
    AR_ASSERT(own_head_ast != NULL, "If head AST creation should succeed");
    ar_list_t *own_head_arg_asts = ar_list__create();
    AR_ASSERT(own_head_arg_asts != NULL, "Head argument AST list creation should succeed");
    AR_ASSERT(ar_list__add_last(own_head_arg_asts, ar_expression_ast__create_literal_int(1)),
              "Head condition AST should be added");
    AR_ASSERT(ar_list__add_last(
        own_head_arg_asts,
        _parse_condition_test_expression(ref_log, "head([4, 5])")
    ), "Head branch AST should be added");
    AR_ASSERT(ar_list__add_last(own_head_arg_asts, ar_expression_ast__create_literal_int(0)),
              "Head false branch AST should be added");
    AR_ASSERT(ar_instruction_ast__set_function_arg_asts(own_head_ast, own_head_arg_asts),
              "Head if instruction should take ownership of argument ASTs");

    // When evaluating the head() branch instruction
    bool result =
        ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_head_ast);

    // Then the selected branch should store the head result
    AR_ASSERT(result == true, "If head evaluation should succeed");
    AR_ASSERT(ar_data__get_map_integer(mut_memory, "first") == 4,
              "Selected head branch should store first item");

    // When building an if instruction whose selected branch is tail()
    const char *ref_tail_args[] = {"0", "0", "tail([5, 6, 7])"};
    ar_instruction_ast_t *own_tail_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", ref_tail_args, 3, "memory.rest"
    );
    AR_ASSERT(own_tail_ast != NULL, "If tail AST creation should succeed");
    ar_list_t *own_tail_arg_asts = ar_list__create();
    AR_ASSERT(own_tail_arg_asts != NULL, "Tail argument AST list creation should succeed");
    AR_ASSERT(ar_list__add_last(own_tail_arg_asts, ar_expression_ast__create_literal_int(0)),
              "Tail condition AST should be added");
    AR_ASSERT(ar_list__add_last(own_tail_arg_asts, ar_expression_ast__create_literal_int(0)),
              "Tail true branch AST should be added");
    AR_ASSERT(ar_list__add_last(
        own_tail_arg_asts,
        _parse_condition_test_expression(ref_log, "tail([5, 6, 7])")
    ), "Tail branch AST should be added");
    AR_ASSERT(ar_instruction_ast__set_function_arg_asts(own_tail_ast, own_tail_arg_asts),
              "Tail if instruction should take ownership of argument ASTs");

    // When evaluating the tail() branch instruction
    result = ar_condition_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_tail_ast);

    // Then the selected branch should store the tail result
    AR_ASSERT(result == true, "If tail evaluation should succeed");
    ar_data_t *ref_rest = ar_data__get_map_data(mut_memory, "rest");
    AR_ASSERT(ref_rest != NULL, "Selected tail branch should store a result");
    AR_ASSERT(ar_data__get_type(ref_rest) == AR_DATA_TYPE__LIST,
              "Selected tail branch should store a list");
    AR_ASSERT(ar_data__list_count(ref_rest) == 2,
              "Selected tail branch should contain remaining items");
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_rest)) == 6,
              "Selected tail branch first item should match");
    AR_ASSERT(ar_data__get_integer(ar_data__list_last(ref_rest)) == 7,
              "Selected tail branch last item should match");

    // Cleanup
    ar_instruction_ast__destroy(own_head_ast);
    ar_instruction_ast__destroy(own_tail_ast);
    ar_condition_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_true_condition(void) {
    // Given a test fixture with memory containing a condition
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_true_condition");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(10)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with true condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 5
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *five_ast = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, five_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: 100
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: 200
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_int(200);
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and store the true value
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 100);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_false_condition(void) {
    // Given a test fixture with memory containing a condition
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_false_condition");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(3)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with false condition
    const char *args[] = {"memory.x > 5", "100", "200"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 5 (will be false since x=3)
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *five_ast = ar_expression_ast__create_literal_int(5);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, five_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: 100
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: 200
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_int(200);
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and store the false value
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 200);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_with_expressions(void) {
    // Given a test fixture with memory containing values
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_with_expressions");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "a", ar_data__create_integer(10)));
    assert(ar_data__set_map_data(mut_memory, "b", ar_data__create_integer(20)));
    assert(ar_data__set_map_data(mut_memory, "flag", ar_data__create_integer(1)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating an if AST node with expression arguments
    const char *args[] = {"memory.flag", "memory.a + memory.b", "memory.a - memory.b"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.flag
    const char *flag_path[] = {"flag"};
    ar_expression_ast_t *flag_ast = ar_expression_ast__create_memory_access("memory", flag_path, 1);
    ar_list__add_last(arg_asts, flag_ast);
    
    // True value: memory.a + memory.b
    const char *a_path[] = {"a"};
    const char *b_path[] = {"b"};
    ar_expression_ast_t *a_ast = ar_expression_ast__create_memory_access("memory", a_path, 1);
    ar_expression_ast_t *b_ast = ar_expression_ast__create_memory_access("memory", b_path, 1);
    ar_expression_ast_t *add_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__ADD, a_ast, b_ast);
    ar_list__add_last(arg_asts, add_ast);
    
    // False value: memory.a - memory.b
    const char *a2_path[] = {"a"};
    const char *b2_path[] = {"b"};
    ar_expression_ast_t *a2_ast = ar_expression_ast__create_memory_access("memory", a2_path, 1);
    ar_expression_ast_t *b2_ast = ar_expression_ast__create_memory_access("memory", b2_path, 1);
    ar_expression_ast_t *sub_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__SUBTRACT, a2_ast, b2_ast);
    ar_list__add_last(arg_asts, sub_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and evaluate the true expression
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(result_value) == 30);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_nested(void) {
    // Given a test fixture with memory
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_nested");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    assert(ar_data__set_map_data(mut_memory, "x", ar_data__create_integer(15)));
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // When creating a nested if expression (simulated as string)
    // if(x > 10, if(x > 20, "large", "medium"), "small")
    // Since we can't nest function calls, we'll test with simple return value
    const char *args[] = {"memory.x > 10", "\"medium\"", "\"small\""};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    assert(ast != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts = ar_list__create();
    assert(arg_asts != NULL);
    
    // Condition: memory.x > 10
    const char *x_path[] = {"x"};
    ar_expression_ast_t *x_ast = ar_expression_ast__create_memory_access("memory", x_path, 1);
    ar_expression_ast_t *ten_ast = ar_expression_ast__create_literal_int(10);
    ar_expression_ast_t *cond_ast = ar_expression_ast__create_binary_op(AR_BINARY_OPERATOR__GREATER, x_ast, ten_ast);
    ar_list__add_last(arg_asts, cond_ast);
    
    // True value: "medium"
    ar_expression_ast_t *true_ast = ar_expression_ast__create_literal_string("medium");
    ar_list__add_last(arg_asts, true_ast);
    
    // False value: "small"
    ar_expression_ast_t *false_ast = ar_expression_ast__create_literal_string("small");
    ar_list__add_last(arg_asts, false_ast);
    
    bool ast_set = ar_instruction_ast__set_function_arg_asts(ast, arg_asts);
    assert(ast_set == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    // When evaluating the if instruction with frame
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);
    
    // Then it should succeed and return the correct string
    assert(result == true);
    ar_data_t *result_value = ar_data__get_map_data(mut_memory, "result");
    assert(result_value != NULL);
    assert(ar_data__get_type(result_value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(result_value), "medium") == 0);
    
    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_deep_copies_selected_branch(void) {
    // Given an if(...) evaluator with a selected nested branch in memory
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_deep_copy_branch");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_data_t *own_selected = ar_data__create_map();
    ar_data_t *own_selected_items = ar_data__create_list();
    AR_ASSERT(own_selected != NULL, "Selected map should be created");
    AR_ASSERT(own_selected_items != NULL, "Selected nested list should be created");
    AR_ASSERT(ar_data__list_add_last_string(own_selected_items, "selected"), "Selected nested item should be stored");
    AR_ASSERT(ar_data__set_map_data(own_selected, "items", own_selected_items), "Selected nested list should be stored");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "selected", own_selected), "Selected branch should be stored");

    ar_data_t *own_fallback = ar_data__create_map();
    AR_ASSERT(own_fallback != NULL, "Fallback map should be created");
    AR_ASSERT(ar_data__set_map_string(own_fallback, "unused", "fallback"), "Fallback field should be stored");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "fallback", own_fallback), "Fallback branch should be stored");

    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    AR_ASSERT(evaluator != NULL, "Condition evaluator creation should succeed");

    const char *args[] = {"1", "memory.selected", "memory.fallback"};
    ar_instruction_ast_t *ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args, 3, "memory.result"
    );
    AR_ASSERT(ast != NULL, "If AST creation should succeed");

    ar_list_t *arg_asts = ar_list__create();
    AR_ASSERT(arg_asts != NULL, "Argument AST list should be created");
    ar_list__add_last(arg_asts, ar_expression_ast__create_literal_int(1));
    const char *selected_path[] = {"selected"};
    ar_list__add_last(arg_asts, ar_expression_ast__create_memory_access("memory", selected_path, 1));
    const char *fallback_path[] = {"fallback"};
    ar_list__add_last(arg_asts, ar_expression_ast__create_memory_access("memory", fallback_path, 1));
    AR_ASSERT(ar_instruction_ast__set_function_arg_asts(ast, arg_asts) == true, "Argument ASTs should be attached");

    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    // When evaluating the if(...) instruction
    bool result = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast);

    // Then the selected branch should be stored as an independent deep copy
    AR_ASSERT(result == true, "If evaluation should succeed");
    ar_data_t *ref_source = ar_data__get_map_data(mut_memory, "selected");
    ar_data_t *ref_copy = ar_data__get_map_data(mut_memory, "result");
    AR_ASSERT(ref_copy != NULL, "Selected branch copy should be stored");
    AR_ASSERT(ref_copy != ref_source, "Selected branch copy should be independent");
    ar_data_t *ref_source_items = ar_data__get_map_data(ref_source, "items");
    ar_data_t *ref_copy_items = ar_data__get_map_data(ref_copy, "items");
    AR_ASSERT(ref_copy_items != NULL, "Copied nested list should exist");
    AR_ASSERT(ref_copy_items != ref_source_items, "Nested list should be deep-copied");
    AR_ASSERT(ar_data__list_count(ref_copy_items) == 1, "Copied nested list count should match source");
    AR_ASSERT(ar_data__list_add_last_string(ref_source_items, "source-only"), "Source nested list should mutate");
    AR_ASSERT(ar_data__list_count(ref_source_items) == 2, "Source nested list should grow");
    AR_ASSERT(ar_data__list_count(ref_copy_items) == 1, "Copied nested list should remain independent");

    // Cleanup
    ar_instruction_ast__destroy(ast);
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_instruction_evaluator__evaluate_if_invalid_args(void) {
    // Given a test fixture with memory
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("test_if_invalid_args");
    assert(own_fixture != NULL);
    
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(own_fixture);
    
    ar_condition_instruction_evaluator_t *evaluator = ar_condition_instruction_evaluator__create(
        ref_log, ref_expr_eval
    );
    assert(evaluator != NULL);
    
    // Test case 1: Wrong number of arguments (2 instead of 3)
    const char *args1[] = {"1", "100"};
    ar_instruction_ast_t *ast1 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args1, 2, NULL
    );
    assert(ast1 != NULL);
    
    // Create and attach the expression ASTs for arguments
    ar_list_t *arg_asts1 = ar_list__create();
    assert(arg_asts1 != NULL);
    
    // Only 2 arguments instead of required 3
    ar_expression_ast_t *one_ast = ar_expression_ast__create_literal_int(1);
    ar_list__add_last(arg_asts1, one_ast);
    
    ar_expression_ast_t *hundred_ast = ar_expression_ast__create_literal_int(100);
    ar_list__add_last(arg_asts1, hundred_ast);
    
    bool ast_set1 = ar_instruction_ast__set_function_arg_asts(ast1, arg_asts1);
    assert(ast_set1 == true);
    
    // Create a frame for evaluation
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    
    bool result1 = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast1);
    assert(result1 == false);
    
    ar_instruction_ast__destroy(ast1);
    
    // Test case 2: Invalid condition expression (NULL AST)
    const char *args2[] = {"invalid expression", "100", "200"};
    ar_instruction_ast_t *ast2 = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__IF, "if", args2, 3, NULL
    );
    assert(ast2 != NULL);
    
    // Don't attach any ASTs to simulate parsing failure
    
    bool result2 = ar_condition_instruction_evaluator__evaluate(evaluator, ref_frame, ast2);
    assert(result2 == false);
    
    ar_instruction_ast__destroy(ast2);
    
    // Cleanup
    ar_condition_instruction_evaluator__destroy(evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    printf("Starting condition instruction_evaluator tests...\n");
    
    test_condition_instruction_evaluator__create_destroy();
    printf("test_condition_instruction_evaluator__create_destroy passed!\n");
    
    test_condition_instruction_evaluator__evaluate_with_instance();
    printf("test_condition_instruction_evaluator__evaluate_with_instance passed!\n");
    
    test_condition_instruction_evaluator__evaluate_without_legacy();
    printf("test_condition_instruction_evaluator__evaluate_without_legacy passed!\n");

    test_condition_instruction_evaluator__accepts_empty_list_condition();
    printf("test_condition_instruction_evaluator__accepts_empty_list_condition passed!\n");

    test_condition_instruction_evaluator__selected_branch_allows_parse_expression();
    printf("test_condition_instruction_evaluator__selected_branch_allows_parse_expression passed!\n");

    test_condition_instruction_evaluator__selected_branch_allows_build_expression();
    printf("test_condition_instruction_evaluator__selected_branch_allows_build_expression passed!\n");

    test_condition_instruction_evaluator__selected_branch_allows_head_tail_expression();
    printf("test_condition_instruction_evaluator__selected_branch_allows_head_tail_expression passed!\n");

    test_instruction_evaluator__evaluate_if_true_condition();
    printf("test_instruction_evaluator__evaluate_if_true_condition passed!\n");
    
    test_instruction_evaluator__evaluate_if_false_condition();
    printf("test_instruction_evaluator__evaluate_if_false_condition passed!\n");
    
    test_instruction_evaluator__evaluate_if_with_expressions();
    printf("test_instruction_evaluator__evaluate_if_with_expressions passed!\n");
    
    test_instruction_evaluator__evaluate_if_nested();
    printf("test_instruction_evaluator__evaluate_if_nested passed!\n");

    test_instruction_evaluator__evaluate_if_deep_copies_selected_branch();
    printf("test_instruction_evaluator__evaluate_if_deep_copies_selected_branch passed!\n");
    
    test_instruction_evaluator__evaluate_if_invalid_args();
    printf("test_instruction_evaluator__evaluate_if_invalid_args passed!\n");
    
    printf("All condition instruction_evaluator tests passed!\n");
    
    return 0;
}
