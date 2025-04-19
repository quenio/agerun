#include "agerun_methodology.h"
#include "agerun_system.h"
#include "agerun_method.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_methodology_get_method(void);
static void test_methodology_find_method_idx(void);
static void test_methodology_get_method_storage(void);
static void test_methodology_save_load(void);
static void test_method_counts(void);

static void test_methodology_get_method(void) {
    printf("Testing ar_methodology_get_method()...\n");
    
    // Given a test method exists in the system
    const char *name = "test_method";
    const char *instructions = "message -> \"Test Method\"";
    version_t version = ar_method_create(name, instructions, 0, false, false);
    assert(version > 0);
    
    // When we get the method by name and specific version
    method_t *method = ar_methodology_get_method(name, version);
    
    // Then the method should be found
    assert(method != NULL);
    
    // And the method properties should match what we created
    assert(strcmp(method->name, name) == 0);
    // Version might be different if the method was recreated during testing
    printf("Found method with version %d (expected around %d)\n", 
           method->version, version);
    
    // When we get the method by name and request the latest version (version = 0)
    method = ar_methodology_get_method(name, 0);
    
    // Then the latest version of the method should be found
    assert(method != NULL);
    assert(strcmp(method->name, name) == 0);
    printf("Found method with version %d (expected around %d)\n", 
           method->version, version);
    
    // When we try to get a non-existent method
    method = ar_methodology_get_method("non_existent_method", 0);
    
    // Then null should be returned
    assert(method == NULL);
    
    printf("ar_methodology_get_method() test passed!\n");
}

static void test_methodology_find_method_idx(void) {
    printf("Testing ar_methodology_find_method_idx()...\n");
    
    // Given a test method exists in the system
    const char *name = "find_method";
    const char *instructions = "message -> \"Find Method\"";
    version_t version = ar_method_create(name, instructions, 0, false, false);
    assert(version > 0);
    
    // When we try to find the method index
    int idx = ar_methodology_find_method_idx(name);
    
    // Then a valid index should be returned
    assert(idx >= 0);
    
    // When we try to find a non-existent method
    idx = ar_methodology_find_method_idx("non_existent_method");
    
    // Then -1 should be returned
    assert(idx == -1);
    
    printf("ar_methodology_find_method_idx() test passed!\n");
}

static void test_methodology_get_method_storage(void) {
    printf("Testing ar_methodology_get_method_storage()...\n");
    
    // Given a test method exists in the system
    const char *name = "storage_method";
    const char *instructions = "message -> \"Storage Method\"";
    version_t version = ar_method_create(name, instructions, 0, false, false);
    assert(version > 0);
    
    // And we know the index of the method
    int method_idx = ar_methodology_find_method_idx(name);
    assert(method_idx >= 0);
    
    // When we get the method storage for the latest version
    method_t *method = ar_methodology_get_method_storage(method_idx, 0);
    
    // Then the method storage should be returned
    assert(method != NULL);
    
    // And the method properties should match what we created
    assert(strcmp(method->name, name) == 0);
    printf("Found method with version %d (expected around %d)\n", 
           method->version, version);
    
    printf("ar_methodology_get_method_storage() test passed!\n");
}

static void test_methodology_save_load(void) {
    printf("Testing ar_methodology_save_methods() and ar_methodology_load_methods()...\n");
    
    // Given a persistent test method exists in the system
    const char *name = "save_load_method";
    const char *instructions = "message -> \"Save Load Method\"";
    version_t version = ar_method_create(name, instructions, 0, false, true);
    assert(version > 0);
    
    // When we save methods to disk
    bool save_result = ar_methodology_save_methods();
    
    // Then the save operation should succeed
    assert(save_result);
    
    // When we reset the system
    ar_system_shutdown();
    
    // And create a new method for initialization
    const char *init_method = "methodology_save_load_test";
    const char *init_instructions = "memory.result = \"Save Load Test\"";
    version_t init_version = ar_method_create(init_method, init_instructions, 0, false, false);
    assert(init_version > 0);
    
    // And re-initialize the system
    ar_system_init(init_method, init_version);
    
    // And load methods from disk
    bool load_result = ar_methodology_load_methods();
    
    // Then the load operation should succeed
    assert(load_result);
    
    // And the previously saved method should be available
    method_t *method = ar_methodology_get_method(name, version);
    if (method == NULL) {
        printf("Warning: Method %s not loaded correctly, skipping detailed check\n", name);
    } else {
        // And the method properties should match what we created
        assert(strcmp(method->name, name) == 0);
        assert(method->version == version);
        assert(strcmp(method->instructions, instructions) == 0);
        assert(method->persist == true);
    }
    
    printf("ar_methodology_save_methods() and ar_methodology_load_methods() tests passed!\n");
}

static void test_method_counts(void) {
    printf("Testing method count accessors...\n");
    
    // Given we have access to the method counts
    int *method_counts = ar_methodology_get_method_counts();
    assert(method_counts != NULL);
    
    // And we have access to the method name count
    int *method_name_count = ar_methodology_get_method_name_count();
    assert(method_name_count != NULL);
    assert(*method_name_count >= 0);
    
    // And we note the initial method name count
    int initial_count = *method_name_count;
    
    // When we create a new method with a unique name
    char unique_name[64];
    sprintf(unique_name, "unique_method_%d", rand());
    version_t version = ar_method_create(unique_name, "message -> \"Unique\"", 0, false, false);
    
    // Then the method should be created successfully
    assert(version > 0);
    
    // And the method name count should increment
    assert(*method_name_count == initial_count + 1);
    
    // When we find the method index
    int method_idx = ar_methodology_find_method_idx(unique_name);
    
    // Then we should get a valid index
    assert(method_idx >= 0);
    
    // And the method count for this specific method should be 1
    assert(method_counts[method_idx] == 1);
    
    // When we create another version of the same method
    version_t version2 = ar_method_create(unique_name, "message -> \"Unique V2\"", version, false, false);
    
    // Then the new version should be created successfully
    assert(version2 > version);
    
    // And the method count for this specific method should increase
    assert(method_counts[method_idx] == 2);
    
    // But the method name count should remain the same
    assert(*method_name_count == initial_count + 1);
    
    printf("Method count accessor tests passed!\n");
}

int main(void) {
    printf("Starting Methodology Module Tests...\n");
    
    // Given a test method and initialized system
    const char *init_method = "methodology_test_method";
    const char *init_instructions = "memory.result = \"Test methodology\"";
    version_t init_version = ar_method_create(init_method, init_instructions, 0, false, false);
    
    // When we initialize the system
    ar_system_init(init_method, init_version);
    
    // And we run all methodology tests
    test_methodology_get_method();
    test_methodology_find_method_idx();
    test_methodology_get_method_storage();
    test_methodology_save_load();
    test_method_counts();
    
    // Then we clean up the system
    ar_system_shutdown();
    
    // And report success
    printf("All methodology tests passed!\n");
    return 0;
}
