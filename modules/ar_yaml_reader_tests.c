#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ar_yaml_reader.h"
#include "ar_yaml_writer.h"
#include "ar_data.h"
#include "ar_heap.h"

/**
 * Test that ar_yaml_reader can read simple string from file
 */
static void test_yaml_reader__read_simple_string_from_file(void) {
    printf("Testing simple string read from YAML file...\n");
    
    // First write a string to file using writer
    ar_data_t *own_original = ar_data__create_string("test value");
    assert(ar_yaml_writer__write_to_file(own_original, "test_read_string.yaml") == true);
    
    // Now read it back using reader
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_read_string.yaml");
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_loaded), "test value") == 0);
    
    // Cleanup
    ar_data__destroy(own_original);
    ar_data__destroy(own_loaded);
    unlink("test_read_string.yaml");
    
    printf("✓ Simple string read test passed\n");
}

/**
 * Test round-trip for maps
 */
static void test_yaml_reader__round_trip_map(void) {
    printf("Testing map round-trip (write then read)...\n");
    
    // Create a map
    ar_data_t *own_original = ar_data__create_map();
    ar_data__set_map_string(own_original, "name", "TestAgent");
    ar_data__set_map_integer(own_original, "id", 42);
    ar_data__set_map_double(own_original, "ratio", 3.14);
    
    // Write to file
    assert(ar_yaml_writer__write_to_file(own_original, "test_roundtrip_map.yaml") == true);
    
    // Read back
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_roundtrip_map.yaml");
    
    // Verify it's a map with correct values
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__MAP);
    
    // Check string value
    ar_data_t *ref_name = ar_data__get_map_data(own_loaded, "name");
    assert(ref_name != NULL);
    assert(ar_data__get_type(ref_name) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_name), "TestAgent") == 0);
    
    // Check integer value
    ar_data_t *ref_id = ar_data__get_map_data(own_loaded, "id");
    assert(ref_id != NULL);
    assert(ar_data__get_type(ref_id) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_id) == 42);
    
    // Check double value
    ar_data_t *ref_ratio = ar_data__get_map_data(own_loaded, "ratio");
    assert(ref_ratio != NULL);
    assert(ar_data__get_type(ref_ratio) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(ref_ratio) == 3.14);
    
    // Cleanup
    ar_data__destroy(own_original);
    ar_data__destroy(own_loaded);
    unlink("test_roundtrip_map.yaml");
    
    printf("✓ Map round-trip test passed\n");
}

/**
 * Test round-trip for lists
 */
static void test_yaml_reader__round_trip_list(void) {
    printf("Testing list round-trip (write then read)...\n");
    
    // Create a list with mixed types
    ar_data_t *own_original = ar_data__create_list();
    ar_data__list_add_last_string(own_original, "first");
    ar_data__list_add_last_integer(own_original, 42);
    ar_data__list_add_last_double(own_original, 3.14);
    ar_data__list_add_last_string(own_original, "last");
    
    // Write to file
    assert(ar_yaml_writer__write_to_file(own_original, "test_roundtrip_list.yaml") == true);
    
    // Read back
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_roundtrip_list.yaml");
    
    // Verify it's a list with correct values
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(own_loaded) == 4);
    
    // Check items
    ar_data_t **own_items = ar_data__list_items(own_loaded);
    
    // First item - string
    assert(ar_data__get_type(own_items[0]) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_items[0]), "first") == 0);
    
    // Second item - integer
    assert(ar_data__get_type(own_items[1]) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(own_items[1]) == 42);
    
    // Third item - double
    assert(ar_data__get_type(own_items[2]) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(own_items[2]) == 3.14);
    
    // Fourth item - string
    assert(ar_data__get_type(own_items[3]) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(own_items[3]), "last") == 0);
    
    // Free the items array (ownership transferred from list_items)
    AR__HEAP__FREE(own_items);
    
    // Cleanup
    ar_data__destroy(own_original);
    ar_data__destroy(own_loaded);
    unlink("test_roundtrip_list.yaml");
    
    printf("✓ List round-trip test passed\n");
}

/**
 * Test round-trip for maps containing lists
 */
