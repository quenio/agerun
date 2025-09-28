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
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run from a bin directory\n");
            return 1;
        }
    }
    
    // Clean state - remove persistence files
    remove("methodology.agerun");
    remove("agerun.agency");
    
    // Initialize system
    
    // Run tests
    test_fixture_create_destroy();
    test_fixture_create_agent();
    test_fixture_execute_instruction();
    test_fixture_data_tracking();
    
    // Cleanup - remove persistence files
    remove("methodology.agerun");
    remove("agerun.agency");
    
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
    int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture,
        "memory.value := 42"
    );
    
    // Then it should succeed (returns non-zero agent ID)
    if (temp_agent_id == 0) {
        const char *error = ar_log__get_last_error_message(ar_interpreter_fixture__get_log(own_fixture));
        if (error) {
            fprintf(stderr, "ERROR: Execute instruction failed: %s\n", error);
        }
    }
    assert(temp_agent_id > 0);
    
    // And the memory should be updated
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_value = ar_data__get_map_data(mut_memory, "value");
    assert(ref_value != NULL);
    assert(ar_data__get_type(ref_value) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_value) == 42);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test execution with message - we need to create new values through operations
    // Direct assignment of message or its fields fails because they're const references
    ar_data_t *own_message = ar_data__create_map();
    ar_data__set_map_string(own_message, "text", "Hello");
    ar_data__set_map_integer(own_message, "count", 42);
    
    // Take ownership of the message (fixture owns it during execution)
    ar_data__take_ownership(own_message, own_fixture);
    
    // Test string concatenation with message field
    temp_agent_id = ar_interpreter_fixture__execute_with_message(
        own_fixture,
        "memory.greeting := \"Message says: \" + message.text",
        own_message
    );
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_greeting = ar_data__get_map_data(mut_memory, "greeting");
    assert(ref_greeting != NULL);
    assert(ar_data__get_type(ref_greeting) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_greeting), "Message says: Hello") == 0);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Destroy message if fixture still owns it
    ar_data__destroy_if_owned(own_message, own_fixture);
    
    // Create a new message for the second test
    own_message = ar_data__create_map();
    ar_data__set_map_string(own_message, "text", "World");
    ar_data__set_map_integer(own_message, "count", 42);
    
    // Take ownership again
    ar_data__take_ownership(own_message, own_fixture);
    
    // Test arithmetic with message field
    temp_agent_id = ar_interpreter_fixture__execute_with_message(
        own_fixture,
        "memory.doubled := message.count * 2",
        own_message
    );
    
    // Destroy message if fixture still owns it
    ar_data__destroy_if_owned(own_message, own_fixture);
    
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_doubled = ar_data__get_map_data(mut_memory, "doubled");
    assert(ref_doubled != NULL);
    assert(ar_data__get_type(ref_doubled) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_doubled) == 84);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
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
