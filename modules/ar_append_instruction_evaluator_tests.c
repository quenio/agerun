#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ar_append_instruction_evaluator.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_evaluator_fixture.h"
#include "ar_expression_ast.h"
#include "ar_expression_parser.h"
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

static ar_expression_ast_t *_parse_append_test_expression(
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

static void test_append_instruction_evaluator__deep_copies_borrowed_nested_memory_value(void) {
    printf("Testing append deep-copies borrowed nested memory value...\n");

    // Given a memory-owned list and a borrowed nested memory value
    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_nested_memory");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", ar_data__create_list()), "Results list should be stored");
    ar_data_t *own_payload = ar_data__create_map();
    ar_data_t *own_items = ar_data__create_list();
    AR_ASSERT(own_payload != NULL, "Payload map should be created");
    AR_ASSERT(own_items != NULL, "Nested list should be created");
    AR_ASSERT(ar_data__list_add_last_string(own_items, "first"), "Nested list item should be stored");
    AR_ASSERT(ar_data__set_map_data(own_payload, "items", own_items), "Nested list should be stored");
    own_items = NULL;
    AR_ASSERT(ar_data__set_map_data(mut_memory, "payload", own_payload), "Payload should be stored");
    own_payload = NULL;

    const char *target_path[] = {"results"};
    const char *value_path[] = {"payload"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "memory.payload",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_memory_access("memory", value_path, 1)
    );

    // When appending the borrowed nested value
    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    // Then the appended value should be an independent deep copy
    AR_ASSERT(result == true, "Append should succeed");
    ar_data_t *ref_original = ar_data__get_map_data(mut_memory, "payload");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    ar_data_t *ref_appended = ar_data__list_first(ref_results);
    AR_ASSERT(ref_appended != NULL, "Appended value should exist");
    AR_ASSERT(ref_appended != ref_original, "Nested borrowed memory value should be copied before append");
    ar_data_t *ref_original_items = ar_data__get_map_data(ref_original, "items");
    ar_data_t *ref_appended_items = ar_data__get_map_data(ref_appended, "items");
    AR_ASSERT(ref_appended_items != NULL, "Appended nested list should exist");
    AR_ASSERT(ref_appended_items != ref_original_items, "Nested list should be copied");
    AR_ASSERT(ar_data__list_count(ref_appended_items) == 1, "Appended nested list count should match");
    AR_ASSERT(ar_data__list_add_last_string(ref_original_items, "second"), "Original nested list should mutate");
    AR_ASSERT(ar_data__list_count(ref_original_items) == 2, "Original nested list should grow");
    AR_ASSERT(ar_data__list_count(ref_appended_items) == 1, "Appended nested list should remain independent");

    // Cleanup
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

