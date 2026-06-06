#include <stdio.h>

#include "ar_assert.h"
#include "ar_data.h"
#include "ar_evaluator_fixture.h"
#include "ar_expression_ast.h"
#include "ar_frame.h"
#include "ar_heap.h"
#include "ar_instruction_ast.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_tail_instruction_evaluator.h"

static ar_instruction_ast_t* _create_tail_ast(
    const char *ref_list_arg,
    const char *ref_result_path,
    ar_expression_ast_t *own_list_ast
) {
    const char *args[] = {ref_list_arg};
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_function_call(
        AR_INSTRUCTION_AST_TYPE__TAIL,
        "tail",
        args,
        1,
        ref_result_path
    );
    AR_ASSERT(own_ast != NULL, "Tail AST creation should succeed");

    ar_list_t *own_arg_asts = ar_list__create();
    AR_ASSERT(own_arg_asts != NULL, "Argument AST list creation should succeed");
    AR_ASSERT(ar_list__add_last(own_arg_asts, own_list_ast), "List AST should be added");
    AR_ASSERT(
        ar_instruction_ast__set_function_arg_asts(own_ast, own_arg_asts),
        "Argument ASTs should be attached"
    );

    return own_ast; // Ownership transferred to caller
}

static ar_tail_instruction_evaluator_t* _create_evaluator(ar_evaluator_fixture_t *ref_fixture) {
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(ref_fixture);
    ar_expression_evaluator_t *ref_expr_eval = ar_evaluator_fixture__get_expression_evaluator(ref_fixture);
    ar_tail_instruction_evaluator_t *own_evaluator = ar_tail_instruction_evaluator__create(
        ref_log,
        ref_expr_eval
    );
    AR_ASSERT(own_evaluator != NULL, "Tail evaluator creation should succeed");
    return own_evaluator; // Ownership transferred to caller
}

static ar_expression_ast_t* _create_int_list_ast(int first, int second, int third) {
    ar_expression_ast_t *own_items[3];
    own_items[0] = ar_expression_ast__create_literal_int(first);
    own_items[1] = ar_expression_ast__create_literal_int(second);
    own_items[2] = ar_expression_ast__create_literal_int(third);
    return ar_expression_ast__create_literal_list(own_items, 3); // Ownership transferred to caller
}

static ar_expression_ast_t* _create_single_int_list_ast(int value) {
    ar_expression_ast_t *own_items[1];
    own_items[0] = ar_expression_ast__create_literal_int(value);
    return ar_expression_ast__create_literal_list(own_items, 1); // Ownership transferred to caller
}

static void _assert_result_integer(ar_data_t *ref_memory, const char *ref_key, int expected) {
    ar_data_t *ref_result = ar_data__get_map_data(ref_memory, ref_key);
    AR_ASSERT(ref_result != NULL, "Result should be stored");
    AR_ASSERT(ar_data__get_type(ref_result) == AR_DATA_TYPE__INTEGER, "Result should be an integer");
    AR_ASSERT(ar_data__get_integer(ref_result) == expected, "Result integer should match expected value");
}

static ar_data_t* _assert_result_list(ar_data_t *ref_memory, const char *ref_key, size_t expected_count) {
    ar_data_t *ref_result = ar_data__get_map_data(ref_memory, ref_key);
    AR_ASSERT(ref_result != NULL, "Result should be stored");
    AR_ASSERT(ar_data__get_type(ref_result) == AR_DATA_TYPE__LIST, "Result should be a list");
    AR_ASSERT(ar_data__list_count(ref_result) == expected_count, "Result list count should match");
    return ref_result;
}

