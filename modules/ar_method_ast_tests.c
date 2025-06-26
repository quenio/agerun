#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include "ar_method_ast.h"
#include "ar_heap.h"
#include "ar_instruction_ast.h"
#include "ar_instruction_parser.h"

// Test create and destroy
static void test_method_ast__create_destroy(void) {
    printf("Testing method AST create and destroy...\n");
    
    // Given the need to create a method AST
    
    // When creating a new method AST
    ar_method_ast_t *own_ast = ar__method_ast__create();
    
    // Then the AST should be created successfully
    assert(own_ast != NULL);
    
    // And it should be destroyable without issues
    ar__method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__create_destroy passed\n");
}

// Test destroy with NULL
static void test_method_ast__destroy_null(void) {
    printf("Testing method AST destroy with NULL...\n");
    
    // Given a NULL pointer
    
    // When destroying NULL
    ar__method_ast__destroy(NULL);
    
    // Then no crash should occur
    
    printf("✓ test_method_ast__destroy_null passed\n");
}

// Test add instruction
static void test_method_ast__add_instruction(void) {
    printf("Testing method AST add instruction...\n");
    
    // Given a method AST and a parsed instruction
    ar_method_ast_t *own_ast = ar__method_ast__create();
    assert(own_ast != NULL);
    
    instruction_parser_t *own_parser = ar__instruction_parser__create();
    assert(own_parser != NULL);
    
    const char *instruction = "memory.x := 42";
    ar_instruction_ast_t *own_instruction = ar_instruction_parser__parse(own_parser, instruction);
    assert(own_instruction != NULL);
    
    // When adding the instruction to the AST
    ar__method_ast__add_instruction(own_ast, own_instruction);
    
    // Then the instruction should be added successfully
    // (will verify with get_instruction_count in next test)
    
    // Cleanup
    ar__instruction_parser__destroy(own_parser);
    ar__method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__add_instruction passed\n");
}

// Test add instruction with NULL AST
static void test_method_ast__add_instruction_null_ast(void) {
    printf("Testing method AST add instruction with NULL AST...\n");
    
    // Given a parsed instruction but NULL AST
    instruction_parser_t *own_parser = ar__instruction_parser__create();
    assert(own_parser != NULL);
    
    const char *instruction = "memory.x := 42";
    ar_instruction_ast_t *own_instruction = ar_instruction_parser__parse(own_parser, instruction);
    assert(own_instruction != NULL);
    
    // When adding the instruction to NULL AST
    ar__method_ast__add_instruction(NULL, own_instruction);
    
    // Then no crash should occur (instruction should be cleaned up internally)
    
    // Cleanup
    ar__instruction_parser__destroy(own_parser);
    
    printf("✓ test_method_ast__add_instruction_null_ast passed\n");
}

// Test get instruction count
static void test_method_ast__get_instruction_count(void) {
    printf("Testing method AST get instruction count...\n");
    
    // Given a method AST
    ar_method_ast_t *own_ast = ar__method_ast__create();
    assert(own_ast != NULL);
    
    // When checking count on empty AST
    size_t count = ar__method_ast__get_instruction_count(own_ast);
    
    // Then it should be 0
    assert(count == 0);
    
    // When adding an instruction
    instruction_parser_t *own_parser = ar__instruction_parser__create();
    const char *instruction1 = "memory.x := 42";
    ar_instruction_ast_t *own_instruction1 = ar_instruction_parser__parse(own_parser, instruction1);
    ar__method_ast__add_instruction(own_ast, own_instruction1);
    
    // Then count should be 1
    count = ar__method_ast__get_instruction_count(own_ast);
    assert(count == 1);
    
    // When adding another instruction
    const char *instruction2 = "memory.y := 100";
    ar_instruction_ast_t *own_instruction2 = ar_instruction_parser__parse(own_parser, instruction2);
    ar__method_ast__add_instruction(own_ast, own_instruction2);
    
    // Then count should be 2
    count = ar__method_ast__get_instruction_count(own_ast);
    assert(count == 2);
    
    // Cleanup
    ar__instruction_parser__destroy(own_parser);
    ar__method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__get_instruction_count passed\n");
}

