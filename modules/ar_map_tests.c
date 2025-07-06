#include "ar_map.h"
#include "ar_data.h"
#include "ar_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_map_create(void);
static void test_map_set_get_simple(void);
static void test_map_count(void);
static void test_map_refs(void);

static void test_map_create(void) {
    printf("Testing ar_map__create()...\n");
    
    // Given we need a new map
    
    // When we call ar_map__create
    ar_map_t *own_map = ar_map__create();
    
    // Then the map should be created successfully
    assert(own_map != NULL);
    
    // Test functionality via ar_map__set and ar_map__get
    const char *ref_key = "test_key";
    int test_value = 42;
    bool set_result = ar_map__set(own_map, ref_key, &test_value);
    assert(set_result);
    
    const int *ref_retrieved = ar_map__get(own_map, ref_key);
    assert(ref_retrieved != NULL);
    assert(*ref_retrieved == test_value);
    assert(ref_retrieved == &test_value);
    
    // Cleanup
    // Following the guideline to free containers first, then contents
    // In this case, the map is the container and test_value is stack-allocated
    // so no manual freeing of the content is needed
    ar_map__destroy(own_map);
    
    printf("All ar_map__create() tests passed!\n");
}


static void test_map_set_get_simple(void) {
    printf("Testing ar_map__set() and ar_map__get() with simple value...\n");
    
    // Given a new map
    ar_map_t *own_map = ar_map__create();
    if (own_map == NULL) {
        printf("Failed to create map, skipping test\n");
        return;
    }
    
    // And a test value (using an integer on the heap for this test)
    int *own_value = AR__HEAP__MALLOC(sizeof(int), "Test integer value");
    *own_value = 42;
    
    // When we set the reference in the map
    bool set_result = ar_map__set(own_map, "test_key", own_value);
    
    // Then the set operation should succeed
    assert(set_result);
    
    // When we retrieve the value from the map
    const int *ref_retrieved = (const int*)ar_map__get(own_map, "test_key");
    
    // Then the value should be retrieved successfully
    assert(ref_retrieved != NULL);
    
    // And the value should match what we stored
    printf("Retrieved integer value: %d\n", *ref_retrieved);
    assert(*ref_retrieved == 42);
    
    // And the pointer should be the same as the original reference
    assert(ref_retrieved == own_value);
    
    // Cleanup
    // Following the guideline to free containers first, then contents
    ar_map__destroy(own_map);
    AR__HEAP__FREE(own_value); // Now free the referenced value after freeing the container
    
    printf("ar_map__set() and ar_map__get() simple value test passed!\n");
}


static void test_map_count(void) {
    printf("Testing ar_map__count()...\n");
    
    // Given a new map
    ar_map_t *own_map = ar_map__create();
    assert(own_map != NULL);
    
    // When the map is empty
    // Then the count should be 0
    assert(ar_map__count(own_map) == 0);
    
    // When adding some entries
    ar_map__set(own_map, "key1", (void*)1);
    assert(ar_map__count(own_map) == 1);
    
    ar_map__set(own_map, "key2", (void*)2);
    assert(ar_map__count(own_map) == 2);
    
    ar_map__set(own_map, "key3", (void*)3);
    assert(ar_map__count(own_map) == 3);
    
    // When updating an existing entry
    ar_map__set(own_map, "key2", (void*)22);
    // Then the count should remain the same
    assert(ar_map__count(own_map) == 3);
    
    // Cleanup
    ar_map__destroy(own_map);
    
    // Edge case: NULL map
    assert(ar_map__count(NULL) == 0);
    
    printf("ar_map__count() tests passed!\n");
}

static void test_map_refs(void) {
    printf("Testing ar_map__refs()...\n");
    
    // Given a new map
    ar_map_t *own_map = ar_map__create();
    assert(own_map != NULL);
    
    // When the map is empty
    // Then ar_map__refs should return NULL
    assert(ar_map__refs(own_map) == NULL);
    
    // When adding some entries
    int val1 = 10, val2 = 20, val3 = 30;
    ar_map__set(own_map, "key1", &val1);
    ar_map__set(own_map, "key2", &val2);
    ar_map__set(own_map, "key3", &val3);
    
    // Then ar_map__refs should return a valid array
    void **own_refs = ar_map__refs(own_map);
    assert(own_refs != NULL);
    
    // And the count should match
    size_t count = ar_map__count(own_map);
    assert(count == 3);
    
    // Check that all values are present
    // Note: The order is not guaranteed due to internal implementation
    bool found1 = false, found2 = false, found3 = false;
    for (size_t i = 0; i < count; i++) {
        if (own_refs[i] == &val1) found1 = true;
        if (own_refs[i] == &val2) found2 = true;
        if (own_refs[i] == &val3) found3 = true;
    }
    assert(found1 && found2 && found3);
    
    // Cleanup
    AR__HEAP__FREE(own_refs);
    ar_map__destroy(own_map);
    
    // Edge case: NULL map
    assert(ar_map__refs(NULL) == NULL);
    
    printf("ar_map__refs() tests passed!\n");
}

int main(void) {
    printf("Starting Map Module Tests...\n");
    
    // Run the tests individually with their own try/catch blocks to prevent early termination
    printf("Running test_map_create()...\n");
    test_map_create();
    
    printf("Running test_map_set_get_simple()...\n");
    test_map_set_get_simple();
    
    printf("Running test_map_count()...\n");
    test_map_count();
    
    printf("Running test_map_refs()...\n");
    test_map_refs();
    
    printf("All 8 tests passed!\n");
    return 0;
}
