#include "ar_data.h"
#include "ar_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_data_creation(void);
static void test_data_getters(void);
static void test_integer_values(void);
static void test_string_values(void);
static void test_nested_maps(void);
static void test_map_data_getters(void);
static void test_map_data_setters(void);
static void test_map_data_path_getters(void);
static void test_map_data_path_setters(void);
static void test_list_operations(void); // Now enabled after TDD Cycle 3
static void test_list_basic_create_destroy(void);
static void test_list_add_integers(void);
static void test_list_add_remove_integers(void);
static void test_map_empty_destroy(void);
static void test_map_add_one_integer(void);
static void test_map_update_integer(void);
//static void test_map_get_keys(void);
static void test_data_ownership(void);
static void test_list_ownership(void);
static void test_list_remove_ownership(void);
static void test_map_ownership(void);
static void test_data_shallow_copy(void);
static void test_data_is_primitive_type(void);
static void test_data_map_contains_only_primitives(void);
static void test_data_list_contains_only_primitives(void);
static void test_data_claim_or_copy(void);
static void test_data_destroy_if_owned(void);
static void test_data_set_map_data_if_root_matched(void);
static void test_data_list_items(void);
static void test_data_format_structure(void);

static void test_data_creation(void) {
    printf("Testing data creation for different types...\n");
    
    // Given we need to create data of different types
    
    // When we create an integer data item with default value
    ar_data_t *own_int_data_default = ar_data__create_integer(0); // We own this value
    
    // Then it should have the correct type and default value
    assert(own_int_data_default != NULL);
    assert(ar_data__get_type(own_int_data_default) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_int_data_default) == 0);
    
    // When we create a double data item with default value
    ar_data_t *own_double_data_default = ar_data__create_double(0.0); // We own this value
    
    // Then it should have the correct type and default value
    assert(own_double_data_default != NULL);
    assert(ar_data__get_type(own_double_data_default) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(own_double_data_default) == 0.0);
    
    // When we create a string data item with NULL value
    ar_data_t *own_string_data_default = ar_data__create_string(NULL); // We own this value
    
    // Then it should have the correct type and null string
    assert(own_string_data_default != NULL);
    assert(ar_data__get_type(own_string_data_default) == AR_DATA_TYPE__STRING);
    assert(ar_data__get_string(own_string_data_default) == NULL);
    
    // When we create an empty list data item
    ar_data_t *own_list_data_default = ar_data__create_list(); // We own this value
    
    // Then it should have the correct type
    assert(own_list_data_default != NULL);
    assert(ar_data__get_type(own_list_data_default) == AR_DATA_TYPE__LIST);
    
    // When we create an empty map data item
    ar_data_t *own_map_data_default = ar_data__create_map(); // We own this value
    
    // Then it should have the correct type and a valid map
    assert(own_map_data_default != NULL);
    assert(ar_data__get_type(own_map_data_default) == AR_DATA_TYPE__MAP);
    // We can verify it's a map by checking its type - can't check ar_data_get_map anymore
    
    // When we create data items with the specialized functions
    ar_data_t *own_int_data = ar_data__create_integer(42); // We own this value
    ar_data_t *own_double_data = ar_data__create_double(3.14159); // We own this value
    ar_data_t *own_string_data = ar_data__create_string("Hello, World!"); // We own this value
    ar_data_t *own_list_data = ar_data__create_list(); // We own this value
    ar_data_t *own_map_data = ar_data__create_map(); // We own this value
    
    // Then they should have the correct types and values
    assert(own_int_data != NULL);
    assert(ar_data__get_type(own_int_data) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_int_data) == 42);
    
    assert(own_double_data != NULL);
    assert(ar_data__get_type(own_double_data) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(own_double_data) == 3.14159);
    
    assert(own_string_data != NULL);
    assert(ar_data__get_type(own_string_data) == AR_DATA_TYPE__STRING);
    assert(ar_data__get_string(own_string_data) != NULL);
    assert(strcmp(ar_data__get_string(own_string_data), "Hello, World!") == 0);
    
    assert(own_list_data != NULL);
    assert(ar_data__get_type(own_list_data) == AR_DATA_TYPE__LIST);
    
    assert(own_map_data != NULL);
    assert(ar_data__get_type(own_map_data) == AR_DATA_TYPE__MAP);
    // We can verify it's a map by checking its type - can't check ar_data_get_map anymore
    
    // Cleanup - we destroy all values we own
    ar_data__destroy(own_map_data_default);
    ar_data__destroy(own_list_data_default);
    ar_data__destroy(own_string_data_default);
    ar_data__destroy(own_double_data_default);
    ar_data__destroy(own_int_data_default);
    
    ar_data__destroy(own_map_data);
    ar_data__destroy(own_list_data);
    ar_data__destroy(own_string_data);
    ar_data__destroy(own_double_data);
    ar_data__destroy(own_int_data);
    
    printf("Data creation tests passed!\n");
}

static void test_data_getters(void) {
    printf("Testing data getters...\n");
    
    // Given data values of different types
    ar_data_t *int_data = ar_data__create_integer(42);
    ar_data_t *double_data = ar_data__create_double(3.14159);
    ar_data_t *string_data = ar_data__create_string("Hello, World!");
    ar_data_t *list_data = ar_data__create_list();
    ar_data_t *map_data = ar_data__create_map();
    
    // When we use the type getter
    ar_data_type_t int_type = ar_data__get_type(int_data);
    ar_data_type_t double_type = ar_data__get_type(double_data);
    ar_data_type_t string_type = ar_data__get_type(string_data);
    ar_data_type_t list_type = ar_data__get_type(list_data);
    ar_data_type_t map_type = ar_data__get_type(map_data);
    ar_data_type_t null_type = ar_data__get_type(NULL);
    
    // Then the types should be correct
    assert(int_type == AR_DATA_TYPE__INTEGER);
    assert(double_type == AR_DATA_TYPE__DOUBLE);
    assert(string_type == AR_DATA_TYPE__STRING);
    assert(list_type == AR_DATA_TYPE__LIST);
    assert(map_type == AR_DATA_TYPE__MAP);
    assert(null_type == AR_DATA_TYPE__INTEGER); // Default to int if NULL
    
    // When we use the value getters with the correct types
    int int_value = ar_data__get_integer(int_data);
    double double_value = ar_data__get_double(double_data);
    const char *string_value = ar_data__get_string(string_data);
    
    // Then they should return the correct values
    assert(int_value == 42);
    assert(double_value == 3.14159);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // When we use the getters with incorrect types
    int wrong_int = ar_data__get_integer(string_data);
    double wrong_double = ar_data__get_double(int_data);
    const char *wrong_string = ar_data__get_string(int_data);
    
    // Then they should return default values
    assert(wrong_int == 0);
    assert(wrong_double == 0.0);
    assert(wrong_string == NULL);
    
    // When we use the getters with NULL
    int null_int = ar_data__get_integer(NULL);
    double null_double = ar_data__get_double(NULL);
    const char *null_string = ar_data__get_string(NULL);
    
    // Then they should return default values
    assert(null_int == 0);
    assert(null_double == 0.0);
    assert(null_string == NULL);
    
    // Cleanup
    ar_data__destroy(int_data);
    ar_data__destroy(double_data);
    ar_data__destroy(string_data);
    ar_data__destroy(list_data);
    ar_data__destroy(map_data);
    
    printf("Data getter tests passed!\n");
}

static void test_integer_values(void) {
    printf("Testing integer values in map...\n");
    
    // Create a data wrapper for the map parameter
    ar_data_t *own_map_data = ar_data__create_map(); // We own this value
    
    // When we set an integer value in the map
    bool result = ar_data__set_map_integer(own_map_data, "answer", 42);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const ar_data_t *ref_value = ar_data__get_map_data(own_map_data, "answer"); // Borrowed reference
    
    // Then the value should be correctly retrieved
    assert(ref_value != NULL);
    assert(ar_data__get_type(ref_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_value) == 42);
    
    // Verify the integer is accessible via getter
    int int_value = ar_data__get_map_integer(own_map_data, "answer");
    assert(int_value == 42);
    
    // Cleanup - destroy the value we own
    ar_data__destroy(own_map_data);
    // ref_value is a borrowed reference, so we don't destroy it
    
    printf("Integer value tests passed!\n");
}

static void test_string_values(void) {
    printf("Testing string values in map...\n");
    
    // Create a data wrapper for the map parameter
    ar_data_t *map_data = ar_data__create_map();
    
    // When we set a string value in the map
    bool result = ar_data__set_map_string(map_data, "greeting", "Hello, World!");
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const ar_data_t *value = ar_data__get_map_data(map_data, "greeting");
    
    // Then the value should be correctly retrieved
    assert(value != NULL);
    assert(ar_data__get_type(value) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(value), "Hello, World!") == 0);
    
    // Verify the string is accessible via getter
    const char *string_value = ar_data__get_map_string(map_data, "greeting");
    assert(string_value != NULL);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // Cleanup
    ar_data__destroy(map_data);
    
    printf("String value tests passed!\n");
}

static void test_nested_maps(void) {
    printf("Testing nested maps...\n");
    
    // Given a data structure to store nested maps
    ar_data_t *own_root_data = ar_data__create_map(); // We own this value
    assert(own_root_data != NULL);
    assert(ar_data__get_type(own_root_data) == AR_DATA_TYPE__MAP);
    
    // Create first level map
    ar_data_t *own_first_level = ar_data__create_map(); // We own this value
    assert(own_first_level != NULL);
    
    // Set an integer value in the first level map
    bool result = ar_data__set_map_integer(own_first_level, "count", 100);
    assert(result);
    
    // Add the first level map to the root - ownership is transferred
    result = ar_data__set_map_data(own_root_data, "user_data", own_first_level);
    assert(result);
    own_first_level = NULL; // Don't use after this point
    
    // Verify the value can be retrieved via the path
    int count_value = ar_data__get_map_integer(own_root_data, "user_data.count");
    assert(count_value == 100);
    
    // Create second level map
    ar_data_t *own_second_level = ar_data__create_map(); // We own this value
    assert(own_second_level != NULL);
    
    // Create third level map with a string value
    ar_data_t *own_third_level = ar_data__create_map(); // We own this value
    assert(own_third_level != NULL);
    result = ar_data__set_map_string(own_third_level, "key", "Deep value!");
    assert(result);
    
    // Add the third level map to the second level map - ownership is transferred
    result = ar_data__set_map_data(own_second_level, "more_data", own_third_level);
    assert(result);
    own_third_level = NULL; // Don't use after this point
    
    // Get a borrowed reference to the first level map
    ar_data_t *ref_first_level = ar_data__get_map_data(own_root_data, "user_data");
    assert(ref_first_level != NULL);
    
    // Add the second level map to the first level map - ownership is transferred
    result = ar_data__set_map_data(ref_first_level, "nested", own_second_level);
    assert(result);
    own_second_level = NULL; // Don't use after this point
    
    // Verify deep nested value can be retrieved via path
    const char *ref_deep_value = ar_data__get_map_string(own_root_data, "user_data.nested.more_data.key");
    assert(ref_deep_value != NULL);
    assert(strcmp(ref_deep_value, "Deep value!") == 0);
    
    // Cleanup - destroy the root value, which will recursively destroy all contained values
    ar_data__destroy(own_root_data);
    // At this point all the other values we created have had their ownership transferred
    
    printf("Nested maps tests passed!\n");
}

