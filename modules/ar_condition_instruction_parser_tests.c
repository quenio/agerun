#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_condition_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_heap.h"

static void test_condition_parser__create_destroy(void) {
    printf("Testing condition parser create/destroy...\n");
    
    // Given the need to create a parser
    // When creating a condition instruction parser
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // And can be destroyed without issues
    ar_condition_instruction_parser__destroy(own_parser);
}

static void test_condition_parser__parse_simple_if(void) {
    printf("Testing simple if parsing...\n");
    
    // Given an if instruction
    const char *instruction = "if(1 > 0, \"true\", \"false\")";
    
    // When creating a parser and parsing the instruction
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__IF);
    assert(strcmp(ar_instruction_ast__get_function_name(own_ast), "if") == 0);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == false);
    
    // Verify arguments
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 3);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "1 > 0") == 0);
    assert(strcmp((const char*)own_items[1], "\"true\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"false\"") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_parser__destroy(own_parser);
}

static void test_condition_parser__parse_if_with_assignment(void) {
    printf("Testing if with assignment parsing...\n");
    
    // Given an if function call with assignment
    const char *instruction = "memory.level := if(memory.count > 5, \"High\", \"Low\")";
    
    // When creating a parser and parsing the instruction
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, "memory.level");
    
    // Then it should parse as an if function with assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__IF);
    assert(strcmp(ar_instruction_ast__get_function_name(own_ast), "if") == 0);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.level") == 0);
    
    // Verify arguments
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 3);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "memory.count > 5") == 0);
    assert(strcmp((const char*)own_items[1], "\"High\"") == 0);
    assert(strcmp((const char*)own_items[2], "\"Low\"") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_parser__destroy(own_parser);
}

// TODO: Expression parser doesn't support logical operators yet
/*
static void test_condition_parser__parse_nested_conditions(void) {
    printf("Testing nested condition parsing...\n");
    
    // Given a condition with nested expressions
    const char *instruction = "if(memory.age >= 18 && memory.registered, \"Welcome\", \"Access Denied\")";
    
    // When creating a parser and parsing the instruction
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse the complex condition
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__IF);
    
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 3);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "memory.age >= 18 && memory.registered") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_parser__destroy(own_parser);
}
*/

// TODO: Function calls in expressions not supported yet
/*
static void test_condition_parser__parse_nested_function_calls(void) {
    printf("Testing nested function calls in if arguments...\n");
    
    // Given an if with function calls in arguments
    const char *instruction = "if(send(0, \"check\"), send(1, \"true\"), send(1, \"false\"))";
    
    // When creating a parser and parsing the instruction
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse with nested function calls preserved
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__IF);
    
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 3);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "send(0, \"check\")") == 0);
    assert(strcmp((const char*)own_items[1], "send(1, \"true\")") == 0);
    assert(strcmp((const char*)own_items[2], "send(1, \"false\")") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_parser__destroy(own_parser);
}
*/

static void test_condition_parser__error_wrong_function(void) {
    printf("Testing error handling for wrong function...\n");
    
    // Given a non-if instruction
    const char *instruction = "send(0, \"hello\")";
    
    // When trying to parse as if
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_condition_instruction_parser__get_error(own_parser) != NULL);
    assert(ar_condition_instruction_parser__get_error_position(own_parser) == 0);
    
    ar_condition_instruction_parser__destroy(own_parser);
}

static void test_condition_parser__error_missing_parenthesis(void) {
    printf("Testing error handling for missing parenthesis...\n");
    
    // Given an if without opening parenthesis
    const char *instruction = "if 1 > 0, \"true\", \"false\")";
    
    // When trying to parse
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_condition_instruction_parser__get_error(own_parser) != NULL);
    
    ar_condition_instruction_parser__destroy(own_parser);
}

static void test_condition_parser__error_wrong_arg_count(void) {
    printf("Testing error handling for wrong argument count...\n");
    
    // Given an if with wrong number of arguments
    const char *instruction = "if(1 > 0, \"true\")";  // Missing else clause
    
    // When trying to parse
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_condition_instruction_parser__get_error(own_parser) != NULL);
    
    ar_condition_instruction_parser__destroy(own_parser);
}

static void test_condition_parser__reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a parser
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    // First parse
    const char *instruction1 = "if(1, 1, 0)";
    ar_instruction_ast_t *own_ast1 = ar_condition_instruction_parser__parse(own_parser, instruction1, NULL);
    assert(own_ast1 != NULL);
    ar_instruction_ast__destroy(own_ast1);
    
    // Second parse with same parser
    const char *instruction2 = "if(0, \"yes\", \"no\")";
    ar_instruction_ast_t *own_ast2 = ar_condition_instruction_parser__parse(own_parser, instruction2, NULL);
    assert(own_ast2 != NULL);
    ar_instruction_ast__destroy(own_ast2);
    
    // Error should be cleared between parses
    assert(ar_condition_instruction_parser__get_error(own_parser) == NULL);
    
    ar_condition_instruction_parser__destroy(own_parser);
}

static void test_condition_parser__parse_with_expression_asts(void) {
    printf("Testing condition parsing with expression ASTs...\n");
    
    // Given an if instruction with various expression types
    const char *instruction = "if(memory.count > 5, \"High\", memory.default)";
    ar_condition_instruction_parser_t *own_parser = ar_condition_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_condition_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__IF);
    
    // And the arguments should be available as expression ASTs
    const list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 3);
    
    // First argument should be a comparison expression AST
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    const ar_expression_ast_t *ref_condition = (const ar_expression_ast_t*)items[0];
    assert(ref_condition != NULL);
    assert(ar_expression_ast__get_type(ref_condition) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(ref_condition) == AR_OP__GREATER);
    
    // Second argument should be a string literal AST
    const ar_expression_ast_t *ref_then_expr = (const ar_expression_ast_t*)items[1];
    assert(ref_then_expr != NULL);
    assert(ar_expression_ast__get_type(ref_then_expr) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_then_expr), "High") == 0);
    
    // Third argument should be a memory access AST
    const ar_expression_ast_t *ref_else_expr = (const ar_expression_ast_t*)items[2];
    assert(ref_else_expr != NULL);
    assert(ar_expression_ast__get_type(ref_else_expr) == AR_EXPR__MEMORY_ACCESS);
    // Verify memory path
    size_t path_count = 0;
    char **path_components = ar_expression_ast__get_memory_path(ref_else_expr, &path_count);
    assert(path_components != NULL);
    assert(path_count == 1);
    assert(strcmp(path_components[0], "default") == 0);
    AR__HEAP__FREE(path_components);
    
    AR__HEAP__FREE(items);
    ar_instruction_ast__destroy(own_ast);
    ar_condition_instruction_parser__destroy(own_parser);
}

int main(void) {
    printf("Running condition instruction parser tests...\n\n");
    
    // Basic functionality
    test_condition_parser__create_destroy();
    test_condition_parser__parse_simple_if();
    test_condition_parser__parse_if_with_assignment();
    // test_condition_parser__parse_nested_conditions(); // TODO: Expression parser doesn't support logical operators yet
    // test_condition_parser__parse_nested_function_calls(); // TODO: Function calls in expressions not supported yet
    
    // Error handling
    test_condition_parser__error_wrong_function();
    test_condition_parser__error_missing_parenthesis();
    test_condition_parser__error_wrong_arg_count();
    
    // Reusability
    test_condition_parser__reusability();
    
    // Expression AST integration
    test_condition_parser__parse_with_expression_asts();
    
    printf("\nAll condition_instruction_parser tests passed!\n");
    return 0;
}