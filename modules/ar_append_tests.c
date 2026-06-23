#include <stdio.h>
#include <string.h>

#include "ar_append.h"
#include "ar_assert.h"
#include "ar_data.h"
#include "ar_heap.h"

static void _assert_integer_value(const ar_data_t *ref_data, int expected) {
    AR_ASSERT(ref_data != NULL, "Result should exist");
    AR_ASSERT(ar_data__get_type(ref_data) == AR_DATA_TYPE__INTEGER,
              "Result should be an integer");
    AR_ASSERT(ar_data__get_integer(ref_data) == expected,
              "Integer result should match expected value");
}

static void test_append__non_empty_list_returns_new_list_with_deep_copies(void) {
    printf("Testing pure append copies non-empty list and appended value...\n");

    // Given a source list with a scalar item
    ar_data_t *own_source = ar_data__create_list();
    AR_ASSERT(own_source != NULL, "Source list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_source, 1),
              "Source list should store first item");

    // Given the source list also contains nested data
    ar_data_t *own_payload = ar_data__create_map();
    AR_ASSERT(own_payload != NULL, "Payload map creation should succeed");
    ar_data_t *own_nested_items = ar_data__create_list();
    AR_ASSERT(own_nested_items != NULL, "Nested list creation should succeed");
    AR_ASSERT(ar_data__list_add_last_integer(own_nested_items, 2),
              "Nested list should store first item");
    AR_ASSERT(ar_data__set_map_data(own_payload, "items", own_nested_items),
              "Payload should store nested list");
    own_nested_items = NULL;
    AR_ASSERT(ar_data__list_add_last_data(own_source, own_payload),
              "Source list should store payload");
    own_payload = NULL;

    // Given an appended value with owned nested content
    ar_data_t *own_value = ar_data__create_map();
    AR_ASSERT(own_value != NULL, "Appended value creation should succeed");
    AR_ASSERT(ar_data__set_map_string(own_value, "label", "new"),
              "Appended value should store label");

    // When creating the pure append result
    ar_data_t *own_result = ar_append__create_result(own_source, own_value);

    // Then the result is a new list and the source list is unchanged
    AR_ASSERT(own_result != NULL, "Append result should exist");
    AR_ASSERT(ar_data__get_type(own_result) == AR_DATA_TYPE__LIST,
              "Append result should be a list");
    AR_ASSERT(ar_data__list_count(own_result) == 3,
              "Append result should contain copied items plus appended value");
    AR_ASSERT(ar_data__list_count(own_source) == 2,
              "Source list should remain unchanged");

    // Then source items and appended value are deep-copied
    ar_data_t **own_source_items = ar_data__list_items(own_source);
    ar_data_t **own_result_items = ar_data__list_items(own_result);
    AR_ASSERT(own_source_items != NULL, "Source items should be available");
    AR_ASSERT(own_result_items != NULL, "Result items should be available");
    AR_ASSERT(own_result_items[0] != own_source_items[0],
              "First source item should be deep-copied");
    AR_ASSERT(own_result_items[1] != own_source_items[1],
              "Nested source item should be deep-copied");
    AR_ASSERT(own_result_items[2] != own_value,
              "Appended value should be deep-copied");
    _assert_integer_value(own_result_items[0], 1);

    // Then nested source data remains independent from the result
    ar_data_t *ref_source_nested = ar_data__get_map_data(own_source_items[1], "items");
    ar_data_t *ref_result_nested = ar_data__get_map_data(own_result_items[1], "items");
    AR_ASSERT(ref_source_nested != NULL, "Source nested list should exist");
    AR_ASSERT(ref_result_nested != NULL, "Result nested list should exist");
    AR_ASSERT(ref_result_nested != ref_source_nested, "Nested list should be deep-copied");
    AR_ASSERT(ar_data__list_add_last_integer(ref_source_nested, 99),
              "Mutating source nested list should succeed");
    AR_ASSERT(ar_data__list_count(ref_source_nested) == 2,
              "Source nested list should grow");
    AR_ASSERT(ar_data__list_count(ref_result_nested) == 1,
              "Result nested list should stay independent");

    // Then mutating the original appended value does not affect the result
    AR_ASSERT(strcmp(ar_data__get_map_string(own_result_items[2], "label"), "new") == 0,
              "Appended copy should match original value");
    AR_ASSERT(ar_data__set_map_string(own_value, "label", "changed"),
              "Mutating original appended value should succeed");
    AR_ASSERT(strcmp(ar_data__get_map_string(own_result_items[2], "label"), "new") == 0,
              "Appended copy should stay independent");

    // Cleanup
    AR__HEAP__FREE(own_source_items);
    AR__HEAP__FREE(own_result_items);
    ar_data__destroy(own_result);
    ar_data__destroy(own_value);
    ar_data__destroy(own_source);
}

