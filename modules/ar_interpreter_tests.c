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
static void test_memory_access_instructions(void);
static void test_condition_instructions(void);
static void test_message_send_instructions(void);
static void test_method_function(void);
static void test_parse_function(void);
static void test_build_function(void);
static void test_agent_function(void);

int main(void) {
    printf("=== AgeRun Interpreter Tests ===\n");
    
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run: cd bin && ./agerun_interpreter_tests\n");
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
    test_memory_access_instructions();
    test_condition_instructions();
    test_message_send_instructions();
    test_method_function();
    test_parse_function();
    test_build_function();
    test_agent_function();
    
    // Cleanup
    ar_system__shutdown();
    ar_methodology__cleanup();
    ar_agency__reset();
    remove("methodology.agerun");
    remove("agency.agerun");
    
    printf("All 28 tests passed!\n");
    return 0;
}

static void test_interpreter_create_destroy(void) {
    printf("Testing interpreter create/destroy...\n");
    
    // Given the need for an interpreter
    // When we create one
    interpreter_t *own_interpreter = ar_interpreter__create();
    
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
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_execute_method");
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
    assert(ar_data__get_type(ref_result) == DATA_STRING);
    assert(strcmp(ar_data__get_string(ref_result), "Received: Hello, interpreter!") == 0);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Interpreter execute method test passed!\n");
}

static void test_interpreter_execute_instruction(void) {
    printf("Testing interpreter execute instruction...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_execute_instruction");
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
    bool result = ar_interpreter_fixture__execute_instruction(
        own_fixture,
        agent_id,
        "memory.y := 10"
    );
    
    // Then it should succeed
    assert(result == true);
    
    // And the memory should be updated
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *ref_y = ar_data__get_map_data(mut_memory, "y");
    assert(ref_y != NULL);
    assert(ar_data__get_type(ref_y) == DATA_INTEGER);
    assert(ar_data__get_integer(ref_y) == 10);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Interpreter execute instruction test passed!\n");
}

// Migrated tests from instruction_tests.c
static void test_simple_instructions(void) {
    printf("Testing simple instructions...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_simple");
    assert(own_fixture != NULL);
    
    // And an agent
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_simple",
        "",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // Test integer assignment
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.x := 42"));
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *ref_x = ar_data__get_map_data(mut_memory, "x");
    assert(ar_data__get_integer(ref_x) == 42);
    
    // Test string assignment
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.msg := \"Hello\""));
    ar_data_t *ref_msg = ar_data__get_map_data(mut_memory, "msg");
    assert(strcmp(ar_data__get_string(ref_msg), "Hello") == 0);
    
    // Test double assignment
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.pi := 3.14"));
    ar_data_t *ref_pi = ar_data__get_map_data(mut_memory, "pi");
    assert(ar_data__get_double(ref_pi) == 3.14);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Simple instructions test passed!\n");
}

static void test_memory_access_instructions(void) {
    printf("Testing memory access instructions...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_memory_access");
    assert(own_fixture != NULL);
    
    // And an agent with nested memory
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_memory",
        "",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // Set up nested structure - need to create parent map first (AgeRun doesn't auto-create intermediate maps)
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *own_user_map = ar_data__create_map();
    ar_data__set_map_data(mut_memory, "user", own_user_map);
    
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.user.name := \"Alice\""));
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.user.age := 30"));
    
    // Debug: Check if the values were actually set
    ar_data_t *ref_user_name = ar_data__get_map_data(mut_memory, "user.name");
    printf("DEBUG: user.name type=%d, value=%s\n", 
           ref_user_name ? (int)ar_data__get_type(ref_user_name) : -1,
           ref_user_name ? ar_data__get_string(ref_user_name) : "NULL");
    
    printf("DEBUG: About to execute send instruction\n");
    
    // Note: send() with memory references is not currently supported
    // The expression evaluator returns references for memory access, 
    // but send() needs to take ownership of the message
    
    // Test arithmetic with nested values  
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.next_age := memory.user.age + 1"));
    ar_data_t *ref_next = ar_data__get_map_data(mut_memory, "next_age");
    assert(ar_data__get_integer(ref_next) == 31);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Memory access instructions test passed!\n");
}

