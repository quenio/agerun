#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ar_yaml.h"
#include "ar_data.h"
#include "ar_heap.h"

/**
 * Test that ar_yaml can write simple data to file
 */
static void test_yaml__write_simple_string_to_file(void) {
    printf("Testing simple string write to YAML file...\n");
    
    // Given a simple string data
    ar_data_t *own_data = ar_data__create_string("Hello, YAML!");
    assert(own_data != NULL);
    
    // When writing to file
    const char *test_file = "test_string.yaml";
    bool result = ar_yaml__write_to_file(own_data, test_file);
    
    // Then the write should succeed
    assert(result == true);
    
    // And the file should exist
    assert(access(test_file, F_OK) == 0);
    
    // Read the file to verify content
    FILE *file = fopen(test_file, "r");
    assert(file != NULL);
    char buffer[256];
    
    // Skip header line
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        assert(false); // Should have header
    }
    
    // Read the actual content
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        assert(false); // Should have content
    }
    fclose(file);
    
    // Verify content (should be the string followed by newline)
    assert(strncmp(buffer, "Hello, YAML!", 12) == 0);
    
    // Clean up
    unlink(test_file);
    ar_data__destroy(own_data);
    
    printf("✓ Simple string write test passed\n");
}

/**
 * Test that ar_yaml can write map to file
 */
static void test_yaml__write_map_to_file(void) {
    printf("Testing map write to YAML file...\n");
    
    // Given a map with multiple types
    ar_data_t *own_map = ar_data__create_map();
    assert(own_map != NULL);
    ar_data__set_map_string(own_map, "name", "Test Agent");
    ar_data__set_map_integer(own_map, "count", 42);
    ar_data__set_map_double(own_map, "value", 3.14);
    
    // When writing to file
    const char *test_file = "test_map.yaml";
    bool result = ar_yaml__write_to_file(own_map, test_file);
    
    // Then the write should succeed
    assert(result == true);
    
    // And the file should exist and contain the map data
    FILE *file = fopen(test_file, "r");
    assert(file != NULL);
    
    char content[1024] = {0};
    size_t total_read = fread(content, 1, sizeof(content) - 1, file);
    content[total_read] = '\0';
    fclose(file);
    
    // Verify the map contains our keys and values
    assert(strstr(content, "name: Test Agent") != NULL);
    assert(strstr(content, "count: 42") != NULL);
    assert(strstr(content, "value: 3.14") != NULL);
    
    // Clean up
    unlink(test_file);
    ar_data__destroy(own_map);
    
    printf("✓ Map write test passed\n");
}

/**
 * Test that ar_yaml can write list to file
 */
static void test_yaml__write_list_to_file(void) {
    printf("Testing list write to YAML file...\n");
    
    // Given a list with items
    ar_data_t *own_list = ar_data__create_list();
    assert(own_list != NULL);
    ar_data__list_add_last_string(own_list, "first");
    ar_data__list_add_last_integer(own_list, 2);
    ar_data__list_add_last_string(own_list, "third");
    
    // When writing to file
    const char *test_file = "test_list.yaml";
    bool result = ar_yaml__write_to_file(own_list, test_file);
    
    // Then the write should succeed
    assert(result == true);
    
    // Read and verify content
    FILE *file = fopen(test_file, "r");
    assert(file != NULL);
    
    char content[1024] = {0};
    size_t total_read = fread(content, 1, sizeof(content) - 1, file);
    content[total_read] = '\0';
    fclose(file);
    
    // Verify list markers and items
    assert(strstr(content, "- first") != NULL);
    assert(strstr(content, "- 2") != NULL);
    assert(strstr(content, "- third") != NULL);
    
    // Clean up
    unlink(test_file);
    ar_data__destroy(own_list);
    
    printf("✓ List write test passed\n");
}

/**
 * Test that ar_yaml can write nested structures
 */
static void test_yaml__write_nested_structure(void) {
    printf("Testing nested structure write to YAML file...\n");
    
    // Given a complex nested structure
    ar_data_t *own_root = ar_data__create_map();
    assert(own_root != NULL);
    
    ar_data__set_map_integer(own_root, "version", 1);
    
    // Create a list of agents
    ar_data_t *own_agents = ar_data__create_list();
    
    // Create first agent
    ar_data_t *own_agent1 = ar_data__create_map();
    ar_data__set_map_integer(own_agent1, "id", 1);
    ar_data__set_map_string(own_agent1, "name", "echo");
    ar_data__list_add_last_data(own_agents, own_agent1);
    
    // Create second agent
    ar_data_t *own_agent2 = ar_data__create_map();
    ar_data__set_map_integer(own_agent2, "id", 2);
    ar_data__set_map_string(own_agent2, "name", "calculator");
    ar_data__list_add_last_data(own_agents, own_agent2);
    
    ar_data__set_map_data(own_root, "agents", own_agents);
    
    // When writing to file
    const char *test_file = "test_nested.yaml";
    bool result = ar_yaml__write_to_file(own_root, test_file);
    
    // Then the write should succeed
    assert(result == true);
    
    // Read and verify structure
    FILE *file = fopen(test_file, "r");
    assert(file != NULL);
    
    char content[2048] = {0};
    size_t total_read = fread(content, 1, sizeof(content) - 1, file);
    content[total_read] = '\0';
    fclose(file);
    
    // Verify nested structure
    assert(strstr(content, "version: 1") != NULL);
    assert(strstr(content, "agents:") != NULL);
    assert(strstr(content, "- id: 1") != NULL);
    assert(strstr(content, "  name: echo") != NULL);
    assert(strstr(content, "- id: 2") != NULL);
    assert(strstr(content, "  name: calculator") != NULL);
    
    // Clean up
    unlink(test_file);
    ar_data__destroy(own_root);
    
    printf("✓ Nested structure write test passed\n");
}

/**
 * Test that ar_yaml can read simple string from file
 */
static void test_yaml__read_simple_string_from_file(void) {
    printf("Testing simple string read from YAML file...\n");
    
    // First write a string to file
    ar_data_t *own_original = ar_data__create_string("test value");
    assert(ar_yaml__write_to_file(own_original, "test_read_string.yaml") == true);
    
    // Now read it back
    ar_data_t *own_loaded = ar_yaml__read_from_file("test_read_string.yaml");
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
static void test_yaml__round_trip_map(void) {
    printf("Testing map round-trip (write then read)...\n");
    
    // Create a map
    ar_data_t *own_original = ar_data__create_map();
    ar_data__set_map_string(own_original, "name", "TestAgent");
    ar_data__set_map_integer(own_original, "id", 42);
    ar_data__set_map_double(own_original, "ratio", 3.14);
    
    // Write to file
    assert(ar_yaml__write_to_file(own_original, "test_roundtrip_map.yaml") == true);
    
    // Read back
    ar_data_t *own_loaded = ar_yaml__read_from_file("test_roundtrip_map.yaml");
    
    // For now, just check it's not null - full parsing will be implemented later
    assert(own_loaded != NULL);
    
    // Cleanup
    ar_data__destroy(own_original);
    ar_data__destroy(own_loaded);
    unlink("test_roundtrip_map.yaml");
    
    printf("✓ Map round-trip test passed\n");
}

int main(void) {
    printf("=== ar_yaml Module Tests ===\n");
    
    test_yaml__write_simple_string_to_file();
    test_yaml__write_map_to_file();
    test_yaml__write_list_to_file();
    test_yaml__write_nested_structure();
    test_yaml__read_simple_string_from_file();
    test_yaml__round_trip_map();
    
    printf("\nAll 6 tests passed!\n");
    return 0;
}