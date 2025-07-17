#include "ar_interpreter.h"
#include "ar_interpreter_fixture.h"
#include "ar_instruction.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agent.h"
#include "ar_agency.h"
#include "ar_system.h"
#include "ar_data.h"
#include "ar_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

// Forward declarations
static void test_interpreter_create_destroy(void);
static void test_interpreter_execute_method(void);
static void test_interpreter_execute_instruction(void);
static void test_simple_instructions(void);
static void test_condition_instructions(void);
static void test_method_function(void);
static void test_parse_function(void);

int main(void) {
    printf("=== AgeRun Interpreter Tests ===\n");
    
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
    
    // Clean state
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    // Initialize system
    ar_system__init(NULL, NULL);
    
    // Run tests
    test_interpreter_create_destroy();
    test_interpreter_execute_method();
    test_interpreter_execute_instruction();
    
    // Migrated execution tests
    test_simple_instructions();
    test_condition_instructions();
    test_method_function();
    test_parse_function();
    
    // Cleanup
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    printf("All interpreter tests passed!\n");
    return 0;
}

static void test_interpreter_create_destroy(void) {
    printf("Testing interpreter create/destroy...\n");
    
    // Given the need for an interpreter
    // When we create one
    ar_interpreter_t *own_interpreter = ar_interpreter__create();
    
    // Then it should be created successfully
    assert(own_interpreter != NULL);
    
    // When we destroy it
    ar_interpreter__destroy(own_interpreter);
    own_interpreter = NULL;
    
    // Then no memory leaks should occur
    printf("Interpreter create/destroy test passed!\n");
}

static void test_interpreter_execute_method(void) {
    printf("Testing interpreter execute method...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_execute_method");
    assert(own_fixture != NULL);
    
    // When we create an agent with a method that uses string concatenation
    // (which creates a new value that can be owned)
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_echo",
        "memory.result := \"Received: \" + message",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // And send a message
    ar_data_t *own_message = ar_data__create_string("Hello, interpreter!");
    bool sent = ar_interpreter_fixture__send_message(own_fixture, agent_id, own_message);
    assert(sent == true);
    // Ownership transferred
    
    // Then the agent's memory should contain the result
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    assert(mut_memory != NULL);
    
    // Debug: Print what's in memory - simplified version
    printf("DEBUG: Checking memory contents...\n");
    printf("DEBUG: Memory is %s\n", mut_memory ? "not null" : "null");
    if (mut_memory) {
        printf("DEBUG: Memory type is %d\n", ar_data__get_type(mut_memory));
    }
    
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    printf("DEBUG: Result is %s\n", ref_result ? "not null" : "null");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_result), "Received: Hello, interpreter!") == 0);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Interpreter execute method test passed!\n");
}

static void test_interpreter_execute_instruction(void) {
    printf("Testing interpreter execute instruction...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_execute_instruction");
    assert(own_fixture != NULL);
    
    // And an agent
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_calc",
        "memory.x := 5",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // When we execute a single instruction
    int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture,
        "memory.y := 10"
    );
    
    // Then it should succeed (returns non-zero agent ID)
    assert(temp_agent_id > 0);
    
    // And the memory should be updated
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_y = ar_data__get_map_data(mut_memory, "y");
    assert(ref_y != NULL);
    assert(ar_data__get_type(ref_y) == AR_DATA_TYPE__INTEGER);
    assert(ar_data__get_integer(ref_y) == 10);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Interpreter execute instruction test passed!\n");
}

// Migrated tests from instruction_tests.c
static void test_simple_instructions(void) {
    printf("Testing simple instructions...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_simple");
    assert(own_fixture != NULL);
    
    // Test integer assignment
    int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.x := 42");
    assert(temp_agent_id > 0);
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_x = ar_data__get_map_data(mut_memory, "x");
    assert(ar_data__get_integer(ref_x) == 42);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test string assignment
    temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.msg := \"Hello\"");
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_msg = ar_data__get_map_data(mut_memory, "msg");
    assert(strcmp(ar_data__get_string(ref_msg), "Hello") == 0);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test double assignment
    temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.pi := 3.14");
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_pi = ar_data__get_map_data(mut_memory, "pi");
    assert(ar_data__get_double(ref_pi) == 3.14);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Simple instructions test passed!\n");
}

static void test_condition_instructions(void) {
    printf("Testing condition instructions...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_conditions");
    assert(own_fixture != NULL);
    
    // Test if with true condition
    int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.result := if(1, \"yes\", \"no\")");
    assert(temp_agent_id > 0);
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(strcmp(ar_data__get_string(ref_result), "yes") == 0);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test if with false condition
    temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.result := if(0, \"yes\", \"no\")");
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(strcmp(ar_data__get_string(ref_result), "no") == 0);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test if with string condition
    temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.result := if(\"test\", 1, 0)");
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 1);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test if with empty string condition
    temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.result := if(\"\", 1, 0)");
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 0);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Condition instructions test passed!\n");
}

static void test_method_function(void) {
    printf("Testing method function...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_method_func");
    assert(own_fixture != NULL);
    
    // Create a method dynamically
    int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        "memory.result := method(\"dynamic\", \"memory.x := 99\", \"2.0.0\")"
    );
    assert(temp_agent_id > 0);
    
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 1);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Verify method was created
    const ar_method_t *ref_method = ar_methodology__get_method("dynamic", "2.0.0");
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_name(ref_method), "dynamic") == 0);
    assert(strcmp(ar_method__get_version(ref_method), "2.0.0") == 0);
    
    // Test creating method with integer version
    temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        "memory.result := method(\"versioned\", \"memory.y := 88\", 3)"
    );
    assert(temp_agent_id > 0);
    
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 1);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    ref_method = ar_methodology__get_method("versioned", "3.0.0");
    assert(ref_method != NULL);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Method function test passed!\n");
}

static void test_parse_function(void) {
    printf("Testing parse function...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_parse");
    assert(own_fixture != NULL);
    
    // Test simple parsing
    int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        "memory.parsed := parse(\"Hello {name}\", \"Hello World\")"
    );
    assert(temp_agent_id > 0);
    
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_parsed = ar_data__get_map_data(mut_memory, "parsed");
    assert(ar_data__get_type(ref_parsed) == AR_DATA_TYPE__MAP);
    
    ar_data_t *ref_name = ar_data__get_map_data(ref_parsed, "name");
    assert(ref_name != NULL);
    assert(strcmp(ar_data__get_string(ref_name), "World") == 0);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test parsing with multiple variables
    temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        "memory.user := parse(\"Name: {name}, Age: {age}\", \"Name: Bob, Age: 25\")"
    );
    assert(temp_agent_id > 0);
    
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_user = ar_data__get_map_data(mut_memory, "user");
    assert(ar_data__get_type(ref_user) == AR_DATA_TYPE__MAP);
    
    ref_name = ar_data__get_map_data(ref_user, "name");
    assert(strcmp(ar_data__get_string(ref_name), "Bob") == 0);
    
    ar_data_t *ref_age = ar_data__get_map_data(ref_user, "age");
    assert(ar_data__get_integer(ref_age) == 25);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Parse function test passed!\n");
}