static void test_map_data_getters(void) {
    printf("Testing map data getters...\n");
    
    // Given a map data structure with different data types
    ar_data_t *map_data = ar_data__create_map();
    
    // Set values of different types in the map using ar_data__set_map_* functions
    bool int_result = ar_data__set_map_integer(map_data, "int_key", 42);
    bool double_result = ar_data__set_map_double(map_data, "double_key", 3.14159);
    bool string_result = ar_data__set_map_string(map_data, "string_key", "Hello, World!");
    
    // Create a nested map and set it in the parent map
    ar_data_t *nested_map_data = ar_data__create_map();
    bool map_result = ar_data__set_map_data(map_data, "map_key", nested_map_data);
    
    // All operations should succeed
    assert(int_result);
    assert(double_result);
    assert(string_result);
    assert(map_result);
    
    // When we use the map data getters with the correct keys
    int int_value = ar_data__get_map_integer(map_data, "int_key");
    double double_value = ar_data__get_map_double(map_data, "double_key");
    const char *string_value = ar_data__get_map_string(map_data, "string_key");
    
    // Then they should return the correct values
    assert(int_value == 42);
    assert(double_value == 3.14159);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // When we use ar_data__get_map_data with correct keys
    const ar_data_t *int_data_direct = ar_data__get_map_data(map_data, "int_key");
    const ar_data_t *double_data_direct = ar_data__get_map_data(map_data, "double_key");
    const ar_data_t *string_data_direct = ar_data__get_map_data(map_data, "string_key");
    const ar_data_t *map_data_direct = ar_data__get_map_data(map_data, "map_key");
    
    // Then it should return the correct data objects
    assert(int_data_direct != NULL);
    assert(double_data_direct != NULL);
    assert(string_data_direct != NULL);
    assert(map_data_direct != NULL);
    
    // And the data objects should have the correct types
    assert(ar_data__get_type(int_data_direct) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_type(double_data_direct) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_type(string_data_direct) == AR_DATA_TYPE__STRING);
    assert(ar_data__get_type(map_data_direct) == AR_DATA_TYPE__MAP);
    
    // And the data objects should contain the correct values
    assert(ar_data__get_integer(int_data_direct) == 42);
    assert(ar_data__get_double(double_data_direct) == 3.14159);
    assert(strcmp(ar_data__get_string(string_data_direct), "Hello, World!") == 0);
    // For map data, we verify the type is correct
    assert(ar_data__get_type(map_data_direct) == AR_DATA_TYPE__MAP);
    
    // When we use the map data getters with incorrect keys
    int wrong_int = ar_data__get_map_integer(map_data, "nonexistent_key");
    double wrong_double = ar_data__get_map_double(map_data, "nonexistent_key");
    const char *wrong_string = ar_data__get_map_string(map_data, "nonexistent_key");
    const ar_data_t *wrong_data = ar_data__get_map_data(map_data, "nonexistent_key");
    
    // Then they should return default values
    assert(wrong_int == 0);
    assert(wrong_double == 0.0);
    assert(wrong_string == NULL);
    assert(wrong_data == NULL);
    
    // When we use the map data getters with NULL data or key
    int null_data_int = ar_data__get_map_integer(NULL, "int_key");
    double null_key_double = ar_data__get_map_double(map_data, NULL);
    const ar_data_t *null_data_obj = ar_data__get_map_data(NULL, "int_key");
    const ar_data_t *null_key_obj = ar_data__get_map_data(map_data, NULL);
    
    // Then they should return default values
    assert(null_data_int == 0);
    assert(null_key_double == 0.0);
    assert(null_data_obj == NULL);
    assert(null_key_obj == NULL);
    
    // When we use the map data getters with incorrect types
    int wrong_type_int = ar_data__get_map_integer(map_data, "string_key");
    double wrong_type_double = ar_data__get_map_double(map_data, "int_key");
    const char *wrong_type_string = ar_data__get_map_string(map_data, "double_key");
    
    // Then they should return default values
    assert(wrong_type_int == 0);
    assert(wrong_type_double == 0.0);
    assert(wrong_type_string == NULL);
    
    // Create a data wrapper for the nested map value
    ar_data_t *nested_data = ar_data__create_map();
    
    // Set a value in the nested map
    bool nested_result = ar_data__set_map_integer(nested_data, "nested_int", 100);
    assert(nested_result);
    
    // Cleanup
    ar_data__destroy(nested_data);
    
    // Cleanup
    ar_data__destroy(map_data);
    
    printf("Map data getter tests passed!\n");
}

static void test_map_data_setters(void) {
    printf("Testing map data setters...\n");
    
    // Given a map data structure for storing different data types
    ar_data_t *map_data = ar_data__create_map();
    
    // When we set different data types using the map data setters
    bool int_result = ar_data__set_map_integer(map_data, "int_key", 42);
    bool double_result = ar_data__set_map_double(map_data, "double_key", 3.14159);
    bool string_result = ar_data__set_map_string(map_data, "string_key", "Hello, World!");
    
    // Then the operations should succeed
    assert(int_result);
    assert(double_result);
    assert(string_result);
    
    // And the values should be retrievable using the corresponding getters
    int int_value = ar_data__get_map_integer(map_data, "int_key");
    double double_value = ar_data__get_map_double(map_data, "double_key");
    const char *string_value = ar_data__get_map_string(map_data, "string_key");
    
    // Print the values for debugging
    printf("int_value: %d\n", int_value);
    printf("double_value: %f\n", double_value);
    printf("string_value: %s\n", string_value ? string_value : "NULL");
    
    // Then they should match the set values
    assert(int_value == 42);
    assert(double_value == 3.14159);
    assert(string_value != NULL);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // When we update existing values
    bool update_int = ar_data__set_map_integer(map_data, "int_key", 100);
    bool update_double = ar_data__set_map_double(map_data, "double_key", 2.71828);
    bool update_string = ar_data__set_map_string(map_data, "string_key", "Updated text");
    
    // Then the operations should succeed
    assert(update_int);
    assert(update_double);
    assert(update_string);
    
    // And the updated values should be retrievable
    int updated_int = ar_data__get_map_integer(map_data, "int_key");
    double updated_double = ar_data__get_map_double(map_data, "double_key");
    const char *updated_string = ar_data__get_map_string(map_data, "string_key");
    
    // Then they should match the updated values
    assert(updated_int == 100);
    assert(updated_double == 2.71828);
    assert(strcmp(updated_string, "Updated text") == 0);
    
    // When we set values with NULL data or key
    bool null_data_result = ar_data__set_map_integer(NULL, "int_key", 42);
    bool null_key_result = ar_data__set_map_double(map_data, NULL, 3.14159);
    
    // Then the operations should fail
    assert(!null_data_result);
    assert(!null_key_result);
    
    // When we set values in a non-map data type
    ar_data_t *int_data = ar_data__create_integer(42);
    bool wrong_type_result = ar_data__set_map_string(int_data, "key", "value");
    
    // Then the operation should fail
    assert(!wrong_type_result);
    
    // Cleanup
    ar_data__destroy(int_data);
    ar_data__destroy(map_data);
    
    printf("Map data setter tests passed!\n");
}

static void test_map_data_path_getters(void) {
    printf("Testing map data path getters...\n");
    
    // Given a nested map data structure with various data types
    ar_data_t *root_map = ar_data__create_map();
    
    // Create the nested maps
    ar_data_t *user_map = ar_data__create_map();
    ar_data_t *address_map = ar_data__create_map();
    ar_data_t *scores_map = ar_data__create_map();
    
    // Set values in the address map
    bool result = ar_data__set_map_string(address_map, "street", "123 Main St");
    assert(result);
    result = ar_data__set_map_string(address_map, "city", "Anytown");
    assert(result);
    result = ar_data__set_map_integer(address_map, "zip", 12345);
    assert(result);
    
    // Set values in the scores map
    result = ar_data__set_map_integer(scores_map, "math", 95);
    assert(result);
    result = ar_data__set_map_integer(scores_map, "science", 87);
    assert(result);
    result = ar_data__set_map_double(scores_map, "average", 91.0);
    assert(result);
    
    // Set values in the user map
    result = ar_data__set_map_string(user_map, "name", "John Doe");
    assert(result);
    result = ar_data__set_map_integer(user_map, "age", 30);
    assert(result);
    
    // Add address and scores maps to the user map
    result = ar_data__set_map_data(user_map, "address", address_map);
    assert(result);
    result = ar_data__set_map_data(user_map, "scores", scores_map);
    assert(result);
    
    // Add user map to the root map
    result = ar_data__set_map_data(root_map, "user", user_map);
    assert(result);
    
    // When we use the path-based getters
    int age = ar_data__get_map_integer(root_map, "user.age");
    const char *name = ar_data__get_map_string(root_map, "user.name");
    const char *street = ar_data__get_map_string(root_map, "user.address.street");
    const char *city = ar_data__get_map_string(root_map, "user.address.city");
    int zip = ar_data__get_map_integer(root_map, "user.address.zip");
    int math_score = ar_data__get_map_integer(root_map, "user.scores.math");
    int science_score = ar_data__get_map_integer(root_map, "user.scores.science");
    double avg_score = ar_data__get_map_double(root_map, "user.scores.average");
    
    // Then the values should be correctly retrieved
    assert(age == 30);
    assert(strcmp(name, "John Doe") == 0);
    assert(strcmp(street, "123 Main St") == 0);
    assert(strcmp(city, "Anytown") == 0);
    assert(zip == 12345);
    assert(math_score == 95);
    assert(science_score == 87);
    assert(avg_score == 91.0);
    
    // When we use ar_data__get_map_data with paths
    const ar_data_t *user_data = ar_data__get_map_data(root_map, "user");
    const ar_data_t *address_data = ar_data__get_map_data(root_map, "user.address");
    const ar_data_t *scores_data = ar_data__get_map_data(root_map, "user.scores");
    const ar_data_t *name_data = ar_data__get_map_data(root_map, "user.name");
    const ar_data_t *age_data = ar_data__get_map_data(root_map, "user.age");
    const ar_data_t *street_data = ar_data__get_map_data(root_map, "user.address.street");
    const ar_data_t *math_data = ar_data__get_map_data(root_map, "user.scores.math");
    const ar_data_t *avg_data = ar_data__get_map_data(root_map, "user.scores.average");
    
    // Then the data objects should be correctly retrieved
    assert(user_data != NULL);
    assert(address_data != NULL);
    assert(scores_data != NULL);
    assert(name_data != NULL);
    assert(age_data != NULL);
    assert(street_data != NULL);
    assert(math_data != NULL);
    assert(avg_data != NULL);
    
    // And the data objects should have the correct types
    assert(ar_data__get_type(user_data) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_type(address_data) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_type(scores_data) == AR_DATA_TYPE__MAP);
    assert(ar_data__get_type(name_data) == AR_DATA_TYPE__STRING);
    assert(ar_data__get_type(age_data) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_type(street_data) == AR_DATA_TYPE__STRING);
    assert(ar_data__get_type(math_data) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_type(avg_data) == AR_DATA_TYPE__DOUBLE);
    
    // And the data objects should contain the correct values
    assert(ar_data__get_integer(age_data) == 30);
    assert(strcmp(ar_data__get_string(name_data), "John Doe") == 0);
    assert(strcmp(ar_data__get_string(street_data), "123 Main St") == 0);
    assert(ar_data__get_integer(math_data) == 95);
    assert(ar_data__get_double(avg_data) == 91.0);
    
    // When we use path-based getters with invalid paths
    int invalid_age = ar_data__get_map_integer(root_map, "user.invalid.age");
    const char *invalid_name = ar_data__get_map_string(root_map, "invalid.user.name");
    double invalid_avg = ar_data__get_map_double(root_map, "user.scores.invalid");
    const ar_data_t *invalid_data = ar_data__get_map_data(root_map, "user.invalid.path");
    const ar_data_t *nonexistent_data = ar_data__get_map_data(root_map, "nonexistent");
    
    // Then they should return default values
    assert(invalid_age == 0);
    assert(invalid_name == NULL);
    assert(invalid_avg == 0.0);
    assert(invalid_data == NULL);
    assert(nonexistent_data == NULL);
    
    // When we use path-based getters with incorrect types
    int wrong_type = ar_data__get_map_integer(root_map, "user.name");
    double wrong_double = ar_data__get_map_double(root_map, "user.age");
    const char *wrong_string = ar_data__get_map_string(root_map, "user.scores.math");
    
    // Then they should return default values
    assert(wrong_type == 0);
    assert(wrong_double == 0.0);
    assert(wrong_string == NULL);
    
    // Cleanup
    ar_data__destroy(root_map);
    
    printf("Map data path getter tests passed!\n");
}

