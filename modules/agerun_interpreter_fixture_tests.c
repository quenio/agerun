#include "agerun_interpreter_fixture.h"
#include "agerun_agency.h"
#include "agerun_system.h"
#include "agerun_methodology.h"
#include "agerun_data.h"
#include "agerun_heap.h"
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
            fprintf(stderr, "Please run: cd bin && ./agerun_interpreter_fixture_tests\n");
            return 1;
        }
    }
    
    // Clean state
    ar__system__shutdown();
    ar__methodology__cleanup();
    ar__agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system
    ar__system__init(NULL, NULL);
    
    // Run tests
    test_fixture_create_destroy();
    test_fixture_create_agent();
    test_fixture_execute_instruction();
    test_fixture_data_tracking();
    
    // Cleanup
    ar__system__shutdown();
    ar__methodology__cleanup();
    ar__agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    printf("All 8 tests passed!\n");
    return 0;
}

static void test_fixture_create_destroy(void) {
    printf("Testing fixture create/destroy...\n");
    
    // Given the need for a fixture
    // When we create one
    interpreter_fixture_t *own_fixture = ar__interpreter_fixture__create("test_create");
    
    // Then it should be created successfully
    assert(own_fixture != NULL);
    assert(strcmp(ar__interpreter_fixture__get_name(own_fixture), "test_create") == 0);
    
    // And it should have an interpreter
    interpreter_t *ref_interpreter = ar__interpreter_fixture__get_interpreter(own_fixture);
    assert(ref_interpreter != NULL);
    
    // When we destroy it
    ar__interpreter_fixture__destroy(own_fixture);
    
    // Then no memory leaks should occur
    printf("Fixture create/destroy test passed!\n");
}

static void test_fixture_create_agent(void) {
    printf("Testing fixture create agent...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar__interpreter_fixture__create("test_agent");
    assert(own_fixture != NULL);
    
    // When we create an agent
    int64_t agent_id = ar__interpreter_fixture__create_agent(
        own_fixture,
        "test_method",
        "memory.result := \"test\"",
        NULL  // Use default version
    );
    
    // Then it should be created successfully
    assert(agent_id > 0);
    
    // And we should be able to get its memory
    data_t *mut_memory = ar__interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    assert(mut_memory != NULL);
    
    // Clean up
    ar__interpreter_fixture__destroy(own_fixture);
    
    printf("Fixture create agent test passed!\n");
}

static void test_fixture_execute_instruction(void) {
    printf("Testing fixture execute instruction...\n");
    
    // Given a fixture with an agent
    interpreter_fixture_t *own_fixture = ar__interpreter_fixture__create("test_execute");
    assert(own_fixture != NULL);
    
    int64_t agent_id = ar__interpreter_fixture__create_agent(
        own_fixture,
        "test_exec",
        "memory.x := 1",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // When we execute an instruction
    bool result = ar__interpreter_fixture__execute_instruction(
        own_fixture,
        agent_id,
        "memory.value := 42"
    );
    
    // Then it should succeed
    assert(result == true);
    
    // And the memory should be updated
    data_t *mut_memory = ar__interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    data_t *ref_value = ar__data__get_map_data(mut_memory, "value");
    assert(ref_value != NULL);
    assert(ar__data__get_type(ref_value) == DATA_INTEGER);
    assert(ar__data__get_integer(ref_value) == 42);
    
    // Test execution with message - we need to create new values through operations
    // Direct assignment of message or its fields fails because they're const references
    data_t *own_message = ar__data__create_map();
    ar__data__set_map_string(own_message, "text", "Hello");
    ar__data__set_map_integer(own_message, "count", 42);
    
    // Test string concatenation with message field
    result = ar__interpreter_fixture__execute_with_message(
        own_fixture,
        agent_id,
        "memory.greeting := \"Message says: \" + message.text",
        own_message
    );
    assert(result == true);
    data_t *ref_greeting = ar__data__get_map_data(mut_memory, "greeting");
    assert(ref_greeting != NULL);
    assert(ar__data__get_type(ref_greeting) == DATA_STRING);
    assert(strcmp(ar__data__get_string(ref_greeting), "Message says: Hello") == 0);
    
    // Test arithmetic with message field
    result = ar__interpreter_fixture__execute_with_message(
        own_fixture,
        agent_id,
        "memory.doubled := message.count * 2",
        own_message
    );
    ar__data__destroy(own_message);
    
    assert(result == true);
    data_t *ref_doubled = ar__data__get_map_data(mut_memory, "doubled");
    assert(ref_doubled != NULL);
    assert(ar__data__get_type(ref_doubled) == DATA_INTEGER);
    assert(ar__data__get_integer(ref_doubled) == 84);
    
    // Clean up
    ar__interpreter_fixture__destroy(own_fixture);
    
    printf("Fixture execute instruction test passed!\n");
}

static void test_fixture_data_tracking(void) {
    printf("Testing fixture data tracking...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar__interpreter_fixture__create("test_tracking");
    assert(own_fixture != NULL);
    
    // When we create a test map
    data_t *ref_map = ar__interpreter_fixture__create_test_map(own_fixture, "test_data");
    
    // Then it should be created with test values
    assert(ref_map != NULL);
    assert(ar__data__get_type(ref_map) == DATA_MAP);
    
    data_t *ref_name = ar__data__get_map_data(ref_map, "name");
    assert(ref_name != NULL);
    assert(strcmp(ar__data__get_string(ref_name), "test_data") == 0);
    
    data_t *ref_count = ar__data__get_map_data(ref_map, "count");
    assert(ref_count != NULL);
    assert(ar__data__get_integer(ref_count) == 42);
    
    // When we track additional data
    data_t *own_extra = ar__data__create_string("tracked");
    ar__interpreter_fixture__track_data(own_fixture, own_extra);
    
    // Then the fixture should clean up everything when destroyed
    ar__interpreter_fixture__destroy(own_fixture);
    
    printf("Fixture data tracking test passed!\n");
}
