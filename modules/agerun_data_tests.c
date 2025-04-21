#include "agerun_data.h"
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
static void test_list_operations(void);

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
    
    // When we create an empty list data item
    data_t *list_data_default = ar_data_create_list();
    
    // Then it should have the correct type
    assert(list_data_default != NULL);
    assert(ar_data_get_type(list_data_default) == DATA_LIST);
    
    // When we create an empty map data item
    data_t *map_data_default = ar_data_create_map();
    
    // Then it should have the correct type and a valid map
    assert(map_data_default != NULL);
    assert(ar_data_get_type(map_data_default) == DATA_MAP);
    // We can verify it's a map by checking its type - can't check ar_data_get_map anymore
    
    // When we create data items with the specialized functions
    data_t *int_data = ar_data_create_integer(42);
    data_t *double_data = ar_data_create_double(3.14159);
    data_t *string_data = ar_data_create_string("Hello, World!");
    data_t *list_data = ar_data_create_list();
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
    
    assert(list_data != NULL);
    assert(ar_data_get_type(list_data) == DATA_LIST);
    
    assert(map_data != NULL);
    assert(ar_data_get_type(map_data) == DATA_MAP);
    // We can verify it's a map by checking its type - can't check ar_data_get_map anymore
    
    // Cleanup
    ar_data_destroy(map_data_default);
    ar_data_destroy(list_data_default);
    ar_data_destroy(string_data_default);
    ar_data_destroy(double_data_default);
    ar_data_destroy(int_data_default);
    
    ar_data_destroy(map_data);
    ar_data_destroy(list_data);
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
    data_t *list_data = ar_data_create_list();
    data_t *map_data = ar_data_create_map();
    
    // When we use the type getter
    data_type_t int_type = ar_data_get_type(int_data);
    data_type_t double_type = ar_data_get_type(double_data);
    data_type_t string_type = ar_data_get_type(string_data);
    data_type_t list_type = ar_data_get_type(list_data);
    data_type_t map_type = ar_data_get_type(map_data);
    data_type_t null_type = ar_data_get_type(NULL);
    
    // Then the types should be correct
    assert(int_type == DATA_INTEGER);
    assert(double_type == DATA_DOUBLE);
    assert(string_type == DATA_STRING);
    assert(list_type == DATA_LIST);
    assert(map_type == DATA_MAP);
    assert(null_type == DATA_INTEGER); // Default to int if NULL
    
    // When we use the value getters with the correct types
    int int_value = ar_data_get_integer(int_data);
    double double_value = ar_data_get_double(double_data);
    const char *string_value = ar_data_get_string(string_data);
    
    // Then they should return the correct values
    assert(int_value == 42);
    assert(double_value == 3.14159);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // When we use the getters with incorrect types
    int wrong_int = ar_data_get_integer(string_data);
    double wrong_double = ar_data_get_double(int_data);
    const char *wrong_string = ar_data_get_string(int_data);
    
    // Then they should return default values
    assert(wrong_int == 0);
    assert(wrong_double == 0.0);
    assert(wrong_string == NULL);
    
    // When we use the getters with NULL
    int null_int = ar_data_get_integer(NULL);
    double null_double = ar_data_get_double(NULL);
    const char *null_string = ar_data_get_string(NULL);
    
    // Then they should return default values
    assert(null_int == 0);
    assert(null_double == 0.0);
    assert(null_string == NULL);
    
    // Cleanup
    ar_data_destroy(int_data);
    ar_data_destroy(double_data);
    ar_data_destroy(string_data);
    ar_data_destroy(list_data);
    ar_data_destroy(map_data);
    
    printf("Data getter tests passed!\n");
}

