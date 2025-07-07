#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include "ar_method_ast.h"
#include "ar_heap.h"
#include "ar_instruction_ast.h"

// Constants for memory tests
#define STRESS_TEST_INSTRUCTION_COUNT 100
#define MIXED_OPS_INITIAL_COUNT 10
#define MIXED_OPS_TOTAL_COUNT 20
#define REPEATED_ACCESS_COUNT 8
#define REPEATED_ACCESS_ITERATIONS 100
#define EMPTY_AST_TEST_COUNT 10

// Test create and destroy
static void test_method_ast__create_destroy(void) {
    printf("Testing method AST create and destroy...\n");
    
    // Given the need to create a method AST
    
    // When creating a new method AST
    ar_method_ast_t *own_ast = ar_method_ast__create();
    
    // Then the AST should be created successfully
    assert(own_ast != NULL);
    
    // And it should be destroyable without issues
    ar_method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__create_destroy passed\n");
}

// Test destroy with NULL
static void test_method_ast__destroy_null(void) {
    printf("Testing method AST destroy with NULL...\n");
    
    // Given a NULL pointer
    
    // When destroying NULL
    ar_method_ast__destroy(NULL);
    
    // Then no crash should occur
    
    printf("✓ test_method_ast__destroy_null passed\n");
}

// Test add instruction
static void test_method_ast__add_instruction(void) {
    printf("Testing method AST add instruction...\n");
    
    // Given a method AST and a manually created instruction AST
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    ar_instruction_ast_t *own_instruction = ar_instruction_ast__create_assignment("memory.x", "42");
    assert(own_instruction != NULL);
    
    // When adding the instruction to the AST
    ar_method_ast__add_instruction(own_ast, own_instruction);
    
    // Then the instruction should be added successfully
    // (will verify with get_instruction_count in next test)
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__add_instruction passed\n");
}

// Test add instruction with NULL AST
static void test_method_ast__add_instruction_null_ast(void) {
    printf("Testing method AST add instruction with NULL AST...\n");
    
    // Given a parsed instruction but NULL AST
    ar_instruction_ast_t *own_instruction = ar_instruction_ast__create_assignment("memory.x", "42");
    assert(own_instruction != NULL);
    
    // When adding the instruction to NULL AST
    ar_method_ast__add_instruction(NULL, own_instruction);
    
    // Then no crash should occur (instruction should be cleaned up internally)
    
    printf("✓ test_method_ast__add_instruction_null_ast passed\n");
}

// Test get instruction count
static void test_method_ast__get_instruction_count(void) {
    printf("Testing method AST get instruction count...\n");
    
    // Given a method AST
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    // When checking count on empty AST
    size_t count = ar_method_ast__get_instruction_count(own_ast);
    
    // Then it should be 0
    assert(count == 0);
    
    // When adding an instruction
    ar_instruction_ast_t *own_instruction1 = ar_instruction_ast__create_assignment("memory.x", "42");
    ar_method_ast__add_instruction(own_ast, own_instruction1);
    
    // Then count should be 1
    count = ar_method_ast__get_instruction_count(own_ast);
    assert(count == 1);
    
    // When adding another instruction
    ar_instruction_ast_t *own_instruction2 = ar_instruction_ast__create_assignment("memory.y", "100");
    ar_method_ast__add_instruction(own_ast, own_instruction2);
    
    // Then count should be 2
    count = ar_method_ast__get_instruction_count(own_ast);
    assert(count == 2);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__get_instruction_count passed\n");
}

// Test get instruction count with NULL
static void test_method_ast__get_instruction_count_null(void) {
    printf("Testing method AST get instruction count with NULL...\n");
    
    // When checking count on NULL AST
    size_t count = ar_method_ast__get_instruction_count(NULL);
    
    // Then it should be 0
    assert(count == 0);
    
    printf("✓ test_method_ast__get_instruction_count_null passed\n");
}

