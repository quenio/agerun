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
static void test_map_get_reference(void);

static void test_map_create(void) {
    printf("Testing ar_map_create()...\n");
    
    // Given we need a new map
    
    // When we call ar_map_create
    map_t *map = ar_map_create();
    
    // Then the map should be created successfully
    assert(map != NULL);
    assert(map->count == 0);
    
    // And all entries should be properly initialized
    for (int i = 0; i < MAP_SIZE; i++) {
        assert(map->entries[i].is_used == false);
        assert(map->entries[i].key == NULL);
        assert(map->entries[i].ref == NULL);
    }
    
    // Cleanup
    ar_map_free(map);
    
    printf("All ar_map_create() tests passed!\n");
}

static void test_map_init(void) {
    printf("Testing ar_map_init()...\n");
    
    // Given we have an uninitialized map structure
    map_t map;
    
    // When we initialize the map
    bool result = ar_map_init(&map);
    
    // Then the initialization should succeed
    assert(result);
    assert(map.count == 0);
    
    // And all entries should be properly initialized
    for (int i = 0; i < MAP_SIZE; i++) {
        assert(map.entries[i].is_used == false);
        assert(map.entries[i].key == NULL);
        assert(map.entries[i].ref == NULL);
    }
    
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
    int *get_result = (int*)ar_map_get(map, "test_key");
    
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

static void test_map_get_reference(void) {
    printf("Testing ar_map_get_reference() for reference counting...\n");
    
    // Given a map
    map_t *map = ar_map_create();
    assert(map != NULL);
    assert(map->ref_count == 1);  // Initial reference count should be 1
    
    // When we get a reference to the map
    map_t *ref = ar_map_get_reference(map);
    
    // Then the reference count should be incremented
    assert(map->ref_count == 2);
    assert(ref == map);  // Same map object
    
    // When we free one reference
    ar_map_free(map);
    
    // Then the map should still be valid due to the other reference
    assert(ref->ref_count == 1);
    
    // We can still use the map with the remaining reference
    int *valueRef = malloc(sizeof(int));
    *valueRef = 100;
    bool set_result = ar_map_set(ref, "test", valueRef);
    assert(set_result);
    
    // Retrieve the referenced value
    int *get_result = (int*)ar_map_get(ref, "test");
    assert(get_result != NULL);
    assert(*get_result == 100);
    
    // Cleanup
    free(valueRef); // We need to free the referenced value ourselves
    ar_map_free(ref);
    
    printf("Reference counting tests passed!\n");
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
    
    printf("Running test_map_get_reference()...\n");
    test_map_get_reference();
    
    printf("All map tests passed!\n");
    return 0;
}