static void test_append__empty_list_returns_one_item_list(void) {
    printf("Testing pure append on empty list...\n");

    // Given an empty source list and a value to append
    ar_data_t *own_source = ar_data__create_list();
    AR_ASSERT(own_source != NULL, "Source list creation should succeed");
    ar_data_t *own_value = ar_data__create_string("first");
    AR_ASSERT(own_value != NULL, "Value creation should succeed");

    // When creating the pure append result
    ar_data_t *own_result = ar_append__create_result(own_source, own_value);

    // Then the result is a new one-item list
    AR_ASSERT(own_result != NULL, "Append result should exist");
    AR_ASSERT(ar_data__get_type(own_result) == AR_DATA_TYPE__LIST,
              "Append result should be a list");
    AR_ASSERT(ar_data__list_count(own_result) == 1,
              "Append result should contain one item");
    AR_ASSERT(ar_data__list_count(own_source) == 0,
              "Source list should remain empty");
    ar_data_t *ref_first = ar_data__list_first(own_result);
    AR_ASSERT(ref_first != NULL, "Appended item should exist");
    AR_ASSERT(ref_first != own_value, "Appended item should be copied");
    AR_ASSERT(strcmp(ar_data__get_string(ref_first), "first") == 0,
              "Appended copy should match source value");

    // Cleanup
    ar_data__destroy(own_result);
    ar_data__destroy(own_value);
    ar_data__destroy(own_source);
}

static void test_append__invalid_inputs_return_zero(void) {
    printf("Testing pure append invalid input fallbacks...\n");

    // Given a valid value to append
    ar_data_t *own_value = ar_data__create_integer(7);
    AR_ASSERT(own_value != NULL, "Value creation should succeed");

    // When the list input is missing
    ar_data_t *own_missing_list = ar_append__create_result(NULL, own_value);
    _assert_integer_value(own_missing_list, 0);
    ar_data__destroy(own_missing_list);

    // When the list input is not a list
    ar_data_t *own_non_list = ar_data__create_string("not-list");
    AR_ASSERT(own_non_list != NULL, "Non-list creation should succeed");
    ar_data_t *own_non_list_result = ar_append__create_result(own_non_list, own_value);
    _assert_integer_value(own_non_list_result, 0);
    ar_data__destroy(own_non_list_result);
    ar_data__destroy(own_non_list);

    // When the value input is missing
    ar_data_t *own_empty = ar_data__create_list();
    AR_ASSERT(own_empty != NULL, "Empty list creation should succeed");
    ar_data_t *own_missing_value = ar_append__create_result(own_empty, NULL);
    _assert_integer_value(own_missing_value, 0);
    ar_data__destroy(own_missing_value);
    ar_data__destroy(own_empty);

    // Cleanup
    ar_data__destroy(own_value);
}

int main(void) {
    printf("Starting append value tests...\n");

    test_append__non_empty_list_returns_new_list_with_deep_copies();
    test_append__empty_list_returns_one_item_list();
    test_append__invalid_inputs_return_zero();

    printf("All append value tests passed!\n");
    return 0;
}
