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
 * Test appending items to a list
 */
static void test_append(void) {
    printf("Running test_append...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // And some test string items
    char *item1 = strdup("item1");
    char *item2 = strdup("item2");
    char *item3 = strdup("item3");
    
    // When appending items
    assert(ar_list_append(list, item1) == true);
    assert(ar_list_append(list, item2) == true);
    assert(ar_list_append(list, item3) == true);
    
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
    
    printf("test_append passed\n");
}

/**
 * Test appending many items to ensure linked list works properly
 */
static void test_append_many(void) {
    printf("Running test_append_many...\n");
    
    // Given an empty list
    list_t *list = ar_list_create();
    assert(list != NULL);
    
    // And an array to track items for later verification
    #define TEST_COUNT 100
    char *expected_items[TEST_COUNT];
    
    // When appending many items
    for (int i = 0; i < TEST_COUNT; i++) {
        char buffer[20];
        sprintf(buffer, "item%d", i);
        expected_items[i] = strdup(buffer);
        assert(ar_list_append(list, expected_items[i]) == true);
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
    
    printf("test_append_many passed\n");
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
    assert(ar_list_append(NULL, NULL) == false);
    
    // Create a valid list for testing NULL items
    list_t *list = ar_list_create();
    assert(ar_list_append(list, NULL) == true);  // NULL items are allowed
    assert(ar_list_count(list) == 1);
    
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
    test_append();
    test_append_many();
    test_null_parameters();
    
    printf("All agerun_list tests passed!\n");
    return 0;
}