static void test_map_data_path_setters(void) {
    printf("Testing map data path setters...\n");
    
    // Given a root map data structure
    ar_data_t *root_map = ar_data__create_map();
    
    // When we try to set values using path-based setters for paths that don't exist yet
    printf("Setting integer value on non-existent path...\n");
    bool set_int_result = ar_data__set_map_integer(root_map, "user.preferences.notifications", 1);
    
    // Then the operation should fail since the paths don't exist
    assert(!set_int_result);
    
    printf("Setting double value on non-existent path...\n");
    bool set_double_result = ar_data__set_map_double(root_map, "user.account.balance", 1250.75);
    
    // Then the operation should fail since the paths don't exist
    assert(!set_double_result);
    
    printf("Setting string value on non-existent path...\n");
    bool set_string_result = ar_data__set_map_string(root_map, "user.profile.email", "john.doe@example.com");
    
    // Then the operation should fail since the paths don't exist
    assert(!set_string_result);
    
    // First create intermediate maps for a proper test
    // Create the intermediate maps for a test path
    ar_data_t *user_map = ar_data__create_map();
    ar_data_t *prefs_map = ar_data__create_map();
    ar_data_t *account_map = ar_data__create_map();
    ar_data_t *profile_map = ar_data__create_map();
    
    // Add to root using data interface
    ar_data__set_map_data(root_map, "user", user_map);
    
    // Add to user map using data interface
    ar_data__set_map_data(user_map, "preferences", prefs_map);
    ar_data__set_map_data(user_map, "account", account_map);
    ar_data__set_map_data(user_map, "profile", profile_map);
    
    // Now set on valid paths
    printf("Setting integer value on valid path...\n");
    set_int_result = ar_data__set_map_integer(root_map, "user.preferences.notifications", 1);
    printf("Set integer result: %d\n", set_int_result ? 1 : 0);
    printf("Testing integer value after setting...\n");
    int int_val = ar_data__get_map_integer(root_map, "user.preferences.notifications");
    printf("integer value: %d\n", int_val);
    
    printf("Setting double value on valid path...\n");
    
    // Verify account map exists before setting
    ar_data_t *pre_account_data = ar_data__get_map_data(root_map, "user.account");
    printf("account data exists before setting: %s\n", pre_account_data ? "yes" : "no");
    
    // Set the double value and check result
    set_double_result = ar_data__set_map_double(root_map, "user.account.balance", 1250.75);
    printf("Set double result: %d\n", set_double_result ? 1 : 0);
    printf("Testing double value after setting...\n");
    
    // Get the target map to directly inspect if needed
    ar_data_t *user_data = ar_data__get_map_data(root_map, "user");
    printf("user data type: %d\n", ar_data__get_type(user_data));
    ar_data_t *account_data = ar_data__get_map_data(root_map, "user.account");
    printf("account data type: %d\n", ar_data__get_type(account_data));
    
    // Use data interface for inspection
    printf("Direct check of account_map entries:\n");
    ar_data_t *balance_direct = ar_data__get_map_data(account_data, "balance");
    printf("Direct map lookup of 'balance': %p\n", (void *)balance_direct);
    if (balance_direct) {
        printf("Direct balance data type: %d\n", ar_data__get_type(balance_direct));
        if (ar_data__get_type(balance_direct) == AR_DATA_TYPE__DOUBLE) {
            printf("Direct balance value: %f\n", ar_data__get_double(balance_direct));
        }
    }
    
    ar_data_t *balance_data = ar_data__get_map_data(root_map, "user.account.balance");
    printf("balance data type: %d\n", balance_data ? (int)ar_data__get_type(balance_data) : -1);
    
    double double_val = ar_data__get_map_double(root_map, "user.account.balance");
    printf("double value: %f\n", double_val);
    
    printf("Setting string value on valid path...\n");
    set_string_result = ar_data__set_map_string(root_map, "user.profile.email", "john.doe@example.com");
    printf("Set string result: %d\n", set_string_result ? 1 : 0);
    printf("Testing string value after setting...\n");
    const char *string_val = ar_data__get_map_string(root_map, "user.profile.email");
    printf("string value: %s\n", string_val ? string_val : "NULL");
    
    // Then the operations on valid paths should succeed
    assert(set_int_result);
    assert(set_double_result);
    assert(set_string_result);
    
    // And the values should match what we set
    assert(int_val == 1);
    assert(double_val == 1250.75);
    assert(string_val != NULL && strcmp(string_val, "john.doe@example.com") == 0);
    
    // When we update existing values
    bool update_int = ar_data__set_map_integer(root_map, "user.preferences.notifications", 0);
    bool update_double = ar_data__set_map_double(root_map, "user.account.balance", 2000.50);
    bool update_string = ar_data__set_map_string(root_map, "user.profile.email", "johndoe@example.com");
    
    // Print the update operation results
    printf("Update integer result: %d\n", update_int);
    printf("Update double result: %d\n", update_double);
    printf("Update string result: %d\n", update_string);
    
    // Then the operations should succeed
    assert(update_int);
    assert(update_double);
    assert(update_string);
    
    // When we set a value at a non-map location, it should fail
    // First, set a string value
    ar_data__set_map_string(root_map, "config", "settings");
    
    // Then try to set a nested value through it
    printf("Setting nested value through non-map node...\n");
    bool invalid_path = ar_data__set_map_integer(root_map, "config.value", 123);
    
    // It should fail since "config" is a string, not a map
    assert(!invalid_path);
    
    // Cleanup
    ar_data__destroy(root_map);
    
    printf("Map data path setter tests passed!\n");
}