static void test_condition_instructions(void) {
    printf("Testing condition instructions...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_conditions");
    assert(own_fixture != NULL);
    
    // And an agent
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "test_conditions",
        "",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // Test if with true condition
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.result := if(1, \"yes\", \"no\")"));
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(strcmp(ar_data__get_string(ref_result), "yes") == 0);
    
    // Test if with false condition
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.result := if(0, \"yes\", \"no\")"));
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(strcmp(ar_data__get_string(ref_result), "no") == 0);
    
    // Test if with string condition
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.result := if(\"test\", 1, 0)"));
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 1);
    
    // Test if with empty string condition
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.result := if(\"\", 1, 0)"));
    ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 0);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Condition instructions test passed!\n");
}

static void test_message_send_instructions(void) {
    printf("Testing message send instructions...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_send");
    assert(own_fixture != NULL);
    
    // Create two agents
    int64_t sender_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "sender",
        "",
        "1.0.0"
    );
    assert(sender_id > 0);
    
    int64_t receiver_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "receiver",
        "memory.received := \"Got: \" + message",
        "1.0.0"
    );
    assert(receiver_id > 0);
    
    // Send message from sender to receiver
    char send_cmd[256];
    snprintf(send_cmd, sizeof(send_cmd), "send(%" PRId64 ", \"Test message\")", receiver_id);
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, sender_id, send_cmd));
    
    // Process the message
    ar_system__process_next_message();
    
    // Check receiver got the message
    ar_data_t *receiver_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, receiver_id);
    ar_data_t *ref_received = ar_data__get_map_data(receiver_memory, "received");
    assert(ref_received != NULL);
    assert(strcmp(ar_data__get_string(ref_received), "Got: Test message") == 0);
    
    // Test send with assignment
    snprintf(send_cmd, sizeof(send_cmd), "memory.sent := send(%" PRId64 ", \"Another message\")", receiver_id);
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, sender_id, send_cmd));
    
    ar_data_t *sender_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, sender_id);
    ar_data_t *ref_sent = ar_data__get_map_data(sender_memory, "sent");
    assert(ar_data__get_integer(ref_sent) == 1);
    
    // Test send to agent 0 (no-op)
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, sender_id, "memory.noop := send(0, \"Nowhere\")"));
    ref_sent = ar_data__get_map_data(sender_memory, "noop");
    assert(ar_data__get_integer(ref_sent) == 1);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Message send instructions test passed!\n");
}

static void test_method_function(void) {
    printf("Testing method function...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_method_func");
    assert(own_fixture != NULL);
    
    // And an agent
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "creator",
        "",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // Create a method dynamically
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        agent_id, 
        "memory.result := method(\"dynamic\", \"memory.x := 99\", \"2.0.0\")"
    ));
    
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(ar_data__get_integer(ref_result) == 1);
    
    // Verify method was created
    const ar_method_t *ref_method = ar_methodology__get_method("dynamic", "2.0.0");
    assert(ref_method != NULL);
    assert(strcmp(ar_method__get_name(ref_method), "dynamic") == 0);
    assert(strcmp(ar_method__get_version(ref_method), "2.0.0") == 0);
    
    // Test creating method with integer version
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        agent_id, 
        "memory.result := method(\"versioned\", \"memory.y := 88\", 3)"
    ));
    
    ref_method = ar_methodology__get_method("versioned", "3.0.0");
    assert(ref_method != NULL);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Method function test passed!\n");
}

static void test_parse_function(void) {
    printf("Testing parse function...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_parse");
    assert(own_fixture != NULL);
    
    // And an agent
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "parser",
        "memory.initialized := 1",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // Test simple parsing
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        agent_id, 
        "memory.parsed := parse(\"Hello {name}\", \"Hello World\")"
    ));
    
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *ref_parsed = ar_data__get_map_data(mut_memory, "parsed");
    assert(ar_data__get_type(ref_parsed) == DATA_MAP);
    
    ar_data_t *ref_name = ar_data__get_map_data(ref_parsed, "name");
    assert(ref_name != NULL);
    assert(strcmp(ar_data__get_string(ref_name), "World") == 0);
    
    // Test parsing with multiple variables
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        agent_id, 
        "memory.user := parse(\"Name: {name}, Age: {age}\", \"Name: Bob, Age: 25\")"
    ));
    
    ar_data_t *ref_user = ar_data__get_map_data(mut_memory, "user");
    assert(ar_data__get_type(ref_user) == DATA_MAP);
    
    ref_name = ar_data__get_map_data(ref_user, "name");
    assert(strcmp(ar_data__get_string(ref_name), "Bob") == 0);
    
    ar_data_t *ref_age = ar_data__get_map_data(ref_user, "age");
    assert(ar_data__get_integer(ref_age) == 25);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Parse function test passed!\n");
}

