#include "../src/agerun_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("Starting Data Module Tests...\n");
    
    // Test 1: Create a dictionary
    dict_t *dict = ar_dict_create();
    assert(dict != NULL);
    assert(dict->count == 0);
    
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
    
    data_t dict_data_default = ar_data_create(DATA_DICT);
    assert(dict_data_default.type == DATA_DICT);
    assert(dict_data_default.data.dict_value != NULL);
    
    // Test 2: Set and get primitive values
    data_t int_data;
    int_data.type = DATA_INT;
    int_data.data.int_value = 42;
    
    bool result = ar_dict_set(dict, "answer", &int_data);
    assert(result);
    
    data_t *value = ar_dict_get(dict, "answer");
    assert(value != NULL);
    assert(value->type == DATA_INT);
    assert(value->data.int_value == 42);
    
    // Test 3: Set and get string values
    data_t string_data;
    string_data.type = DATA_STRING;
    string_data.data.string_value = strdup("Hello, World!");
    
    result = ar_dict_set(dict, "greeting", &string_data);
    assert(result);
    // We should NOT free the string here as ar_dict_set now uses references
    
    value = ar_dict_get(dict, "greeting");
    assert(value != NULL);
    assert(value->type == DATA_STRING);
    assert(strcmp(value->data.string_value, "Hello, World!") == 0);
    
    // Test 4: Nested dictionary
    data_t nested_dict_data = ar_data_create(DATA_DICT);
    assert(nested_dict_data.type == DATA_DICT);
    assert(nested_dict_data.data.dict_value != NULL);
    
    // Add some data to the nested dictionary
    data_t nested_int_data;
    nested_int_data.type = DATA_INT;
    nested_int_data.data.int_value = 100;
    
    result = ar_dict_set(nested_dict_data.data.dict_value, "count", &nested_int_data);
    assert(result);
    
    // Add the nested dictionary to the main dictionary
    result = ar_dict_set(dict, "user_data", &nested_dict_data);
    assert(result);
    
    // Get the nested dictionary and verify its contents
    value = ar_dict_get(dict, "user_data");
    assert(value != NULL);
    assert(value->type == DATA_DICT);
    assert(value->data.dict_value != NULL);
    
    data_t *nested_value = ar_dict_get(value->data.dict_value, "count");
    assert(nested_value != NULL);
    assert(nested_value->type == DATA_INT);
    assert(nested_value->data.int_value == 100);
    
    // The nested_dict_data now is shared between the dictionary and the value that was set
    
    // Nested a third level deep
    data_t third_level_dict = ar_data_create(DATA_DICT);
    data_t deep_string;
    deep_string.type = DATA_STRING;
    deep_string.data.string_value = strdup("Deep value!");
    
    result = ar_dict_set(third_level_dict.data.dict_value, "key", &deep_string);
    assert(result);
    // Do not free deep_string.data.string_value as ar_dict_set now uses references
    
    // Add third level to the nested dictionary
    result = ar_dict_set(value->data.dict_value, "more_data", &third_level_dict);
    assert(result);
    
    // Now we have dict->user_data->more_data->key with value "Deep value!"
    
    printf("All data tests passed!\n");
    
    // Cleanup is done by one call to free the top-level dictionary
    // This will recursively free all nested dictionaries and values
    data_t temp_data;
    temp_data.type = DATA_DICT;
    temp_data.data.dict_value = dict;
    ar_data_free(&temp_data);
    
    return 0;
}
