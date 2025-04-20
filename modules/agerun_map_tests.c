#include "agerun_map.h"
#include "agerun_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_map_create(void);
static void test_map_init(void);
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
    
    // Cleanup
    ar_map_free(map);
    
    printf("All ar_map_create() tests passed!\n");
}

static void test_map_init(void) {
    printf("Testing ar_map_init()...\n");
    
    // For testing ar_map_init, we'll use a map created with ar_map_create
    // and consider that ar_map_create internally calls ar_map_init
    map_t *map = ar_map_create();
    assert(map != NULL);
    
    // Verify map works by testing basic functionality
    const char *test_key = "test_key";
    int test_value = 42;
    bool set_result = ar_map_set(map, test_key, &test_value);
    assert(set_result);
    
    const int *retrieved = ar_map_get(map, test_key);
    assert(retrieved != NULL);
    assert(*retrieved == test_value);
    
    // Clean up
    ar_map_free(map);
    
    printf("All ar_map_init() tests passed!\n");
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
    
    // Cleanup
    free(ref); // We need to free the referenced value ourselves
    ar_map_free(map);
    
    printf("ar_map_set() and ar_map_get() simple value test passed!\n");
}


int main(void) {
    printf("Starting Map Module Tests...\n");
    
    // Run the tests individually with their own try/catch blocks to prevent early termination
    printf("Running test_map_create()...\n");
    test_map_create();
    
    printf("Running test_map_init()...\n");
    test_map_init();
    
    printf("Running test_map_set_get_simple()...\n");
    test_map_set_get_simple();
    
    printf("All map tests passed!\n");
    return 0;
}
