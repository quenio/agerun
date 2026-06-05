#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ar_append_instruction_evaluator.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_evaluator_fixture.h"
#include "ar_expression_ast.h"
#include "ar_frame.h"
#include "ar_instruction_ast.h"
#include "ar_list.h"
#include "ar_log.h"

static ar_instruction_ast_t* _create_append_ast(
    const char *ref_target_arg,
    const char *ref_value_arg,
    const char *ref_result_path,
    ar_expression_ast_t *own_target_ast,
    ar_expression_ast_t *own_value_ast
) {
    const char *args[] = {ref_target_arg, ref_value_arg};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__APPEND,
        "append",
        args,
        2,
        ref_result_path
    );
    AR_ASSERT(own_ast != NULL, "Append AST creation should succeed");

    ar_list_t *own_arg_asts = ar_list__create();
    AR_ASSERT(own_arg_asts != NULL, "Argument AST list creation should succeed");
    AR_ASSERT(ar_list__add_last(own_arg_asts, own_target_ast), "Target AST should be added");
    AR_ASSERT(ar_list__add_last(own_arg_asts, own_value_ast), "Value AST should be added");
    AR_ASSERT(
        ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts),
        "Argument ASTs should be attached"
    );

    return own_ast; // Ownership transferred to caller
}

static ar_append_instruction_evaluator_t* _create_evaluator(ar_evaluator_fixture_t *ref_fixture) {
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(ref_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(ref_fixture);
    ar_append_instruction_evaluator_t *own_evaluator = ar_append_instruction_evaluator__create(
        ref_log,
        ref_expr_eval
    );
    AR_ASSERT(own_evaluator != NULL, "Append evaluator creation should succeed");
    return own_evaluator; // Ownership transferred to caller
}

static void test_append_instruction_evaluator__create_destroy(void) {
    printf("Testing append instruction evaluator create/destroy...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_create_destroy");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);

    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__appends_literal_to_memory_list(void) {
    printf("Testing append literal to memory list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_literal");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_results = ar_data__create_list();
    AR_ASSERT(own_results != NULL, "Results list creation should succeed");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", own_results), "Results list should be stored");

    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "42",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Append should succeed");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    AR_ASSERT(ref_results != NULL, "Results list should still exist");
    AR_ASSERT(ar_data__list_count(ref_results) == 1, "Results list should contain one item");
    ar_data_t *ref_first = ar_data__list_first(ref_results);
    AR_ASSERT(ar_data__get_type(ref_first) == AR_DATA_TYPE__INTEGER, "Appended item should be integer");
    AR_ASSERT(ar_data__get_integer(ref_first) == 42, "Appended item should match literal");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__copies_borrowed_memory_value(void) {
    printf("Testing append copies borrowed memory value...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_borrowed_memory");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", ar_data__create_list()), "Results list should be stored");
    AR_ASSERT(ar_data__set_map_string(mut_memory, "value", "borrowed"), "Borrowed value should be stored");

    const char *target_path[] = {"results"};
    const char *value_path[] = {"value"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "memory.value",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_memory_access("memory", value_path, 1)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Append should succeed");
    ar_data_t *ref_original = ar_data__get_map_data(mut_memory, "value");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    ar_data_t *ref_appended = ar_data__list_first(ref_results);
    AR_ASSERT(ref_original != NULL, "Original memory value should remain");
    AR_ASSERT(ref_appended != NULL, "Appended value should exist");
    AR_ASSERT(ref_original != ref_appended, "Borrowed memory value should be copied before append");
    AR_ASSERT(strcmp(ar_data__get_string(ref_original), "borrowed") == 0, "Original value should remain unchanged");
    AR_ASSERT(strcmp(ar_data__get_string(ref_appended), "borrowed") == 0, "Appended copy should match");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__copies_borrowed_message_value(void) {
    printf("Testing append copies borrowed message value...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_borrowed_message");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", ar_data__create_list()), "Results list should be stored");
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Context creation should succeed");
    AR_ASSERT(own_message != NULL, "Message creation should succeed");
    AR_ASSERT(ar_data__set_map_string(own_message, "value", "message-value"), "Message value should be stored");
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    AR_ASSERT(own_frame != NULL, "Frame creation should succeed");

    const char *target_path[] = {"results"};
    const char *value_path[] = {"value"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "message.value",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_memory_access("message", value_path, 1)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    AR_ASSERT(result == true, "Append should succeed");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    ar_data_t *ref_appended = ar_data__list_first(ref_results);
    AR_ASSERT(strcmp(ar_data__get_string(ref_appended), "message-value") == 0, "Appended copy should match");
    AR_ASSERT(ar_data__get_map_data(own_message, "value") != ref_appended, "Message value should be copied");

    ar_instruction_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__transfers_owned_list_literal(void) {
    printf("Testing append transfers owned list literal...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_owned_literal");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", ar_data__create_list()), "Results list should be stored");

    ar_expression_ast_t *own_items[2];
    own_items[0] = ar_expression_ast__create_literal_int(7);
    own_items[1] = ar_expression_ast__create_literal_int(8);
    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "[7, 8]",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_list(own_items, 2)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Append should succeed");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    ar_data_t *ref_appended = ar_data__list_first(ref_results);
    AR_ASSERT(ar_data__get_type(ref_appended) == AR_DATA_TYPE__LIST, "Appended value should be a list");
    AR_ASSERT(ar_data__list_count(ref_appended) == 2, "Appended list should keep its items");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__rejects_message_target(void) {
    printf("Testing append rejects message target at evaluation...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_message_target");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "message.results",
        "42",
        NULL,
        ar_expression_ast__create_memory_access("message", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == false, "Append to message target should fail");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__rejects_non_list_target(void) {
    printf("Testing append rejects non-list target...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_non_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_integer(mut_memory, "results", 99), "Non-list target should be stored");
    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "42",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == false, "Append to non-list target should fail");
    AR_ASSERT(ar_data__get_integer(ar_data__get_map_data(mut_memory, "results")) == 99, "Target should remain unchanged");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__stores_result_assignment(void) {
    printf("Testing append stores result assignment...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_result_assignment");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", ar_data__create_list()), "Results list should be stored");
    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "42",
        "memory.append_ok",
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Assigned append instruction should complete");
    ar_data_t *ref_append_ok = ar_data__get_map_data(mut_memory, "append_ok");
    AR_ASSERT(ref_append_ok != NULL, "Result assignment should be stored");
    AR_ASSERT(ar_data__get_integer(ref_append_ok) == 1, "Successful append should store 1");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__stores_zero_result_for_failure(void) {
    printf("Testing append stores zero result for failure...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_result_failure");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_integer(mut_memory, "results", 99), "Non-list target should be stored");
    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "42",
        "memory.append_ok",
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Assigned append instruction should complete even when append fails");
    ar_data_t *ref_append_ok = ar_data__get_map_data(mut_memory, "append_ok");
    AR_ASSERT(ref_append_ok != NULL, "Result assignment should be stored");
    AR_ASSERT(ar_data__get_integer(ref_append_ok) == 0, "Failed append should store 0");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    printf("Starting append instruction_evaluator tests...\n");

    test_append_instruction_evaluator__create_destroy();
    test_append_instruction_evaluator__appends_literal_to_memory_list();
    test_append_instruction_evaluator__copies_borrowed_memory_value();
    test_append_instruction_evaluator__copies_borrowed_message_value();
    test_append_instruction_evaluator__transfers_owned_list_literal();
    test_append_instruction_evaluator__rejects_message_target();
    test_append_instruction_evaluator__rejects_non_list_target();
    test_append_instruction_evaluator__stores_result_assignment();
    test_append_instruction_evaluator__stores_zero_result_for_failure();

    printf("All append instruction_evaluator tests passed!\n");
    return 0;
}