static void test_tail_instruction_evaluator__create_destroy(void) {
    printf("Testing tail instruction evaluator create/destroy...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_create_destroy");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");

    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);

    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__stores_remaining_items_from_literal_list(void) {
    printf("Testing tail stores remaining items from literal list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_literal_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "[1, 2, 3]",
        "memory.result",
        _create_int_list_ast(1, 2, 3)
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Tail instruction should complete");
    ar_data_t *ref_result = _assert_result_list(mut_memory, "result", 2);
    ar_data_t **own_items = ar_data__list_items(ref_result);
    AR_ASSERT(own_items != NULL, "Tail result should expose items");
    AR_ASSERT(ar_data__get_integer(own_items[0]) == 2, "First tail item should be second source item");
    AR_ASSERT(ar_data__get_integer(own_items[1]) == 3, "Second tail item should be third source item");
    AR__HEAP__FREE(own_items);

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__single_item_tail_is_empty_list(void) {
    printf("Testing tail of single-item list stores empty list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_single_item");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "[1]",
        "memory.result",
        _create_single_int_list_ast(1)
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Tail instruction should complete for single-item list");
    _assert_result_list(mut_memory, "result", 0);

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__empty_list_tail_is_empty_list(void) {
    printf("Testing tail of empty list stores empty list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_empty_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "[]",
        "memory.result",
        ar_expression_ast__create_literal_list(NULL, 0)
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Tail instruction should complete for empty list");
    _assert_result_list(mut_memory, "result", 0);

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__stores_zero_for_non_list(void) {
    printf("Testing tail stores zero for non-list input...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_non_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "\"not-a-list\"",
        "memory.result",
        ar_expression_ast__create_literal_string("not-a-list")
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Tail instruction should complete for non-list input");
    _assert_result_integer(mut_memory, "result", 0);

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__stores_zero_for_missing_message_field(void) {
    printf("Testing tail stores zero for missing message field...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_missing_message");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Context creation should succeed");
    AR_ASSERT(own_message != NULL, "Message creation should succeed");
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    AR_ASSERT(own_frame != NULL, "Frame creation should succeed");

    const char *path[] = {"missing"};
    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "message.missing",
        "memory.result",
        ar_expression_ast__create_memory_access("message", path, 1)
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    AR_ASSERT(result == true, "Tail instruction should complete for missing message field");
    _assert_result_integer(mut_memory, "result", 0);

    ar_instruction_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__does_not_mutate_source_list(void) {
    printf("Testing tail does not mutate source list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_source_unchanged");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
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
    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "memory.targets",
        "memory.result",
        ar_expression_ast__create_memory_access("memory", path, 1)
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Tail instruction should complete for memory list");
    ar_data_t *ref_result = _assert_result_list(mut_memory, "result", 1);
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_result)) == 20, "Tail result should contain second source item");
    ar_data_t *ref_targets = ar_data__get_map_data(mut_memory, "targets");
    AR_ASSERT(ref_targets != NULL, "Source list should remain stored");
    AR_ASSERT(ar_data__list_count(ref_targets) == 2, "Source list should not be shortened");
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_targets)) == 10, "First source item should remain");

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__can_overwrite_source_list(void) {
    printf("Testing tail can overwrite source list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_overwrite_source");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
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
    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "memory.targets",
        "memory.targets",
        ar_expression_ast__create_memory_access("memory", path, 1)
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Tail instruction should complete when overwriting source");
    ar_data_t *ref_result = _assert_result_list(mut_memory, "targets", 1);
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_result)) == 20, "Tail should replace source with remaining item");

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_tail_instruction_evaluator__deep_copies_nested_container(void) {
    printf("Testing tail deep-copies nested container...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("tail_nested_limit");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_tail_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_targets = ar_data__create_list();
    ar_data_t *own_nested = ar_data__create_list();
    ar_data_t *own_inner = ar_data__create_list();
    AR_ASSERT(own_targets != NULL, "Targets list creation should succeed");
    AR_ASSERT(own_nested != NULL, "Nested list creation should succeed");
    AR_ASSERT(own_inner != NULL, "Inner list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_targets, 5), "First value should be stored");
    AR_ASSERT(ar_data__list_add_last_integer(own_inner, 1), "Inner value should be stored");
    AR_ASSERT(ar_data__list_add_last_data(own_nested, own_inner), "Inner list should be nested");
    own_inner = NULL;
    AR_ASSERT(ar_data__list_add_last_data(own_targets, own_nested), "Nested list should be in tail");
    own_nested = NULL;
    AR_ASSERT(ar_data__set_map_data(mut_memory, "targets", own_targets), "Targets list should be stored");
    own_targets = NULL;

    const char *path[] = {"targets"};
    ar_instruction_ast_t *own_ast = _create_tail_ast(
        "memory.targets",
        "memory.result",
        ar_expression_ast__create_memory_access("memory", path, 1)
    );

    bool result = ar_tail_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Tail should complete for nested containers");
    ar_data_t *ref_result = _assert_result_list(mut_memory, "result", 1);
    ar_data_t *ref_targets = ar_data__get_map_data(mut_memory, "targets");
    AR_ASSERT(ar_data__list_count(ref_targets) == 2, "Source list should remain unchanged");
    ar_data_t **own_source_items = ar_data__list_items(ref_targets);
    AR_ASSERT(own_source_items != NULL, "Source items should be available");
    ar_data_t *ref_source_tail_item = own_source_items[1];
    AR__HEAP__FREE(own_source_items);
    ar_data_t *ref_result_tail_item = ar_data__list_first(ref_result);
    AR_ASSERT(ar_data__get_type(ref_result_tail_item) == AR_DATA_TYPE__LIST, "Tail item should be a copied list");
    AR_ASSERT(ref_result_tail_item != ref_source_tail_item, "Tail item should be independent from source");
    ar_data_t *ref_source_inner = ar_data__list_first(ref_source_tail_item);
    ar_data_t *ref_result_inner = ar_data__list_first(ref_result_tail_item);
    AR_ASSERT(ref_result_inner != NULL, "Copied inner list should exist");
    AR_ASSERT(ref_result_inner != ref_source_inner, "Inner list should be deep-copied");
    AR_ASSERT(ar_data__list_count(ref_result_inner) == 1, "Copied inner list count should match");
    AR_ASSERT(ar_data__list_add_last_integer(ref_source_inner, 2), "Source inner list should mutate");
    AR_ASSERT(ar_data__list_count(ref_source_inner) == 2, "Source inner list should grow");
    AR_ASSERT(ar_data__list_count(ref_result_inner) == 1, "Copied inner list should remain independent");

    ar_instruction_ast__destroy(own_ast);
    ar_tail_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

int main(void) {
    printf("Starting tail instruction_evaluator tests...\n");

    test_tail_instruction_evaluator__create_destroy();
    test_tail_instruction_evaluator__stores_remaining_items_from_literal_list();
    test_tail_instruction_evaluator__single_item_tail_is_empty_list();
    test_tail_instruction_evaluator__empty_list_tail_is_empty_list();
    test_tail_instruction_evaluator__stores_zero_for_non_list();
    test_tail_instruction_evaluator__stores_zero_for_missing_message_field();
    test_tail_instruction_evaluator__does_not_mutate_source_list();
    test_tail_instruction_evaluator__can_overwrite_source_list();
    test_tail_instruction_evaluator__deep_copies_nested_container();

    printf("All tail instruction_evaluator tests passed!\n");
    return 0;
}