// Test get instruction by line number
static void test_method_ast__get_instruction_by_line(void) {
    printf("Testing method AST get instruction by line number...\n");
    
    // Given a method AST with multiple instructions
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    // Add three instructions
    ar_instruction_ast_t *own_instruction1 = ar_instruction_ast__create_assignment("memory.x", "10");
    ar_method_ast__add_instruction(own_ast, own_instruction1);
    
    ar_instruction_ast_t *own_instruction2 = ar_instruction_ast__create_assignment("memory.y", "20");
    ar_method_ast__add_instruction(own_ast, own_instruction2);
    
    ar_instruction_ast_t *own_instruction3 = ar_instruction_ast__create_assignment("memory.z", "30");
    ar_method_ast__add_instruction(own_ast, own_instruction3);
    
    // When getting instruction at line 1
    const ar_instruction_ast_t *ref_instr1 = ar_method_ast__get_instruction(own_ast, 1);
    
    // Then it should return the first instruction
    assert(ref_instr1 != NULL);
    assert(ar_instruction_ast__get_type(ref_instr1) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    // When getting instruction at line 2
    const ar_instruction_ast_t *ref_instr2 = ar_method_ast__get_instruction(own_ast, 2);
    
    // Then it should return the second instruction
    assert(ref_instr2 != NULL);
    assert(ar_instruction_ast__get_type(ref_instr2) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    // When getting instruction at line 3
    const ar_instruction_ast_t *ref_instr3 = ar_method_ast__get_instruction(own_ast, 3);
    
    // Then it should return the third instruction
    assert(ref_instr3 != NULL);
    assert(ar_instruction_ast__get_type(ref_instr3) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    
    // When getting instruction at line 0 (invalid)
    const ar_instruction_ast_t *ref_instr0 = ar_method_ast__get_instruction(own_ast, 0);
    
    // Then it should return NULL
    assert(ref_instr0 == NULL);
    
    // When getting instruction beyond count
    const ar_instruction_ast_t *ref_instr4 = ar_method_ast__get_instruction(own_ast, 4);
    
    // Then it should return NULL
    assert(ref_instr4 == NULL);
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__get_instruction_by_line passed\n");
}

// Test get instruction with NULL AST
static void test_method_ast__get_instruction_null(void) {
    printf("Testing method AST get instruction with NULL...\n");
    
    // When getting instruction from NULL AST
    const ar_instruction_ast_t *ref_instr = ar_method_ast__get_instruction(NULL, 1);
    
    // Then it should return NULL
    assert(ref_instr == NULL);
    
    printf("✓ test_method_ast__get_instruction_null passed\n");
}

// Test memory management with many instructions
static void test_method_ast__memory_stress_test(void) {
    printf("Testing method AST memory stress test...\n");
    
    // Given a method AST
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    // When adding many instructions to test memory management
    for (int i = 0; i < STRESS_TEST_INSTRUCTION_COUNT; i++) {
        char memory_path[256];
        char expression[256];
        snprintf(memory_path, sizeof(memory_path), "memory.var%d", i);
        snprintf(expression, sizeof(expression), "%d", i * 10);
        
        ar_instruction_ast_t *own_instruction = ar_instruction_ast__create_assignment(memory_path, expression);
        assert(own_instruction != NULL);
        ar_method_ast__add_instruction(own_ast, own_instruction);
    }
    
    // Then the count should match
    assert(ar_method_ast__get_instruction_count(own_ast) == STRESS_TEST_INSTRUCTION_COUNT);
    
    // And we can access all instructions without memory leaks
    for (size_t i = 1; i <= STRESS_TEST_INSTRUCTION_COUNT; i++) {
        const ar_instruction_ast_t *ref_instruction = ar_method_ast__get_instruction(own_ast, i);
        assert(ref_instruction != NULL);
        assert(ar_instruction_ast__get_type(ref_instruction) == AR_INSTRUCTION_AST_TYPE__ASSIGNMENT);
    }
    
    // Cleanup - this should free all instructions without leaks
    ar_method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__memory_stress_test passed\n");
}

// Test memory with mixed operations
static void test_method_ast__memory_mixed_operations(void) {
    printf("Testing method AST memory with mixed operations...\n");
    
    // Given a method AST with some instructions
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    // Add initial set of instructions
    for (int i = 0; i < MIXED_OPS_INITIAL_COUNT; i++) {
        char memory_path[256];
        char expression[256];
        snprintf(memory_path, sizeof(memory_path), "memory.x%d", i);
        snprintf(expression, sizeof(expression), "memory.y + %d", i);
        
        ar_instruction_ast_t *own_instruction = ar_instruction_ast__create_assignment(memory_path, expression);
        ar_method_ast__add_instruction(own_ast, own_instruction);
    }
    
    // Access instructions multiple times (testing for leaks in get_instruction)
    for (int j = 0; j < 5; j++) {
        for (size_t i = 1; i <= MIXED_OPS_INITIAL_COUNT; i++) {
            const ar_instruction_ast_t *ref_instruction = ar_method_ast__get_instruction(own_ast, i);
            assert(ref_instruction != NULL);
        }
    }
    
    // Add more instructions to test mixed scenarios
    for (int i = MIXED_OPS_INITIAL_COUNT; i < MIXED_OPS_TOTAL_COUNT; i++) {
        char memory_path[256];
        char expression[256];
        snprintf(memory_path, sizeof(memory_path), "memory.msg%d", i);
        snprintf(expression, sizeof(expression), "\"message%d\"", i);
        
        ar_instruction_ast_t *own_instruction = ar_instruction_ast__create_assignment(memory_path, expression);
        ar_method_ast__add_instruction(own_ast, own_instruction);
    }
    
    // Then we should have the expected total
    assert(ar_method_ast__get_instruction_count(own_ast) == MIXED_OPS_TOTAL_COUNT);
    
    // Access again to ensure no leaks
    for (size_t i = 1; i <= MIXED_OPS_TOTAL_COUNT; i++) {
        const ar_instruction_ast_t *ref_instruction = ar_method_ast__get_instruction(own_ast, i);
        assert(ref_instruction != NULL);
    }
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__memory_mixed_operations passed\n");
}

// Test repeated access patterns
static void test_method_ast__memory_repeated_access(void) {
    printf("Testing method AST memory with repeated access...\n");
    
    // Given a method AST with simple assignment instructions
    ar_method_ast_t *own_ast = ar_method_ast__create();
    assert(own_ast != NULL);
    
    // Add a fixed number of instructions for repeated access testing
    size_t num_instructions = REPEATED_ACCESS_COUNT;
    for (size_t i = 0; i < num_instructions; i++) {
        char memory_path[256];
        char expression[256];
        snprintf(memory_path, sizeof(memory_path), "memory.var%zu", i);
        snprintf(expression, sizeof(expression), "%zu", i * 100);
        
        ar_instruction_ast_t *own_instruction = ar_instruction_ast__create_assignment(memory_path, expression);
        assert(own_instruction != NULL);
        ar_method_ast__add_instruction(own_ast, own_instruction);
    }
    
    // Repeatedly access the same instructions (tests for memory leaks in get_instruction)
    for (int repeat = 0; repeat < REPEATED_ACCESS_ITERATIONS; repeat++) {
        // Access first instruction many times
        const ar_instruction_ast_t *ref_first = ar_method_ast__get_instruction(own_ast, 1);
        assert(ref_first != NULL);
        
        // Access last instruction many times
        const ar_instruction_ast_t *ref_last = ar_method_ast__get_instruction(own_ast, num_instructions);
        assert(ref_last != NULL);
        
        // Access middle instruction
        const ar_instruction_ast_t *ref_middle = ar_method_ast__get_instruction(own_ast, num_instructions / 2);
        assert(ref_middle != NULL);
    }
    
    // Cleanup
    ar_method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__memory_repeated_access passed\n");
}

// Test empty AST destruction
static void test_method_ast__memory_empty_destruction(void) {
    printf("Testing method AST memory with empty AST...\n");
    
    // Given multiple empty ASTs to test cleanup without content
    for (int i = 0; i < EMPTY_AST_TEST_COUNT; i++) {
        ar_method_ast_t *own_ast = ar_method_ast__create();
        assert(own_ast != NULL);
        
        // Access on empty AST shouldn't leak
        assert(ar_method_ast__get_instruction_count(own_ast) == 0);
        assert(ar_method_ast__get_instruction(own_ast, 1) == NULL);
        
        // Destroy empty AST
        ar_method_ast__destroy(own_ast);
    }
    
    printf("✓ test_method_ast__memory_empty_destruction passed\n");
}

int main(void) {
    printf("Running method AST tests...\n\n");
    
    test_method_ast__create_destroy();
    test_method_ast__destroy_null();
    test_method_ast__add_instruction();
    test_method_ast__add_instruction_null_ast();
    test_method_ast__get_instruction_count();
    test_method_ast__get_instruction_count_null();
    test_method_ast__get_instruction_by_line();
    test_method_ast__get_instruction_null();
    test_method_ast__memory_stress_test();
    test_method_ast__memory_mixed_operations();
    test_method_ast__memory_repeated_access();
    test_method_ast__memory_empty_destruction();
    
    printf("\nAll method AST tests passed!\n");
    return 0;
}