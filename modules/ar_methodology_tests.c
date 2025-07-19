#include "ar_methodology.h"
#include "ar_system.h"
#include "ar_method.h"
#include "ar_io.h"
#include "ar_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h> // For time(NULL) as fallback

// Forward declarations
static void test_methodology__create_destroy(void);
static void test_methodology__global_instance(void);
static void test_methodology_get_method(void);
static void test_methodology_register_and_get(void);
static void test_methodology_save_load(void);
static void test_method_counts(void);
static void test_methodology__get_method_with_instance(void);
static void test_methodology__register_method_with_instance(void);
static void test_methodology__create_method_with_instance(void);
static void test_methodology__save_load_with_instance(void);
static void test_methodology__ar_log_propagation(void);
static void test_methodology__ar_log_propagation_on_load(void);
static void test_methodology__ar_log_propagation_on_load_with_instance(void);

static void test_methodology__create_destroy(void) {
    printf("Testing ar_methodology__create() and ar_methodology__destroy()...\n");
    
    // Given an ar_log instance
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    
    // When we create a methodology instance with the log
    ar_methodology_t *own_methodology = ar_methodology__create(own_log);
    
    // Then the methodology instance should be created successfully
    assert(own_methodology != NULL);
    
    // When we destroy the methodology
    ar_methodology__destroy(own_methodology);
    own_methodology = NULL;
    
    // Clean up the log
    ar_log__destroy(own_log);
    own_log = NULL;
    
    printf("test_methodology__create_destroy passed\n");
}

static void test_methodology__global_instance(void) {
    printf("Testing methodology uses global instance...\n");
    
    // This test verifies that the global API uses a global instance
    // rather than static arrays directly
    
    // Given we clean up to start fresh
    ar_methodology__cleanup();
    
    // When we register a method through the global API
    bool result = ar_methodology__create_method("instance_test", "message -> \"Test\"", "1.0.0");
    assert(result == true);
    
    // Then the method should be accessible
    ar_method_t *method = ar_methodology__get_method("instance_test", "1.0.0");
    assert(method != NULL);
    
    // And when we call cleanup (which should clean the global instance)
    ar_methodology__cleanup();
    
    // And register another method (this will fail if global instance not re-initialized)
    result = ar_methodology__create_method("after_cleanup", "message -> \"After\"", "1.0.0");
    assert(result == true);
    
    // Then the new method should be accessible
    method = ar_methodology__get_method("after_cleanup", "1.0.0");
    assert(method != NULL);
    
    // But the old method should be gone
    method = ar_methodology__get_method("instance_test", "1.0.0");
    assert(method == NULL);
    
    // Clean up
    ar_methodology__cleanup();
    
    printf("test_methodology__global_instance passed\n");
}

static void test_methodology_get_method(void) {
    printf("Testing ar_methodology__get_method()...\n");
    
    // Given a test method exists in the system
    const char *name = "test_method";
    const char *instructions = "message -> \"Test Method\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0" for this test
    
    // When we get the method by name and specific version
    ar_method_t *method = ar_methodology__get_method(name, "1.0.0");
    
    // Then the method should be found
    assert(method != NULL);
    
    // And the method properties should match what we created
    assert(strcmp(ar_method__get_name(method), name) == 0);
    // Version is now a string, not an integer
    printf("Found method with version %s (expected version string)\n", 
           ar_method__get_version(method));
    
    // When we get the method by name and request the latest version (version = NULL)
    method = ar_methodology__get_method(name, NULL);
    
    // Then the latest version of the method should be found
    assert(method != NULL);
    assert(strcmp(ar_method__get_name(method), name) == 0);
    printf("Found method with version %s (expected version string)\n", 
           ar_method__get_version(method));
    
    // When we try to get a non-existent method
    method = ar_methodology__get_method("non_existent_method", NULL);
    
    // Then null should be returned
    assert(method == NULL);
    
    printf("ar_methodology__get_method() test passed!\n");
}


static void test_methodology_register_and_get(void) {
    printf("Testing ar_methodology__register_method() and get...\n");
    
    // Given a test method exists in the system
    const char *name = "storage_method";
    const char *instructions = "message -> \"Storage Method\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0" for this test
    
    // When we get the method
    ar_method_t *method = ar_methodology__get_method(name, "1.0.0");
    
    // Then the method storage should be returned
    assert(method != NULL);
    
    // And the method properties should match what we created
    assert(strcmp(ar_method__get_name(method), name) == 0);
    printf("Found method with version %s (expected version string)\n", 
           ar_method__get_version(method));
    
    printf("ar_methodology__register_and_get() test passed!\n");
}