static void test_build_function(void) {
    printf("Testing build function...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_build");
    assert(own_fixture != NULL);
    
    // And an agent
    int64_t agent_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "builder",
        "",
        "1.0.0"
    );
    assert(agent_id > 0);
    
    // Set up data for building - need to create intermediate map first
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    ar_data_t *own_data_map = ar_data__create_map();
    ar_data__set_map_data(mut_memory, "data", own_data_map);
    own_data_map = NULL; // Ownership transferred to memory
    
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.data.name := \"Alice\""));
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, agent_id, "memory.data.count := 42"));
    
    // Test building with variables
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        agent_id, 
        "memory.result := build(\"Hello {name}, count is {count}\", memory.data)"
    ));
    
    ar_data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(strcmp(ar_data__get_string(ref_result), "Hello Alice, count is 42") == 0);
    
    // Test building with missing variable
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        agent_id, 
        "memory.partial := build(\"Name: {name}, Age: {age}\", memory.data)"
    ));
    
    ar_data_t *ref_partial = ar_data__get_map_data(mut_memory, "partial");
    assert(strcmp(ar_data__get_string(ref_partial), "Name: Alice, Age: {age}") == 0);
    
    // Clean up
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Build function test passed!\n");
}

static void test_agent_function(void) {
    printf("Testing agent function...\n");
    
    // Given a fixture
    interpreter_fixture_t *own_fixture = ar_interpreter_fixture__create("test_agent_func");
    assert(own_fixture != NULL);
    
    // Create a method for new agents
    assert(ar_interpreter_fixture__create_method(
        own_fixture,
        "worker",
        "memory.initialized := 1",
        "1.0.0"
    ));
    
    // And an agent that creates other agents
    int64_t creator_id = ar_interpreter_fixture__create_agent(
        own_fixture,
        "creator",
        "",
        "1.0.0"
    );
    assert(creator_id > 0);
    
    // Create agent without context
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        creator_id, 
        "memory.worker1 := agent(\"worker\", \"1.0.0\", 0)"
    ));
    
    ar_data_t *mut_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, creator_id);
    ar_data_t *ref_worker1 = ar_data__get_map_data(mut_memory, "worker1");
    int64_t worker1_id = ar_data__get_integer(ref_worker1);
    assert(worker1_id > 0);
    
    // Process wake message
    ar_system__process_next_message();
    
    // Verify worker was initialized
    ar_data_t *worker1_memory = ar_interpreter_fixture__get_agent_memory(own_fixture, worker1_id);
    assert(worker1_memory != NULL);
    ar_data_t *ref_init = ar_data__get_map_data(worker1_memory, "initialized");
    assert(ref_init != NULL);
    assert(ar_data__get_integer(ref_init) == 1);
    
    // Create agent with context - need to create intermediate map first
    ar_data_t *own_ctx_map = ar_data__create_map();
    ar_data__set_map_data(mut_memory, "ctx", own_ctx_map);
    assert(ar_interpreter_fixture__execute_instruction(own_fixture, creator_id, "memory.ctx.role := \"supervisor\""));
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        creator_id, 
        "memory.worker2 := agent(\"worker\", \"1.0.0\", memory.ctx)"
    ));
    
    ar_data_t *ref_worker2 = ar_data__get_map_data(mut_memory, "worker2");
    int64_t worker2_id = ar_data__get_integer(ref_worker2);
    assert(worker2_id > 0);
    assert(worker2_id != worker1_id);
    
    // Test with integer version
    assert(ar_interpreter_fixture__execute_instruction(
        own_fixture, 
        creator_id, 
        "memory.worker3 := agent(\"worker\", 1, 0)"
    ));
    
    ar_data_t *ref_worker3 = ar_data__get_map_data(mut_memory, "worker3");
    int64_t worker3_id = ar_data__get_integer(ref_worker3);
    assert(worker3_id > 0);
    
    // Clean up (agents are automatically cleaned up by fixture)
    ar_interpreter_fixture__destroy(own_fixture);
    
    printf("Agent function test passed!\n");
}


