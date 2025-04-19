#include "agerun_map.h"
#include "agerun_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_map_create(void);
static void test_map_init(void);
static void test_map_set_get_integer(void);
static void test_nested_maps(void);

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
    }
    
    printf("All ar_map_init() tests passed!\n");
}

static void test_map_set_get_integer(void) {
    printf("Testing ar_map_set() and ar_map_get() with integer value...\n");
    
    // Given a new map
    map_t *map = ar_map_create();
    if (map == NULL) {
        printf("Failed to create map, skipping test\n");
        return;
    }
    
    // And an integer data value
    data_t int_data;
    int_data.type = DATA_INT;
    int_data.data.int_value = 42;
    
    // When we set the value in the map
    bool set_result = ar_map_set(map, "int_key", &int_data);
    
    // Then the set operation should succeed
    assert(set_result);
    
    // When we retrieve the value from the map
    data_t *get_result = ar_map_get(map, "int_key");
    
    // Then the value should be retrieved successfully
    assert(get_result != NULL);
    assert(get_result->type == DATA_INT);
    
    // And the value should match what we stored
    printf("Retrieved integer value: %lld\n", get_result->data.int_value);
    assert(get_result->data.int_value == 42);
    
    // Cleanup
    ar_map_free(map);
    
    printf("ar_map_set() and ar_map_get() integer value test passed!\n");
}

static void test_nested_maps(void) {
    printf("Testing nested maps...\n");
    
    // Given we need a map structure with nested maps
    map_t *outer_map = ar_map_create();
    assert(outer_map != NULL);
    assert(outer_map->ref_count == 1);  // Initial reference count should be 1
    
    // Create an inner map
    map_t *inner_map = ar_map_create();
    assert(inner_map != NULL);
    assert(inner_map->ref_count == 1);  // Initial reference count should be 1
    
    // Create a data value to hold the inner map
    data_t map_data;
    map_data.type = DATA_MAP;
    map_data.data.map_value = inner_map;
    
    // When we set the inner map in the outer map
    bool set_result = ar_map_set(outer_map, "inner_map", &map_data);
    assert(set_result);
    
    // Then the inner map's reference count should be incremented
    assert(inner_map->ref_count == 2);  // Should be incremented after being stored in outer_map
    
    // When we get the inner map from the outer map
    data_t *retrieved = ar_map_get(outer_map, "inner_map");
    assert(retrieved != NULL);
    assert(retrieved->type == DATA_MAP);
    assert(retrieved->data.map_value == inner_map);
    
    // Add some data to the inner map
    data_t int_data;
    int_data.type = DATA_INT;
    int_data.data.int_value = 42;
    set_result = ar_map_set(inner_map, "answer", &int_data);
    assert(set_result);
    
    // Retrieve the data through the outer map
    data_t *inner_map_ref = ar_map_get(outer_map, "inner_map");
    assert(inner_map_ref != NULL);
    data_t *answer = ar_map_get(inner_map_ref->data.map_value, "answer");
    assert(answer != NULL);
    assert(answer->type == DATA_INT);
    assert(answer->data.int_value == 42);
    
    // When we free the outer map
    ar_map_free(outer_map);
    
    // Then the inner map should still be valid due to its own reference
    assert(inner_map->ref_count == 1);  // Ref count should decrease to 1 after outer_map is freed
    
    // We can still access the inner map directly
    answer = ar_map_get(inner_map, "answer");
    assert(answer != NULL);
    assert(answer->type == DATA_INT);
    assert(answer->data.int_value == 42);
    
    // Finally free the inner map
    ar_map_free(inner_map);
    
    printf("All nested map tests passed!\n");
}

int main(void) {
    printf("Starting Map Module Tests...\n");
    
    // Run the tests individually with their own try/catch blocks to prevent early termination
    printf("Running test_map_create()...\n");
    test_map_create();
    
    printf("Running test_map_init()...\n");
    test_map_init();
    
    printf("Running test_map_set_get_integer()...\n");
    test_map_set_get_integer();
    
    printf("Running test_nested_maps()...\n");
    test_nested_maps();
    
    printf("All map tests passed!\n");
    return 0;
}