static void test_yaml_reader__nested_map_with_list(void) {
    printf("Testing nested map with list round-trip...\n");
    
    // Create a map with a list value
    ar_data_t *own_original = ar_data__create_map();
    ar_data__set_map_string(own_original, "name", "TestEntity");
    ar_data__set_map_integer(own_original, "count", 3);
    
    // Create a list and add to map
    ar_data_t *own_list = ar_data__create_list();
    ar_data__list_add_last_string(own_list, "item1");
    ar_data__list_add_last_integer(own_list, 42);
    ar_data__list_add_last_string(own_list, "item3");
    ar_data__set_map_data(own_original, "items", own_list);
    
    // Write to file
    assert(ar_yaml_writer__write_to_file(own_original, "test_nested_map_list.yaml") == true);
    
    // Read back
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_nested_map_list.yaml");
    
    // Verify structure
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__MAP);
    
    // Check scalar fields
    ar_data_t *ref_name = ar_data__get_map_data(own_loaded, "name");
    assert(ref_name != NULL);
    assert(strcmp(ar_data__get_string(ref_name), "TestEntity") == 0);
    
    ar_data_t *ref_count = ar_data__get_map_data(own_loaded, "count");
    assert(ref_count != NULL);
    assert(ar_data__get_integer(ref_count) == 3);
    
    // Check nested list
    ar_data_t *ref_items = ar_data__get_map_data(own_loaded, "items");
    assert(ref_items != NULL);
    assert(ar_data__get_type(ref_items) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(ref_items) == 3);
    
    ar_data_t **own_items = ar_data__list_items(ref_items);
    assert(strcmp(ar_data__get_string(own_items[0]), "item1") == 0);
    assert(ar_data__get_integer(own_items[1]) == 42);
    assert(strcmp(ar_data__get_string(own_items[2]), "item3") == 0);
    AR__HEAP__FREE(own_items);
    
    // Cleanup
    ar_data__destroy(own_original);
    ar_data__destroy(own_loaded);
    unlink("test_nested_map_list.yaml");
    
    printf("✓ Nested map with list test passed\n");
}

/**
 * Test round-trip for lists containing maps
 */