static void test_integer_values(void) {
    printf("Testing integer values in map...\n");
    
    // Create a data wrapper for the map parameter
    data_t *map_data = ar_data_create_map();
    
    // When we set an integer value in the map
    bool result = ar_data_set_map_integer(map_data, "answer", 42);
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const data_t *value = ar_data_get_map_data(map_data, "answer");
    
    // Then the value should be correctly retrieved
    assert(value != NULL);
    assert(ar_data_get_type(value) == DATA_INTEGER);
    assert(ar_data_get_integer(value) == 42);
    
    // Verify the integer is accessible via getter
    int int_value = ar_data_get_map_integer(map_data, "answer");
    assert(int_value == 42);
    
    // Cleanup
    ar_data_destroy(map_data);
    
    printf("Integer value tests passed!\n");
}

static void test_string_values(void) {
    printf("Testing string values in map...\n");
    
    // Create a data wrapper for the map parameter
    data_t *map_data = ar_data_create_map();
    
    // When we set a string value in the map
    bool result = ar_data_set_map_string(map_data, "greeting", "Hello, World!");
    
    // Then the operation should succeed
    assert(result);
    
    // When we retrieve the referenced value from the map
    const data_t *value = ar_data_get_map_data(map_data, "greeting");
    
    // Then the value should be correctly retrieved
    assert(value != NULL);
    assert(ar_data_get_type(value) == DATA_STRING);
    assert(strcmp(ar_data_get_string(value), "Hello, World!") == 0);
    
    // Verify the string is accessible via getter
    const char *string_value = ar_data_get_map_string(map_data, "greeting");
    assert(string_value != NULL);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // Cleanup
    ar_data_destroy(map_data);
    
    printf("String value tests passed!\n");
}

static void test_nested_maps(void) {
    printf("Testing nested maps...\n");
    
    // Given a data structure to store nested maps
    data_t *root_data = ar_data_create_map();
    assert(root_data != NULL);
    assert(ar_data_get_type(root_data) == DATA_MAP);
    
    // Create first level map
    data_t *first_level = ar_data_create_map();
    assert(first_level != NULL);
    
    // Set an integer value in the first level map
    bool result = ar_data_set_map_integer(first_level, "count", 100);
    assert(result);
    
    // Add the first level map to the root
    result = ar_data_set_map_data(root_data, "user_data", first_level);
    assert(result);
    
    // Verify the value can be retrieved via the path
    int count_value = ar_data_get_map_integer(root_data, "user_data.count");
    assert(count_value == 100);
    
    // Create second level map
    data_t *second_level = ar_data_create_map();
    assert(second_level != NULL);
    
    // Create third level map with a string value
    data_t *third_level = ar_data_create_map();
    assert(third_level != NULL);
    result = ar_data_set_map_string(third_level, "key", "Deep value!");
    assert(result);
    
    // Add the third level map to the second level map
    result = ar_data_set_map_data(second_level, "more_data", third_level);
    assert(result);
    
    // Add the second level map to the first level map
    result = ar_data_set_map_data(first_level, "nested", second_level);
    assert(result);
    
    // Verify deep nested value can be retrieved via path
    const char *deep_value = ar_data_get_map_string(root_data, "user_data.nested.more_data.key");
    assert(deep_value != NULL);
    assert(strcmp(deep_value, "Deep value!") == 0);
    
    // Cleanup
    ar_data_destroy(root_data);
    
    printf("Nested maps tests passed!\n");
}

