#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "ar_method_store.h"
#include "ar_method_registry.h"
#include "ar_method.h"
#include "ar_heap.h"

/* Test file path constant - relative to bin/run-tests */
#define TEST_FILE_PATH "./methodology.agerun"

/**
 * Test that ar_method_store can be created and destroyed
 */
static void test_method_store__create_and_destroy(void) {
    /* Given: A method registry */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    
    /* When: Creating a method store with explicit file path */
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    
    /* Then: Store should be created successfully */
    assert(own_store != NULL);
    
    /* Cleanup */
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}

/**
 * Test that ar_method_store__get_path returns the correct path
 */
static void test_method_store__get_path_returns_correct_filename(void) {
    /* Given: A method store with explicit file path */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    
    /* When: Getting the method store path */
    const char *path = ar_method_store__get_path(own_store);
    
    /* Then: It should return the expected filename */
    assert(path != NULL);
    assert(strcmp(path, TEST_FILE_PATH) == 0);
    
    /* Cleanup */
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}

/**
 * Test that ar_method_store__exists returns false when file doesn't exist
 */
static void test_method_store__exists_returns_false_when_no_file(void) {
    /* Given: No methodology file exists (clean environment) */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    ar_method_store__delete(own_store); /* Clean up any existing file */
    
    /* When: Checking if store exists */
    bool exists = ar_method_store__exists(own_store);
    
    /* Then: It should return false */
    assert(exists == false);
    
    /* Cleanup */
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}

/**
 * Test that ar_method_store__save saves a single method from the registry
 */
static void test_method_store__save_writes_single_method(void) {
    /* Given: A registry with one method */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_t *own_method = ar_method__create("test_method", "send(0, \"hello\")", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method);
    
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    ar_method_store__delete(own_store); /* Clean up any existing file */
    
    /* When: Saving the store */
    bool result = ar_method_store__save(own_store);
    
    /* Then: Save should succeed and file should contain the method */
    assert(result == true);
    assert(ar_method_store__exists(own_store) == true);
    
    /* Verify file contents match ar_methodology format */
    FILE *fp = fopen(TEST_FILE_PATH, "r");
    assert(fp != NULL);
    
    char buffer[256];
    /* First line should be method count: 1 */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "1\n") == 0);
    
    /* Second line should be method name and version count */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "test_method 1\n") == 0);
    
    /* Third line should be version */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "1.0.0\n") == 0);
    
    /* Fourth line should be instructions */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "send(0, \"hello\")\n") == 0);
    
    fclose(fp);
    
    /* Cleanup */
    ar_method_store__delete(own_store);
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}

/**
 * Test that ar_method_store__save creates an empty file when no methods exist
 */
static void test_method_store__save_creates_empty_file(void) {
    /* Given: No methodology file exists and no methods are registered */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    ar_method_store__delete(own_store); /* Clean up any existing file */
    
    /* When: Saving the store */
    bool result = ar_method_store__save(own_store);
    
    /* Then: Save should succeed and file should exist */
    assert(result == true);
    assert(ar_method_store__exists(own_store) == true);
    
    /* Cleanup */
    ar_method_store__delete(own_store);
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}


/**
 * Test that ar_method_store__save saves multiple methods with multiple versions
 */
static void test_method_store__save_writes_multiple_methods(void) {
    /* Given: A registry with multiple methods and versions */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    
    /* Register multiple versions of method1 */
    ar_method_t *own_method1_v1 = ar_method__create("method1", "send(0, \"v1\")", "1.0.0");
    ar_method_t *own_method1_v2 = ar_method__create("method1", "send(0, \"v2\")", "2.0.0");
    ar_method_registry__register_method(own_registry, own_method1_v1);
    ar_method_registry__register_method(own_registry, own_method1_v2);
    
    /* Register a different method */
    ar_method_t *own_method2 = ar_method__create("method2", "send(0, \"hello\")", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method2);
    
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    ar_method_store__delete(own_store); /* Clean up any existing file */
    
    /* When: Saving the store */
    bool result = ar_method_store__save(own_store);
    
    /* Then: Save should succeed and file should contain all methods grouped by name */
    assert(result == true);
    assert(ar_method_store__exists(own_store) == true);
    
    /* Verify file contents match ar_methodology format */
    FILE *fp = fopen(TEST_FILE_PATH, "r");
    assert(fp != NULL);
    
    char buffer[256];
    /* First line should be method count: 2 (unique names) */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "2\n") == 0);
    
    /* Should have method1 with 2 versions */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "method1 2\n") == 0);
    
    /* Version 1.0.0 */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "1.0.0\n") == 0);
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "send(0, \"v1\")\n") == 0);
    
    /* Version 2.0.0 */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "2.0.0\n") == 0);
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "send(0, \"v2\")\n") == 0);
    
    /* Should have method2 with 1 version */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "method2 1\n") == 0);
    
    /* Version 1.0.0 */
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "1.0.0\n") == 0);
    assert(fgets(buffer, sizeof(buffer), fp) != NULL);
    assert(strcmp(buffer, "send(0, \"hello\")\n") == 0);
    
    fclose(fp);
    
    /* Cleanup */
    ar_method_store__delete(own_store);
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}


