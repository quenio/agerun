#include <stdio.h>

#include "ar_assert.h"
#include "ar_data.h"
#include "ar_evaluator_fixture.h"
#include "ar_expression_ast.h"
#include "ar_frame.h"
#include "ar_head_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_instruction_ast.h"
#include "ar_list.h"
#include "ar_log.h"

static ar_instruction_ast_t* _create_head_ast(
    const char *ref_list_arg,
    const char *ref_result_path,
    ar_expression_ast_t *own_list_ast
) {
    const char *args[] = {ref_list_arg};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__HEAD,
        "head",
        args,
        1,
        ref_result_path
    );
    AR_ASSERT(own_ast != NULL, "Head AST creation should succeed");

    ar_list_t *own_arg_asts = ar_list__create();
    AR_ASSERT(own_arg_asts != NULL, "Argument AST list creation should succeed");
    AR_ASSERT(ar_list__add_last(own_arg_asts, own_list_ast), "List AST should be added");
    AR_ASSERT(
        ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts),
        "Argument ASTs should be attached"
    );

    return own_ast; // Ownership transferred to caller
}

static ar_head_instruction_evaluator_t* _create_evaluator(ar_evaluator_fixture_t *ref_fixture) {
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(ref_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(ref_fixture);
    ar_head_instruction_evaluator_t *own_evaluator = ar_head_instruction_evaluator__create(
        ref_log,
        ref_expr_eval
    );
    AR_ASSERT(own_evaluator != NULL, "Head evaluator creation should succeed");
    return own_evaluator; // Ownership transferred to caller
}

static ar_expression_ast_t* _create_int_list_ast(int first, int second, int third) {
    ar_expression_ast_t *own_items[3];
    own_items[0] = ar_expression_ast__create_literal_int(first);
    own_items[1] = ar_expression_ast__create_literal_int(second);
    own_items[2] = ar_expression_ast__create_literal_int(third);
    return ar_expression_ast__create_literal_list(own_items, 3); // Ownership transferred to caller
}

static void _assert_result_integer(ar_data_t *ref_memory, const char *ref_key, int expected) {
    ar_data_t *ref_result = ar_data__get_map_data(ref_memory, ref_key);
    AR_ASSERT(ref_result != NULL, "Result should be stored");
    AR_ASSERT(ar_data__get_type(ref_result) == AR_DATA_TYPE__INTEGER, "Result should be an integer");
    AR_ASSERT(ar_data__get_integer(ref_result) == expected, "Result integer should match expected value");
}

static void test_head_instruction_evaluator__create_destroy(void) {
    printf("Testing head instruction evaluator create/destroy...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_create_destroy");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);

    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_head_instruction_evaluator__stores_first_item_from_literal_list(void) {
    printf("Testing head stores first item from literal list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_literal_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_instruction_ast_t *own_ast = _create_head_ast(
        "[1, 2, 3]",
        "memory.result",
        _create_int_list_ast(1, 2, 3)
    );

    bool result = ar_head_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Head instruction should complete");
    _assert_result_integer(mut_memory, "result", 1);

    ar_instruction_ast__destroy(own_ast);
    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_head_instruction_evaluator__stores_zero_for_empty_list(void) {
    printf("Testing head stores zero for empty list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_empty_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_instruction_ast_t *own_ast = _create_head_ast(
        "[]",
        "memory.result",
        ar_expression_ast__create_literal_list(NULL, 0)
    );

    bool result = ar_head_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Head instruction should complete for empty list");
    _assert_result_integer(mut_memory, "result", 0);

    ar_instruction_ast__destroy(own_ast);
    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_head_instruction_evaluator__stores_zero_for_non_list(void) {
    printf("Testing head stores zero for non-list input...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_non_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_instruction_ast_t *own_ast = _create_head_ast(
        "\"not-a-list\"",
        "memory.result",
        ar_expression_ast__create_literal_string("not-a-list")
    );

    bool result = ar_head_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Head instruction should complete for non-list input");
    _assert_result_integer(mut_memory, "result", 0);

    ar_instruction_ast__destroy(own_ast);
    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_head_instruction_evaluator__stores_zero_for_missing_message_field(void) {
    printf("Testing head stores zero for missing message field...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_missing_message");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Context creation should succeed");
    AR_ASSERT(own_message != NULL, "Message creation should succeed");
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    AR_ASSERT(own_frame != NULL, "Frame creation should succeed");

    const char *path[] = {"missing"};
    ar_instruction_ast_t *own_ast = _create_head_ast(
        "message.missing",
        "memory.result",
        ar_expression_ast__create_memory_access("message", path, 1)
    );

    bool result = ar_head_instruction_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    AR_ASSERT(result == true, "Head instruction should complete for missing message field");
    _assert_result_integer(mut_memory, "result", 0);

    ar_instruction_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_head_instruction_evaluator__does_not_mutate_source_list(void) {
    printf("Testing head does not mutate source list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_source_unchanged");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Targets list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 10), "First target should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 20), "Second target should be stored");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "targets", own_targets), "Targets list should be stored");
    own_targets = NULL;

    const char *path[] = {"targets"};
    ar_instruction_ast_t *own_ast = _create_head_ast(
        "memory.targets",
        "memory.result",
        ar_expression_ast__create_memory_access("memory", path, 1)
    );

    bool result = ar_head_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Head instruction should complete for memory list");
    _assert_result_integer(mut_memory, "result", 10);
    ar_data_t *ref_targets = ar_data__get_map_data(mut_memory, "targets");
    AR_ASSERT(ref_targets != NULL, "Source list should remain stored");
    AR_ASSERT(ar_data__list_count(ref_targets) == 2, "Source list should not be shortened");
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_targets)) == 10, "First source item should remain");

    ar_instruction_ast__destroy(own_ast);
    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_head_instruction_evaluator__can_overwrite_source_list(void) {
    printf("Testing head can overwrite source list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_overwrite_source");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_targets = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Targets list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 10), "First target should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 20), "Second target should be stored");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "targets", own_targets), "Targets list should be stored");
    own_targets = NULL;

    const char *path[] = {"targets"};
    ar_instruction_ast_t *own_ast = _create_head_ast(
        "memory.targets",
        "memory.targets",
        ar_expression_ast__create_memory_access("memory", path, 1)
    );

    bool result = ar_head_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Head instruction should complete when overwriting source");
    _assert_result_integer(mut_memory, "targets", 10);

    ar_instruction_ast__destroy(own_ast);
    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_head_instruction_evaluator__deep_copies_nested_container(void) {
    printf("Testing head deep-copies nested container...\n");

    // Given a memory list whose first item is a nested container
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("head_nested_limit");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_head_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_targets = ar_data__create_list();
    ar_data_t *own_nested = ar_data__create_list();
    ar_data_t *own_inner = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Targets list creation should succeed");
    AR_ASSERT(own_nested != NULL, "Nested list creation should succeed");
    AR_ASSERT(own_inner != NULL, "Inner list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_inner, 1), "Inner value should be stored");
    AR_ASSERT(ar_data__list_add_last_data(own_nested, own_inner), "Inner list should be nested");
    own_inner = NULL;
    AR_ASSERT(ar_data__list_add_last_data(own_targets, own_nested), "Nested list should be target head");
    own_nested = NULL;
    AR_ASSERT(ar_data__set_map_data(mut_memory, "targets", own_targets), "Targets list should be stored");
    own_targets = NULL;

    const char *path[] = {"targets"};
    ar_instruction_ast_t *own_ast = _create_head_ast(
        "memory.targets",
        "memory.result",
        ar_expression_ast__create_memory_access("memory", path, 1)
    );

    // When evaluating head(...) with a result assignment
    bool result = ar_head_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    // Then the result should be an independent deep copy of the first item
    AR_ASSERT(result == true, "Head should complete for nested containers");
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    AR_ASSERT(ref_result != NULL, "Result should be stored");
    AR_ASSERT(ar_data__get_type(ref_result) == AR_DATA_TYPE__LIST, "Result should be a copied list");
    AR_ASSERT(ar_data__list_count(ref_result) == 1, "Result list should contain copied inner list");
    ar_data_t *ref_targets = ar_data__get_map_data(mut_memory, "targets");
    AR_ASSERT(ar_data__list_count(ref_targets) == 1, "Source list should remain unchanged");
    ar_data_t *ref_source_head = ar_data__list_first(ref_targets);
    AR_ASSERT(ar_data__get_type(ref_source_head) == AR_DATA_TYPE__LIST, "Nested source item should remain a list");
    AR_ASSERT(ref_result != ref_source_head, "Result list should be independent from source item");
    ar_data_t *ref_source_inner = ar_data__list_first(ref_source_head);
    ar_data_t *ref_result_inner = ar_data__list_first(ref_result);
    AR_ASSERT(ref_result_inner != NULL, "Copied inner list should exist");
    AR_ASSERT(ref_result_inner != ref_source_inner, "Inner list should be deep-copied");
    AR_ASSERT(ar_data__list_count(ref_result_inner) == 1, "Copied inner list count should match");
    AR_ASSERT(ar_data__list_add_last_integer(ref_source_inner, 2), "Source inner list should mutate");
    AR_ASSERT(ar_data__list_count(ref_source_inner) == 2, "Source inner list should grow");
    AR_ASSERT(ar_data__list_count(ref_result_inner) == 1, "Copied inner list should remain independent");

    // Cleanup
    ar_instruction_ast__destroy(own_ast);
    ar_head_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    printf("Starting head instruction_evaluator tests...\n");

    test_head_instruction_evaluator__create_destroy();
    test_head_instruction_evaluator__stores_first_item_from_literal_list();
    test_head_instruction_evaluator__stores_zero_for_empty_list();
    test_head_instruction_evaluator__stores_zero_for_non_list();
    test_head_instruction_evaluator__stores_zero_for_missing_message_field();
    test_head_instruction_evaluator__does_not_mutate_source_list();
    test_head_instruction_evaluator__can_overwrite_source_list();
    test_head_instruction_evaluator__deep_copies_nested_container();

    printf("All head instruction_evaluator tests passed!\n");
    return 0;
}
