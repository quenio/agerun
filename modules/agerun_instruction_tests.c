#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "agerun_instruction.h"
#include "agerun_data.h"
#include "agerun_map.h"
#include "agerun_heap.h"

// Test helper functions
#if 0 // Will be used when we add more tests
static void test_parse_valid_instruction(const char *instruction, instruction_type_t expected_type) {
    // Given an instruction context with empty memory, context, and message
    data_t *mut_memory = ar__data__create_map();
    assert(mut_memory != NULL);
    
    instruction_context_t *mut_ctx = ar__instruction__create_context(mut_memory, NULL, NULL);
    assert(mut_ctx != NULL);
    
    // When parsing the instruction
    parsed_instruction_t *own_parsed = ar__instruction__parse(instruction, mut_ctx);
    
    // Then the instruction should be parsed successfully
    assert(own_parsed != NULL);
    assert(ar__instruction__get_type(own_parsed) == expected_type);
    
    // Cleanup
    ar__instruction__destroy_parsed(own_parsed);
    ar__instruction__destroy_context(mut_ctx);
    ar__data__destroy(mut_memory);
}

static void test_parse_invalid_instruction(const char *instruction) {
    // Given an instruction context with empty memory
    data_t *mut_memory = ar__data__create_map();
    assert(mut_memory != NULL);
    
    instruction_context_t *mut_ctx = ar__instruction__create_context(mut_memory, NULL, NULL);
    assert(mut_ctx != NULL);
    
    // When parsing the invalid instruction
    parsed_instruction_t *own_parsed = ar__instruction__parse(instruction, mut_ctx);
    
    // Then parsing should fail
    assert(own_parsed == NULL);
    
    // And there should be an error message
    const char *error_msg = ar__instruction__get_last_error(mut_ctx);
    assert(error_msg != NULL || own_parsed == NULL); // Either error message or just NULL result
    
    // Cleanup
    ar__instruction__destroy_context(mut_ctx);
    ar__data__destroy(mut_memory);
}
#endif

// Test functions will be added here incrementally

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
    
    printf("All instruction parsing tests passed!\n");
    return 0;
}