static void test_map_data_getters(void) {
    printf("Testing map data getters...\n");
    
    // Given a map data structure with different data types
    data_t *map_data = ar_data_create_map();
    
    // Set values of different types in the map using ar_data_set_map_* functions
    bool int_result = ar_data_set_map_integer(map_data, "int_key", 42);
    bool double_result = ar_data_set_map_double(map_data, "double_key", 3.14159);
    bool string_result = ar_data_set_map_string(map_data, "string_key", "Hello, World!");
    
    // Create a nested map and set it in the parent map
    data_t *nested_map_data = ar_data_create_map();
    bool map_result = ar_data_set_map_data(map_data, "map_key", nested_map_data);
    
    // All operations should succeed
    assert(int_result);
    assert(double_result);
    assert(string_result);
    assert(map_result);
    
    // When we use the map data getters with the correct keys
    int int_value = ar_data_get_map_integer(map_data, "int_key");
    double double_value = ar_data_get_map_double(map_data, "double_key");
    const char *string_value = ar_data_get_map_string(map_data, "string_key");
    
    // Then they should return the correct values
    assert(int_value == 42);
    assert(double_value == 3.14159);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // When we use ar_data_get_map_data with correct keys
    const data_t *int_data_direct = ar_data_get_map_data(map_data, "int_key");
    const data_t *double_data_direct = ar_data_get_map_data(map_data, "double_key");
    const data_t *string_data_direct = ar_data_get_map_data(map_data, "string_key");
    const data_t *map_data_direct = ar_data_get_map_data(map_data, "map_key");
    
    // Then it should return the correct data objects
    assert(int_data_direct != NULL);
    assert(double_data_direct != NULL);
    assert(string_data_direct != NULL);
    assert(map_data_direct != NULL);
    
    // And the data objects should have the correct types
    assert(ar_data_get_type(int_data_direct) == DATA_INTEGER);
    assert(ar_data_get_type(double_data_direct) == DATA_DOUBLE);
    assert(ar_data_get_type(string_data_direct) == DATA_STRING);
    assert(ar_data_get_type(map_data_direct) == DATA_MAP);
    
    // And the data objects should contain the correct values
    assert(ar_data_get_integer(int_data_direct) == 42);
    assert(ar_data_get_double(double_data_direct) == 3.14159);
    assert(strcmp(ar_data_get_string(string_data_direct), "Hello, World!") == 0);
    // For map data, we verify the type is correct
    assert(ar_data_get_type(map_data_direct) == DATA_MAP);
    
    // When we use the map data getters with incorrect keys
    int wrong_int = ar_data_get_map_integer(map_data, "nonexistent_key");
    double wrong_double = ar_data_get_map_double(map_data, "nonexistent_key");
    const char *wrong_string = ar_data_get_map_string(map_data, "nonexistent_key");
    const data_t *wrong_data = ar_data_get_map_data(map_data, "nonexistent_key");
    
    // Then they should return default values
    assert(wrong_int == 0);
    assert(wrong_double == 0.0);
    assert(wrong_string == NULL);
    assert(wrong_data == NULL);
    
    // When we use the map data getters with NULL data or key
    int null_data_int = ar_data_get_map_integer(NULL, "int_key");
    double null_key_double = ar_data_get_map_double(map_data, NULL);
    const data_t *null_data_obj = ar_data_get_map_data(NULL, "int_key");
    const data_t *null_key_obj = ar_data_get_map_data(map_data, NULL);
    
    // Then they should return default values
    assert(null_data_int == 0);
    assert(null_key_double == 0.0);
    assert(null_data_obj == NULL);
    assert(null_key_obj == NULL);
    
    // When we use the map data getters with incorrect types
    int wrong_type_int = ar_data_get_map_integer(map_data, "string_key");
    double wrong_type_double = ar_data_get_map_double(map_data, "int_key");
    const char *wrong_type_string = ar_data_get_map_string(map_data, "double_key");
    
    // Then they should return default values
    assert(wrong_type_int == 0);
    assert(wrong_type_double == 0.0);
    assert(wrong_type_string == NULL);
    
    // Create a data wrapper for the nested map value
    data_t *nested_data = ar_data_create_map();
    
    // Set a value in the nested map
    bool nested_result = ar_data_set_map_integer(nested_data, "nested_int", 100);
    assert(nested_result);
    
    // Cleanup
    ar_data_destroy(nested_data);
    
    // Cleanup
    ar_data_destroy(map_data);
    
    printf("Map data getter tests passed!\n");
}

