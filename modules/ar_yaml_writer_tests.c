#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ar_yaml_writer.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_log.h"

/**
 * Test instance creation and destruction
 */
static void test_yaml_writer__create_and_destroy_instance(void) {
    printf("Testing instance creation and destruction...\n");
    
    // Given/When creating an instance
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);
    
    // Then the instance should be created
    assert(own_writer != NULL);
    
    // When destroying the instance
    ar_yaml_writer__destroy(own_writer);
    
    // Then no memory leaks should occur (verified by test framework)
    
    // Also test NULL handling
    ar_yaml_writer__destroy(NULL);  // Should not crash
    
    printf("✓ Instance creation and destruction test passed\n");
}

/**
 * Test writing with instance-based API
 */
static void test_yaml_writer__write_with_instance(void) {
    printf("Testing writing with instance-based API...\n");
    
    // Given a writer instance
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);
    assert(own_writer != NULL);
    
    // And some data to write
    ar_data_t *own_data = ar_data__create_string("Instance-based writing!");
    assert(own_data != NULL);
    
    // When writing using the instance
    const char *test_file = "test_instance.yaml";
    bool result = ar_yaml_writer__write_to_file(own_writer, own_data, test_file);
    
    // Then the write should succeed
    assert(result == true);
    
    // And the file should exist
    assert(access(test_file, F_OK) == 0);
    
    // Clean up
    unlink(test_file);
    ar_data__destroy(own_data);
    ar_yaml_writer__destroy(own_writer);
    
    printf("✓ Instance-based writing test passed\n");
}

/**
 * Test that ar_yaml_writer can write simple string to file
 */
static void test_yaml_writer__write_simple_string_to_file(void) {
    printf("Testing simple string write to YAML file...\n");
    
    // Given a writer instance
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);
    assert(own_writer != NULL);
    
    // And a simple string data
    ar_data_t *own_data = ar_data__create_string("Hello, YAML!");
    assert(own_data != NULL);
    
    // When writing to file using instance
    const char *test_file = "test_string.yaml";
    bool result = ar_yaml_writer__write_to_file(own_writer, own_data, test_file);
    
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
    ar_data__destroy(own_data);
    ar_yaml_writer__destroy(own_writer);
    unlink(test_file);
    
    printf("✓ Simple string write test passed\n");
}

/**
 * Test that ar_yaml_writer can write map to file
 */
static void test_yaml_writer__write_map_to_file(void) {
    printf("Testing map write to YAML file...\n");
    
    // Given a writer instance
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);
    assert(own_writer != NULL);
    
    // And a map with multiple types
    ar_data_t *own_map = ar_data__create_map();
    assert(own_map != NULL);
    ar_data__set_map_string(own_map, "name", "Test Agent");
    ar_data__set_map_integer(own_map, "count", 42);
    ar_data__set_map_double(own_map, "value", 3.14);
    
    // When writing to file using instance
    const char *test_file = "test_map.yaml";
    bool result = ar_yaml_writer__write_to_file(own_writer, own_map, test_file);
    
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
    ar_yaml_writer__destroy(own_writer);
    
    printf("✓ Map write test passed\n");
}

/**
 * Test that ar_yaml_writer can write list to file
 */
static void test_yaml_writer__write_list_to_file(void) {
    printf("Testing list write to YAML file...\n");
    
    // Given a writer instance
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);
    assert(own_writer != NULL);
    
    // And a list with items
    ar_data_t *own_list = ar_data__create_list();
    assert(own_list != NULL);
    ar_data__list_add_last_string(own_list, "first");
    ar_data__list_add_last_integer(own_list, 2);
    ar_data__list_add_last_string(own_list, "third");
    
    // When writing to file using instance
    const char *test_file = "test_list.yaml";
    bool result = ar_yaml_writer__write_to_file(own_writer, own_list, test_file);
    
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
    ar_yaml_writer__destroy(own_writer);
    
    printf("✓ List write test passed\n");
}

/**
 * Test error logging when file cannot be opened
 */
static void test_yaml_writer__error_logging(void) {
    printf("Testing error logging...\n");
    
    // Given a log instance
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    
    // And a writer instance with the log
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(own_log);
    assert(own_writer != NULL);
    
    // And some data to write
    ar_data_t *own_data = ar_data__create_string("test");
    assert(own_data != NULL);
    
    // When trying to write to a non-existent directory
    const char *bad_path = "/nonexistent/directory/file.yaml";
    bool result = ar_yaml_writer__write_to_file(own_writer, own_data, bad_path);
    
    // Then the write should fail
    assert(result == false);
    
    // And an error should be logged
    const char *error_msg = ar_log__get_last_error_message(own_log);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "Failed to open file for writing") != NULL);
    
    // Clean up
    ar_data__destroy(own_data);
    ar_yaml_writer__destroy(own_writer);
    ar_log__destroy(own_log);
    
    printf("✓ Error logging test passed\n");
}

/**
 * Test that ar_yaml_writer can write nested structures
 */
static void test_yaml_writer__write_nested_structure(void) {
    printf("Testing nested structure write to YAML file...\n");
    
    // Given a writer instance
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);
    assert(own_writer != NULL);
    
    // And a complex nested structure
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
    
    // When writing to file using instance
    const char *test_file = "test_nested.yaml";
    bool result = ar_yaml_writer__write_to_file(own_writer, own_root, test_file);
    
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
    ar_yaml_writer__destroy(own_writer);
    
    printf("✓ Nested structure write test passed\n");
}

int main(void) {
    printf("Running ar_yaml_writer tests...\n\n");
    
    test_yaml_writer__create_and_destroy_instance();
    test_yaml_writer__write_with_instance();
    test_yaml_writer__write_simple_string_to_file();
    test_yaml_writer__write_map_to_file();
    test_yaml_writer__write_list_to_file();
    test_yaml_writer__error_logging();
    test_yaml_writer__write_nested_structure();
    
    printf("\nAll 7 tests passed!\n");
    return 0;
}