// Now enabled after TDD Cycle 3 - list remove functions handle ownership transfer
static void test_list_operations(void) {
    printf("Testing list operations...\n");
    
    // Given a list data structure
    ar_data_t *own_list_data = ar_data__create_list(); // We own this value
    assert(own_list_data != NULL);
    assert(ar_data__get_type(own_list_data) == AR_DATA_TYPE__LIST);
    
    // When we check the initial list count
    size_t initial_count = ar_data__list_count(own_list_data);
    
    // Then it should be empty
    assert(initial_count == 0);
    
    // When we add integers to the list
    bool add_first_result = ar_data__list_add_first_integer(own_list_data, 10);
    bool add_last_result = ar_data__list_add_last_integer(own_list_data, 20);
    
    // Then the operations should succeed
    assert(add_first_result);
    assert(add_last_result);
    
    // And the list count should be updated
    assert(ar_data__list_count(own_list_data) == 2);
    
    // When we get the first and last items - these are borrowed references
    ar_data_t *ref_first_item = ar_data__list_first(own_list_data);
    ar_data_t *ref_last_item = ar_data__list_last(own_list_data);
    
    // Then they should be valid data items with the correct values
    assert(ref_first_item != NULL);
    assert(ref_last_item != NULL);
    assert(ar_data__get_type(ref_first_item) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_type(ref_last_item) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_first_item) == 10);
    assert(ar_data__get_integer(ref_last_item) == 20);
    
    // When we add doubles to the list
    bool add_first_double_result = ar_data__list_add_first_double(own_list_data, 3.14);
    bool add_last_double_result = ar_data__list_add_last_double(own_list_data, 2.71);
    
    // Then the operations should succeed
    assert(add_first_double_result);
    assert(add_last_double_result);
    
    // And the list count should be updated
    assert(ar_data__list_count(own_list_data) == 4);
    
    // When we get the first and last items again - these are borrowed references
    ref_first_item = ar_data__list_first(own_list_data);
    ref_last_item = ar_data__list_last(own_list_data);
    
    // Then they should be valid data items with the correct values
    assert(ref_first_item != NULL);
    assert(ref_last_item != NULL);
    assert(ar_data__get_type(ref_first_item) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_type(ref_last_item) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(ref_first_item) == 3.14);
    assert(ar_data__get_double(ref_last_item) == 2.71);
    
    // When we add strings to the list
    bool add_first_string_result = ar_data__list_add_first_string(own_list_data, "hello");
    bool add_last_string_result = ar_data__list_add_last_string(own_list_data, "world");
    
    // Then the operations should succeed
    assert(add_first_string_result);
    assert(add_last_string_result);
    
    // And the list count should be updated
    assert(ar_data__list_count(own_list_data) == 6);
    
    // When we get the first and last items again - these are borrowed references
    ref_first_item = ar_data__list_first(own_list_data);
    ref_last_item = ar_data__list_last(own_list_data);
    
    // Then they should be valid data items with the correct values
    assert(ref_first_item != NULL);
    assert(ref_last_item != NULL);
    assert(ar_data__get_type(ref_first_item) == AR_DATA_TYPE__STRING);
    assert(ar_data__get_type(ref_last_item) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_first_item), "hello") == 0);
    assert(strcmp(ar_data__get_string(ref_last_item), "world") == 0);
    
    // When we add data directly to the list
    ar_data_t *own_int_data = ar_data__create_integer(42); // We own this value
    ar_data_t *own_double_data = ar_data__create_double(3.14159); // We own this value
    
    // These operations transfer ownership to the list
    bool add_first_data_result = ar_data__list_add_first_data(own_list_data, own_int_data);
    bool add_last_data_result = ar_data__list_add_last_data(own_list_data, own_double_data);
    
    // Then the operations should succeed
    assert(add_first_data_result);
    assert(add_last_data_result);
    
    // Mark transferred pointers as NULL
    own_int_data = NULL; // Don't use after this point
    own_double_data = NULL; // Don't use after this point
    
    // And the list count should be updated
    assert(ar_data__list_count(own_list_data) == 8);
    
    // When we get the first and last items again - these are borrowed references
    ref_first_item = ar_data__list_first(own_list_data);
    ref_last_item = ar_data__list_last(own_list_data);
    
    // Then they should be valid data items with the correct values
    assert(ref_first_item != NULL);
    assert(ref_last_item != NULL);
    assert(ar_data__get_type(ref_first_item) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_type(ref_last_item) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_integer(ref_first_item) == 42);
    assert(ar_data__get_double(ref_last_item) == 3.14159);
    
    // When we remove items from the list - we get owned values back
    ar_data_t *own_removed_first = ar_data__list_remove_first(own_list_data);
    ar_data_t *own_removed_last = ar_data__list_remove_last(own_list_data);
    
    // Then the operations should succeed and return the correct items
    assert(own_removed_first != NULL);
    assert(own_removed_last != NULL);
    assert(ar_data__get_type(own_removed_first) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_type(own_removed_last) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_integer(own_removed_first) == 42);
    assert(ar_data__get_double(own_removed_last) == 3.14159);
    
    // And the list count should be updated
    assert(ar_data__list_count(own_list_data) == 6);
    
    // Test the typed removal functions
    
    // Start with a fresh list
    ar_data_t *own_typed_list = ar_data__create_list(); // We own this value
    assert(own_typed_list != NULL);
    
    // Add items of different types in a specific order for our tests
    ar_data__list_add_last_integer(own_typed_list, 100);  // first item - for first integer test
    ar_data__list_add_last_double(own_typed_list, 2.5);   // second item - for first double test
    ar_data__list_add_last_string(own_typed_list, "test string"); // third item - for first string test
    ar_data__list_add_last_string(own_typed_list, "another string"); // fourth item
    ar_data__list_add_last_double(own_typed_list, 3.5);   // fifth item - for last double test
    ar_data__list_add_last_integer(own_typed_list, 200);  // last item - for last integer test
    
    // Test ar_data__list_remove_first_integer - returns a primitive value, not ownership
    int first_int = ar_data__list_remove_first_integer(own_typed_list);
    assert(first_int == 100);
    assert(ar_data__list_count(own_typed_list) == 5);
    
    // Test ar_data__list_remove_first_double - returns a primitive value, not ownership
    double first_double = ar_data__list_remove_first_double(own_typed_list);
    assert(first_double == 2.5);
    assert(ar_data__list_count(own_typed_list) == 4);
    
    // Test ar_data__list_remove_first_string - returns an owned string that must be freed
    char *own_first_string = ar_data__list_remove_first_string(own_typed_list);
    assert(own_first_string != NULL);
    assert(strcmp(own_first_string, "test string") == 0);
    AR__HEAP__FREE(own_first_string); // We must free the owned string
    own_first_string = NULL; // Mark as freed
    assert(ar_data__list_count(own_typed_list) == 3);
    
    // Test ar_data__list_remove_last_integer - returns a primitive value, not ownership
    int last_int = ar_data__list_remove_last_integer(own_typed_list);
    assert(last_int == 200);
    assert(ar_data__list_count(own_typed_list) == 2);
    
    // Test ar_data__list_remove_last_double - returns a primitive value, not ownership
    double last_double = ar_data__list_remove_last_double(own_typed_list);
    assert(last_double == 3.5);
    assert(ar_data__list_count(own_typed_list) == 1);
    
    // Test ar_data__list_remove_last_string - returns an owned string that must be freed
    char *own_last_string = ar_data__list_remove_last_string(own_typed_list);
    assert(own_last_string != NULL);
    assert(strcmp(own_last_string, "another string") == 0);
    AR__HEAP__FREE(own_last_string); // We must free the owned string
    own_last_string = NULL; // Mark as freed
    assert(ar_data__list_count(own_typed_list) == 0);
    
    // Test removing from an empty list
    assert(ar_data__list_remove_first_integer(own_typed_list) == 0);
    assert(ar_data__list_remove_first_double(own_typed_list) == 0.0);
    assert(ar_data__list_remove_first_string(own_typed_list) == NULL);
    assert(ar_data__list_remove_last_integer(own_typed_list) == 0);
    assert(ar_data__list_remove_last_double(own_typed_list) == 0.0);
    assert(ar_data__list_remove_last_string(own_typed_list) == NULL);
    
    // Test removing with type mismatch
    ar_data__list_add_last_integer(own_typed_list, 300);
    assert(ar_data__list_remove_first_double(own_typed_list) == 0.0); // Should return 0.0 and not remove
    assert(ar_data__list_remove_first_string(own_typed_list) == NULL); // Should return NULL and not remove
    assert(ar_data__list_count(own_typed_list) == 1); // Should still be 1
    assert(ar_data__list_remove_first_integer(own_typed_list) == 300); // Now remove properly
    assert(ar_data__list_count(own_typed_list) == 0);
    
    // Add mixed types in reverse order
    ar_data__list_add_last_string(own_typed_list, "string first");
    ar_data__list_add_last_double(own_typed_list, 4.5);
    ar_data__list_add_last_integer(own_typed_list, 400);
    
    // Test last with type mismatch
    assert(ar_data__list_remove_last_double(own_typed_list) == 0.0); // Should return 0.0 and not remove
    assert(ar_data__list_remove_last_string(own_typed_list) == NULL); // Should return NULL and not remove
    assert(ar_data__list_count(own_typed_list) == 3); // Should still be 3
    assert(ar_data__list_remove_last_integer(own_typed_list) == 400); // Now remove properly
    assert(ar_data__list_count(own_typed_list) == 2);
    assert(ar_data__list_remove_last_double(own_typed_list) == 4.5);
    assert(ar_data__list_count(own_typed_list) == 1);
    
    // Remove the remaining string before cleanup
    char *own_remaining_string = ar_data__list_remove_first_string(own_typed_list);
    assert(own_remaining_string != NULL);
    assert(strcmp(own_remaining_string, "string first") == 0);
    AR__HEAP__FREE(own_remaining_string); // We must free the owned string
    own_remaining_string = NULL; // Mark as freed
    assert(ar_data__list_count(own_typed_list) == 0);
    
    // Cleanup typed_list - we own this value
    ar_data__destroy(own_typed_list);
    
    // Cleanup removed items - we own these values
    ar_data__destroy(own_removed_first);
    ar_data__destroy(own_removed_last);
    
    // Cleanup original list - we own this value
    ar_data__destroy(own_list_data);
    
    // Test error handling for null data
    assert(ar_data__list_count(NULL) == 0);
    assert(!ar_data__list_add_first_integer(NULL, 42));
    assert(!ar_data__list_add_last_integer(NULL, 42));
    assert(!ar_data__list_add_first_double(NULL, 3.14));
    assert(!ar_data__list_add_last_double(NULL, 3.14));
    assert(!ar_data__list_add_first_string(NULL, "test"));
    assert(!ar_data__list_add_last_string(NULL, "test"));
    assert(ar_data__list_first(NULL) == NULL);
    assert(ar_data__list_last(NULL) == NULL);
    assert(ar_data__list_remove_first(NULL) == NULL);
    assert(ar_data__list_remove_last(NULL) == NULL);
    assert(ar_data__list_remove_first_integer(NULL) == 0);
    assert(ar_data__list_remove_first_double(NULL) == 0.0);
    assert(ar_data__list_remove_first_string(NULL) == NULL);
    assert(ar_data__list_remove_last_integer(NULL) == 0);
    assert(ar_data__list_remove_last_double(NULL) == 0.0);
    assert(ar_data__list_remove_last_string(NULL) == NULL);
    
    // Test error handling for wrong data type
    ar_data_t *own_int_data2 = ar_data__create_integer(42); // We own this value
    assert(!ar_data__list_add_first_integer(own_int_data2, 10));
    assert(!ar_data__list_add_last_integer(own_int_data2, 10));
    assert(!ar_data__list_add_first_double(own_int_data2, 3.14));
    assert(!ar_data__list_add_last_double(own_int_data2, 3.14));
    assert(!ar_data__list_add_first_string(own_int_data2, "test"));
    assert(!ar_data__list_add_last_string(own_int_data2, "test"));
    assert(ar_data__list_first(own_int_data2) == NULL);
    assert(ar_data__list_last(own_int_data2) == NULL);
    assert(ar_data__list_remove_first(own_int_data2) == NULL);
    assert(ar_data__list_remove_last(own_int_data2) == NULL);
    assert(ar_data__list_remove_first_integer(own_int_data2) == 0);
    assert(ar_data__list_remove_first_double(own_int_data2) == 0.0);
    assert(ar_data__list_remove_first_string(own_int_data2) == NULL);
    assert(ar_data__list_remove_last_integer(own_int_data2) == 0);
    assert(ar_data__list_remove_last_double(own_int_data2) == 0.0);
    assert(ar_data__list_remove_last_string(own_int_data2) == NULL);
    ar_data__destroy(own_int_data2); // We must destroy what we own
    
    printf("List operations tests passed!\n");
}

static void test_list_basic_create_destroy(void) {
    printf("Testing list basic create/destroy...\n");
    
    // Create and immediately destroy an empty list
    ar_data_t *list = ar_data__create_list();
    assert(list != NULL);
    assert(ar_data__get_type(list) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(list) == 0);
    ar_data__destroy(list);
    
    printf("List basic create/destroy test passed!\n");
}

static void test_list_add_integers(void) {
    printf("Testing list add integers...\n");
    
    ar_data_t *list = ar_data__create_list();
    
    // Add one integer
    assert(ar_data__list_add_first_integer(list, 42) == true);
    assert(ar_data__list_count(list) == 1);
    
    // Destroy list with integer
    ar_data__destroy(list);
    
    printf("List add integers test passed!\n");
}

static void test_list_add_remove_integers(void) {
    printf("Testing list add/remove integers...\n");
    
    ar_data_t *list = ar_data__create_list();
    
    // Add integers
    assert(ar_data__list_add_first_integer(list, 10) == true);
    assert(ar_data__list_add_last_integer(list, 20) == true);
    assert(ar_data__list_count(list) == 2);
    
    // Remove integers
    assert(ar_data__list_remove_first_integer(list) == 10);
    assert(ar_data__list_remove_last_integer(list) == 20);
    assert(ar_data__list_count(list) == 0);
    
    ar_data__destroy(list);
    
    printf("List add/remove integers test passed!\n");
}

static void test_map_empty_destroy(void) {
    printf("Testing map empty create/destroy...\n");
    
    ar_data_t *map = ar_data__create_map();
    assert(map != NULL);
    assert(ar_data__get_type(map) == AR_DATA_TYPE__MAP);
    ar_data__destroy(map);
    
    printf("Map empty create/destroy test passed!\n");
}

