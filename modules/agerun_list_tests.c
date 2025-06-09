#include "agerun_list.h"
#include "agerun_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Test creating and destroying an empty list
 */
static void test_create_destroy(void) {
    printf("Running test_create_destroy...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // When checking its properties
    // Then it should be empty and have zero items
    assert(ar__list__empty(own_list) == true);
    assert(ar__list__count(own_list) == 0);
    
    // When getting all items
    void **own_items = ar__list__items(own_list);
    
    // Then it should return NULL items and count should be zero
    assert(own_items == NULL);
    assert(ar__list__count(own_list) == 0);
    
    // Cleanup
    if (own_items) AR_HEAP_FREE(own_items);  // Free items array if not NULL
    ar__list__destroy(own_list);
    
    printf("test_create_destroy passed\n");
}

/**
 * Test adding items to the end of a list
 */
static void test_add_last(void) {
    printf("Running test_add_last...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // And some test string items
    char *own_item1 = AR_HEAP_STRDUP("item1", "Test item string");
    char *own_item2 = AR_HEAP_STRDUP("item2", "Test item string");
    char *own_item3 = AR_HEAP_STRDUP("item3", "Test item string");
    
    // When adding items to the end
    assert(ar__list__add_last(own_list, own_item1) == true);
    assert(ar__list__add_last(own_list, own_item2) == true);
    assert(ar__list__add_last(own_list, own_item3) == true);
    
    // Then the list should not be empty and have the correct count
    assert(ar__list__empty(own_list) == false);
    assert(ar__list__count(own_list) == 3);
    
    // When getting all items
    void **own_items = ar__list__items(own_list);
    size_t count = ar__list__count(own_list);
    
    // Then it should return the correct items in order
    assert(own_items != NULL);
    assert(count == 3);
    assert(strcmp((char*)own_items[0], "item1") == 0);
    assert(strcmp((char*)own_items[1], "item2") == 0);
    assert(strcmp((char*)own_items[2], "item3") == 0);
    
    // Cleanup
    AR_HEAP_FREE(own_items);
    AR_HEAP_FREE(own_item1);
    AR_HEAP_FREE(own_item2);
    AR_HEAP_FREE(own_item3);
    ar__list__destroy(own_list);
    
    printf("test_add_last passed\n");
}

/**
 * Test adding items to the beginning of a list
 */
static void test_add_first(void) {
    printf("Running test_add_first...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // And some test string items
    char *own_item1 = AR_HEAP_STRDUP("item1", "Test item string");
    char *own_item2 = AR_HEAP_STRDUP("item2", "Test item string");
    char *own_item3 = AR_HEAP_STRDUP("item3", "Test item string");
    
    // When adding items to the beginning
    assert(ar__list__add_first(own_list, own_item1) == true);
    assert(ar__list__add_first(own_list, own_item2) == true);
    assert(ar__list__add_first(own_list, own_item3) == true);
    
    // Then the list should not be empty and have the correct count
    assert(ar__list__empty(own_list) == false);
    assert(ar__list__count(own_list) == 3);
    
    // When getting all items
    void **own_items = ar__list__items(own_list);
    size_t count = ar__list__count(own_list);
    
    // Then it should return the correct items in reverse order of insertion
    assert(own_items != NULL);
    assert(count == 3);
    assert(strcmp((char*)own_items[0], "item3") == 0);
    assert(strcmp((char*)own_items[1], "item2") == 0);
    assert(strcmp((char*)own_items[2], "item1") == 0);
    
    // Cleanup
    AR_HEAP_FREE(own_items);
    AR_HEAP_FREE(own_item1);
    AR_HEAP_FREE(own_item2);
    AR_HEAP_FREE(own_item3);
    ar__list__destroy(own_list);
    
    printf("test_add_first passed\n");
}

/**
 * Test getting the first and last items of a list
 */
static void test_first_last(void) {
    printf("Running test_first_last...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // When getting first and last items from empty list
    // Then both should be NULL
    assert(ar__list__first(own_list) == NULL);
    assert(ar__list__last(own_list) == NULL);
    
    // And some test string items
    char *own_item1 = AR_HEAP_STRDUP("item1", "Test item string");
    char *own_item2 = AR_HEAP_STRDUP("item2", "Test item string");
    char *own_item3 = AR_HEAP_STRDUP("item3", "Test item string");
    
    // When adding items
    assert(ar__list__add_last(own_list, own_item1) == true);
    
    // Then first and last should be the same for a single-item list
    assert(ar__list__first(own_list) == own_item1);
    assert(ar__list__last(own_list) == own_item1);
    
    // When adding more items
    assert(ar__list__add_last(own_list, own_item2) == true);
    assert(ar__list__add_last(own_list, own_item3) == true);
    
    // Then first and last should reflect the ends of the list
    assert(ar__list__first(own_list) == own_item1);
    assert(ar__list__last(own_list) == own_item3);
    
    // Cleanup
    AR_HEAP_FREE(own_item1);
    AR_HEAP_FREE(own_item2);
    AR_HEAP_FREE(own_item3);
    ar__list__destroy(own_list);
    
    printf("test_first_last passed\n");
}

/**
 * Test removing items from a list
 */
static void test_remove_first_last(void) {
    printf("Running test_remove_first_last...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // When removing from an empty list
    // Then both operations should return NULL
    assert(ar__list__remove_first(own_list) == NULL);
    assert(ar__list__remove_last(own_list) == NULL);
    
    // And some test string items
    char *own_item1 = AR_HEAP_STRDUP("item1", "Test item string");
    char *own_item2 = AR_HEAP_STRDUP("item2", "Test item string");
    char *own_item3 = AR_HEAP_STRDUP("item3", "Test item string");
    
    // When adding items
    assert(ar__list__add_last(own_list, own_item1) == true);
    assert(ar__list__add_last(own_list, own_item2) == true);
    assert(ar__list__add_last(own_list, own_item3) == true);
    assert(ar__list__count(own_list) == 3);
    
    // When removing the first item
    void *ref_removed = ar__list__remove_first(own_list);
    
    // Then it should return the first item and update the list
    assert(ref_removed == own_item1);
    assert(ar__list__count(own_list) == 2);
    assert(ar__list__first(own_list) == own_item2);
    
    // When removing the last item
    ref_removed = ar__list__remove_last(own_list);
    
    // Then it should return the last item and update the list
    assert(ref_removed == own_item3);
    assert(ar__list__count(own_list) == 1);
    assert(ar__list__first(own_list) == own_item2);
    assert(ar__list__last(own_list) == own_item2);
    
    // When removing the only remaining item
    ref_removed = ar__list__remove_first(own_list);
    
    // Then it should return the item and empty the list
    assert(ref_removed == own_item2);
    assert(ar__list__count(own_list) == 0);
    assert(ar__list__empty(own_list) == true);
    assert(ar__list__first(own_list) == NULL);
    assert(ar__list__last(own_list) == NULL);
    
    // Cleanup
    AR_HEAP_FREE(own_item1);
    AR_HEAP_FREE(own_item2);
    AR_HEAP_FREE(own_item3);
    ar__list__destroy(own_list);
    
    printf("test_remove_first_last passed\n");
}

/**
 * Test using the list as a stack (LIFO)
 */
static void test_stack_operations(void) {
    printf("Running test_stack_operations...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // And some test string items
    char *own_item1 = AR_HEAP_STRDUP("item1", "Test item string");
    char *own_item2 = AR_HEAP_STRDUP("item2", "Test item string");
    char *own_item3 = AR_HEAP_STRDUP("item3", "Test item string");
    
    // When using add_first (push) operations
    assert(ar__list__add_first(own_list, own_item1) == true);
    assert(ar__list__add_first(own_list, own_item2) == true);
    assert(ar__list__add_first(own_list, own_item3) == true);
    
    // Then the list should be in reverse order
    assert(ar__list__count(own_list) == 3);
    assert(ar__list__first(own_list) == own_item3);
    
    // When using remove_first (pop) operations
    void *ref_popped1 = ar__list__remove_first(own_list);
    void *ref_popped2 = ar__list__remove_first(own_list);
    void *ref_popped3 = ar__list__remove_first(own_list);
    
    // Then items should be popped in LIFO order
    assert(ref_popped1 == own_item3);
    assert(ref_popped2 == own_item2);
    assert(ref_popped3 == own_item1);
    assert(ar__list__empty(own_list) == true);
    
    // Cleanup
    AR_HEAP_FREE(own_item1);
    AR_HEAP_FREE(own_item2);
    AR_HEAP_FREE(own_item3);
    ar__list__destroy(own_list);
    
    printf("test_stack_operations passed\n");
}

/**
 * Test using the list as a queue (FIFO)
 */
static void test_queue_operations(void) {
    printf("Running test_queue_operations...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // And some test string items
    char *own_item1 = AR_HEAP_STRDUP("item1", "Test item string");
    char *own_item2 = AR_HEAP_STRDUP("item2", "Test item string");
    char *own_item3 = AR_HEAP_STRDUP("item3", "Test item string");
    
    // When using add_last (enqueue) operations
    assert(ar__list__add_last(own_list, own_item1) == true);
    assert(ar__list__add_last(own_list, own_item2) == true);
    assert(ar__list__add_last(own_list, own_item3) == true);
    
    // Then the list should be in order
    assert(ar__list__count(own_list) == 3);
    assert(ar__list__first(own_list) == own_item1);
    assert(ar__list__last(own_list) == own_item3);
    
    // When using remove_first (dequeue) operations
    void *ref_dequeued1 = ar__list__remove_first(own_list);
    void *ref_dequeued2 = ar__list__remove_first(own_list);
    void *ref_dequeued3 = ar__list__remove_first(own_list);
    
    // Then items should be dequeued in FIFO order
    assert(ref_dequeued1 == own_item1);
    assert(ref_dequeued2 == own_item2);
    assert(ref_dequeued3 == own_item3);
    assert(ar__list__empty(own_list) == true);
    
    // Cleanup
    AR_HEAP_FREE(own_item1);
    AR_HEAP_FREE(own_item2);
    AR_HEAP_FREE(own_item3);
    ar__list__destroy(own_list);
    
    printf("test_queue_operations passed\n");
}

/**
 * Test adding many items to ensure linked list works properly
 */
static void test_add_many(void) {
    printf("Running test_add_many...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // And an array to track items for later verification
    #define TEST_COUNT 100
    char *own_expected_items[TEST_COUNT];
    
    // When adding many items
    for (int i = 0; i < TEST_COUNT; i++) {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "item%d", i);
        own_expected_items[i] = AR_HEAP_STRDUP(buffer, "Test item string");
        assert(ar__list__add_last(own_list, own_expected_items[i]) == true);
    }
    
    // Then the list should have the correct count
    assert(ar__list__count(own_list) == TEST_COUNT);
    
    // When getting all items
    void **own_items = ar__list__items(own_list);
    size_t count = ar__list__count(own_list);
    
    // Then it should return the correct items in order
    assert(own_items != NULL);
    assert(count == TEST_COUNT);
    
    for (int i = 0; i < TEST_COUNT; i++) {
        assert(strcmp((char*)own_items[i], own_expected_items[i]) == 0);
    }
    
    // Cleanup
    AR_HEAP_FREE(own_items);
    for (int i = 0; i < TEST_COUNT; i++) {
        AR_HEAP_FREE(own_expected_items[i]);
    }
    ar__list__destroy(own_list);
    
    printf("test_add_many passed\n");
}

/**
 * Test handling of NULL parameters
 */
static void test_null_parameters(void) {
    printf("Running test_null_parameters...\n");
    
    // Given NULL parameters
    // When calling functions with NULL parameters
    // Then they should handle them gracefully
    assert(ar__list__count(NULL) == 0);
    assert(ar__list__empty(NULL) == true);
    assert(ar__list__add_last(NULL, NULL) == false);
    assert(ar__list__add_first(NULL, NULL) == false);
    assert(ar__list__first(NULL) == NULL);
    assert(ar__list__last(NULL) == NULL);
    assert(ar__list__remove_first(NULL) == NULL);
    assert(ar__list__remove_last(NULL) == NULL);
    
    // Create a valid list for testing NULL items
    list_t *own_list = ar__list__create();
    assert(ar__list__add_last(own_list, NULL) == true);  // NULL items are allowed
    assert(ar__list__count(own_list) == 1);
    assert(ar__list__first(own_list) == NULL);
    assert(ar__list__last(own_list) == NULL);
    
    // Test getting items with NULL list
    void **own_items = ar__list__items(NULL);
    assert(own_items == NULL);
    
    // Cleanup
    ar__list__destroy(own_list);
    ar__list__destroy(NULL);  // Should not crash
    
    printf("test_null_parameters passed\n");
}

/**
 * Test removing specific items from a list
 */
static void test_remove(void) {
    printf("Running test_remove...\n");
    
    // Given an empty list
    list_t *own_list = ar__list__create();
    assert(own_list != NULL);
    
    // When trying to remove from an empty list
    // Then it should return NULL
    assert(ar__list__remove(own_list, NULL) == NULL);
    
    // And some test string items
    char *own_item1 = AR_HEAP_STRDUP("item1", "Test item string");
    char *own_item2 = AR_HEAP_STRDUP("item2", "Test item string");
    char *own_item3 = AR_HEAP_STRDUP("item3", "Test item string");
    char *own_item4 = AR_HEAP_STRDUP("item4", "Test item string");
    char *own_item5 = AR_HEAP_STRDUP("item5", "Test item string");
    
    // And a list with items
    assert(ar__list__add_last(own_list, own_item1) == true);
    assert(ar__list__add_last(own_list, own_item2) == true);
    assert(ar__list__add_last(own_list, own_item3) == true);
    assert(ar__list__add_last(own_list, own_item4) == true);
    assert(ar__list__add_last(own_list, own_item5) == true);
    assert(ar__list__count(own_list) == 5);
    
    // When removing an item from the middle
    // Then it should return the item and update the list
    void *removed_item = ar__list__remove(own_list, own_item3);
    assert(removed_item == own_item3);
    assert(ar__list__count(own_list) == 4);
    
    // And the items should be in the correct order
    void **own_items = ar__list__items(own_list);
    assert(own_items != NULL);
    assert(own_items[0] == own_item1);
    assert(own_items[1] == own_item2);
    assert(own_items[2] == own_item4);
    assert(own_items[3] == own_item5);
    AR_HEAP_FREE(own_items);
    
    // When removing the first item
    // Then it should return the item and update the list
    void *removed_item2 = ar__list__remove(own_list, own_item1);
    assert(removed_item2 == own_item1);
    assert(ar__list__count(own_list) == 3);
    
    // And the list should be updated correctly
    own_items = ar__list__items(own_list);
    assert(own_items != NULL);
    assert(own_items[0] == own_item2);
    assert(own_items[1] == own_item4);
    assert(own_items[2] == own_item5);
    AR_HEAP_FREE(own_items);
    
    // When removing the last item
    // Then it should return the item and update the list
    void *removed_item3 = ar__list__remove(own_list, own_item5);
    assert(removed_item3 == own_item5);
    assert(ar__list__count(own_list) == 2);
    
    // And the list should be updated correctly
    own_items = ar__list__items(own_list);
    assert(own_items != NULL);
    assert(own_items[0] == own_item2);
    assert(own_items[1] == own_item4);
    AR_HEAP_FREE(own_items);
    
    // When removing an item that doesn't exist in the list
    // Then it should return NULL and not modify the list
    char *own_non_existent = AR_HEAP_STRDUP("non_existent", "Test non-existent item");
    assert(ar__list__remove(own_list, own_non_existent) == NULL);
    assert(ar__list__count(own_list) == 2);
    AR_HEAP_FREE(own_non_existent);
    
    // When adding a duplicate item
    assert(ar__list__add_last(own_list, own_item2) == true);
    assert(ar__list__count(own_list) == 3);
    
    // And removing the duplicated item
    // Then it should remove the first occurrence and return the item
    void *removed_item4 = ar__list__remove(own_list, own_item2);
    assert(removed_item4 == own_item2);
    assert(ar__list__count(own_list) == 2);
    
    // And the list should contain the remaining items
    assert(ar__list__first(own_list) == own_item4);
    assert(ar__list__last(own_list) == own_item2);
    
    // Cleanup
    AR_HEAP_FREE(own_item1);
    AR_HEAP_FREE(own_item2);
    AR_HEAP_FREE(own_item3);
    AR_HEAP_FREE(own_item4);
    AR_HEAP_FREE(own_item5);
    ar__list__destroy(own_list);
    
    printf("test_remove passed\n");
}

/**
 * Main test runner
 */
int main(void) {
    printf("Running agerun_list tests...\n");
    
    test_create_destroy();
    test_add_last();
    test_add_first();
    test_first_last();
    test_remove_first_last();
    test_stack_operations();
    test_queue_operations();
    test_add_many();
    test_null_parameters();
    test_remove();
    
    printf("All agerun_list tests passed!\n");
    return 0;
}
