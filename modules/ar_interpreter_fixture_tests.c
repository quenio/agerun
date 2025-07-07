#include "ar_interpreter_fixture.h"
#include "ar_agency.h"
#include "ar_system.h"
#include "ar_methodology.h"
#include "ar_data.h"
#include "ar_heap.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// Forward declarations
static void test_fixture_create_destroy(void);
static void test_fixture_create_agent(void);
static void test_fixture_execute_instruction(void);
static void test_fixture_data_tracking(void);

int main(void) {
    printf("=== AgeRun Interpreter Fixture Tests ===\n");
    
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd bin && ./ar_interpreter_fixture_tests\n");
            return 1;
        }
    }
    
    // Clean state
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system
    ar_system__init(NULL, NULL);
    
    // Run tests
    test_fixture_create_destroy();
    test_fixture_create_agent();
    test_fixture_execute_instruction();
    test_fixture_data_tracking();
    
    // Cleanup
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    printf("All 8 tests passed!\n");
    return 0;
}

static void test_fixture_create_destroy(void) {
    printf("Testing fixture create/destroy...\n");
    
    // Given the need for a fixture
    // When we create one
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_create");
    
    // Then it should be created successfully
    assert(own_fixture != NULL);
    assert(strcmp(ar_interpreter_fixture__get_name(own_fixture), "test_create") == 0);
    
    // And it should have an interpreter
    ar_interpreter_t *ref_interpreter = ar_interpreter_fixture__get_interpreter(own_fixture);
    assert(ref_interpreter != NULL);
    
    // When we destroy it
    ar_interpreter_fixture__destroy(own_fixture);
    
    // Then no memory leaks should occur
    printf("Fixture create/destroy test passed!\n");
}

static void test_fixture_create_agent(void) {
    printf("Testing fixture create agent...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_agent");
    assert(own_fixture != NULL);
    
    // When we create an agent
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_method",
        "memory.result := \"test\"",
        NULL  // Use default version
    );
    
    // Then it should be created successfully
    assert(agent_id > 0);
    
    // And we should be able to get its memory
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    assert(mut_memory != NULL);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Fixture create agent test passed!\n");
}

static void test_fixture_execute_instruction(void) {
    printf("Testing fixture execute instruction...\n");
    
    // Given a fixture with an agent
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_execute");
    assert(own_fixture != NULL);
    
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_exec",
        "memory.x := 1",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // When we execute an instruction
    bool result = ar_interpreter_fixture__execute_instruction(
        own_fixture,
        agent_id,
        "memory.value := 42"
    );
    
    // Then it should succeed
    assert(result == true);
    
    // And the memory should be updated
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *ref_value = ar_data__get_map_data(mut_memory, "value");
    assert(ref_value != NULL);
    assert(ar_data__get_type(ref_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_value) == 42);
    
    // Test execution with message - we need to create new values through operations
    // Direct assignment of message or its fields fails because they're const references
    ar_data_t *own_message = ar_data__create_map();
    ar_data__set_map_string(own_message, "text", "Hello");
    ar_data__set_map_integer(own_message, "count", 42);
    
    // Test string concatenation with message field
    result = ar_interpreter_fixture__execute_with_message(
        own_fixture,
        agent_id,
        "memory.greeting := \"Message says: \" + message.text",
        own_message
    );
    assert(result == true);
    ar_data_t *ref_greeting = ar_data__get_map_data(mut_memory, "greeting");
    assert(ref_greeting != NULL);
    assert(ar_data__get_type(ref_greeting) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_greeting), "Message says: Hello") == 0);
    
    // Test arithmetic with message field
    result = ar_interpreter_fixture__execute_with_message(
        own_fixture,
        agent_id,
        "memory.doubled := message.count * 2",
        own_message
    );
    ar_data__destroy(own_message);
    
    assert(result == true);
    ar_data_t *ref_doubled = ar_data__get_map_data(mut_memory, "doubled");
    assert(ref_doubled != NULL);
    assert(ar_data__get_type(ref_doubled) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_doubled) == 84);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Fixture execute instruction test passed!\n");
}

static void test_fixture_data_tracking(void) {
    printf("Testing fixture data tracking...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_tracking");
    assert(own_fixture != NULL);
    
    // When we create a test map
    ar_data_t *ref_map = ar_interpreter_fixture__create_test_map(own_fixture, "test_data");
    
    // Then it should be created with test values
    assert(ref_map != NULL);
    assert(ar_data__get_type(ref_map) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_name = ar_data__get_map_data(ref_map, "name");
    assert(ref_name != NULL);
    assert(strcmp(ar_data__get_string(ref_name), "test_data") == 0);
    
    ar_data_t *ref_count = ar_data__get_map_data(ref_map, "count");
    assert(ref_count != NULL);
    assert(ar_data__get_integer(ref_count) == 42);
    
    // When we track additional data
    ar_data_t *own_extra = ar_data__create_string("tracked");
    ar_interpreter_fixture__track_data(own_fixture, own_extra);
    own_extra = NULL; // Ownership transferred to fixture
    
    // Then the fixture should clean up everything when destroyed
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Fixture data tracking test passed!\n");
}
