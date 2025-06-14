#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "agerun_instruction.h"
#include "agerun_data.h"
#include "agerun_map.h"
#include "agerun_heap.h"

// Test functions

static void test_parse_assignment_instructions(void) {
    // Given an instruction context with memory
    data_t *mut_memory = ar__data__create_map();
    assert(mut_memory != NULL);
    
    instruction_context_t *mut_ctx = ar__instruction__create_context(mut_memory, NULL, NULL);
    assert(mut_ctx != NULL);
    
    // Test 1: Simple assignment with integer literal
    {
        // When parsing an integer assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.x := 42", mut_ctx);
        
        // Then it should parse successfully as an assignment
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_ASSIGNMENT);
        assert(strcmp(ar__instruction__get_assignment_path(own_parsed), "x") == 0);  // Path stores only the part after "memory."
        assert(strcmp(ar__instruction__get_assignment_expression(own_parsed), "42") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 2: Assignment with string literal
    {
        // When parsing a string assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.greeting := \"Hello, World!\"", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_ASSIGNMENT);
        assert(strcmp(ar__instruction__get_assignment_path(own_parsed), "greeting") == 0);
        assert(strcmp(ar__instruction__get_assignment_expression(own_parsed), "\"Hello, World!\"") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 3: Assignment with nested memory path
    {
        // When parsing an assignment with nested path
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.user.name := \"Alice\"", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_ASSIGNMENT);
        assert(strcmp(ar__instruction__get_assignment_path(own_parsed), "user.name") == 0);
        assert(strcmp(ar__instruction__get_assignment_expression(own_parsed), "\"Alice\"") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 4: Assignment with arithmetic expression
    {
        // When parsing an assignment with arithmetic
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.result := 2 + 3 * 4", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_ASSIGNMENT);
        assert(strcmp(ar__instruction__get_assignment_path(own_parsed), "result") == 0);
        assert(strcmp(ar__instruction__get_assignment_expression(own_parsed), "2 + 3 * 4") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 5: Assignment with memory reference in expression
    {
        // First, add a value to memory so the reference is valid
        ar__data__set_map_string(mut_memory, "original", "test value");
        
        // When parsing an assignment referencing other memory
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.copy := memory.original", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_ASSIGNMENT);
        assert(strcmp(ar__instruction__get_assignment_path(own_parsed), "copy") == 0);
        assert(strcmp(ar__instruction__get_assignment_expression(own_parsed), "memory.original") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 6: Assignment with whitespace variations
    {
        // When parsing with extra whitespace
        parsed_instruction_t *own_parsed = ar__instruction__parse("  memory.x  :=  42  ", mut_ctx);
        
        // Then it should parse successfully, trimming whitespace
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_ASSIGNMENT);
        assert(strcmp(ar__instruction__get_assignment_path(own_parsed), "x") == 0);
        assert(strcmp(ar__instruction__get_assignment_expression(own_parsed), "42") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Cleanup
    ar__instruction__destroy_context(mut_ctx);
    ar__data__destroy(mut_memory);
}

static void test_parse_function_call_instructions(void) {
    // Given an instruction context with memory
    data_t *mut_memory = ar__data__create_map();
    assert(mut_memory != NULL);
    
    instruction_context_t *mut_ctx = ar__instruction__create_context(mut_memory, NULL, NULL);
    assert(mut_ctx != NULL);
    
    // Test 1: Simple send function call
    {
        // When parsing a send function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("send(0, \"Hello\")", mut_ctx);
        
        // Then it should parse successfully as a function call
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_SEND);
        
        // Verify function call details
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "send") == 0);
        assert(arg_count == 2);
        assert(args != NULL);
        assert(strcmp(args[0], "0") == 0);
        assert(strcmp(args[1], "\"Hello\"") == 0);
        assert(result_path == NULL);  // No assignment
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 2: Parse function call
    {
        // When parsing a parse function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("parse(\"name={name}\", \"name=John\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_PARSE);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "parse") == 0);
        assert(arg_count == 2);
        assert(strcmp(args[0], "\"name={name}\"") == 0);
        assert(strcmp(args[1], "\"name=John\"") == 0);
        assert(result_path == NULL);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 3: Build function call
    {
        // Add a map value for the build function to reference
        data_t *own_values = ar__data__create_map();
        ar__data__set_map_string(own_values, "name", "Alice");
        ar__data__set_map_data(mut_memory, "values", own_values);
        
        // When parsing a build function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("build(\"Hello {name}\", memory.values)", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_BUILD);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "build") == 0);
        assert(arg_count == 2);
        assert(strcmp(args[0], "\"Hello {name}\"") == 0);
        assert(strcmp(args[1], "memory.values") == 0);
        assert(result_path == NULL);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 4: Method function call
    {
        // When parsing a method function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("method(\"greet\", \"memory.msg := \\\"Hi\\\"\", \"1.0.0\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_METHOD);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "method") == 0);
        assert(arg_count == 3);
        assert(strcmp(args[0], "\"greet\"") == 0);
        assert(strcmp(args[1], "\"memory.msg := \\\"Hi\\\"\"") == 0);
        assert(strcmp(args[2], "\"1.0.0\"") == 0);
        assert(result_path == NULL);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 5: Agent function call with context
    {
        // Add context for agent creation
        data_t *own_context = ar__data__create_map();
        ar__data__set_map_string(own_context, "name", "Test Agent");
        ar__data__set_map_data(mut_memory, "ctx", own_context);
        
        // When parsing an agent function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("agent(\"echo\", \"1.0.0\", memory.ctx)", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_AGENT);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "agent") == 0);
        assert(arg_count == 3);
        assert(strcmp(args[0], "\"echo\"") == 0);
        assert(strcmp(args[1], "\"1.0.0\"") == 0);
        assert(strcmp(args[2], "memory.ctx") == 0);
        assert(result_path == NULL);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 6: Destroy function call (single argument)
    {
        // When parsing a destroy agent function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("destroy(42)", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_DESTROY);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "destroy") == 0);
        assert(arg_count == 1);
        assert(strcmp(args[0], "42") == 0);
        assert(result_path == NULL);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 7: Destroy function call (two arguments)
    {
        // When parsing a destroy method function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("destroy(\"calculator\", \"1.0.0\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_DESTROY);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "destroy") == 0);
        assert(arg_count == 2);
        assert(strcmp(args[0], "\"calculator\"") == 0);
        assert(strcmp(args[1], "\"1.0.0\"") == 0);
        assert(result_path == NULL);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 8: If function call
    {
        // When parsing an if function call
        parsed_instruction_t *own_parsed = ar__instruction__parse("if(5 > 3, \"yes\", \"no\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_IF);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "if") == 0);
        assert(arg_count == 3);
        assert(strcmp(args[0], "5 > 3") == 0);
        assert(strcmp(args[1], "\"yes\"") == 0);
        assert(strcmp(args[2], "\"no\"") == 0);
        assert(result_path == NULL);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Cleanup
    ar__instruction__destroy_context(mut_ctx);
    ar__data__destroy(mut_memory);
}

static void test_parse_function_calls_with_assignment(void) {
    // Given an instruction context with memory
    data_t *mut_memory = ar__data__create_map();
    assert(mut_memory != NULL);
    
    instruction_context_t *mut_ctx = ar__instruction__create_context(mut_memory, NULL, NULL);
    assert(mut_ctx != NULL);
    
    // Test 1: Send with assignment
    {
        // When parsing a send function call with assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.success := send(0, \"Hello\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_SEND);
        
        // Verify function call details with assignment
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "send") == 0);
        assert(arg_count == 2);
        assert(strcmp(args[0], "0") == 0);
        assert(strcmp(args[1], "\"Hello\"") == 0);
        assert(result_path != NULL);
        assert(strcmp(result_path, "success") == 0);  // Path without "memory."
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 2: Parse with assignment
    {
        // When parsing a parse function call with assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.parsed := parse(\"name={name}\", \"name=John\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_PARSE);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "parse") == 0);
        assert(arg_count == 2);
        assert(result_path != NULL);
        assert(strcmp(result_path, "parsed") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 3: Build with assignment
    {
        // Add a map value for the build function to reference
        data_t *own_values = ar__data__create_map();
        ar__data__set_map_string(own_values, "name", "Alice");
        ar__data__set_map_data(mut_memory, "values", own_values);
        
        // When parsing a build function call with assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.greeting := build(\"Hello {name}\", memory.values)", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_BUILD);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "build") == 0);
        assert(arg_count == 2);
        assert(result_path != NULL);
        assert(strcmp(result_path, "greeting") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 4: Method with assignment
    {
        // When parsing a method function call with assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.created := method(\"greet\", \"memory.msg := \\\"Hi\\\"\", \"1.0.0\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_METHOD);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "method") == 0);
        assert(arg_count == 3);
        assert(result_path != NULL);
        assert(strcmp(result_path, "created") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 5: Agent with assignment
    {
        // Add context for agent creation
        data_t *own_context = ar__data__create_map();
        ar__data__set_map_string(own_context, "name", "Test Agent");
        ar__data__set_map_data(mut_memory, "ctx", own_context);
        
        // When parsing an agent function call with assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.agent_id := agent(\"echo\", \"1.0.0\", memory.ctx)", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_AGENT);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "agent") == 0);
        assert(arg_count == 3);
        assert(result_path != NULL);
        assert(strcmp(result_path, "agent_id") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 6: Destroy with assignment
    {
        // When parsing a destroy function call with assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.destroyed := destroy(42)", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_DESTROY);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "destroy") == 0);
        assert(arg_count == 1);
        assert(result_path != NULL);
        assert(strcmp(result_path, "destroyed") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 7: If with assignment
    {
        // When parsing an if function call with assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.result := if(5 > 3, \"yes\", \"no\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_IF);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "if") == 0);
        assert(arg_count == 3);
        assert(result_path != NULL);
        assert(strcmp(result_path, "result") == 0);
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Test 8: Nested path assignment
    {
        // When parsing a function call with nested path assignment
        parsed_instruction_t *own_parsed = ar__instruction__parse("memory.user.status := if(1 = 1, \"active\", \"inactive\")", mut_ctx);
        
        // Then it should parse successfully
        assert(own_parsed != NULL);
        assert(ar__instruction__get_type(own_parsed) == INST_IF);
        
        const char *function_name = NULL;
        const char **args = NULL;
        int arg_count = 0;
        const char *result_path = NULL;
        
        bool is_function = ar__instruction__get_function_call(own_parsed, &function_name, &args, &arg_count, &result_path);
        assert(is_function == true);
        assert(strcmp(function_name, "if") == 0);
        assert(arg_count == 3);
        assert(result_path != NULL);
        assert(strcmp(result_path, "user.status") == 0);  // Nested path without "memory."
        
        ar__instruction__destroy_parsed(own_parsed);
    }
    
    // Cleanup
    ar__instruction__destroy_context(mut_ctx);
    ar__data__destroy(mut_memory);
}

static void test_basic_context_creation(void) {
    // Given empty data structures
    data_t *mut_memory = ar__data__create_map();
    assert(mut_memory != NULL);
    
    // When creating an instruction context
    instruction_context_t *mut_ctx = ar__instruction__create_context(mut_memory, NULL, NULL);
    
    // Then the context should be created successfully
    assert(mut_ctx != NULL);
    assert(ar__instruction__get_memory(mut_ctx) == mut_memory);
    assert(ar__instruction__get_context(mut_ctx) == NULL);
    assert(ar__instruction__get_message(mut_ctx) == NULL);
    assert(ar__instruction__get_last_error(mut_ctx) == NULL);
    assert(ar__instruction__get_error_position(mut_ctx) == 0);
    
    // Cleanup
    ar__instruction__destroy_context(mut_ctx);
    ar__data__destroy(mut_memory);
}

int main(void) {
    printf("Starting instruction parsing tests...\n");
    
    // Test functions will be called here incrementally
    test_basic_context_creation();
    test_parse_assignment_instructions();
    test_parse_function_call_instructions();
    test_parse_function_calls_with_assignment();
    
    printf("All instruction parsing tests passed!\n");
    return 0;
}