static void test_yaml_reader__list_of_maps(void) {
    printf("Testing list of maps round-trip...\n");
    
    // Create a list of maps
    ar_data_t *own_original = ar_data__create_list();
    
    // First map
    ar_data_t *own_map1 = ar_data__create_map();
    ar_data__set_map_string(own_map1, "name", "Alice");
    ar_data__set_map_integer(own_map1, "age", 30);
    ar_data__list_add_last_data(own_original, own_map1);
    
    // Second map
    ar_data_t *own_map2 = ar_data__create_map();
    ar_data__set_map_string(own_map2, "name", "Bob");
    ar_data__set_map_integer(own_map2, "age", 25);
    ar_data__list_add_last_data(own_original, own_map2);
    
    // Write to file
    assert(ar_yaml_writer__write_to_file(own_original, "test_list_of_maps.yaml") == true);
    
    // Read back
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_list_of_maps.yaml");
    
    // Verify structure
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(own_loaded) == 2);
    
    // Check first map
    ar_data_t **own_items = ar_data__list_items(own_loaded);
    assert(ar_data__get_type(own_items[0]) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_name1 = ar_data__get_map_data(own_items[0], "name");
    assert(ref_name1 != NULL);
    assert(strcmp(ar_data__get_string(ref_name1), "Alice") == 0);
    
    ar_data_t *ref_age1 = ar_data__get_map_data(own_items[0], "age");
    assert(ref_age1 != NULL);
    assert(ar_data__get_integer(ref_age1) == 30);
    
    // Check second map
    assert(ar_data__get_type(own_items[1]) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_name2 = ar_data__get_map_data(own_items[1], "name");
    assert(ref_name2 != NULL);
    assert(strcmp(ar_data__get_string(ref_name2), "Bob") == 0);
    
    ar_data_t *ref_age2 = ar_data__get_map_data(own_items[1], "age");
    assert(ref_age2 != NULL);
    assert(ar_data__get_integer(ref_age2) == 25);
    
    AR__HEAP__FREE(own_items);
    
    // Cleanup
    ar_data__destroy(own_original);
    ar_data__destroy(own_loaded);
    unlink("test_list_of_maps.yaml");
    
    printf("✓ List of maps test passed\n");
}

/**
 * Test round-trip for empty containers
 */
static void test_yaml_reader__empty_containers(void) {
    printf("Testing empty containers round-trip...\n");
    
    // Test empty map
    ar_data_t *own_empty_map = ar_data__create_map();
    assert(ar_yaml_writer__write_to_file(own_empty_map, "test_empty_map.yaml") == true);
    
    ar_data_t *own_loaded_map = ar_yaml_reader__read_from_file("test_empty_map.yaml");
    assert(own_loaded_map != NULL);
    assert(ar_data__get_type(own_loaded_map) == AR_DATA_TYPE__MAP);
    
    ar_data_t *own_keys = ar_data__get_map_keys(own_loaded_map);
    assert(ar_data__list_count(own_keys) == 0);
    ar_data__destroy(own_keys);
    
    ar_data__destroy(own_empty_map);
    ar_data__destroy(own_loaded_map);
    unlink("test_empty_map.yaml");
    
    // Test empty list
    ar_data_t *own_empty_list = ar_data__create_list();
    assert(ar_yaml_writer__write_to_file(own_empty_list, "test_empty_list.yaml") == true);
    
    ar_data_t *own_loaded_list = ar_yaml_reader__read_from_file("test_empty_list.yaml");
    assert(own_loaded_list != NULL);
    assert(ar_data__get_type(own_loaded_list) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(own_loaded_list) == 0);
    
    ar_data__destroy(own_empty_list);
    ar_data__destroy(own_loaded_list);
    unlink("test_empty_list.yaml");
    
    // Test map with empty list value
    ar_data_t *own_map_with_empty = ar_data__create_map();
    ar_data__set_map_string(own_map_with_empty, "name", "Test");
    ar_data_t *own_empty_items = ar_data__create_list();
    ar_data__set_map_data(own_map_with_empty, "items", own_empty_items);
    
    assert(ar_yaml_writer__write_to_file(own_map_with_empty, "test_map_empty_list.yaml") == true);
    
    ar_data_t *own_loaded_mixed = ar_yaml_reader__read_from_file("test_map_empty_list.yaml");
    assert(own_loaded_mixed != NULL);
    assert(ar_data__get_type(own_loaded_mixed) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_items = ar_data__get_map_data(own_loaded_mixed, "items");
    assert(ref_items != NULL);
    assert(ar_data__get_type(ref_items) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(ref_items) == 0);
    
    ar_data__destroy(own_map_with_empty);
    ar_data__destroy(own_loaded_mixed);
    unlink("test_map_empty_list.yaml");
    
    printf("✓ Empty containers test passed\n");
}

/**
 * Test type inference edge cases
 */
static void test_yaml_reader__type_inference_edge_cases(void) {
    printf("Testing type inference edge cases...\n");
    
    // Create a YAML file with edge cases for type inference
    FILE *file = fopen("test_type_inference.yaml", "w");
    assert(file != NULL);
    fprintf(file, "# AgeRun YAML File\n");
    fprintf(file, "quoted_number: \"42\"\n");        // Should stay string
    fprintf(file, "unquoted_number: 42\n");         // Should become integer
    fprintf(file, "quoted_double: \"3.14\"\n");     // Should stay string
    fprintf(file, "unquoted_double: 3.14\n");       // Should become double
    fprintf(file, "quoted_true: \"true\"\n");       // Should stay string
    fprintf(file, "unquoted_true: true\n");         // Should stay string (we don't have boolean)
    fprintf(file, "quoted_false: \"false\"\n");     // Should stay string
    fprintf(file, "unquoted_false: false\n");       // Should stay string
    fprintf(file, "quoted_null: \"null\"\n");       // Should stay string
    fprintf(file, "unquoted_null: null\n");         // Should stay string
    fprintf(file, "mixed_string: 42abc\n");         // Should stay string
    fprintf(file, "leading_zero: 0042\n");          // Should become integer 42
    fprintf(file, "negative_int: -123\n");          // Should become integer -123
    fprintf(file, "negative_double: -45.67\n");     // Should become double -45.67
    fprintf(file, "scientific: 1.23e-4\n");         // Should become double
    fclose(file);
    
    // Read and verify types
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_type_inference.yaml");
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__MAP);
    
    // Check quoted number stays string
    ar_data_t *ref_quoted_num = ar_data__get_map_data(own_loaded, "quoted_number");
    assert(ref_quoted_num != NULL);
    assert(ar_data__get_type(ref_quoted_num) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_quoted_num), "42") == 0);
    
    // Check unquoted number becomes integer
    ar_data_t *ref_unquoted_num = ar_data__get_map_data(own_loaded, "unquoted_number");
    assert(ref_unquoted_num != NULL);
    assert(ar_data__get_type(ref_unquoted_num) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_unquoted_num) == 42);
    
    // Check quoted double stays string
    ar_data_t *ref_quoted_dbl = ar_data__get_map_data(own_loaded, "quoted_double");
    assert(ref_quoted_dbl != NULL);
    assert(ar_data__get_type(ref_quoted_dbl) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_quoted_dbl), "3.14") == 0);
    
    // Check unquoted double becomes double
    ar_data_t *ref_unquoted_dbl = ar_data__get_map_data(own_loaded, "unquoted_double");
    assert(ref_unquoted_dbl != NULL);
    assert(ar_data__get_type(ref_unquoted_dbl) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(ref_unquoted_dbl) == 3.14);
    
    // Check YAML keywords stay strings (we don't have boolean type)
    ar_data_t *ref_quoted_true = ar_data__get_map_data(own_loaded, "quoted_true");
    assert(ref_quoted_true != NULL);
    assert(ar_data__get_type(ref_quoted_true) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_quoted_true), "true") == 0);
    
    ar_data_t *ref_unquoted_true = ar_data__get_map_data(own_loaded, "unquoted_true");
    assert(ref_unquoted_true != NULL);
    assert(ar_data__get_type(ref_unquoted_true) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_unquoted_true), "true") == 0);
    
    // Check mixed strings stay strings
    ar_data_t *ref_mixed = ar_data__get_map_data(own_loaded, "mixed_string");
    assert(ref_mixed != NULL);
    assert(ar_data__get_type(ref_mixed) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_mixed), "42abc") == 0);
    
    // Check leading zeros still parse as integers
    ar_data_t *ref_leading_zero = ar_data__get_map_data(own_loaded, "leading_zero");
    assert(ref_leading_zero != NULL);
    assert(ar_data__get_type(ref_leading_zero) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_leading_zero) == 42);
    
    // Check negative integer
    ar_data_t *ref_neg_int = ar_data__get_map_data(own_loaded, "negative_int");
    assert(ref_neg_int != NULL);
    assert(ar_data__get_type(ref_neg_int) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_neg_int) == -123);
    
    // Check negative double
    ar_data_t *ref_neg_dbl = ar_data__get_map_data(own_loaded, "negative_double");
    assert(ref_neg_dbl != NULL);
    assert(ar_data__get_type(ref_neg_dbl) == AR_DATA_TYPE__DOUBLE);
    assert(ar_data__get_double(ref_neg_dbl) == -45.67);
    
    // Check scientific notation
    ar_data_t *ref_sci = ar_data__get_map_data(own_loaded, "scientific");
    assert(ref_sci != NULL);
    assert(ar_data__get_type(ref_sci) == AR_DATA_TYPE__DOUBLE);
    // Use approximate check for scientific notation
    double sci_val = ar_data__get_double(ref_sci);
    assert(sci_val > 0.000122 && sci_val < 0.000124);  // 1.23e-4 = 0.000123
    
    // Cleanup
    ar_data__destroy(own_loaded);
    unlink("test_type_inference.yaml");
    
    printf("✓ Type inference edge cases test passed\n");
}

