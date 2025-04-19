#include "agerun_data.h"
#include "agerun_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("Starting Data Module Tests...\n");
    
    // Test 1: Create a map
    map_t *map = ar_map_create();
    assert(map != NULL);
    assert(map->count == 0);
    
    // Test data creation for different types
    data_t int_data_default = ar_data_create(DATA_INT);
    assert(int_data_default.type == DATA_INT);
    assert(int_data_default.data.int_value == 0);
    
    data_t double_data_default = ar_data_create(DATA_DOUBLE);
    assert(double_data_default.type == DATA_DOUBLE);
    assert(double_data_default.data.double_value == 0.0);
    
    data_t string_data_default = ar_data_create(DATA_STRING);
    assert(string_data_default.type == DATA_STRING);
    assert(string_data_default.data.string_value == NULL);
    
    data_t map_data_default = ar_data_create(DATA_MAP);
    assert(map_data_default.type == DATA_MAP);
    assert(map_data_default.data.map_value != NULL);
    
    // Test 2: Set and get primitive values
    data_t int_data;
    int_data.type = DATA_INT;
    int_data.data.int_value = 42;
    
    bool result = ar_map_set(map, "answer", &int_data);
    assert(result);
    
    data_t *value = ar_map_get(map, "answer");
    assert(value != NULL);
    assert(value->type == DATA_INT);
    assert(value->data.int_value == 42);
    
    // Test 3: Set and get string values
    data_t string_data;
    string_data.type = DATA_STRING;
    string_data.data.string_value = strdup("Hello, World!");
    
    result = ar_map_set(map, "greeting", &string_data);
    assert(result);
    // We should NOT free the string here as ar_map_set now uses references
    
    value = ar_map_get(map, "greeting");
    assert(value != NULL);
    assert(value->type == DATA_STRING);
    assert(strcmp(value->data.string_value, "Hello, World!") == 0);
    
    // Test 4: Nested map
    data_t nested_map_data = ar_data_create(DATA_MAP);
    assert(nested_map_data.type == DATA_MAP);
    assert(nested_map_data.data.map_value != NULL);
    
    // Add some data to the nested map
    data_t nested_int_data;
    nested_int_data.type = DATA_INT;
    nested_int_data.data.int_value = 100;
    
    result = ar_map_set(nested_map_data.data.map_value, "count", &nested_int_data);
    assert(result);
    
    // Add the nested map to the main map
    result = ar_map_set(map, "user_data", &nested_map_data);
    assert(result);
    
    // Get the nested map and verify its contents
    value = ar_map_get(map, "user_data");
    assert(value != NULL);
    assert(value->type == DATA_MAP);
    assert(value->data.map_value != NULL);
    
    data_t *nested_value = ar_map_get(value->data.map_value, "count");
    assert(nested_value != NULL);
    assert(nested_value->type == DATA_INT);
    assert(nested_value->data.int_value == 100);
    
    // The nested_map_data now is shared between the map and the value that was set
    
    // Nested a third level deep
    data_t third_level_map = ar_data_create(DATA_MAP);
    data_t deep_string;
    deep_string.type = DATA_STRING;
    deep_string.data.string_value = strdup("Deep value!");
    
    result = ar_map_set(third_level_map.data.map_value, "key", &deep_string);
    assert(result);
    // Do not free deep_string.data.string_value as ar_map_set now uses references
    
    // Add third level to the nested map
    result = ar_map_set(value->data.map_value, "more_data", &third_level_map);
    assert(result);
    
    // Now we have map->user_data->more_data->key with value "Deep value!"
    
    printf("All data tests passed!\n");
    
    // Cleanup is done by one call to free the top-level map
    // This will recursively free all nested maps and values
    data_t temp_data;
    temp_data.type = DATA_MAP;
    temp_data.data.map_value = map;
    ar_data_free(&temp_data);
    
    return 0;
}
