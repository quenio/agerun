#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "agerun_assignment_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_expression_ast.h"
#include "agerun_heap.h"

static void test_assignment_instruction_parser__create_destroy(void) {
    printf("Testing assignment instruction parser create/destroy...\n");
    
    // Given the need for an assignment parser
    // When creating a parser
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    
    // Then it should create successfully
    assert(own_parser != NULL);
    
    // And it should destroy without issues
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_simple_assignment(void) {
    printf("Testing simple assignment parsing...\n");
    
    // Given an assignment instruction and a parser
    const char *instruction = "memory.x := 42";
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully as an assignment
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_ast), "memory.x") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_ast), "42") == 0);
    
    ar__instruction_ast__destroy(own_ast);
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_string_assignment(void) {
    printf("Testing string assignment parsing...\n");
    
    // Given a string assignment and a parser
    const char *instruction = "memory.greeting := \"Hello, World!\"";
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse the string correctly
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_ast), "memory.greeting") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_ast), "\"Hello, World!\"") == 0);
    
    ar__instruction_ast__destroy(own_ast);
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_nested_assignment(void) {
    printf("Testing nested assignment parsing...\n");
    
    // Given a nested path assignment and a parser
    const char *instruction = "memory.user.name := \"John\"";
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse the nested path
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_ast), "memory.user.name") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_ast), "\"John\"") == 0);
    
    ar__instruction_ast__destroy(own_ast);
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_expression_assignment(void) {
    printf("Testing expression assignment parsing...\n");
    
    // Given an expression assignment and a parser
    const char *instruction = "memory.result := 2 + 3 * 4";
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should preserve the full expression
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_ast), "2 + 3 * 4") == 0);
    
    ar__instruction_ast__destroy(own_ast);
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_whitespace_handling(void) {
    printf("Testing whitespace handling...\n");
    
    // Given an assignment with extra whitespace and a parser
    const char *instruction = "  memory.x  :=  42  ";
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should handle whitespace correctly
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_ast), "memory.x") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_ast), "42") == 0);
    
    ar__instruction_ast__destroy(own_ast);
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_error_invalid_operator(void) {
    printf("Testing error handling - invalid assignment operator...\n");
    
    // Given an invalid assignment operator and a parser
    const char *instruction = "memory.x = 42";  // Should be :=
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    // And error information should be available
    const char *error = ar_assignment_instruction_parser__get_error(own_parser);
    assert(error != NULL);
    assert(ar_assignment_instruction_parser__get_error_position(own_parser) > 0);
    
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_error_invalid_path(void) {
    printf("Testing error handling - invalid memory path...\n");
    
    // Given a path without memory prefix and a parser
    const char *instruction = "x := 42";  // Missing memory prefix
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_empty_instruction(void) {
    printf("Testing empty instruction...\n");
    
    // Given an empty instruction and a parser
    const char *instruction = "";
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the empty instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a parser instance
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing multiple instructions with the same parser
    const char *instruction1 = "memory.x := 10";
    const char *instruction2 = "memory.y := 20";
    
    instruction_ast_t *own_ast1 = ar_assignment_instruction_parser__parse(own_parser, instruction1);
    instruction_ast_t *own_ast2 = ar_assignment_instruction_parser__parse(own_parser, instruction2);
    
    // Then both should parse successfully
    assert(own_ast1 != NULL);
    assert(own_ast2 != NULL);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_ast1), "memory.x") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_ast1), "10") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_ast2), "memory.y") == 0);
    assert(strcmp(ar__instruction_ast__get_assignment_expression(own_ast2), "20") == 0);
    
    ar__instruction_ast__destroy(own_ast1);
    ar__instruction_ast__destroy(own_ast2);
    ar_assignment_instruction_parser__destroy(own_parser);
}

static void test_assignment_instruction_parser__parse_with_expression_ast(void) {
    printf("Testing assignment parsing with expression AST...\n");
    
    // Given an assignment instruction with integer literal and a parser
    const char *instruction = "memory.x := 42";
    ar_assignment_instruction_parser_t *own_parser = ar_assignment_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_assignment_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully with an expression AST
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_ASSIGNMENT);
    assert(strcmp(ar__instruction_ast__get_assignment_path(own_ast), "memory.x") == 0);
    
    // And the expression should be available as an AST
    const expression_ast_t *ref_expr_ast = ar__instruction_ast__get_assignment_expression_ast(own_ast);
    assert(ref_expr_ast != NULL);
    assert(ar__expression_ast__get_type(ref_expr_ast) == EXPR_AST_LITERAL_INT);
    assert(ar__expression_ast__get_int_value(ref_expr_ast) == 42);
    
    ar__instruction_ast__destroy(own_ast);
    ar_assignment_instruction_parser__destroy(own_parser);
}

int main(void) {
    printf("Running assignment instruction parser tests...\n\n");
    
    // Basic functionality
    test_assignment_instruction_parser__create_destroy();
    test_assignment_instruction_parser__parse_simple_assignment();
    test_assignment_instruction_parser__parse_string_assignment();
    test_assignment_instruction_parser__parse_nested_assignment();
    test_assignment_instruction_parser__parse_expression_assignment();
    
    // Edge cases
    test_assignment_instruction_parser__parse_whitespace_handling();
    test_assignment_instruction_parser__parse_error_invalid_operator();
    test_assignment_instruction_parser__parse_error_invalid_path();
    test_assignment_instruction_parser__parse_empty_instruction();
    test_assignment_instruction_parser__reusability();
    
    // Expression AST integration
    test_assignment_instruction_parser__parse_with_expression_ast();
    
    printf("\nAll assignment_instruction_parser tests passed!\n");
    return 0;
}