static void test_map_data_setters(void) {
    printf("Testing map data setters...\n");
    
    // Given a map data structure for storing different data types
    data_t *map_data = ar_data_create_map();
    
    // When we set different data types using the map data setters
    bool int_result = ar_data_set_map_integer(map_data, "int_key", 42);
    bool double_result = ar_data_set_map_double(map_data, "double_key", 3.14159);
    bool string_result = ar_data_set_map_string(map_data, "string_key", "Hello, World!");
    
    // Then the operations should succeed
    assert(int_result);
    assert(double_result);
    assert(string_result);
    
    // And the values should be retrievable using the corresponding getters
    int int_value = ar_data_get_map_integer(map_data, "int_key");
    double double_value = ar_data_get_map_double(map_data, "double_key");
    const char *string_value = ar_data_get_map_string(map_data, "string_key");
    
    // Print the values for debugging
    printf("int_value: %d\n", int_value);
    printf("double_value: %f\n", double_value);
    printf("string_value: %s\n", string_value ? string_value : "NULL");
    
    // Then they should match the set values
    assert(int_value == 42);
    assert(double_value == 3.14159);
    assert(strcmp(string_value, "Hello, World!") == 0);
    
    // When we update existing values
    bool update_int = ar_data_set_map_integer(map_data, "int_key", 100);
    bool update_double = ar_data_set_map_double(map_data, "double_key", 2.71828);
    bool update_string = ar_data_set_map_string(map_data, "string_key", "Updated text");
    
    // Then the operations should succeed
    assert(update_int);
    assert(update_double);
    assert(update_string);
    
    // And the updated values should be retrievable
    int updated_int = ar_data_get_map_integer(map_data, "int_key");
    double updated_double = ar_data_get_map_double(map_data, "double_key");
    const char *updated_string = ar_data_get_map_string(map_data, "string_key");
    
    // Then they should match the updated values
    assert(updated_int == 100);
    assert(updated_double == 2.71828);
    assert(strcmp(updated_string, "Updated text") == 0);
    
    // When we set values with NULL data or key
    bool null_data_result = ar_data_set_map_integer(NULL, "int_key", 42);
    bool null_key_result = ar_data_set_map_double(map_data, NULL, 3.14159);
    
    // Then the operations should fail
    assert(!null_data_result);
    assert(!null_key_result);
    
    // When we set values in a non-map data type
    data_t *int_data = ar_data_create_integer(42);
    bool wrong_type_result = ar_data_set_map_string(int_data, "key", "value");
    
    // Then the operation should fail
    assert(!wrong_type_result);
    
    // Cleanup
    ar_data_destroy(int_data);
    ar_data_destroy(map_data);
    
    printf("Map data setter tests passed!\n");
}

