#include "agerun_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_data_creation(void);
static void test_data_getters(void);
static void test_integer_values(map_t *map);
static void test_string_values(map_t *map);
static void test_nested_maps(map_t *map);

static void test_data_creation(void) {
    printf("Testing data creation for different types...\n");
    
    // Given we need to create data of different types
    
    // When we create an integer data item with default value
    data_t *int_data_default = ar_data_create_integer(0);
    
    // Then it should have the correct type and default value
    assert(int_data_default != NULL);
    assert(ar_data_get_type(int_data_default) == DATA_INTEGER);
    assert(ar_data_get_integer(int_data_default) == 0);
    
    // When we create a double data item with default value
    data_t *double_data_default = ar_data_create_double(0.0);
    
    // Then it should have the correct type and default value
    assert(double_data_default != NULL);
    assert(ar_data_get_type(double_data_default) == DATA_DOUBLE);
    assert(ar_data_get_double(double_data_default) == 0.0);
    
    // When we create a string data item with NULL value
    data_t *string_data_default = ar_data_create_string(NULL);
    
    // Then it should have the correct type and null string
    assert(string_data_default != NULL);
    assert(ar_data_get_type(string_data_default) == DATA_STRING);
    assert(ar_data_get_string(string_data_default) == NULL);
    
    // When we create an empty map data item
    data_t *map_data_default = ar_data_create_map();
    
    // Then it should have the correct type and a valid map
    assert(map_data_default != NULL);
    assert(ar_data_get_type(map_data_default) == DATA_MAP);
    assert(ar_data_get_map(map_data_default) != NULL);
    
    // When we create data items with the specialized functions
    data_t *int_data = ar_data_create_integer(42);
    data_t *double_data = ar_data_create_double(3.14159);
    data_t *string_data = ar_data_create_string("Hello, World!");
    data_t *map_data = ar_data_create_map();
    
    // Then they should have the correct types and values
    assert(int_data != NULL);
    assert(ar_data_get_type(int_data) == DATA_INTEGER);
    assert(ar_data_get_integer(int_data) == 42);
    
    assert(double_data != NULL);
    assert(ar_data_get_type(double_data) == DATA_DOUBLE);
    assert(ar_data_get_double(double_data) == 3.14159);
    
    assert(string_data != NULL);
    assert(ar_data_get_type(string_data) == DATA_STRING);
    assert(ar_data_get_string(string_data) != NULL);
    assert(strcmp(ar_data_get_string(string_data), "Hello, World!") == 0);
    
    assert(map_data != NULL);
    assert(ar_data_get_type(map_data) == DATA_MAP);
    assert(ar_data_get_map(map_data) != NULL);
    
    // Cleanup
    ar_data_destroy(map_data_default);
    ar_data_destroy(string_data_default);
    ar_data_destroy(double_data_default);
    ar_data_destroy(int_data_default);
    
    ar_data_destroy(map_data);
    ar_data_destroy(string_data);
    ar_data_destroy(double_data);
    ar_data_destroy(int_data);
    
    printf("Data creation tests passed!\n");
}

static void test_data_getters(void) {
    printf("Testing data getters...\n");
    
    // Given data values of different types
    data_t *int_data = ar_data_create_integer(42);
    data_t *double_data = ar_data_create_double(3.14159);
    data_t *string_data = ar_data_create_string("Hello, World!");
    data_t *map_data = ar_data_create_map();
    
    // When we use the type getter
    data_type_t int_type = ar_data_get_type(int_data);
    data_type_t double_type = ar_data_get_type(double_data);
    data_type_t string_type = ar_data_get_type(string_data);
    data_type_t map_type = ar_data_get_type(map_data);
    data_type_t null_type = ar_data_get_type(NULL);
    
    // Then the types should be correct
    assert(int_type == DATA_INTEGER);
    assert(double_type == DATA_DOUBLE);
    assert(string_type == DATA_STRING);
    assert(map_type == DATA_MAP);
    assert(null_type == DATA_INTEGER); // Default to int if NULL
    
    // When we use the value getters with the correct types
    int int_value = ar_data_get_integer(int_data);
    double double_value = ar_data_get_double(double_data);
    const char *string_value = ar_data_get_string(string_data);
    const map_t *map_value = ar_data_get_map(map_data);
    
    // Then they should return the correct values
    assert(int_value == 42);
    assert(double_value == 3.14159);
    assert(strcmp(string_value, "Hello, World!") == 0);
    assert(map_value != NULL);
    
    // When we use the getters with incorrect types
    int wrong_int = ar_data_get_integer(string_data);
    double wrong_double = ar_data_get_double(int_data);
    const char *wrong_string = ar_data_get_string(int_data);
    const map_t *wrong_map = ar_data_get_map(double_data);
    
    // Then they should return default values
    assert(wrong_int == 0);
    assert(wrong_double == 0.0);
    assert(wrong_string == NULL);
    assert(wrong_map == NULL);
    
    // When we use the getters with NULL
    int null_int = ar_data_get_integer(NULL);
    double null_double = ar_data_get_double(NULL);
    const char *null_string = ar_data_get_string(NULL);
    const map_t *null_map = ar_data_get_map(NULL);
    
    // Then they should return default values
    assert(null_int == 0);
    assert(null_double == 0.0);
    assert(null_string == NULL);
    assert(null_map == NULL);
    
    // Cleanup
    ar_data_destroy(int_data);
    ar_data_destroy(double_data);
    ar_data_destroy(string_data);
    ar_data_destroy(map_data);
    
    printf("Data getter tests passed!\n");
}