static void test_map_add_one_integer(void) {
    printf("Testing map add one integer...\n");
    
    ar_data_t *map = ar_data__create_map();
    assert(ar_data__set_map_integer(map, "count", 42) == true);
    ar_data__destroy(map);
    
    printf("Map add one integer test passed!\n");
}

static void test_map_update_integer(void) {
    printf("Testing map update integer...\n");
    
    ar_data_t *map = ar_data__create_map();
    assert(ar_data__set_map_integer(map, "count", 42) == true);
    assert(ar_data__set_map_integer(map, "count", 100) == true);  // Update same key
    assert(ar_data__get_map_integer(map, "count") == 100);
    ar_data__destroy(map);
    
    printf("Map update integer test passed!\n");
}

#if 0
static void test_map_get_keys(void) {
    printf("Testing map get keys function...\n");
    
    // Test 1: Empty map should return empty list
    {
        // Given an empty map
        ar_data_t *own_empty_map = ar_data__create_map();
        assert(own_empty_map != NULL);
        
        // When we get the keys
        ar_data_t *own_keys_list = ar_data__get_map_keys(own_empty_map);
        
        // Then we should get an empty list (not NULL)
        assert(own_keys_list != NULL);
        assert(ar_data__get_type(own_keys_list) == AR_DATA_TYPE__LIST);
        assert(ar_data__list_count(own_keys_list) == 0);
        
        // Cleanup
        ar_data__destroy(own_keys_list);
        ar_data__destroy(own_empty_map);
    }
    
    // Test 2: Map with single key
    {
        // Given a map with one key
        ar_data_t *own_single_map = ar_data__create_map();
        assert(own_single_map != NULL);
        ar_data__set_map_integer(own_single_map, "count", 42);
        
        // When we get the keys
        ar_data_t *own_keys_list = ar_data__get_map_keys(own_single_map);
        
        // Then we should get a list with one string item
        assert(own_keys_list != NULL);
        assert(ar_data__get_type(own_keys_list) == AR_DATA_TYPE__LIST);
        assert(ar_data__list_count(own_keys_list) == 1);
        
        // Verify the key is correct
        ar_data_t *ref_key_data = ar_data__list_first(own_keys_list);
        assert(ref_key_data != NULL);
        assert(ar_data__get_type(ref_key_data) == AR_DATA_TYPE__STRING);
        assert(strcmp(ar_data__get_string(ref_key_data), "count") == 0);
        
        // Cleanup
        ar_data__destroy(own_keys_list);
        ar_data__destroy(own_single_map);
    }
    
    // Test 3: Map with multiple keys
    {
        // Given a map with multiple keys
        ar_data_t *own_multi_map = ar_data__create_map();
        assert(own_multi_map != NULL);
        ar_data__set_map_integer(own_multi_map, "age", 25);
        ar_data__set_map_string(own_multi_map, "name", "Alice");
        ar_data__set_map_double(own_multi_map, "score", 95.5);
        
        // When we get the keys
        ar_data_t *own_keys_list = ar_data__get_map_keys(own_multi_map);
        
        // Then we should get a list with three string items
        assert(own_keys_list != NULL);
        assert(ar_data__get_type(own_keys_list) == AR_DATA_TYPE__LIST);
        assert(ar_data__list_count(own_keys_list) == 3);
        
        // Collect all keys to verify they are all present
        bool found_age = false;
        bool found_name = false;
        bool found_score = false;
        
        // We need to iterate through the list and check each key
        // Since we can't iterate directly, we'll remove and check each item
        while (ar_data__list_count(own_keys_list) > 0) {
            ar_data_t *own_key_data = ar_data__list_remove_first(own_keys_list);
            assert(own_key_data != NULL);
            assert(ar_data__get_type(own_key_data) == AR_DATA_TYPE__STRING);
            
            const char *key = ar_data__get_string(own_key_data);
            if (strcmp(key, "age") == 0) found_age = true;
            else if (strcmp(key, "name") == 0) found_name = true;
            else if (strcmp(key, "score") == 0) found_score = true;
            
            ar_data__destroy(own_key_data);
        }
        
        assert(found_age);
        assert(found_name);
        assert(found_score);
        
        // Cleanup
        ar_data__destroy(own_keys_list);
        ar_data__destroy(own_multi_map);
    }
    
    // Test 4: NULL input should return NULL
    {
        // When we pass NULL
        ar_data_t *keys = ar_data__get_map_keys(NULL);
        
        // Then we should get NULL
        assert(keys == NULL);
    }
    
    // Test 5: Non-map data type should return NULL
    {
        // Given a non-map data type
        ar_data_t *own_integer = ar_data__create_integer(42);
        assert(own_integer != NULL);
        
        // When we try to get keys from non-map
        ar_data_t *keys = ar_data__get_map_keys(own_integer);
        
        // Then we should get NULL
        assert(keys == NULL);
        
        // Cleanup
        ar_data__destroy(own_integer);
    }
    
    // Test 6: Map with nested map value (keys should only be top-level)
    {
        // Given a map with a nested map
        ar_data_t *own_parent_map = ar_data__create_map();
        ar_data_t *own_child_map = ar_data__create_map();
        assert(own_parent_map != NULL);
        assert(own_child_map != NULL);
        
        ar_data__set_map_integer(own_child_map, "nested_key", 100);
        ar_data__set_map_data(own_parent_map, "child", own_child_map);
        ar_data__set_map_string(own_parent_map, "parent_key", "value");
        
        // When we get the keys
        ar_data_t *own_keys_list = ar_data__get_map_keys(own_parent_map);
        
        // Then we should only get top-level keys
        assert(own_keys_list != NULL);
        assert(ar_data__list_count(own_keys_list) == 2);
        
        // Verify we have "child" and "parent_key" but not "nested_key"
        bool found_child = false;
        bool found_parent_key = false;
        
        while (ar_data__list_count(own_keys_list) > 0) {
            ar_data_t *own_key_data = ar_data__list_remove_first(own_keys_list);
            const char *key = ar_data__get_string(own_key_data);
            
            if (strcmp(key, "child") == 0) found_child = true;
            else if (strcmp(key, "parent_key") == 0) found_parent_key = true;
            else if (strcmp(key, "nested_key") == 0) {
                assert(false); // Should not find nested key
            }
            
            ar_data__destroy(own_key_data);
        }
        
        assert(found_child);
        assert(found_parent_key);
        
        // Cleanup
        ar_data__destroy(own_keys_list);
        ar_data__destroy(own_parent_map);
    }
    
    printf("Map get keys tests passed!\n");
}
#endif

static void test_data_ownership(void) {
    printf("Testing data ownership tracking...\n");
    
    // Test 1: New data has no owner - can be destroyed
    ar_data_t *data1 = ar_data__create_integer(42);
    ar_data__destroy(data1);  // Should succeed - unowned
    
    // Test 2: Can claim unowned data
    ar_data_t *data = ar_data__create_integer(42);
    void *owner1 = (void*)0x1234;
    assert(ar_data__take_ownership(data, owner1) == true);
    
    // Test 3: Can reclaim data you own
    assert(ar_data__take_ownership(data, owner1) == true);
    
    // Test 4: Cannot claim data owned by another
    void *owner2 = (void*)0x5678;
    assert(ar_data__take_ownership(data, owner2) == false);
    
    // Test 5: Only owner can transfer
    assert(ar_data__drop_ownership(data, owner2) == false);
    assert(ar_data__drop_ownership(data, owner1) == true);
    
    // Test 6: After transfer, new owner can claim
    assert(ar_data__take_ownership(data, owner2) == true);
    
    // Test 7: Must transfer to NULL before destroy
    // ar_data__destroy(data);  // Would fail - still owned
    assert(ar_data__drop_ownership(data, owner2) == true);
    ar_data__destroy(data);  // Now succeeds - unowned
    
    printf("Data ownership tests passed!\n");
}

static void test_list_ownership(void) {
    printf("Testing list ownership behavior...\n");
    
    // Test 1: list_add_first_data should hold ownership of added data
    ar_data_t *list = ar_data__create_list();
    ar_data_t *data1 = ar_data__create_integer(42);
    
    // List should hold ownership when data is added
    assert(ar_data__list_add_first_data(list, data1) == true);
    
    // Test 2: list_add_last_data should hold ownership of added data
    ar_data_t *data2 = ar_data__create_integer(84);
    assert(ar_data__list_add_last_data(list, data2) == true);
    
    // Test 3: Cannot add data already owned by another
    ar_data_t *data3 = ar_data__create_integer(100);
    void *other_owner = (void*)0x9999;
    assert(ar_data__take_ownership(data3, other_owner) == true);
    assert(ar_data__list_add_first_data(list, data3) == false);  // Should fail
    assert(ar_data__list_add_last_data(list, data3) == false);   // Should fail
    
    // Clean up data3 since it wasn't added
    assert(ar_data__drop_ownership(data3, other_owner) == true);
    ar_data__destroy(data3);
    
    // Test 4: Test that convenience functions work (they don't use ownership tracking)
    ar_data_t *list2 = ar_data__create_list();
    assert(ar_data__list_add_first_integer(list2, 10) == true);
    assert(ar_data__list_add_first_double(list2, 3.14) == true);
    assert(ar_data__list_add_first_string(list2, "test") == true);
    assert(ar_data__list_add_last_integer(list2, 20) == true);
    assert(ar_data__list_add_last_double(list2, 2.71) == true);
    assert(ar_data__list_add_last_string(list2, "test2") == true);
    
    // Destroying list2 should clean up all contained data
    ar_data__destroy(list2);
    
    // Test 5: Data added to list is properly owned
    // We can't destroy data1 or data2 directly - they're owned by the list
    // Destroying the list should clean up all contained data
    ar_data__destroy(list);
    
    printf("List ownership tests passed!\n");
}

static void test_list_remove_ownership(void) {
    printf("Testing list remove functions with ownership...\n");
    
    // Test 1: list_remove_first should transfer ownership back to caller
    ar_data_t *list = ar_data__create_list();
    ar_data_t *data1 = ar_data__create_integer(42);
    ar_data_t *data2 = ar_data__create_integer(84);
    
    // Add data to list (list takes ownership)
    assert(ar_data__list_add_first_data(list, data1) == true);
    assert(ar_data__list_add_last_data(list, data2) == true);
    
    // Remove first should transfer ownership back
    ar_data_t *removed1 = ar_data__list_remove_first(list);
    assert(removed1 == data1);
    // We should now be able to destroy it since ownership was transferred
    ar_data__destroy(removed1);
    
    // Remove last should transfer ownership back
    ar_data_t *removed2 = ar_data__list_remove_last(list);
    assert(removed2 == data2);
    // We should now be able to destroy it since ownership was transferred
    ar_data__destroy(removed2);
    
    // Test 2: Remove from empty list
    assert(ar_data__list_remove_first(list) == NULL);
    assert(ar_data__list_remove_last(list) == NULL);
    
    // Test 3: Test with mixed ownership (some owned by list, some by others)
    ar_data_t *data3 = ar_data__create_integer(100);
    ar_data_t *data4 = ar_data__create_integer(200);
    ar_data_t *data5 = ar_data__create_integer(300);
    
    // Add data3 and data4 to list
    assert(ar_data__list_add_last_data(list, data3) == true);
    assert(ar_data__list_add_last_data(list, data4) == true);
    
    // Try to add data5 which is owned by another
    void *other_owner = (void*)0x9999;
    assert(ar_data__take_ownership(data5, other_owner) == true);
    assert(ar_data__list_add_last_data(list, data5) == false);  // Should fail
    
    // Remove data3 and data4
    ar_data_t *removed3 = ar_data__list_remove_first(list);
    assert(removed3 == data3);
    ar_data__destroy(removed3);
    
    ar_data_t *removed4 = ar_data__list_remove_first(list);
    assert(removed4 == data4);
    ar_data__destroy(removed4);
    
    // Clean up data5
    assert(ar_data__drop_ownership(data5, other_owner) == true);
    ar_data__destroy(data5);
    
    // Clean up list
    ar_data__destroy(list);
    
    printf("List remove ownership tests passed!\n");
}