static void test_map_data_path_getters(void) {
    printf("Testing map data path getters...\n");
    
    // Given a nested map data structure with various data types
    data_t *root_map = ar_data_create_map();
    
    // Create the nested maps
    data_t *user_map = ar_data_create_map();
    data_t *address_map = ar_data_create_map();
    data_t *scores_map = ar_data_create_map();
    
    // Set values in the address map
    bool result = ar_data_set_map_string(address_map, "street", "123 Main St");
    assert(result);
    result = ar_data_set_map_string(address_map, "city", "Anytown");
    assert(result);
    result = ar_data_set_map_integer(address_map, "zip", 12345);
    assert(result);
    
    // Set values in the scores map
    result = ar_data_set_map_integer(scores_map, "math", 95);
    assert(result);
    result = ar_data_set_map_integer(scores_map, "science", 87);
    assert(result);
    result = ar_data_set_map_double(scores_map, "average", 91.0);
    assert(result);
    
    // Set values in the user map
    result = ar_data_set_map_string(user_map, "name", "John Doe");
    assert(result);
    result = ar_data_set_map_integer(user_map, "age", 30);
    assert(result);
    
    // Add address and scores maps to the user map
    result = ar_data_set_map_data(user_map, "address", address_map);
    assert(result);
    result = ar_data_set_map_data(user_map, "scores", scores_map);
    assert(result);
    
    // Add user map to the root map
    result = ar_data_set_map_data(root_map, "user", user_map);
    assert(result);
    
    // When we use the path-based getters
    int age = ar_data_get_map_integer(root_map, "user.age");
    const char *name = ar_data_get_map_string(root_map, "user.name");
    const char *street = ar_data_get_map_string(root_map, "user.address.street");
    const char *city = ar_data_get_map_string(root_map, "user.address.city");
    int zip = ar_data_get_map_integer(root_map, "user.address.zip");
    int math_score = ar_data_get_map_integer(root_map, "user.scores.math");
    int science_score = ar_data_get_map_integer(root_map, "user.scores.science");
    double avg_score = ar_data_get_map_double(root_map, "user.scores.average");
    
    // Then the values should be correctly retrieved
    assert(age == 30);
    assert(strcmp(name, "John Doe") == 0);
    assert(strcmp(street, "123 Main St") == 0);
    assert(strcmp(city, "Anytown") == 0);
    assert(zip == 12345);
    assert(math_score == 95);
    assert(science_score == 87);
    assert(avg_score == 91.0);
    
    // When we use ar_data_get_map_data with paths
    const data_t *user_data = ar_data_get_map_data(root_map, "user");
    const data_t *address_data = ar_data_get_map_data(root_map, "user.address");
    const data_t *scores_data = ar_data_get_map_data(root_map, "user.scores");
    const data_t *name_data = ar_data_get_map_data(root_map, "user.name");
    const data_t *age_data = ar_data_get_map_data(root_map, "user.age");
    const data_t *street_data = ar_data_get_map_data(root_map, "user.address.street");
    const data_t *math_data = ar_data_get_map_data(root_map, "user.scores.math");
    const data_t *avg_data = ar_data_get_map_data(root_map, "user.scores.average");
    
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
    assert(ar_data_get_type(user_data) == DATA_MAP);
    assert(ar_data_get_type(address_data) == DATA_MAP);
    assert(ar_data_get_type(scores_data) == DATA_MAP);
    assert(ar_data_get_type(name_data) == DATA_STRING);
    assert(ar_data_get_type(age_data) == DATA_INTEGER);
    assert(ar_data_get_type(street_data) == DATA_STRING);
    assert(ar_data_get_type(math_data) == DATA_INTEGER);
    assert(ar_data_get_type(avg_data) == DATA_DOUBLE);
    
    // And the data objects should contain the correct values
    assert(ar_data_get_integer(age_data) == 30);
    assert(strcmp(ar_data_get_string(name_data), "John Doe") == 0);
    assert(strcmp(ar_data_get_string(street_data), "123 Main St") == 0);
    assert(ar_data_get_integer(math_data) == 95);
    assert(ar_data_get_double(avg_data) == 91.0);
    
    // When we use path-based getters with invalid paths
    int invalid_age = ar_data_get_map_integer(root_map, "user.invalid.age");
    const char *invalid_name = ar_data_get_map_string(root_map, "invalid.user.name");
    double invalid_avg = ar_data_get_map_double(root_map, "user.scores.invalid");
    const data_t *invalid_data = ar_data_get_map_data(root_map, "user.invalid.path");
    const data_t *nonexistent_data = ar_data_get_map_data(root_map, "nonexistent");
    
    // Then they should return default values
    assert(invalid_age == 0);
    assert(invalid_name == NULL);
    assert(invalid_avg == 0.0);
    assert(invalid_data == NULL);
    assert(nonexistent_data == NULL);
    
    // When we use path-based getters with incorrect types
    int wrong_type = ar_data_get_map_integer(root_map, "user.name");
    double wrong_double = ar_data_get_map_double(root_map, "user.age");
    const char *wrong_string = ar_data_get_map_string(root_map, "user.scores.math");
    
    // Then they should return default values
    assert(wrong_type == 0);
    assert(wrong_double == 0.0);
    assert(wrong_string == NULL);
    
    // Cleanup
    ar_data_destroy(root_map);
    
    printf("Map data path getter tests passed!\n");
}