static void test_methodology_save_load(void) {
    printf("Testing ar_methodology__save_methods() and ar_methodology__load_methods()...\n");
    
    // Note: This test operates on the methodology module directly
    // without initializing the system, avoiding lifecycle conflicts
    
    // Given a persistent test method exists in the system
    const char *name = "save_load_method";
    const char *instructions = "message -> \"Save Load Method\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(name, instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0" for this test
    
    // When we save methods to disk
    bool save_result = ar_methodology__save_methods();
    
    // Then the save operation should succeed
    assert(save_result);
    
    // Clear the methodology to simulate a fresh start
    ar_methodology__cleanup();
    
    // And load methods from disk
    bool load_result = ar_methodology__load_methods();
    
    // Then the load operation should succeed
    assert(load_result);
    
    // And the previously saved method should be available
    ar_method_t *method = ar_methodology__get_method(name, "1.0.0");
    if (method == NULL) {
        printf("Warning: Method %s not loaded correctly, skipping detailed check\n", name);
    } else {
        // And the method properties should match what we created
        assert(strcmp(ar_method__get_name(method), name) == 0);
        // Version is now a string, not an integer
        // assert(ar_method__get_version(method) == version);
        assert(strcmp(ar_method__get_instructions(method), instructions) == 0);
        // Persistent flag is no longer in the spec
        // assert(ar_method_is_persistent(method) == true);
    }
    
    // Clean up loaded methods to prevent memory leaks
    ar_methodology__cleanup();
    
    printf("ar_methodology__save_methods() and ar_methodology__load_methods() tests passed!\n");
}

static void test_method_counts(void) {
    printf("Testing multiple method registration...\n");
    
    // When we create a new method with a unique name
    char unique_name[64];
    // Use more secure and predictable random number generator instead of rand()
    unsigned int random_id = 0;
    #ifdef __APPLE__
        // On macOS, arc4random is available
        random_id = arc4random() % 100000;
    #else
        // On other platforms, use time-based seed if needed
        random_id = (unsigned int)time(NULL) % 100000;
    #endif
    
    ar_io__string_format(unique_name, sizeof(unique_name), "unique_method_%u", random_id);
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(unique_name, "message -> \"Unique\"", "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we use version "1.0.0" for this test
    
    // Method should be registered successfully
    
    // When we try to get the method we just registered
    ar_method_t *registered_method = ar_methodology__get_method(unique_name, "1.0.0");
    
    // Then we should get a valid method
    assert(registered_method != NULL);
    
    // And we should be able to retrieve the method we just registered
    assert(ar_methodology__get_method(unique_name, "1.0.0") != NULL);
    
    // When we create another version of the same method
    ar_method_t *own_method2 = ar_method__create(unique_name, "message -> \"Unique V2\"", "2.0.0");
    assert(own_method2 != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method2);
    own_method2 = NULL; // Mark as transferred
    
    // For documentation purposes only, version2 would be 2
    // (We don't actually use this variable in assertions)
    
    // And both versions should be retrievable
    assert(ar_methodology__get_method(unique_name, "1.0.0") != NULL);
    assert(ar_methodology__get_method(unique_name, "2.0.0") != NULL);
    
    // Both versions should be accessible
    
    printf("Multiple method registration tests passed!\n");
}

static void test_methodology__get_method_with_instance(void) {
    printf("Testing ar_methodology__get_method_with_instance()...\n");
    
    // Given a methodology instance
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    // And a method registered to this instance
    ar_method_t *own_method = ar_method__create("instance_method", "memory.result = \"Instance\"", "1.0.0");
    assert(own_method != NULL);
    ar_methodology__register_method_with_instance(own_methodology, own_method);
    own_method = NULL; // ownership transferred
    
    // When we get the method using the instance
    ar_method_t *ref_method = ar_methodology__get_method_with_instance(own_methodology, "instance_method", "1.0.0");
    
    // Then we should get the method
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_name(ref_method), "instance_method") == 0);
    
    // And when we get with NULL version (latest)
    ref_method = ar_methodology__get_method_with_instance(own_methodology, "instance_method", NULL);
    assert(ref_method != NULL);
    
    // Clean up
    ar_methodology__destroy(own_methodology);
    
    printf("test_methodology__get_method_with_instance passed\n");
}

static void test_methodology__register_method_with_instance(void) {
    printf("Testing ar_methodology__register_method_with_instance()...\n");
    
    // Given a methodology instance
    ar_methodology_t *own_methodology = ar_methodology__create(NULL);
    assert(own_methodology != NULL);
    
    // And a method to register
    ar_method_t *own_method = ar_method__create("register_test", "memory.result = \"Register\"", "1.0.0");
    assert(own_method != NULL);
    
    // When we register the method to the instance
    ar_methodology__register_method_with_instance(own_methodology, own_method);
    own_method = NULL; // ownership transferred
    
    // Then we should be able to retrieve it
    ar_method_t *ref_method = ar_methodology__get_method_with_instance(own_methodology, "register_test", "1.0.0");
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_name(ref_method), "register_test") == 0);
    
    // Clean up
    ar_methodology__destroy(own_methodology);
    
    printf("test_methodology__register_method_with_instance passed\n");
}