/**
 * Test full agent structure persistence
 */
static void test_yaml_reader__full_agent_structure(void) {
    printf("Testing full agent structure persistence...\n");
    
    // Create a simpler agent structure first to isolate the issue
    ar_data_t *own_agent = ar_data__create_map();
    
    // Basic agent properties
    ar_data__set_map_integer(own_agent, "id", 42);
    ar_data__set_map_string(own_agent, "name", "calculator");
    ar_data__set_map_string(own_agent, "version", "1.0.0");
    
    // Memory state (nested map)
    ar_data_t *own_memory = ar_data__create_map();
    ar_data__set_map_integer(own_memory, "counter", 0);
    ar_data__set_map_string(own_memory, "last_operation", "none");
    
    ar_data__set_map_data(own_agent, "memory", own_memory);
    
    // Write to file
    assert(ar_yaml_writer__write_to_file(own_agent, "test_agent_structure.yaml") == true);
    
    // Read back
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_agent_structure.yaml");
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__MAP);
    
    // Verify basic properties
    ar_data_t *ref_id = ar_data__get_map_data(own_loaded, "id");
    assert(ref_id != NULL);
    assert(ar_data__get_integer(ref_id) == 42);
    
    ar_data_t *ref_name = ar_data__get_map_data(own_loaded, "name");
    assert(ref_name != NULL);
    assert(strcmp(ar_data__get_string(ref_name), "calculator") == 0);
    
    ar_data_t *ref_version = ar_data__get_map_data(own_loaded, "version");
    assert(ref_version != NULL);
    assert(strcmp(ar_data__get_string(ref_version), "1.0.0") == 0);
    
    // Verify memory state
    ar_data_t *ref_memory = ar_data__get_map_data(own_loaded, "memory");
    assert(ref_memory != NULL);
    assert(ar_data__get_type(ref_memory) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_counter = ar_data__get_map_data(ref_memory, "counter");
    assert(ref_counter != NULL);
    assert(ar_data__get_integer(ref_counter) == 0);
    
    ar_data_t *ref_last_op = ar_data__get_map_data(ref_memory, "last_operation");
    assert(ref_last_op != NULL);
    assert(strcmp(ar_data__get_string(ref_last_op), "none") == 0);
    
    // Cleanup
    ar_data__destroy(own_agent);
    ar_data__destroy(own_loaded);
    unlink("test_agent_structure.yaml");
    
    printf("✓ Full agent structure test passed\n");
}