static void test_map_data_path_setters(void) {
    printf("Testing map data path setters...\n");
    
    // Given a root map data structure
    data_t *root_map = ar_data_create_map();
    
    // When we try to set values using path-based setters for paths that don't exist yet
    printf("Setting integer value on non-existent path...\n");
    bool set_int_result = ar_data_set_map_integer(root_map, "user.preferences.notifications", 1);
    
    // Then the operation should fail since the paths don't exist
    assert(!set_int_result);
    
    printf("Setting double value on non-existent path...\n");
    bool set_double_result = ar_data_set_map_double(root_map, "user.account.balance", 1250.75);
    
    // Then the operation should fail since the paths don't exist
    assert(!set_double_result);
    
    printf("Setting string value on non-existent path...\n");
    bool set_string_result = ar_data_set_map_string(root_map, "user.profile.email", "john.doe@example.com");
    
    // Then the operation should fail since the paths don't exist
    assert(!set_string_result);
    
    // First create intermediate maps for a proper test
    // Create the intermediate maps for a test path
    data_t *user_map = ar_data_create_map();
    data_t *prefs_map = ar_data_create_map();
    data_t *account_map = ar_data_create_map();
    data_t *profile_map = ar_data_create_map();
    
    // Add to root using data interface
    ar_data_set_map_data(root_map, "user", user_map);
    
    // Add to user map using data interface
    ar_data_set_map_data(user_map, "preferences", prefs_map);
    ar_data_set_map_data(user_map, "account", account_map);
    ar_data_set_map_data(user_map, "profile", profile_map);
    
    // Now set on valid paths
    printf("Setting integer value on valid path...\n");
    set_int_result = ar_data_set_map_integer(root_map, "user.preferences.notifications", 1);
    printf("Set integer result: %d\n", set_int_result ? 1 : 0);
    printf("Testing integer value after setting...\n");
    int int_val = ar_data_get_map_integer(root_map, "user.preferences.notifications");
    printf("integer value: %d\n", int_val);
    
    printf("Setting double value on valid path...\n");
    
    // Verify account map exists before setting
    data_t *pre_account_data = ar_data_get_map_data(root_map, "user.account");
    printf("account data exists before setting: %s\n", pre_account_data ? "yes" : "no");
    
    // Set the double value and check result
    set_double_result = ar_data_set_map_double(root_map, "user.account.balance", 1250.75);
    printf("Set double result: %d\n", set_double_result ? 1 : 0);
    printf("Testing double value after setting...\n");
    
    // Get the target map to directly inspect if needed
    data_t *user_data = ar_data_get_map_data(root_map, "user");
    printf("user data type: %d\n", ar_data_get_type(user_data));
    data_t *account_data = ar_data_get_map_data(root_map, "user.account");
    printf("account data type: %d\n", ar_data_get_type(account_data));
    
    // Use data interface for inspection
    printf("Direct check of account_map entries:\n");
    data_t *balance_direct = ar_data_get_map_data(account_data, "balance");
    printf("Direct map lookup of 'balance': %p\n", (void *)balance_direct);
    if (balance_direct) {
        printf("Direct balance data type: %d\n", ar_data_get_type(balance_direct));
        if (ar_data_get_type(balance_direct) == DATA_DOUBLE) {
            printf("Direct balance value: %f\n", ar_data_get_double(balance_direct));
        }
    }
    
    data_t *balance_data = ar_data_get_map_data(root_map, "user.account.balance");
    printf("balance data type: %d\n", balance_data ? (int)ar_data_get_type(balance_data) : -1);
    
    double double_val = ar_data_get_map_double(root_map, "user.account.balance");
    printf("double value: %f\n", double_val);
    
    printf("Setting string value on valid path...\n");
    set_string_result = ar_data_set_map_string(root_map, "user.profile.email", "john.doe@example.com");
    printf("Set string result: %d\n", set_string_result ? 1 : 0);
    printf("Testing string value after setting...\n");
    const char *string_val = ar_data_get_map_string(root_map, "user.profile.email");
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
    bool update_int = ar_data_set_map_integer(root_map, "user.preferences.notifications", 0);
    bool update_double = ar_data_set_map_double(root_map, "user.account.balance", 2000.50);
    bool update_string = ar_data_set_map_string(root_map, "user.profile.email", "johndoe@example.com");
    
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
    ar_data_set_map_string(root_map, "config", "settings");
    
    // Then try to set a nested value through it
    printf("Setting nested value through non-map node...\n");
    bool invalid_path = ar_data_set_map_integer(root_map, "config.value", 123);
    
    // It should fail since "config" is a string, not a map
    assert(!invalid_path);
    
    // Cleanup
    ar_data_destroy(root_map);
    
    printf("Map data path setter tests passed!\n");
}