// Test get instruction count with NULL
static void test_method_ast__get_instruction_count_null(void) {
    printf("Testing method AST get instruction count with NULL...\n");
    
    // When checking count on NULL AST
    size_t count = ar__method_ast__get_instruction_count(NULL);
    
    // Then it should be 0
    assert(count == 0);
    
    printf("✓ test_method_ast__get_instruction_count_null passed\n");
}

// Test get instruction by line number
static void test_method_ast__get_instruction_by_line(void) {
    printf("Testing method AST get instruction by line number...\n");
    
    // Given a method AST with multiple instructions
    ar_method_ast_t *own_ast = ar__method_ast__create();
    assert(own_ast != NULL);
    
    instruction_parser_t *own_parser = ar__instruction_parser__create();
    
    // Add three instructions
    const char *instruction1 = "memory.x := 10";
    ar_instruction_ast_t *own_instruction1 = ar_instruction_parser__parse(own_parser, instruction1);
    ar__method_ast__add_instruction(own_ast, own_instruction1);
    
    const char *instruction2 = "memory.y := 20";
    ar_instruction_ast_t *own_instruction2 = ar_instruction_parser__parse(own_parser, instruction2);
    ar__method_ast__add_instruction(own_ast, own_instruction2);
    
    const char *instruction3 = "memory.z := 30";
    ar_instruction_ast_t *own_instruction3 = ar_instruction_parser__parse(own_parser, instruction3);
    ar__method_ast__add_instruction(own_ast, own_instruction3);
    
    // When getting instruction at line 1
    const ar_instruction_ast_t *ref_instr1 = ar__method_ast__get_instruction(own_ast, 1);
    
    // Then it should return the first instruction
    assert(ref_instr1 != NULL);
    assert(ar__instruction_ast__get_type(ref_instr1) == AR_INST__ASSIGNMENT);
    
    // When getting instruction at line 2
    const ar_instruction_ast_t *ref_instr2 = ar__method_ast__get_instruction(own_ast, 2);
    
    // Then it should return the second instruction
    assert(ref_instr2 != NULL);
    assert(ar__instruction_ast__get_type(ref_instr2) == AR_INST__ASSIGNMENT);
    
    // When getting instruction at line 3
    const ar_instruction_ast_t *ref_instr3 = ar__method_ast__get_instruction(own_ast, 3);
    
    // Then it should return the third instruction
    assert(ref_instr3 != NULL);
    assert(ar__instruction_ast__get_type(ref_instr3) == AR_INST__ASSIGNMENT);
    
    // When getting instruction at line 0 (invalid)
    const ar_instruction_ast_t *ref_instr0 = ar__method_ast__get_instruction(own_ast, 0);
    
    // Then it should return NULL
    assert(ref_instr0 == NULL);
    
    // When getting instruction beyond count
    const ar_instruction_ast_t *ref_instr4 = ar__method_ast__get_instruction(own_ast, 4);
    
    // Then it should return NULL
    assert(ref_instr4 == NULL);
    
    // Cleanup
    ar__instruction_parser__destroy(own_parser);
    ar__method_ast__destroy(own_ast);
    
    printf("✓ test_method_ast__get_instruction_by_line passed\n");
}

// Test get instruction with NULL AST
static void test_method_ast__get_instruction_null(void) {
    printf("Testing method AST get instruction with NULL...\n");
    
    // When getting instruction from NULL AST
    const ar_instruction_ast_t *ref_instr = ar__method_ast__get_instruction(NULL, 1);
    
    // Then it should return NULL
    assert(ref_instr == NULL);
    
    printf("✓ test_method_ast__get_instruction_null passed\n");
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
    
    printf("\nAll method AST tests passed!\n");
    return 0;
}