static void test_map_ownership(void) {
    printf("Testing map ownership behavior...\n");
    
    // Test 1: set_map_data should hold ownership of added data
    ar_data_t *map = ar_data__create_map();
    ar_data_t *data1 = ar_data__create_integer(42);
    
    // Map should hold ownership when data is added
    assert(ar_data__set_map_data(map, "key1", data1) == true);
    
    // Test 2: Cannot add data already owned by another
    ar_data_t *data2 = ar_data__create_string("hello");
    void *other_owner = (void*)0x8888;
    assert(ar_data__take_ownership(data2, other_owner) == true);
    assert(ar_data__set_map_data(map, "key2", data2) == false);  // Should fail
    
    // Clean up data2 since it wasn't added
    assert(ar_data__drop_ownership(data2, other_owner) == true);
    ar_data__destroy(data2);
    
    // Test 3: Replacing a value should transfer ownership of old value
    ar_data_t *data3 = ar_data__create_integer(100);
    assert(ar_data__set_map_data(map, "key1", data3) == true);  // Replace existing
    // Old data1 should have been destroyed internally
    
    // Test 4: Test that convenience functions work (they don't use ownership tracking)
    ar_data_t *map2 = ar_data__create_map();
    assert(ar_data__set_map_integer(map2, "int", 10) == true);
    assert(ar_data__set_map_double(map2, "double", 3.14) == true);
    assert(ar_data__set_map_string(map2, "string", "test") == true);
    
    // Update values to test replacement
    assert(ar_data__set_map_integer(map2, "int", 20) == true);
    assert(ar_data__set_map_double(map2, "double", 2.71) == true);
    assert(ar_data__set_map_string(map2, "string", "updated") == true);
    
    // Destroying map2 should clean up all contained data
    ar_data__destroy(map2);
    
    // Test 5: Data added to map is properly owned
    // Destroying the map should clean up all contained data
    ar_data__destroy(map);
    
    printf("Map ownership tests passed!\n");
}

static void test_data_shallow_copy(void) {
    printf("Testing data shallow copy...\n");
    
    // Given a NULL input
    ar_data_t *ref_null_value = NULL;
    
    // When we try to copy NULL
    ar_data_t *own_result = ar_data__shallow_copy(ref_null_value);
    
    // Then we should get NULL back
    assert(own_result == NULL);
    
    // Given an integer value
    ar_data_t *own_int_value = ar_data__create_integer(42);
    assert(own_int_value != NULL);
    
    // When we copy the integer
    ar_data_t *own_int_copy = ar_data__shallow_copy(own_int_value);
    
    // Then we should get a new integer with the same value
    assert(own_int_copy != NULL);
    assert(ar_data__get_type(own_int_copy) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_int_copy) == 42);
    assert(own_int_copy != own_int_value); // Different instance
    
    // Clean up
    ar_data__destroy(own_int_value);
    ar_data__destroy(own_int_copy);
    
    // Given a double value
    ar_data_t *own_double_value = ar_data__create_double(3.14159);
    assert(own_double_value != NULL);
    
    // When we copy the double
    ar_data_t *own_double_copy = ar_data__shallow_copy(own_double_value);
    
    // Then we should get a new double with the same value
    assert(own_double_copy != NULL);
    assert(ar_data__get_type(own_double_copy) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(own_double_copy) == 3.14159);
    assert(own_double_copy != own_double_value); // Different instance
    
    // Clean up
    ar_data__destroy(own_double_value);
    ar_data__destroy(own_double_copy);
    
    // Given a string value
    ar_data_t *own_string_value = ar_data__create_string("Hello, World!");
    assert(own_string_value != NULL);
    
    // When we copy the string
    ar_data_t *own_string_copy = ar_data__shallow_copy(own_string_value);
    
    // Then we should get a new string with the same value
    assert(own_string_copy != NULL);
    assert(ar_data__get_type(own_string_copy) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_string_copy), "Hello, World!") == 0);
    assert(own_string_copy != own_string_value); // Different instance
    
    // Clean up
    ar_data__destroy(own_string_value);
    ar_data__destroy(own_string_copy);
    
    // Given an empty map
    ar_data_t *own_empty_map = ar_data__create_map();
    assert(own_empty_map != NULL);
    
    // When we copy the empty map
    ar_data_t *own_map_copy = ar_data__shallow_copy(own_empty_map);
    
    // Then we should get a new empty map
    assert(own_map_copy != NULL);
    assert(ar_data__get_type(own_map_copy) == AR_DATA_TYPE__MAP);
    assert(own_map_copy != own_empty_map); // Different instance
    
    // Verify it's empty by checking keys
    ar_data_t *own_keys = ar_data__get_map_keys(own_map_copy);
    assert(own_keys != NULL);
    assert(ar_data__list_count(own_keys) == 0);
    
    // Clean up
    ar_data__destroy(own_keys);
    ar_data__destroy(own_empty_map);
    ar_data__destroy(own_map_copy);
    
    // Given a map with primitive values
    ar_data_t *own_map_with_primitives = ar_data__create_map();
    assert(own_map_with_primitives != NULL);
    assert(ar_data__set_map_integer(own_map_with_primitives, "age", 25));
    assert(ar_data__set_map_double(own_map_with_primitives, "score", 95.5));
    assert(ar_data__set_map_string(own_map_with_primitives, "name", "Alice"));
    
    // When we copy the map
    ar_data_t *own_map_primitives_copy = ar_data__shallow_copy(own_map_with_primitives);
    
    // Then we should get a new map with the same primitive values
    assert(own_map_primitives_copy != NULL);
    assert(ar_data__get_type(own_map_primitives_copy) == AR_DATA_TYPE__MAP);
    assert(own_map_primitives_copy != own_map_with_primitives); // Different instance
    
    // Verify the values were copied
    assert(ar_data__get_map_integer(own_map_primitives_copy, "age") == 25);
    assert(ar_data__get_map_double(own_map_primitives_copy, "score") == 95.5);
    assert(strcmp(ar_data__get_map_string(own_map_primitives_copy, "name"), "Alice") == 0);
    
    // Clean up
    ar_data__destroy(own_map_with_primitives);
    ar_data__destroy(own_map_primitives_copy);
    
    // Given a map with a nested map
    ar_data_t *own_map_with_nested = ar_data__create_map();
    assert(own_map_with_nested != NULL);
    assert(ar_data__set_map_integer(own_map_with_nested, "age", 30));
    
    // Create a nested map
    ar_data_t *own_nested_map = ar_data__create_map();
    assert(own_nested_map != NULL);
    assert(ar_data__set_map_string(own_nested_map, "city", "New York"));
    
    // Add the nested map
    assert(ar_data__set_map_data(own_map_with_nested, "address", own_nested_map));
    
    // When we try to copy the map with nested container
    ar_data_t *result_nested_map = ar_data__shallow_copy(own_map_with_nested);
    
    // Then we should get NULL (cannot shallow copy nested containers)
    assert(result_nested_map == NULL);
    
    // Clean up
    ar_data__destroy(own_map_with_nested);
    
    // Given a map with a nested list
    ar_data_t *own_map_with_list = ar_data__create_map();
    assert(own_map_with_list != NULL);
    assert(ar_data__set_map_string(own_map_with_list, "name", "Bob"));
    
    // Create a nested list
    ar_data_t *own_nested_list = ar_data__create_list();
    assert(own_nested_list != NULL);
    assert(ar_data__list_add_last_string(own_nested_list, "reading"));
    assert(ar_data__list_add_last_string(own_nested_list, "gaming"));
    
    // Add the nested list
    assert(ar_data__set_map_data(own_map_with_list, "hobbies", own_nested_list));
    
    // When we try to copy the map with nested list
    ar_data_t *result_nested_list = ar_data__shallow_copy(own_map_with_list);
    
    // Then we should get NULL (cannot shallow copy nested containers)
    assert(result_nested_list == NULL);
    
    // Clean up
    ar_data__destroy(own_map_with_list);
    
    // Given an empty list
    ar_data_t *own_empty_list = ar_data__create_list();
    assert(own_empty_list != NULL);
    
    // When we copy the empty list
    ar_data_t *own_list_copy = ar_data__shallow_copy(own_empty_list);
    
    // Then we should get a new empty list
    assert(own_list_copy != NULL);
    assert(ar_data__get_type(own_list_copy) == AR_DATA_TYPE__LIST);
    assert(own_list_copy != own_empty_list); // Different instance
    assert(ar_data__list_count(own_list_copy) == 0);
    
    // Clean up
    ar_data__destroy(own_empty_list);
    ar_data__destroy(own_list_copy);
    
    // Given a list with primitive values
    ar_data_t *own_list_with_primitives = ar_data__create_list();
    assert(own_list_with_primitives != NULL);
    assert(ar_data__list_add_last_integer(own_list_with_primitives, 10));
    assert(ar_data__list_add_last_double(own_list_with_primitives, 2.5));
    assert(ar_data__list_add_last_string(own_list_with_primitives, "test"));
    
    // When we copy the list
    ar_data_t *own_list_primitives_copy = ar_data__shallow_copy(own_list_with_primitives);
    
    // Then we should get a new list with the same primitive values
    assert(own_list_primitives_copy != NULL);
    assert(ar_data__get_type(own_list_primitives_copy) == AR_DATA_TYPE__LIST);
    assert(own_list_primitives_copy != own_list_with_primitives); // Different instance
    assert(ar_data__list_count(own_list_primitives_copy) == 3);
    
    // Verify the values were copied (check first item)
    ar_data_t *ref_first = ar_data__list_first(own_list_primitives_copy);
    assert(ref_first != NULL);
    assert(ar_data__get_type(ref_first) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_first) == 10);
    
    // Clean up
    ar_data__destroy(own_list_with_primitives);
    ar_data__destroy(own_list_primitives_copy);
    
    // Given a list with a nested map
    ar_data_t *own_list_with_map = ar_data__create_list();
    assert(own_list_with_map != NULL);
    assert(ar_data__list_add_last_integer(own_list_with_map, 5));
    
    // Create a nested map
    ar_data_t *own_nested_map_in_list = ar_data__create_map();
    assert(own_nested_map_in_list != NULL);
    assert(ar_data__set_map_string(own_nested_map_in_list, "key", "value"));
    
    // Add the nested map to the list
    assert(ar_data__list_add_last_data(own_list_with_map, own_nested_map_in_list));
    
    // When we try to copy the list with nested map
    ar_data_t *result_list_with_map = ar_data__shallow_copy(own_list_with_map);
    
    // Then we should get NULL (cannot shallow copy nested containers)
    assert(result_list_with_map == NULL);
    
    // Clean up
    ar_data__destroy(own_list_with_map);
    
    // Given a list with a nested list
    ar_data_t *own_list_with_list = ar_data__create_list();
    assert(own_list_with_list != NULL);
    assert(ar_data__list_add_last_string(own_list_with_list, "first"));
    
    // Create a nested list
    ar_data_t *own_nested_list_in_list = ar_data__create_list();
    assert(own_nested_list_in_list != NULL);
    assert(ar_data__list_add_last_integer(own_nested_list_in_list, 100));
    
    // Add the nested list
    assert(ar_data__list_add_last_data(own_list_with_list, own_nested_list_in_list));
    
    // When we try to copy the list with nested list
    ar_data_t *result_list_with_list = ar_data__shallow_copy(own_list_with_list);
    
    // Then we should get NULL (cannot shallow copy nested containers)
    assert(result_list_with_list == NULL);
    
    // Clean up
    ar_data__destroy(own_list_with_list);
    
    printf("Shallow copy tests passed!\n");
}