static void test_list_operations(void) {
    printf("Testing list operations...\n");
    
    // Given a list data structure
    data_t *list_data = ar_data_create_list();
    assert(list_data != NULL);
    assert(ar_data_get_type(list_data) == DATA_LIST);
    
    // When we check the initial list count
    size_t initial_count = ar_data_list_count(list_data);
    
    // Then it should be empty
    assert(initial_count == 0);
    
    // When we add integers to the list
    bool add_first_result = ar_data_list_add_first_integer(list_data, 10);
    bool add_last_result = ar_data_list_add_last_integer(list_data, 20);
    
    // Then the operations should succeed
    assert(add_first_result);
    assert(add_last_result);
    
    // And the list count should be updated
    assert(ar_data_list_count(list_data) == 2);
    
    // When we get the first and last items
    data_t *first_item = ar_data_list_first(list_data);
    data_t *last_item = ar_data_list_last(list_data);
    
    // Then they should be valid data items with the correct values
    assert(first_item != NULL);
    assert(last_item != NULL);
    assert(ar_data_get_type(first_item) == DATA_INTEGER);
    assert(ar_data_get_type(last_item) == DATA_INTEGER);
    assert(ar_data_get_integer(first_item) == 10);
    assert(ar_data_get_integer(last_item) == 20);
    
    // When we add doubles to the list
    bool add_first_double_result = ar_data_list_add_first_double(list_data, 3.14);
    bool add_last_double_result = ar_data_list_add_last_double(list_data, 2.71);
    
    // Then the operations should succeed
    assert(add_first_double_result);
    assert(add_last_double_result);
    
    // And the list count should be updated
    assert(ar_data_list_count(list_data) == 4);
    
    // When we get the first and last items again
    first_item = ar_data_list_first(list_data);
    last_item = ar_data_list_last(list_data);
    
    // Then they should be valid data items with the correct values
    assert(first_item != NULL);
    assert(last_item != NULL);
    assert(ar_data_get_type(first_item) == DATA_DOUBLE);
    assert(ar_data_get_type(last_item) == DATA_DOUBLE);
    assert(ar_data_get_double(first_item) == 3.14);
    assert(ar_data_get_double(last_item) == 2.71);
    
    // When we add strings to the list
    bool add_first_string_result = ar_data_list_add_first_string(list_data, "hello");
    bool add_last_string_result = ar_data_list_add_last_string(list_data, "world");
    
    // Then the operations should succeed
    assert(add_first_string_result);
    assert(add_last_string_result);
    
    // And the list count should be updated
    assert(ar_data_list_count(list_data) == 6);
    
    // When we get the first and last items again
    first_item = ar_data_list_first(list_data);
    last_item = ar_data_list_last(list_data);
    
    // Then they should be valid data items with the correct values
    assert(first_item != NULL);
    assert(last_item != NULL);
    assert(ar_data_get_type(first_item) == DATA_STRING);
    assert(ar_data_get_type(last_item) == DATA_STRING);
    assert(strcmp(ar_data_get_string(first_item), "hello") == 0);
    assert(strcmp(ar_data_get_string(last_item), "world") == 0);
    
    // When we add data directly to the list
    data_t *int_data = ar_data_create_integer(42);
    data_t *double_data = ar_data_create_double(3.14159);
    
    bool add_first_data_result = ar_data_list_add_first_data(list_data, int_data);
    bool add_last_data_result = ar_data_list_add_last_data(list_data, double_data);
    
    // Then the operations should succeed
    assert(add_first_data_result);
    assert(add_last_data_result);
    
    // And the list count should be updated
    assert(ar_data_list_count(list_data) == 8);
    
    // When we get the first and last items again
    first_item = ar_data_list_first(list_data);
    last_item = ar_data_list_last(list_data);
    
    // Then they should be valid data items with the correct values
    assert(first_item != NULL);
    assert(last_item != NULL);
    assert(ar_data_get_type(first_item) == DATA_INTEGER);
    assert(ar_data_get_type(last_item) == DATA_DOUBLE);
    assert(ar_data_get_integer(first_item) == 42);
    assert(ar_data_get_double(last_item) == 3.14159);
    
    // When we remove items from the list
    data_t *removed_first = ar_data_list_remove_first(list_data);
    data_t *removed_last = ar_data_list_remove_last(list_data);
    
    // Then the operations should succeed and return the correct items
    assert(removed_first != NULL);
    assert(removed_last != NULL);
    assert(ar_data_get_type(removed_first) == DATA_INTEGER);
    assert(ar_data_get_type(removed_last) == DATA_DOUBLE);
    assert(ar_data_get_integer(removed_first) == 42);
    assert(ar_data_get_double(removed_last) == 3.14159);
    
    // And the list count should be updated
    assert(ar_data_list_count(list_data) == 6);
    
    // Cleanup removed items and list
    ar_data_destroy(removed_first);
    ar_data_destroy(removed_last);
    ar_data_destroy(list_data);
    
    // Test error handling for null data
    assert(ar_data_list_count(NULL) == 0);
    assert(!ar_data_list_add_first_integer(NULL, 42));
    assert(!ar_data_list_add_last_integer(NULL, 42));
    assert(!ar_data_list_add_first_double(NULL, 3.14));
    assert(!ar_data_list_add_last_double(NULL, 3.14));
    assert(!ar_data_list_add_first_string(NULL, "test"));
    assert(!ar_data_list_add_last_string(NULL, "test"));
    assert(ar_data_list_first(NULL) == NULL);
    assert(ar_data_list_last(NULL) == NULL);
    assert(ar_data_list_remove_first(NULL) == NULL);
    assert(ar_data_list_remove_last(NULL) == NULL);
    
    // Test error handling for wrong data type
    data_t *int_data2 = ar_data_create_integer(42);
    assert(!ar_data_list_add_first_integer(int_data2, 10));
    assert(!ar_data_list_add_last_integer(int_data2, 10));
    assert(!ar_data_list_add_first_double(int_data2, 3.14));
    assert(!ar_data_list_add_last_double(int_data2, 3.14));
    assert(!ar_data_list_add_first_string(int_data2, "test"));
    assert(!ar_data_list_add_last_string(int_data2, "test"));
    assert(ar_data_list_first(int_data2) == NULL);
    assert(ar_data_list_last(int_data2) == NULL);
    assert(ar_data_list_remove_first(int_data2) == NULL);
    assert(ar_data_list_remove_last(int_data2) == NULL);
    ar_data_destroy(int_data2);
    
    printf("List operations tests passed!\n");
}

int main(void) {
    printf("Starting Data Module Tests...\n");
    
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
    test_list_operations();
    
    // Then all tests should pass
    printf("All data tests passed!\n");
    
    return 0;
}