/**
 * Test handling of comments and blank lines
 */
static void test_yaml_reader__comments_and_blanks(void) {
    printf("Testing comments and blank lines...\n");
    
    // Create a YAML file with comments and blank lines manually
    FILE *file = fopen("test_comments.yaml", "w");
    assert(file != NULL);
    fprintf(file, "# AgeRun YAML File\n");
    fprintf(file, "# This is a comment\n");
    fprintf(file, "name: TestAgent\n");
    fprintf(file, "\n");  // Blank line
    fprintf(file, "# Another comment\n");
    fprintf(file, "version: 1.0.0\n");
    fprintf(file, "\n");  // Another blank
    fprintf(file, "items:\n");
    fprintf(file, "  # Comment in list\n");
    fprintf(file, "  - first\n");
    fprintf(file, "  \n");  // Blank in list
    fprintf(file, "  - second\n");
    fprintf(file, "  # Final comment\n");
    fclose(file);
    
    // Read and verify structure
    ar_data_t *own_loaded = ar_yaml_reader__read_from_file("test_comments.yaml");
    assert(own_loaded != NULL);
    assert(ar_data__get_type(own_loaded) == AR_DATA_TYPE__MAP);
    
    // Check values
    ar_data_t *ref_name = ar_data__get_map_data(own_loaded, "name");
    assert(ref_name != NULL);
    assert(strcmp(ar_data__get_string(ref_name), "TestAgent") == 0);
    
    ar_data_t *ref_version = ar_data__get_map_data(own_loaded, "version");
    assert(ref_version != NULL);
    assert(strcmp(ar_data__get_string(ref_version), "1.0.0") == 0);
    
    ar_data_t *ref_items = ar_data__get_map_data(own_loaded, "items");
    assert(ref_items != NULL);
    assert(ar_data__get_type(ref_items) == AR_DATA_TYPE__LIST);
    assert(ar_data__list_count(ref_items) == 2);
    
    ar_data_t **own_items = ar_data__list_items(ref_items);
    assert(strcmp(ar_data__get_string(own_items[0]), "first") == 0);
    assert(strcmp(ar_data__get_string(own_items[1]), "second") == 0);
    AR__HEAP__FREE(own_items);
    
    // Cleanup
    ar_data__destroy(own_loaded);
    unlink("test_comments.yaml");
    
    printf("✓ Comments and blanks test passed\n");
}

int main(void) {
    printf("Running ar_yaml_reader tests...\n\n");
    
    test_yaml_reader__read_simple_string_from_file();
    test_yaml_reader__round_trip_map();
    test_yaml_reader__round_trip_list();
    test_yaml_reader__nested_map_with_list();
    test_yaml_reader__list_of_maps();
    test_yaml_reader__empty_containers();
    test_yaml_reader__type_inference_edge_cases();
    test_yaml_reader__full_agent_structure();
    test_yaml_reader__comments_and_blanks();
    
    printf("\nAll 9 tests passed!\n");
    return 0;
}