static void test_append_instruction_evaluator__accepts_head_tail_value_arguments(void) {
    printf("Testing append accepts head/tail value arguments...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_head_tail_args");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");
    ar_log_t *ref_log = ar_evaluator_fixture__get_log(own_fixture);

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", ar_data__create_list()),
              "Results list should be stored");

    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_head_ast = _create_append_ast(
        "memory.results",
        "head([7, 8])",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        _parse_append_test_expression(ref_log, "head([7, 8])")
    );

    bool result = ar_append_instruction_evaluator__evaluate(
        own_evaluator, ref_frame, own_head_ast
    );

    AR_ASSERT(result == true, "Append should accept head() as a value argument");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    AR_ASSERT(ar_data__list_count(ref_results) == 1,
              "Results should contain appended head result");
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_results)) == 7,
              "Head argument should append first item");

    ar_instruction_ast_t *own_tail_ast = _create_append_ast(
        "memory.results",
        "tail([7, 8, 9])",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        _parse_append_test_expression(ref_log, "tail([7, 8, 9])")
    );

    result = ar_append_instruction_evaluator__evaluate(
        own_evaluator, ref_frame, own_tail_ast
    );

    AR_ASSERT(result == true, "Append should accept tail() as a value argument");
    AR_ASSERT(ar_data__list_count(ref_results) == 2,
              "Results should contain both appended values");
    ar_data_t *ref_tail = ar_data__list_last(ref_results);
    AR_ASSERT(ref_tail != NULL, "Tail argument should be appended");
    AR_ASSERT(ar_data__get_type(ref_tail) == AR_DATA_TYPE__LIST,
              "Tail argument should append a list");
    AR_ASSERT(ar_data__list_count(ref_tail) == 2,
              "Tail argument list should contain remaining items");
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_tail)) == 8,
              "Tail argument first item should match");
    AR_ASSERT(ar_data__get_integer(ar_data__list_last(ref_tail)) == 9,
              "Tail argument last item should match");

    ar_instruction_ast__destroy(own_head_ast);
    ar_instruction_ast__destroy(own_tail_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__appends_to_nested_memory_owned_list(void) {
    printf("Testing append to nested memory-owned list...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_nested_memory_list");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_data_t *own_wrapper = ar_data__create_map();
    AR_ASSERT(own_wrapper != NULL, "Wrapper map should be created");
    AR_ASSERT(ar_data__set_map_data(own_wrapper, "results", ar_data__create_list()), "Nested list should be stored");
    AR_ASSERT(ar_data__set_map_data(mut_memory, "wrapper", own_wrapper), "Wrapper should be stored in memory");

    const char *target_path[] = {"wrapper", "results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.wrapper.results",
        "77",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 2),
        ar_expression_ast__create_literal_int(77)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Append to nested memory-owned list should succeed");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "wrapper.results");
    AR_ASSERT(ref_results != NULL, "Nested results list should exist");
    AR_ASSERT(ar_data__list_count(ref_results) == 1, "Nested results list should contain one item");
    AR_ASSERT(ar_data__get_integer(ar_data__list_first(ref_results)) == 77, "Appended value should match");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__stores_zero_for_message_owned_target(void) {
    printf("Testing append stores zero for message-owned target...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_message_target");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);

    ar_data_t *own_context = ar_data__create_map();
    ar_data_t *own_message = ar_data__create_map();
    AR_ASSERT(own_context != NULL, "Context creation should succeed");
    AR_ASSERT(own_message != NULL, "Message creation should succeed");
    AR_ASSERT(ar_data__set_map_data(own_message, "results", ar_data__create_list()), "Message list should be stored");
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    AR_ASSERT(own_frame != NULL, "Frame creation should succeed");

    const char *target_path[] = {"results"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "message.results",
        "42",
        "memory.append_ok",
        ar_expression_ast__create_memory_access("message", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, own_frame, own_ast);

    AR_ASSERT(result == true, "Assigned append should complete for message-owned target no-op");
    AR_ASSERT(ar_data__get_integer(ar_data__get_map_data(mut_memory, "append_ok")) == 0, "No-op append should store 0");
    ar_data_t *ref_message_results = ar_data__get_map_data(own_message, "results");
    AR_ASSERT(ref_message_results != NULL, "Message results list should still exist");
    AR_ASSERT(ar_data__list_count(ref_message_results) == 0, "Message results list should remain unchanged");

    ar_instruction_ast__destroy(own_ast);
    ar_frame__destroy(own_frame);
    ar_data__destroy(own_message);
    ar_data__destroy(own_context);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__stores_zero_for_literal_target(void) {
    printf("Testing append stores zero for literal target...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_literal_target");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    ar_expression_ast_t *own_target_items[1];
    own_target_items[0] = ar_expression_ast__create_literal_int(1);
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "[1]",
        "42",
        "memory.append_ok",
        ar_expression_ast__create_literal_list(own_target_items, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Assigned append should complete for literal target no-op");
    AR_ASSERT(ar_data__get_integer(ar_data__get_map_data(mut_memory, "append_ok")) == 0, "No-op append should store 0");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__stores_zero_for_non_list_target(void) {
    printf("Testing append stores zero for non-list target...\n");

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
        "memory.append_ok",
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Assigned append should complete for non-list target no-op");
    AR_ASSERT(ar_data__get_integer(ar_data__get_map_data(mut_memory, "append_ok")) == 0, "No-op append should store 0");
    AR_ASSERT(ar_data__get_integer(ar_data__get_map_data(mut_memory, "results")) == 99, "Target should remain unchanged");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__continues_when_unassigned_value_expression_fails(void) {
    printf("Testing append continues when unassigned value expression fails...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_unassigned_value_failure");
    AR_ASSERT(own_fixture != NULL, "Fixture creation should succeed");
    ar_append_instruction_evaluator_t *own_evaluator = _create_evaluator(own_fixture);
    ar_frame_t *ref_frame = ar_evaluator_fixture__create_frame(own_fixture);
    AR_ASSERT(ref_frame != NULL, "Frame creation should succeed");

    ar_data_t *mut_memory = ar_evaluator_fixture__get_memory(own_fixture);
    AR_ASSERT(ar_data__set_map_data(mut_memory, "results", ar_data__create_list()), "Results list should be stored");

    const char *target_path[] = {"results"};
    const char *value_path[] = {"missing"};
    ar_instruction_ast_t *own_ast = _create_append_ast(
        "memory.results",
        "memory.missing",
        NULL,
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_memory_access("memory", value_path, 1)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == true, "Unassigned append value failure should complete as no-op");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    AR_ASSERT(ref_results != NULL, "Results list should still exist");
    AR_ASSERT(ar_data__list_count(ref_results) == 0, "Failed append should not mutate target list");

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

static void test_append_instruction_evaluator__does_not_mutate_when_result_path_invalid(void) {
    printf("Testing append does not mutate when result path is invalid...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_invalid_result_path");
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
        "context.append_ok",
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == false, "Append should fail when result path is not memory-owned");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    AR_ASSERT(ref_results != NULL, "Results list should still exist");
    AR_ASSERT(ar_data__list_count(ref_results) == 0, "Invalid result path should prevent append mutation");

    ar_instruction_ast__destroy(own_ast);
    ar_append_instruction_evaluator__destroy(own_evaluator);
    ar_evaluator_fixture__destroy(own_fixture);
}

static void test_append_instruction_evaluator__rolls_back_when_result_storage_fails(void) {
    printf("Testing append rolls back when result storage fails...\n");

    ar_evaluator_fixture_t *own_fixture = ar_evaluator_fixture__create("append_result_storage_failure");
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
        "memory.results.append_ok",
        ar_expression_ast__create_memory_access("memory", target_path, 1),
        ar_expression_ast__create_literal_int(42)
    );

    bool result = ar_append_instruction_evaluator__evaluate(own_evaluator, ref_frame, own_ast);

    AR_ASSERT(result == false, "Append should fail when result cannot be stored");
    ar_data_t *ref_results = ar_data__get_map_data(mut_memory, "results");
    AR_ASSERT(ref_results != NULL, "Results list should still exist");
    AR_ASSERT(ar_data__list_count(ref_results) == 0, "Failed result storage should roll back append");

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
    test_append_instruction_evaluator__deep_copies_borrowed_nested_memory_value();
    test_append_instruction_evaluator__copies_borrowed_message_value();
    test_append_instruction_evaluator__transfers_owned_list_literal();
    test_append_instruction_evaluator__accepts_head_tail_value_arguments();
    test_append_instruction_evaluator__appends_to_nested_memory_owned_list();
    test_append_instruction_evaluator__stores_zero_for_message_owned_target();
    test_append_instruction_evaluator__stores_zero_for_literal_target();
    test_append_instruction_evaluator__stores_zero_for_non_list_target();
    test_append_instruction_evaluator__continues_when_unassigned_value_expression_fails();
    test_append_instruction_evaluator__stores_result_assignment();
    test_append_instruction_evaluator__does_not_mutate_when_result_path_invalid();
    test_append_instruction_evaluator__rolls_back_when_result_storage_fails();
    test_append_instruction_evaluator__stores_zero_result_for_failure();

    printf("All append instruction_evaluator tests passed!\n");
    return 0;
}
