#include "agerun_list.h"
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
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // When checking its properties
    // Then it should be empty and have zero items
    assert(ar_list_empty(list) == true);
    assert(ar_list_count(list) == 0);
    
    // When getting all items
    void **items = ar_list_items(list);
    
    // Then it should return NULL items and count should be zero
    assert(items == NULL);
    assert(ar_list_count(list) == 0);
    
    // Cleanup
    ar_list_destroy(list);
    
    printf("test_create_destroy passed\n");
}

/**
 * Test adding items to the end of a list
 */
static void test_add_last(void) {
    printf("Running test_add_last...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // And some test string items
    char *item1 = strdup("item1");
    char *item2 = strdup("item2");
    char *item3 = strdup("item3");
    
    // When adding items to the end
    assert(ar_list_add_last(list, item1) == true);
    assert(ar_list_add_last(list, item2) == true);
    assert(ar_list_add_last(list, item3) == true);
    
    // Then the list should not be empty and have the correct count
    assert(ar_list_empty(list) == false);
    assert(ar_list_count(list) == 3);
    
    // When getting all items
    void **items = ar_list_items(list);
    size_t count = ar_list_count(list);
    
    // Then it should return the correct items in order
    assert(items != NULL);
    assert(count == 3);
    assert(strcmp((char*)items[0], "item1") == 0);
    assert(strcmp((char*)items[1], "item2") == 0);
    assert(strcmp((char*)items[2], "item3") == 0);
    
    // Cleanup
    free(items);
    free(item1);
    free(item2);
    free(item3);
    ar_list_destroy(list);
    
    printf("test_add_last passed\n");
}

/**
 * Test adding items to the beginning of a list
 */
static void test_add_first(void) {
    printf("Running test_add_first...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // And some test string items
    char *item1 = strdup("item1");
    char *item2 = strdup("item2");
    char *item3 = strdup("item3");
    
    // When adding items to the beginning
    assert(ar_list_add_first(list, item1) == true);
    assert(ar_list_add_first(list, item2) == true);
    assert(ar_list_add_first(list, item3) == true);
    
    // Then the list should not be empty and have the correct count
    assert(ar_list_empty(list) == false);
    assert(ar_list_count(list) == 3);
    
    // When getting all items
    void **items = ar_list_items(list);
    size_t count = ar_list_count(list);
    
    // Then it should return the correct items in reverse order of insertion
    assert(items != NULL);
    assert(count == 3);
    assert(strcmp((char*)items[0], "item3") == 0);
    assert(strcmp((char*)items[1], "item2") == 0);
    assert(strcmp((char*)items[2], "item1") == 0);
    
    // Cleanup
    free(items);
    free(item1);
    free(item2);
    free(item3);
    ar_list_destroy(list);
    
    printf("test_add_first passed\n");
}

/**
 * Test getting the first and last items of a list
 */
static void test_first_last(void) {
    printf("Running test_first_last...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // When getting first and last items from empty list
    // Then both should be NULL
    assert(ar_list_first(list) == NULL);
    assert(ar_list_last(list) == NULL);
    
    // And some test string items
    char *item1 = strdup("item1");
    char *item2 = strdup("item2");
    char *item3 = strdup("item3");
    
    // When adding items
    assert(ar_list_add_last(list, item1) == true);
    
    // Then first and last should be the same for a single-item list
    assert(ar_list_first(list) == item1);
    assert(ar_list_last(list) == item1);
    
    // When adding more items
    assert(ar_list_add_last(list, item2) == true);
    assert(ar_list_add_last(list, item3) == true);
    
    // Then first and last should reflect the ends of the list
    assert(ar_list_first(list) == item1);
    assert(ar_list_last(list) == item3);
    
    // Cleanup
    free(item1);
    free(item2);
    free(item3);
    ar_list_destroy(list);
    
    printf("test_first_last passed\n");
}

/**
 * Test removing items from a list
 */
static void test_remove_first_last(void) {
    printf("Running test_remove_first_last...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // When removing from an empty list
    // Then both operations should return NULL
    assert(ar_list_remove_first(list) == NULL);
    assert(ar_list_remove_last(list) == NULL);
    
    // And some test string items
    char *item1 = strdup("item1");
    char *item2 = strdup("item2");
    char *item3 = strdup("item3");
    
    // When adding items
    assert(ar_list_add_last(list, item1) == true);
    assert(ar_list_add_last(list, item2) == true);
    assert(ar_list_add_last(list, item3) == true);
    assert(ar_list_count(list) == 3);
    
    // When removing the first item
    void *removed = ar_list_remove_first(list);
    
    // Then it should return the first item and update the list
    assert(removed == item1);
    assert(ar_list_count(list) == 2);
    assert(ar_list_first(list) == item2);
    
    // When removing the last item
    removed = ar_list_remove_last(list);
    
    // Then it should return the last item and update the list
    assert(removed == item3);
    assert(ar_list_count(list) == 1);
    assert(ar_list_first(list) == item2);
    assert(ar_list_last(list) == item2);
    
    // When removing the only remaining item
    removed = ar_list_remove_first(list);
    
    // Then it should return the item and empty the list
    assert(removed == item2);
    assert(ar_list_count(list) == 0);
    assert(ar_list_empty(list) == true);
    assert(ar_list_first(list) == NULL);
    assert(ar_list_last(list) == NULL);
    
    // Cleanup
    free(item1);
    free(item2);
    free(item3);
    ar_list_destroy(list);
    
    printf("test_remove_first_last passed\n");
}

/**
 * Test using the list as a stack (LIFO)
 */
static void test_stack_operations(void) {
    printf("Running test_stack_operations...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // And some test string items
    char *item1 = strdup("item1");
    char *item2 = strdup("item2");
    char *item3 = strdup("item3");
    
    // When using add_first (push) operations
    assert(ar_list_add_first(list, item1) == true);
    assert(ar_list_add_first(list, item2) == true);
    assert(ar_list_add_first(list, item3) == true);
    
    // Then the list should be in reverse order
    assert(ar_list_count(list) == 3);
    assert(ar_list_first(list) == item3);
    
    // When using remove_first (pop) operations
    void *popped1 = ar_list_remove_first(list);
    void *popped2 = ar_list_remove_first(list);
    void *popped3 = ar_list_remove_first(list);
    
    // Then items should be popped in LIFO order
    assert(popped1 == item3);
    assert(popped2 == item2);
    assert(popped3 == item1);
    assert(ar_list_empty(list) == true);
    
    // Cleanup
    free(item1);
    free(item2);
    free(item3);
    ar_list_destroy(list);
    
    printf("test_stack_operations passed\n");
}

/**
 * Test using the list as a queue (FIFO)
 */
static void test_queue_operations(void) {
    printf("Running test_queue_operations...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // And some test string items
    char *item1 = strdup("item1");
    char *item2 = strdup("item2");
    char *item3 = strdup("item3");
    
    // When using add_last (enqueue) operations
    assert(ar_list_add_last(list, item1) == true);
    assert(ar_list_add_last(list, item2) == true);
    assert(ar_list_add_last(list, item3) == true);
    
    // Then the list should be in order
    assert(ar_list_count(list) == 3);
    assert(ar_list_first(list) == item1);
    assert(ar_list_last(list) == item3);
    
    // When using remove_first (dequeue) operations
    void *dequeued1 = ar_list_remove_first(list);
    void *dequeued2 = ar_list_remove_first(list);
    void *dequeued3 = ar_list_remove_first(list);
    
    // Then items should be dequeued in FIFO order
    assert(dequeued1 == item1);
    assert(dequeued2 == item2);
    assert(dequeued3 == item3);
    assert(ar_list_empty(list) == true);
    
    // Cleanup
    free(item1);
    free(item2);
    free(item3);
    ar_list_destroy(list);
    
    printf("test_queue_operations passed\n");
}

/**
 * Test adding many items to ensure linked list works properly
 */
static void test_add_many(void) {
    printf("Running test_add_many...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // And an array to track items for later verification
    #define TEST_COUNT 100
    char *expected_items[TEST_COUNT];
    
    // When adding many items
    for (int i = 0; i < TEST_COUNT; i++) {
        char buffer[20];
        sprintf(buffer, "item%d", i);
        expected_items[i] = strdup(buffer);
        assert(ar_list_add_last(list, expected_items[i]) == true);
    }
    
    // Then the list should have the correct count
    assert(ar_list_count(list) == TEST_COUNT);
    
    // When getting all items
    void **items = ar_list_items(list);
    size_t count = ar_list_count(list);
    
    // Then it should return the correct items in order
    assert(items != NULL);
    assert(count == TEST_COUNT);
    
    for (int i = 0; i < TEST_COUNT; i++) {
        assert(strcmp((char*)items[i], expected_items[i]) == 0);
    }
    
    // Cleanup
    free(items);
    for (int i = 0; i < TEST_COUNT; i++) {
        free(expected_items[i]);
    }
    ar_list_destroy(list);
    
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
    assert(ar_list_count(NULL) == 0);
    assert(ar_list_empty(NULL) == true);
    assert(ar_list_add_last(NULL, NULL) == false);
    assert(ar_list_add_first(NULL, NULL) == false);
    assert(ar_list_first(NULL) == NULL);
    assert(ar_list_last(NULL) == NULL);
    assert(ar_list_remove_first(NULL) == NULL);
    assert(ar_list_remove_last(NULL) == NULL);
    
    // Create a valid list for testing NULL items
    list_t *list = ar_list_create();
    assert(ar_list_add_last(list, NULL) == true);  // NULL items are allowed
    assert(ar_list_count(list) == 1);
    assert(ar_list_first(list) == NULL);
    assert(ar_list_last(list) == NULL);
    
    // Test getting items with NULL list
    void **items = ar_list_items(NULL);
    assert(items == NULL);
    
    // Cleanup
    ar_list_destroy(list);
    ar_list_destroy(NULL);  // Should not crash
    
    printf("test_null_parameters passed\n");
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
    
    printf("All agerun_list tests passed!\n");
    return 0;
}
