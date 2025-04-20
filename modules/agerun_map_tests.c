#include "agerun_map.h"
#include "agerun_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_map_create(void);
static void test_map_set_get_simple(void);

static void test_map_create(void) {
    printf("Testing ar_map_create()...\n");
    
    // Given we need a new map
    
    // When we call ar_map_create
    map_t *map = ar_map_create();
    
    // Then the map should be created successfully
    assert(map != NULL);
    
    // Test functionality via ar_map_set and ar_map_get
    const char *test_key = "test_key";
    int test_value = 42;
    bool set_result = ar_map_set(map, test_key, &test_value);
    assert(set_result);
    
    const int *retrieved = ar_map_get(map, test_key);
    assert(retrieved != NULL);
    assert(*retrieved == test_value);
    assert(retrieved == &test_value);
    
    // Cleanup
    // Following the guideline to free containers first, then contents
    // In this case, the map is the container and test_value is stack-allocated
    // so no manual freeing of the content is needed
    ar_map_destroy(map);
    
    printf("All ar_map_create() tests passed!\n");
}


static void test_map_set_get_simple(void) {
    printf("Testing ar_map_set() and ar_map_get() with simple value...\n");
    
    // Given a new map
    map_t *map = ar_map_create();
    if (map == NULL) {
        printf("Failed to create map, skipping test\n");
        return;
    }
    
    // And a test value (using an integer on the heap for this test)
    int *ref = malloc(sizeof(int));
    *ref = 42;
    
    // When we set the reference in the map
    bool set_result = ar_map_set(map, "test_key", ref);
    
    // Then the set operation should succeed
    assert(set_result);
    
    // When we retrieve the value from the map
    const int *get_result = (const int*)ar_map_get(map, "test_key");
    
    // Then the value should be retrieved successfully
    assert(get_result != NULL);
    
    // And the value should match what we stored
    printf("Retrieved integer value: %d\n", *get_result);
    assert(*get_result == 42);
    
    // And the pointer should be the same as the original reference
    assert(get_result == ref);
    
    // Cleanup
    // Following the guideline to free containers first, then contents
    ar_map_destroy(map);
    free(ref); // Now free the referenced value after freeing the container
    
    printf("ar_map_set() and ar_map_get() simple value test passed!\n");
}


int main(void) {
    printf("Starting Map Module Tests...\n");
    
    // Run the tests individually with their own try/catch blocks to prevent early termination
    printf("Running test_map_create()...\n");
    test_map_create();
    
    
    printf("Running test_map_set_get_simple()...\n");
    test_map_set_get_simple();
    
    printf("All map tests passed!\n");
    return 0;
}