static void test_data_is_primitive_type(void) {
    printf("Testing ar_data__is_primitive_type...\n");
    
    // Given we need to test type classification
    
    // When we check NULL
    assert(ar_data__is_primitive_type(NULL) == false);
    
    // When we check primitive types
    ar_data_t *own_int = ar_data__create_integer(42);
    assert(ar_data__is_primitive_type(own_int) == true);
    ar_data__destroy(own_int);
    
    ar_data_t *own_double = ar_data__create_double(3.14);
    assert(ar_data__is_primitive_type(own_double) == true);
    ar_data__destroy(own_double);
    
    ar_data_t *own_string = ar_data__create_string("test");
    assert(ar_data__is_primitive_type(own_string) == true);
    ar_data__destroy(own_string);
    
    // When we check container types
    ar_data_t *own_map = ar_data__create_map();
    assert(ar_data__is_primitive_type(own_map) == false);
    ar_data__destroy(own_map);
    
    ar_data_t *own_list = ar_data__create_list();
    assert(ar_data__is_primitive_type(own_list) == false);
    ar_data__destroy(own_list);
    
    printf("is_primitive_type tests passed!\n");
}

static void test_data_map_contains_only_primitives(void) {
    printf("Testing ar_data__map_contains_only_primitives...\n");
    
    // Given we need to test map validation
    
    // When we check NULL
    assert(ar_data__map_contains_only_primitives(NULL) == false);
    
    // When we check non-map type
    ar_data_t *own_int = ar_data__create_integer(42);
    assert(ar_data__map_contains_only_primitives(own_int) == false);
    ar_data__destroy(own_int);
    
    // When we check empty map
    ar_data_t *own_empty_map = ar_data__create_map();
    assert(ar_data__map_contains_only_primitives(own_empty_map) == true);
    ar_data__destroy(own_empty_map);
    
    // When we check map with only primitives
    ar_data_t *own_primitive_map = ar_data__create_map();
    ar_data__set_map_integer(own_primitive_map, "int", 42);
    ar_data__set_map_double(own_primitive_map, "double", 3.14);
    ar_data__set_map_string(own_primitive_map, "string", "test");
    assert(ar_data__map_contains_only_primitives(own_primitive_map) == true);
    ar_data__destroy(own_primitive_map);
    
    // When we check map with nested map
    ar_data_t *own_nested_map = ar_data__create_map();
    ar_data__set_map_integer(own_nested_map, "int", 42);
    ar_data_t *own_inner_map = ar_data__create_map();
    ar_data__set_map_data(own_nested_map, "map", own_inner_map);
    assert(ar_data__map_contains_only_primitives(own_nested_map) == false);
    ar_data__destroy(own_nested_map);
    
    // When we check map with nested list
    ar_data_t *own_list_map = ar_data__create_map();
    ar_data__set_map_string(own_list_map, "string", "test");
    ar_data_t *own_inner_list = ar_data__create_list();
    ar_data__set_map_data(own_list_map, "list", own_inner_list);
    assert(ar_data__map_contains_only_primitives(own_list_map) == false);
    ar_data__destroy(own_list_map);
    
    printf("map_contains_only_primitives tests passed!\n");
}

static void test_data_list_contains_only_primitives(void) {
    printf("Testing ar_data__list_contains_only_primitives...\n");
    
    // Given we need to test list validation
    
    // When we check NULL
    assert(ar_data__list_contains_only_primitives(NULL) == false);
    
    // When we check non-list type
    ar_data_t *own_int = ar_data__create_integer(42);
    assert(ar_data__list_contains_only_primitives(own_int) == false);
    ar_data__destroy(own_int);
    
    // When we check empty list
    ar_data_t *own_empty_list = ar_data__create_list();
    assert(ar_data__list_contains_only_primitives(own_empty_list) == true);
    ar_data__destroy(own_empty_list);
    
    // When we check list with only primitives
    ar_data_t *own_primitive_list = ar_data__create_list();
    ar_data__list_add_last_integer(own_primitive_list, 42);
    ar_data__list_add_last_double(own_primitive_list, 3.14);
    ar_data__list_add_last_string(own_primitive_list, "test");
    assert(ar_data__list_contains_only_primitives(own_primitive_list) == true);
    ar_data__destroy(own_primitive_list);
    
    // When we check list with nested map
    ar_data_t *own_map_list = ar_data__create_list();
    ar_data__list_add_last_integer(own_map_list, 42);
    ar_data_t *own_inner_map = ar_data__create_map();
    ar_data__list_add_last_data(own_map_list, own_inner_map);
    assert(ar_data__list_contains_only_primitives(own_map_list) == false);
    ar_data__destroy(own_map_list);
    
    // When we check list with nested list
    ar_data_t *own_list_list = ar_data__create_list();
    ar_data__list_add_last_string(own_list_list, "test");
    ar_data_t *own_inner_list = ar_data__create_list();
    ar_data__list_add_last_data(own_list_list, own_inner_list);
    assert(ar_data__list_contains_only_primitives(own_list_list) == false);
    ar_data__destroy(own_list_list);
    
    printf("list_contains_only_primitives tests passed!\n");
}

static void test_data_claim_or_copy(void) {
    printf("Testing data claim or copy...\n");
    
    // Test 1: Can claim unowned data
    ar_data_t *own_data = ar_data__create_integer(42);
    void *owner = (void*)0x1234;
    
    // When we call claim_or_copy on unowned data
    ar_data_t *own_result = ar_data__claim_or_copy(own_data, owner);
    
    // Then we should get the same pointer back (claimed original)
    assert(own_result == own_data);
    assert(ar_data__get_integer(own_result) == 42);
    ar_data__destroy(own_result);
    
    // Test 2: Copy when data is owned by someone else
    ar_data_t *own_owned_data = ar_data__create_string("test");
    void *other_owner = (void*)0x5678;
    ar_data__take_ownership(own_owned_data, other_owner);
    
    // When we call claim_or_copy on owned data
    ar_data_t *own_copy = ar_data__claim_or_copy(own_owned_data, owner);
    
    // Then we should get a copy
    assert(own_copy != own_owned_data);
    assert(strcmp(ar_data__get_string(own_copy), "test") == 0);
    ar_data__destroy(own_copy);
    ar_data__drop_ownership(own_owned_data, other_owner);
    ar_data__destroy(own_owned_data);
    
    // Test 3: Return NULL when copy fails (nested container)
    ar_data_t *own_map = ar_data__create_map();
    ar_data_t *own_nested_map = ar_data__create_map();
    ar_data__set_map_data(own_map, "nested", own_nested_map);
    ar_data__take_ownership(own_map, other_owner);
    
    // When we call claim_or_copy on nested container
    ar_data_t *own_failed_copy = ar_data__claim_or_copy(own_map, owner);
    
    // Then we should get NULL (shallow copy fails on nested containers)
    assert(own_failed_copy == NULL);
    ar_data__drop_ownership(own_map, other_owner);
    ar_data__destroy(own_map);
    
    printf("claim_or_copy tests passed!\n");
}

static void test_data_destroy_if_owned(void) {
    printf("Testing data destroy if owned...\n");
    
    // Test 1: Destroy unowned data (should succeed)
    ar_data_t *own_data = ar_data__create_integer(42);
    void *owner = (void*)0x1234;
    
    // When we call destroy_if_owned on unowned data
    ar_data__destroy_if_owned(own_data, owner);
    
    // Then it should be destroyed (no way to test this directly, but it should not crash)
    
    // Test 2: Try to destroy data owned by someone else (should be safe no-op)
    ar_data_t *own_owned_data = ar_data__create_string("test");
    void *other_owner = (void*)0x5678;
    ar_data__take_ownership(own_owned_data, other_owner);
    
    // When we call destroy_if_owned on data owned by someone else
    ar_data__destroy_if_owned(own_owned_data, owner);
    
    // Then it should still be accessible (not destroyed)
    assert(strcmp(ar_data__get_string(own_owned_data), "test") == 0);
    
    // Clean up properly
    ar_data__drop_ownership(own_owned_data, other_owner);
    ar_data__destroy(own_owned_data);
    
    // Test 3: Handle NULL data safely
    ar_data__destroy_if_owned(NULL, owner);
    
    // Test 4: Handle NULL owner safely  
    ar_data_t *own_safe_data = ar_data__create_integer(123);
    ar_data__destroy_if_owned(own_safe_data, NULL);
    // Should still be accessible since NULL owner is invalid
    assert(ar_data__get_integer(own_safe_data) == 123);
    ar_data__destroy(own_safe_data);
    
    printf("destroy_if_owned tests passed!\n");
}

static void test_data_list_items(void) {
    printf("Testing ar_data__list_items...\n");
    
    // Test 1: Empty list
    {
        ar_data_t *own_list = ar_data__create_list();
        assert(own_list != NULL);
        
        ar_data_t **items = ar_data__list_items(own_list);
        size_t count = ar_data__list_count(own_list);
        
        assert(count == 0);
        assert(items == NULL);
        
        ar_data__destroy(own_list);
    }
    
    // Test 2: List with multiple items
    {
        ar_data_t *own_list = ar_data__create_list();
        assert(own_list != NULL);
        
        // Add various types of data
        ar_data__list_add_last_integer(own_list, 42);
        ar_data__list_add_last_string(own_list, "hello");
        ar_data__list_add_last_double(own_list, 3.14);
        
        ar_data_t **items = ar_data__list_items(own_list);
        size_t count = ar_data__list_count(own_list);
        
        assert(count == 3);
        assert(items != NULL);
        
        // Verify items
        assert(ar_data__get_type(items[0]) == AR_DATA_TYPE__INTEGER);
        assert(ar_data__get_integer(items[0]) == 42);
        
        assert(ar_data__get_type(items[1]) == AR_DATA_TYPE__STRING);
        assert(strcmp(ar_data__get_string(items[1]), "hello") == 0);
        
        assert(ar_data__get_type(items[2]) == AR_DATA_TYPE__DOUBLE);
        assert(ar_data__get_double(items[2]) == 3.14);
        
        // Free the items array (but not the items themselves)
        AR__HEAP__FREE(items);
        
        ar_data__destroy(own_list);
    }
    
    printf("list_items tests passed!\n");
}