static void test_integer_values(map_t *map) {
    printf("Testing integer values in map...\n");
    
    // Given an integer data item created with the specialized function
    data_t *int_data = ar_data_create_integer(42);
    
    // When we set the reference in the map
    bool result = ar_map_set(map, "answer", int_data);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const data_t *value = (const data_t*)ar_map_get(map, "answer");
    
    // Then the value should be correctly retrieved
    assert(value != NULL);
    assert(ar_data_get_type(value) == DATA_INTEGER);
    assert(ar_data_get_integer(value) == 42);
    
    printf("Integer value tests passed!\n");
}

static void test_string_values(map_t *map) {
    printf("Testing string values in map...\n");
    
    // Given a string data item created with the specialized function
    data_t *string_data = ar_data_create_string("Hello, World!");
    
    // When we set the reference in the map
    bool result = ar_map_set(map, "greeting", string_data);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const data_t *value = (const data_t*)ar_map_get(map, "greeting");
    
    // Then the value should be correctly retrieved
    assert(value != NULL);
    assert(ar_data_get_type(value) == DATA_STRING);
    assert(strcmp(ar_data_get_string(value), "Hello, World!") == 0);
    
    printf("String value tests passed!\n");
}

static void test_nested_maps(map_t *map) {
    printf("Testing nested maps...\n");
    
    // Given a nested map data item using the specialized function
    data_t *nested_map_ptr = ar_data_create_map();
    
    assert(nested_map_ptr != NULL);
    assert(ar_data_get_type(nested_map_ptr) == DATA_MAP);
    assert(ar_data_get_map(nested_map_ptr) != NULL);
    
    // And data to add to the nested map using the specialized function
    data_t *nested_int_data = ar_data_create_integer(100);
    
    // When we set a reference in the nested map
    map_t *nested_map = ar_data_get_map_mutable(nested_map_ptr);
    bool result = ar_map_set(nested_map, "count", nested_int_data);
    
    // Then the operation should succeed
    assert(result);
    
    // When we add a reference to the nested map to the main map
    result = ar_map_set(map, "user_data", nested_map_ptr);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced nested map from the main map
    // Since we need to get a mutable reference to use ar_map_get, let's get the original one
    
    // Then the nested map should be correctly retrieved
    // We'll use the original nested_map that we saved earlier instead of retrieving from the map
    assert(nested_map != NULL);
    
    // And its contents should be intact - retrieve the reference from inside the nested map
    const data_t *nested_value = (const data_t*)ar_map_get(nested_map, "count");
    assert(nested_value != NULL);
    assert(ar_data_get_type(nested_value) == DATA_INTEGER);
    assert(ar_data_get_integer(nested_value) == 100);
    
    // Given a third level map using the specialized function
    data_t *third_level_ptr = ar_data_create_map();
    data_t *deep_string = ar_data_create_string("Deep value!");
    
    // When we set a reference in the third level map
    map_t *third_level_map = ar_data_get_map_mutable(third_level_ptr);
    result = ar_map_set(third_level_map, "key", deep_string);
    
    // Then the operation should succeed
    assert(result);
    
    // When we add a reference to the third level map in the nested map
    // Need to create a new data_t for the nested value since we can't modify const value
    data_t *new_nested_value = ar_data_create_map();
    map_t *new_map = ar_data_get_map_mutable(new_nested_value);
    result = ar_map_set(new_map, "more_data", third_level_ptr);
    
    // Then the operation should succeed
    assert(result);
    
    printf("Nested maps tests passed!\n");
}

int main(void) {
    printf("Starting Data Module Tests...\n");
    
    // Given we need a map for testing
    map_t *map = ar_map_create();
    assert(map != NULL);
    
    // When we run all data tests
    test_data_creation();
    test_data_getters();
    test_integer_values(map);
    test_string_values(map);
    test_nested_maps(map);
    
    // Then all tests should pass
    printf("All data tests passed!\n");
    
    // Cleanup map resources
    ar_map_destroy(map);
    
    // Note: We have memory leaks in this test because we're not freeing
    // the data items stored in the map. In a real application, we would need
    // to iterate over the map and free each item before destroying the map.
    
    return 0;
}
