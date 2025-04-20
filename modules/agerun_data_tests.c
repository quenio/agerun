#include "agerun_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_data_creation(void);
static void test_integer_values(map_t *map);
static void test_string_values(map_t *map);
static void test_nested_maps(map_t *map);

static void test_data_creation(void) {
    printf("Testing data creation for different types...\n");
    
    // Given we need to create data of different types
    
    // When we create an integer data item
    data_t int_data_default = ar_data_create(DATA_INT);
    
    // Then it should have the correct type and default value
    assert(int_data_default.type == DATA_INT);
    assert(int_data_default.data.int_value == 0);
    
    // When we create a double data item
    data_t double_data_default = ar_data_create(DATA_DOUBLE);
    
    // Then it should have the correct type and default value
    assert(double_data_default.type == DATA_DOUBLE);
    assert(double_data_default.data.double_value == 0.0);
    
    // When we create a string data item
    data_t string_data_default = ar_data_create(DATA_STRING);
    
    // Then it should have the correct type and null string
    assert(string_data_default.type == DATA_STRING);
    assert(string_data_default.data.string_value == NULL);
    
    // When we create a map data item
    data_t map_data_default = ar_data_create(DATA_MAP);
    
    // Then it should have the correct type and a valid map
    assert(map_data_default.type == DATA_MAP);
    assert(map_data_default.data.map_value != NULL);
    
    // Cleanup
    ar_data_free(&map_data_default);
    
    printf("Data creation tests passed!\n");
}

static void test_integer_values(map_t *map) {
    printf("Testing integer values in map...\n");
    
    // Given an integer data item
    data_t *int_data = malloc(sizeof(data_t));
    int_data->type = DATA_INT;
    int_data->data.int_value = 42;
    
    // When we set the reference in the map
    bool result = ar_map_set(map, "answer", int_data);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const data_t *value = (const data_t*)ar_map_get(map, "answer");
    
    // Then the value should be correctly retrieved
    assert(value != NULL);
    assert(value->type == DATA_INT);
    assert(value->data.int_value == 42);
    
    printf("Integer value tests passed!\n");
}

static void test_string_values(map_t *map) {
    printf("Testing string values in map...\n");
    
    // Given a string data item
    data_t *string_data = malloc(sizeof(data_t));
    string_data->type = DATA_STRING;
    string_data->data.string_value = strdup("Hello, World!");
    
    // When we set the reference in the map
    bool result = ar_map_set(map, "greeting", string_data);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const data_t *value = (const data_t*)ar_map_get(map, "greeting");
    
    // Then the value should be correctly retrieved
    assert(value != NULL);
    assert(value->type == DATA_STRING);
    assert(strcmp(value->data.string_value, "Hello, World!") == 0);
    
    printf("String value tests passed!\n");
}

static void test_nested_maps(map_t *map) {
    printf("Testing nested maps...\n");
    
    // Given a nested map data item
    data_t nested_map_data = ar_data_create(DATA_MAP);
    data_t *nested_map_ptr = malloc(sizeof(data_t));
    *nested_map_ptr = nested_map_data;
    
    assert(nested_map_ptr->type == DATA_MAP);
    assert(nested_map_ptr->data.map_value != NULL);
    
    // And data to add to the nested map
    data_t *nested_int_data = malloc(sizeof(data_t));
    nested_int_data->type = DATA_INT;
    nested_int_data->data.int_value = 100;
    
    // When we set a reference in the nested map
    bool result = ar_map_set(nested_map_ptr->data.map_value, "count", nested_int_data);
    
    // Then the operation should succeed
    assert(result);
    
    // When we add a reference to the nested map to the main map
    result = ar_map_set(map, "user_data", nested_map_ptr);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced nested map from the main map
    const data_t *value = (const data_t*)ar_map_get(map, "user_data");
    
    // Then the nested map should be correctly retrieved
    assert(value != NULL);
    assert(value->type == DATA_MAP);
    assert(value->data.map_value != NULL);
    
    // And its contents should be intact - retrieve the reference from inside the nested map
    const data_t *nested_value = (const data_t*)ar_map_get(value->data.map_value, "count");
    assert(nested_value != NULL);
    assert(nested_value->type == DATA_INT);
    assert(nested_value->data.int_value == 100);
    
    // Given a third level map
    data_t third_level_map = ar_data_create(DATA_MAP);
    data_t *third_level_ptr = malloc(sizeof(data_t));
    *third_level_ptr = third_level_map;
    
    data_t *deep_string = malloc(sizeof(data_t));
    deep_string->type = DATA_STRING;
    deep_string->data.string_value = strdup("Deep value!");
    
    // When we set a reference in the third level map
    result = ar_map_set(third_level_ptr->data.map_value, "key", deep_string);
    
    // Then the operation should succeed
    assert(result);
    
    // When we add a reference to the third level map in the nested map
    result = ar_map_set(value->data.map_value, "more_data", third_level_ptr);
    
    // Then the operation should succeed
    assert(result);
    
    printf("Nested maps tests passed!\n");
}

int main(void) {
    printf("Starting Data Module Tests...\n");
    
    // Given we need a map for testing
    map_t *map = ar_map_create();
    assert(map != NULL);
    // We no longer have direct access to count since map_t is opaque
    
    // When we run all data tests
    test_data_creation();
    test_integer_values(map);
    test_string_values(map);
    test_nested_maps(map);
    
    // Then all tests should pass
    printf("All data tests passed!\n");
    
    // Cleanup map resources
    ar_map_destroy(map);
    
    // Note: In this refactored version, the values need to be freed separately
    // For simplicity in this test, we're allowing some memory leaks
    // In a real application, a proper cleanup strategy would be needed
    
    return 0;
}