/**
 * Test that ar_method_store__load loads an empty file correctly
 */
static void test_method_store__load_empty_file(void) {
    /* Given: An empty methodology file */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    
    /* Create empty file first */
    ar_method_store__save(own_store);
    
    /* Create a new registry to load into */
    ar_method_registry_t *own_new_registry = ar_method_registry__create();
    ar_method_store_t *own_new_store = ar_method_store__create(NULL, own_new_registry, TEST_FILE_PATH);
    
    /* When: Loading the store */
    bool result = ar_method_store__load(own_new_store);
    
    /* Then: Load should succeed and registry should be empty */
    assert(result == true);
    ar_list_t *own_methods = ar_method_registry__get_all_methods(own_new_registry);
    assert(ar_list__count(own_methods) == 0);
    ar_list__destroy(own_methods);
    
    /* Cleanup */
    ar_method_store__delete(own_store);
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
    ar_method_store__destroy(own_new_store);
    ar_method_registry__destroy(own_new_registry);
}


/**
 * Test that ar_method_store__load loads a single method correctly
 */
static void test_method_store__load_single_method(void) {
    /* Given: A methodology file with one method */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_t *own_method = ar_method__create("test_method", "send(0, \"hello\")", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method);
    
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    ar_method_store__save(own_store);
    
    /* Create a new registry to load into */
    ar_method_registry_t *own_new_registry = ar_method_registry__create();
    ar_method_store_t *own_new_store = ar_method_store__create(NULL, own_new_registry, TEST_FILE_PATH);
    
    /* When: Loading the store */
    bool result = ar_method_store__load(own_new_store);
    
    /* Then: Load should succeed and method should be in registry */
    assert(result == true);
    
    /* Check method was loaded correctly */
    ar_method_t *loaded_method = ar_method_registry__get_method_by_exact_match(
        own_new_registry, "test_method", "1.0.0");
    assert(loaded_method != NULL);
    
    /* Verify method contents */
    assert(strcmp(ar_method__get_name(loaded_method), "test_method") == 0);
    assert(strcmp(ar_method__get_version(loaded_method), "1.0.0") == 0);
    assert(strcmp(ar_method__get_instructions(loaded_method), "send(0, \"hello\")") == 0);
    
    /* Cleanup */
    ar_method_store__delete(own_store);
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
    ar_method_store__destroy(own_new_store);
    ar_method_registry__destroy(own_new_registry);
}


/**
 * Test that ar_method_store__load loads multiple methods with versions correctly
 */