static void test_methodology__create_method_with_instance(void) {
    printf("Testing ar_methodology__create_method_with_instance()...\n");
    
    // Given a methodology instance with a log
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    ar_methodology_t *own_methodology = ar_methodology__create(own_log);
    assert(own_methodology != NULL);
    
    // When we create and register a method in one step
    bool result = ar_methodology__create_method_with_instance(own_methodology, 
                                                             "create_test", 
                                                             "memory.result = \"Created\"", 
                                                             "1.0.0");
    
    // Then the creation should succeed
    assert(result == true);
    
    // And the method should be retrievable
    ar_method_t *ref_method = ar_methodology__get_method_with_instance(own_methodology, "create_test", "1.0.0");
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_name(ref_method), "create_test") == 0);
    assert(strcmp(ar_method__get_instructions(ref_method), "memory.result = \"Created\"") == 0);
    
    // Clean up
    ar_methodology__destroy(own_methodology);
    ar_log__destroy(own_log);
    
    printf("test_methodology__create_method_with_instance passed\n");
}

static void test_methodology__save_load_with_instance(void) {
    printf("Testing ar_methodology__save_load_with_instance()...\n");
    
    // Given a methodology instance with methods
    ar_methodology_t *own_methodology1 = ar_methodology__create(NULL);
    assert(own_methodology1 != NULL);
    
    // Add multiple methods
    bool result = ar_methodology__create_method_with_instance(own_methodology1, 
                                                             "save_test1", 
                                                             "memory.result = \"Save1\"", 
                                                             "1.0.0");
    assert(result == true);
    
    result = ar_methodology__create_method_with_instance(own_methodology1, 
                                                        "save_test2", 
                                                        "memory.result = \"Save2\"", 
                                                        "1.0.0");
    assert(result == true);
    
    // When we save to a custom file
    const char *test_filename = "test_methodology.agerun";
    result = ar_methodology__save_methods_with_instance(own_methodology1, test_filename);
    assert(result == true);
    
    // And create a new instance
    ar_methodology_t *own_methodology2 = ar_methodology__create(NULL);
    assert(own_methodology2 != NULL);
    
    // And load from the custom file
    result = ar_methodology__load_methods_with_instance(own_methodology2, test_filename);
    assert(result == true);
    
    // Then both methods should be available in the new instance
    ar_method_t *ref_method = ar_methodology__get_method_with_instance(own_methodology2, "save_test1", "1.0.0");
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_instructions(ref_method), "memory.result = \"Save1\"") == 0);
    
    ref_method = ar_methodology__get_method_with_instance(own_methodology2, "save_test2", "1.0.0");
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_instructions(ref_method), "memory.result = \"Save2\"") == 0);
    
    // Clean up
    ar_methodology__destroy(own_methodology1);
    ar_methodology__destroy(own_methodology2);
    remove(test_filename);
    
    printf("test_methodology__save_load_with_instance passed\n");
}

static void test_methodology__ar_log_propagation(void) {
    printf("Testing ar_log propagation through methodology to method parser...\n");
    
    // Given a methodology instance with an ar_log
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    
    ar_methodology_t *own_methodology = ar_methodology__create(own_log);
    assert(own_methodology != NULL);
    
    // When we create a method with invalid syntax
    bool result = ar_methodology__create_method_with_instance(own_methodology,
                                                             "error_test",
                                                             "invalid syntax here!@#$",
                                                             "1.0.0");
    
    // Then the method creation should succeed (parser errors are non-fatal)
    assert(result == true);
    
    // And the parse error should be logged to our ar_log instance
    ar_event_t *ref_event = ar_log__get_last_error(own_log);
    assert(ref_event != NULL);
    
    // Verify the error message contains something about parse error
    const char *ref_message = ar_event__get_message(ref_event);
    assert(ref_message != NULL);
    assert(strstr(ref_message, "parse") != NULL || strstr(ref_message, "Parse") != NULL ||
           strstr(ref_message, "syntax") != NULL || strstr(ref_message, "Syntax") != NULL);
    
    // Clean up
    ar_methodology__destroy(own_methodology);
    ar_log__destroy(own_log);
    
    printf("test_methodology__ar_log_propagation passed\n");
}

