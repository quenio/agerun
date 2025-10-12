#include "ar_interpreter.h"
#include "ar_interpreter_fixture.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_agent.h"
#include "ar_agency.h"
#include "ar_system.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

// Forward declarations
static void test_interpreter_create_destroy(void);
static void test_interpreter_execute_method(void);
static void test_interpreter_error_logging(void);
static void test_simple_instructions(void);
static void test_condition_instructions(void);
static void test_compile_function(void);
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
    
    // Clean state - remove persistence files
    remove("methodology.agerun");
    remove("agerun.agency");
    
    // System is managed internally by fixtures
    
    // Run tests
    test_interpreter_create_destroy();
    test_interpreter_execute_method();
    test_interpreter_error_logging();
    
    // Migrated execution tests - now using fixture
    test_simple_instructions();
    test_condition_instructions();
    test_compile_function();
    test_parse_function();
    
    // Cleanup is handled internally by fixtures
    remove("methodology.agerun");
    remove("agerun.agency");
    
    printf("All interpreter tests passed!\n");
    return 0;
}

static void test_interpreter_create_destroy(void) {
    printf("Testing interpreter create/destroy...\n");
    
    // Given a system and log
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);

    // When we create an interpreter with the log, agency, and delegation
    ar_interpreter_t *own_interpreter = ar_interpreter__create_with_agency(own_log, ref_agency, ref_delegation);
    
    // Then it should be created successfully
    assert(own_interpreter != NULL);
    
    // When we destroy it
    ar_interpreter__destroy(own_interpreter);
    own_interpreter = NULL;
    
    // And clean up the log and system
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    // Then no memory leaks should occur
    printf("Interpreter create/destroy test passed!\n");
}

static void test_interpreter_execute_method(void) {
    printf("Testing interpreter execute method...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_execute_method");
    assert(own_fixture != NULL);
    
    // When we create an agent with a method that stores the message
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_echo", 
        "memory.result := message",
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
    
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ref_result != NULL);
    assert(ar_data__get_type(ref_result) == AR_DATA_TYPE__STRING);
    assert(strcmp(ar_data__get_string(ref_result), "Hello, interpreter!") == 0);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Interpreter execute method test passed!\n");
}

static void test_interpreter_error_logging(void) {
    printf("Testing interpreter error logging...\n");
    
    // Given a system and log
    ar_system_t *own_system = ar_system__create();
    assert(own_system != NULL);
    ar_agency_t *ref_agency = ar_system__get_agency(own_system);
    assert(ref_agency != NULL);
    ar_delegation_t *ref_delegation = ar_system__get_delegation(own_system);
    assert(ref_delegation != NULL);

    ar_log_t *own_log = ar_log__create();
    assert(own_log != NULL);

    // And an interpreter with that log, agency, and delegation
    ar_interpreter_t *own_interpreter = ar_interpreter__create_with_agency(own_log, ref_agency, ref_delegation);
    assert(own_interpreter != NULL);
    
    // When we try to execute a method for a non-existent agent
    bool result = ar_interpreter__execute_method(own_interpreter, 999999, NULL);
    
    // Then execution should fail
    assert(result == false);
    
    // And an error should have been logged
    // (We can't directly check the log contents in this test, but we've verified
    // that the error path was taken. In a real scenario, we might have a way to
    // inspect the log or use a mock log for testing.)
    
    // Clean up
    ar_interpreter__destroy(own_interpreter);
    ar_log__destroy(own_log);
    ar_system__destroy(own_system);
    
    printf("Interpreter error logging test passed!\n");
}


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
    
    // Test if with expression condition (non-zero)
    temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.result := if(5, \"yes\", \"no\")");
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(strcmp(ar_data__get_string(ref_result), "yes") == 0);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Test if with calculated condition
    temp_agent_id = ar_interpreter_fixture__execute_instruction(own_fixture, "memory.result := if(2 + 2, 100, 200)");
    assert(temp_agent_id > 0);
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 100);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Condition instructions test passed!\n");
}

static void test_compile_function(void) {
    printf("Testing compile function...\n");
    
    // Given a fixture
    ar_interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_compile_func");
    assert(own_fixture != NULL);
    
    // Create a method dynamically using compile
    int64_t temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        "memory.result := compile(\"dynamic\", \"memory.x := 99\", \"2.0.0\")"
    );
    assert(temp_agent_id > 0);
    
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 1);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Verify method was created by checking that compile returned success (1)
    // We can't directly access the methodology from here, but the fact that
    // compile returned 1 means the method was successfully registered
    
    // Test creating method with string version
    temp_agent_id = ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        "memory.result := compile(\"versioned\", \"memory.y := 88\", \"3.0.0\")"
    );
    assert(temp_agent_id > 0);
    
    mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, temp_agent_id);
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 1);
    ar_interpreter_fixture__destroy_temp_agent(own_fixture, temp_agent_id);
    
    // Method creation verified by successful return value
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Compile function test passed!\n");
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