static void test_data_format_structure(void) {
    printf("Testing ar_data__format_structure...\n");
    
    // Test 1: Simple types
    {
        printf("Creating integer 42...\n");
        ar_data_t *own_int = ar_data__create_integer(42);
        printf("Formatting integer...\n");
        char *str = ar_data__format_structure(own_int, 3);
        assert(str != NULL);
        printf("Integer format result: '%s'\n", str);
        assert(strcmp(str, "42") == 0);
        printf("Freeing string...\n");
        AR__HEAP__FREE(str);
        printf("Destroying integer data...\n");
        ar_data__destroy(own_int);
        printf("Integer test complete.\n");
        
        printf("Creating double 3.14...\n");
        ar_data_t *own_double = ar_data__create_double(3.14);
        printf("Formatting double...\n");
        str = ar_data__format_structure(own_double, 3);
        assert(str != NULL);
        printf("Double format result: '%s'\n", str);
        // The output format might vary, let's just check it starts with "3.14"
        assert(strncmp(str, "3.14", 4) == 0);
        printf("Freeing string...\n");
        AR__HEAP__FREE(str);
        printf("Destroying double data...\n");
        ar_data__destroy(own_double);
        printf("Double test complete.\n");
        
        printf("Creating string 'hello'...\n");
        ar_data_t *own_string = ar_data__create_string("hello");
        printf("Formatting string...\n");
        char *str2 = ar_data__format_structure(own_string, 3);
        assert(str2 != NULL);
        printf("String format result: '%s'\n", str2);
        assert(strcmp(str2, "\"hello\"") == 0);
        printf("Freeing string...\n");
        AR__HEAP__FREE(str2);
        printf("Destroying string data...\n");
        ar_data__destroy(own_string);
        printf("String test complete.\n");
    }
    
    // Test 2: List formatting
    {
        printf("Creating list...\n");
        ar_data_t *own_list = ar_data__create_list();
        printf("Adding integers to list...\n");
        ar_data__list_add_last_integer(own_list, 1);
        ar_data__list_add_last_integer(own_list, 2);
        ar_data__list_add_last_integer(own_list, 3);
        
        printf("Formatting list...\n");
        char *str = ar_data__format_structure(own_list, 3);
        assert(str != NULL);
        printf("List format result: '%s'\n", str);
        assert(strcmp(str, "LIST[1, 2, 3]") == 0);
        printf("Freeing string...\n");
        AR__HEAP__FREE(str);
        printf("Destroying list...\n");
        ar_data__destroy(own_list);
        printf("List test complete.\n");
    }
    
    // Test 3: Map formatting
    {
        printf("Creating map...\n");
        ar_data_t *own_map = ar_data__create_map();
        printf("Setting map values...\n");
        ar_data__set_map_integer(own_map, "x", 10);
        ar_data__set_map_string(own_map, "name", "test");
        
        printf("Formatting map...\n");
        char *str = ar_data__format_structure(own_map, 3);
        printf("After ar_data__format_structure call, str = %p\n", (void*)str);
        assert(str != NULL);
        printf("Map format result: '%s'\n", str);
        // Note: order might vary, so just check it contains expected parts
        assert(strstr(str, "MAP{") != NULL);
        assert(strstr(str, "x: 10") != NULL);
        assert(strstr(str, "name: \"test\"") != NULL);
        assert(strstr(str, "}") != NULL);
        printf("Freeing string...\n");
        AR__HEAP__FREE(str);
        printf("Destroying map...\n");
        ar_data__destroy(own_map);
        printf("Map test complete.\n");
    }
    
    // Test 4: Nested structures with depth limit
    {
        ar_data_t *own_outer_map = ar_data__create_map();
        ar_data_t *own_inner_list = ar_data__create_list();
        ar_data_t *own_deep_map = ar_data__create_map();
        
        ar_data__set_map_integer(own_deep_map, "value", 999);
        ar_data__list_add_last_data(own_inner_list, own_deep_map);
        ar_data__list_add_last_integer(own_inner_list, 42);
        ar_data__set_map_data(own_outer_map, "items", own_inner_list);
        
        // Test with depth 3 - should show all
        char *str = ar_data__format_structure(own_outer_map, 3);
        assert(str != NULL);
        // Just check that it contains the key parts, order might vary
        assert(strstr(str, "items:") != NULL);
        assert(strstr(str, "LIST[") != NULL);
        assert(strstr(str, "value: 999") != NULL);
        assert(strstr(str, "42") != NULL);
        AR__HEAP__FREE(str);
        
        // Test with depth 1 - should truncate
        str = ar_data__format_structure(own_outer_map, 1);
        assert(str != NULL);
        printf("Depth 1 format result: '%s'\n", str);
        // At depth 1, we see the map and its immediate values. The list shows but its contents are truncated
        assert(strstr(str, "items: LIST[") != NULL);
        assert(strstr(str, "...") != NULL);
        AR__HEAP__FREE(str);
        
        ar_data__destroy(own_outer_map);
    }
    
    printf("format_structure tests passed!\n");
}

static void test_data_set_map_data_if_root_matched(void) {
    printf("Testing ar_data__set_map_data_if_root_matched...\n");
    
    // Test 1: Successful storage when root matches
    {
        // Given a map and a value to store
        ar_data_t *own_map = ar_data__create_map();
        assert(own_map != NULL);
        ar_data_t *own_value = ar_data__create_integer(42);
        assert(own_value != NULL);
        
        // When we set with matching root
        bool result = ar_data__set_map_data_if_root_matched(own_map, "memory", "memory.x", own_value);
        
        // Then it should succeed and the value should be stored
        assert(result == true);
        assert(ar_data__get_map_integer(own_map, "x") == 42);
        
        // Cleanup
        ar_data__destroy(own_map);
    }
    
    // Test 2: Returns false when path is NULL
    {
        // Given a map and a value
        ar_data_t *own_map = ar_data__create_map();
        assert(own_map != NULL);
        ar_data_t *own_value = ar_data__create_integer(42);
        assert(own_value != NULL);
        
        // When we set with NULL path
        bool result = ar_data__set_map_data_if_root_matched(own_map, "memory", NULL, own_value);
        
        // Then it should return false and we still own the value
        assert(result == false);
        
        // Cleanup (we still own the value)
        ar_data__destroy(own_value);
        ar_data__destroy(own_map);
    }
    
    // Test 3: Returns false when root doesn't match
    {
        // Given a map and a value
        ar_data_t *own_map = ar_data__create_map();
        assert(own_map != NULL);
        ar_data_t *own_value = ar_data__create_integer(42);
        assert(own_value != NULL);
        
        // When we set with non-matching root
        bool result = ar_data__set_map_data_if_root_matched(own_map, "memory", "context.x", own_value);
        
        // Then it should return false and we still own the value
        assert(result == false);
        
        // Cleanup (we still own the value)
        ar_data__destroy(own_value);
        ar_data__destroy(own_map);
    }
    
    // Test 4: Different root with simple path
    {
        // Given a map and a value
        ar_data_t *own_map = ar_data__create_map();
        assert(own_map != NULL);
        ar_data_t *own_value = ar_data__create_string("test value");
        assert(own_value != NULL);
        
        // When we set with different root
        bool result = ar_data__set_map_data_if_root_matched(own_map, "context", "context.value", own_value);
        
        // Then it should succeed
        assert(result == true);
        const char *value = ar_data__get_map_string(own_map, "value");
        assert(value != NULL);
        assert(strcmp(value, "test value") == 0);
        
        // Cleanup
        ar_data__destroy(own_map);
    }
    
    // Test 5: Nested path storage with pre-existing intermediate map
    {
        // Given a map with user map already created
        ar_data_t *own_map = ar_data__create_map();
        assert(own_map != NULL);
        ar_data_t *own_user_map = ar_data__create_map();
        assert(own_user_map != NULL);
        ar_data__set_map_data(own_map, "user", own_user_map);
        
        // When we set with nested path
        ar_data_t *own_value = ar_data__create_string("John");
        assert(own_value != NULL);
        bool result = ar_data__set_map_data_if_root_matched(own_map, "memory", "memory.user.name", own_value);
        
        // Then it should succeed
        assert(result == true);
        const char *name = ar_data__get_map_string(own_map, "user.name");
        assert(name != NULL);
        assert(strcmp(name, "John") == 0);
        
        // Cleanup
        ar_data__destroy(own_map);
    }
    
    // Test 6: Empty suffix returns false
    {
        // Given a map and a value
        ar_data_t *own_map = ar_data__create_map();
        assert(own_map != NULL);
        ar_data_t *own_value = ar_data__create_integer(42);
        assert(own_value != NULL);
        
        // When we set with empty suffix
        bool result = ar_data__set_map_data_if_root_matched(own_map, "memory", "memory.", own_value);
        
        // Then it should return false
        assert(result == false);
        
        // Cleanup (we still own the value)
        ar_data__destroy(own_value);
        ar_data__destroy(own_map);
    }
}

int main(void) {
    printf("Starting Data Module Tests...\n");
    printf("DEBUG: About to start tests\n");
    
    // When we run all data tests
    test_data_creation();
    test_data_getters();
    test_integer_values();
    test_string_values();
    test_nested_maps();
    
    // Run map data getter tests
    test_map_data_getters();
    
    // Run map data setter tests
    test_map_data_setters();
    
    // Run map data path getter tests
    test_map_data_path_getters();
    
    // Run map data path setter tests
    test_map_data_path_setters();
    
    // Run list operations tests
    test_list_operations();  // Now enabled after TDD Cycle 3
    test_list_basic_create_destroy();
    test_list_add_integers();
    test_list_add_remove_integers();
    
    // Run map tests  
    test_map_empty_destroy();
    test_map_add_one_integer();
    test_map_update_integer();
    
    // Run ownership tests
    test_data_ownership();
    test_list_ownership();
    test_list_remove_ownership();
    test_map_ownership();
    
    // Run shallow copy tests
    test_data_shallow_copy();
    
    // Run is primitive type tests
    test_data_is_primitive_type();
    
    // Run map contains only primitives tests
    test_data_map_contains_only_primitives();
    
    // Run list contains only primitives tests
    test_data_list_contains_only_primitives();
    
    // Run claim or copy tests
    test_data_claim_or_copy();
    
    // Run destroy if owned tests
    test_data_destroy_if_owned();
    
    // Run set map data if root matched tests
    test_data_set_map_data_if_root_matched();
    
    // Run list items tests
    test_data_list_items();
    
    printf("DEBUG: About to run format structure tests\n");
    // Run format structure tests
    test_data_format_structure();
    
    // Then all tests should pass
    printf("All 29 tests passed!\n");
    
    return 0;
}