static void test_method_store__load_multiple_methods(void) {
    /* Given: A methodology file with multiple methods and versions */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    
    /* Register multiple versions of method1 */
    ar_method_t *own_method1_v1 = ar_method__create("method1", "send(0, \"v1\")", "1.0.0");
    ar_method_t *own_method1_v2 = ar_method__create("method1", "send(0, \"v2\")", "2.0.0");
    ar_method_registry__register_method(own_registry, own_method1_v1);
    ar_method_registry__register_method(own_registry, own_method1_v2);
    
    /* Register a different method */
    ar_method_t *own_method2 = ar_method__create("method2", "send(0, \"hello\")", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method2);
    
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    ar_method_store__save(own_store);
    
    /* Create a new registry to load into */
    ar_method_registry_t *own_new_registry = ar_method_registry__create();
    ar_method_store_t *own_new_store = ar_method_store__create(NULL, own_new_registry, TEST_FILE_PATH);
    
    /* When: Loading the store */
    bool result = ar_method_store__load(own_new_store);
    
    /* Then: Load should succeed and all methods should be in registry */
    assert(result == true);
    
    /* Check method1 version 1.0.0 was loaded */
    ar_method_t *loaded_method1_v1 = ar_method_registry__get_method_by_exact_match(
        own_new_registry, "method1", "1.0.0");
    assert(loaded_method1_v1 != NULL);
    assert(strcmp(ar_method__get_instructions(loaded_method1_v1), "send(0, \"v1\")") == 0);
    
    /* Check method1 version 2.0.0 was loaded */
    ar_method_t *loaded_method1_v2 = ar_method_registry__get_method_by_exact_match(
        own_new_registry, "method1", "2.0.0");
    assert(loaded_method1_v2 != NULL);
    assert(strcmp(ar_method__get_instructions(loaded_method1_v2), "send(0, \"v2\")") == 0);
    
    /* Check method2 was loaded */
    ar_method_t *loaded_method2 = ar_method_registry__get_method_by_exact_match(
        own_new_registry, "method2", "1.0.0");
    assert(loaded_method2 != NULL);
    assert(strcmp(ar_method__get_instructions(loaded_method2), "send(0, \"hello\")") == 0);
    
    /* Verify total count */
    ar_list_t *own_all_methods = ar_method_registry__get_all_methods(own_new_registry);
    assert(ar_list__count(own_all_methods) == 3);
    ar_list__destroy(own_all_methods);
    
    /* Cleanup */
    ar_method_store__delete(own_store);
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
    ar_method_store__destroy(own_new_store);
    ar_method_registry__destroy(own_new_registry);
}


/**
 * Test that ar_method_store__load handles invalid file format gracefully
 */
static void test_method_store__load_invalid_format(void) {
    /* Given: A file with invalid format */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    
    /* Create an invalid file */
    FILE *fp = fopen(TEST_FILE_PATH, "w");
    assert(fp != NULL);
    fprintf(fp, "invalid format\n");
    fprintf(fp, "not a number\n");
    fclose(fp);
    
    /* When: Loading the store */
    bool result = ar_method_store__load(own_store);
    
    /* Then: Load should fail gracefully */
    assert(result == false);
    
    /* Registry should still be empty */
    ar_list_t *own_methods = ar_method_registry__get_all_methods(own_registry);
    assert(ar_list__count(own_methods) == 0);
    ar_list__destroy(own_methods);
    
    /* Cleanup */
    ar_method_store__delete(own_store);
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}


/**
 * Test that ar_method_store__delete removes the file
 */
static void test_method_store__delete_removes_file(void) {
    /* Given: A saved methodology file */
    ar_method_registry_t *own_registry = ar_method_registry__create();
    ar_method_t *own_method = ar_method__create("test_method", "send(0, \"hello\")", "1.0.0");
    ar_method_registry__register_method(own_registry, own_method);
    
    ar_method_store_t *own_store = ar_method_store__create(NULL, own_registry, TEST_FILE_PATH);
    ar_method_store__save(own_store);
    
    /* Verify file exists */
    assert(ar_method_store__exists(own_store) == true);
    
    /* When: Deleting the store file */
    bool result = ar_method_store__delete(own_store);
    
    /* Then: Delete should succeed and file should be gone */
    assert(result == true);
    assert(ar_method_store__exists(own_store) == false);
    
    /* Cleanup */
    ar_method_store__destroy(own_store);
    ar_method_registry__destroy(own_registry);
}


int main(void) {
    test_method_store__create_and_destroy();
    test_method_store__get_path_returns_correct_filename();
    test_method_store__exists_returns_false_when_no_file();
    test_method_store__save_creates_empty_file();
    test_method_store__save_writes_single_method();
    test_method_store__save_writes_multiple_methods();
    test_method_store__load_empty_file();
    test_method_store__load_single_method();
    test_method_store__load_multiple_methods();
    test_method_store__load_invalid_format();
    test_method_store__delete_removes_file();
    
    printf("All 11 tests passed!\n");
    return 0;
}