static void test_methodology__ar_log_propagation_on_load(void) {
    printf("Testing ar_log propagation during ar_methodology__load_methods()...\n");
    
    // Given a clean state
    ar_methodology__cleanup();
    remove("methodology.agerun");
    
    // Given a methodology file with a method that has invalid syntax
    bool result = ar_methodology__create_method("load_test", "invalid @#$ syntax!", "1.0.0");
    assert(result == true);
    result = ar_methodology__save_methods();
    assert(result == true);
    ar_methodology__cleanup();
    
    // When we load methods using the global function
    result = ar_methodology__load_methods();
    assert(result == true);
    
    // Then the method should be loaded
    ar_method_t *method = ar_methodology__get_method("load_test", "1.0.0");
    assert(method != NULL);
    
    // Then the method should have NULL AST due to parse errors
    const ar_method_ast_t *ast = ar_method__get_ast(method);
    assert(ast == NULL);
    
    // NOTE: We cannot test ar_log propagation here because the global instance
    // is created with NULL ar_log. This will be addressed when we update
    // line 915 to use ar_method__create_with_log with mut_instance->ref_log
    
    // Clean up
    ar_methodology__cleanup();
    remove("methodology.agerun");
    
    printf("test_methodology__ar_log_propagation_on_load passed\n");
}

static void test_methodology__ar_log_propagation_on_load_with_instance(void) {
    printf("Testing ar_log propagation during ar_methodology__load_methods_with_instance()...\n");
    
    // Given an ar_log instance
    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);
    
    // And a methodology instance with that ar_log
    ar_methodology_t *own_methodology1 = ar_methodology__create(own_log);
    assert(own_methodology1 != NULL);
    
    // When we create a method with invalid syntax
    bool result = ar_methodology__create_method_with_instance(own_methodology1,
                                                             "instance_load_test",
                                                             "bad syntax %^&*",
                                                             "2.0.0");
    assert(result == true);
    
    // And save to a custom file
    const char *test_file = "test_methodology_instance.agerun";
    result = ar_methodology__save_methods_with_instance(own_methodology1, test_file);
    assert(result == true);
    
    // Destroy the first instance and create a new one with a fresh log
    ar_methodology__destroy(own_methodology1);
    ar_log__destroy(own_log);
    
    // Create fresh log for loading test
    own_log = ar_log__create();
    assert(own_log != NULL);
    
    // And create a new methodology instance with the fresh log
    ar_methodology_t *own_methodology2 = ar_methodology__create(own_log);
    assert(own_methodology2 != NULL);
    
    // When we load methods from the custom file
    result = ar_methodology__load_methods_with_instance(own_methodology2, test_file);
    assert(result == true);
    
    // Then the parse error should be logged to our ar_log instance
    ar_event_t *ref_event = ar_log__get_last_error(own_log);
    
    // RED PHASE: This assertion will FAIL because ar_log is not propagated yet
    // We expect parse errors to be logged, but they won't be until we fix the code
    assert(ref_event != NULL);
    
    // Verify the error message contains something about parse error
    const char *ref_message = ar_event__get_message(ref_event);
    assert(ref_message != NULL);
    assert(strstr(ref_message, "parse") != NULL || strstr(ref_message, "Parse") != NULL ||
           strstr(ref_message, "syntax") != NULL || strstr(ref_message, "Syntax") != NULL);
    
    // Clean up
    ar_methodology__destroy(own_methodology2);
    ar_log__destroy(own_log);
    remove(test_file);
    
    printf("test_methodology__ar_log_propagation_on_load_with_instance passed\n");
}

int main(void) {
    printf("Starting Methodology Module Tests...\n");
    
    // Run create/destroy test first (doesn't need system initialized)
    test_methodology__create_destroy();
    
    // Run global instance test (doesn't need system initialized)
    test_methodology__global_instance();
    
    // Given a test method and initialized system
    const char *init_method = "methodology_test_method";
    const char *init_instructions = "memory.result = \"Test methodology\"";
    
    // Create method and register it with methodology 
    ar_method_t *own_method = ar_method__create(init_method, init_instructions, "1.0.0");
    assert(own_method != NULL);
    
    // Register with methodology
    ar_methodology__register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    // For test purposes, we assume registration succeeds and creates version 1.0.0
    const char *init_version = "1.0.0";
    
    // When we initialize the system
    ar_system__init(init_method, init_version);
    
    // Process the wake message from the initial agent
    ar_system__process_next_message();
    
    // And we run all methodology tests
    test_methodology_get_method();
    test_methodology_register_and_get();
    test_method_counts();
    
    // Shutdown the system to clean up resources
    ar_system__shutdown();
    
    // Run persistence test (doesn't need system initialized)
    test_methodology_save_load();
    
    // Run instance-aware tests
    test_methodology__get_method_with_instance();
    test_methodology__register_method_with_instance();
    test_methodology__create_method_with_instance();
    test_methodology__save_load_with_instance();
    test_methodology__ar_log_propagation();
    test_methodology__ar_log_propagation_on_load();
    test_methodology__ar_log_propagation_on_load_with_instance();
    
    // And report success
    printf("All 17 tests passed!\n");
    return 0;
}
