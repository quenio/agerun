#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "agerun_send_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_expression_ast.h"
#include "agerun_list.h"
#include "agerun_heap.h"

static void test_send_instruction_parser__create_destroy(void) {
    printf("Testing send instruction parser create/destroy...\n");
    
    // Given the need for a send parser
    // When creating a parser
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    
    // Then it should create successfully
    assert(own_parser != NULL);
    
    // And it should destroy without issues
    ar_send_instruction_parser__destroy(own_parser);
}

static void test_send_instruction_parser__parse_simple_send(void) {
    printf("Testing simple send parsing...\n");
    
    // Given a send function call and a parser
    const char *instruction = "send(0, \"Hello\")";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse as a send function
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_SEND);
    assert(strcmp(ar__instruction_ast__get_function_name(own_ast), "send") == 0);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == false);
    
    // Verify arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    void **own_items = ar__list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "0") == 0);
    assert(strcmp((const char*)own_items[1], "\"Hello\"") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
}

static void test_send_instruction_parser__parse_send_with_assignment(void) {
    printf("Testing send with assignment parsing...\n");
    
    // Given a send with result assignment and a parser
    const char *instruction = "send(1, \"Test\")";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing with a result path
    instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, "memory.result");
    
    // Then it should parse with result assignment
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_SEND);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar__instruction_ast__get_function_result_path(own_ast), "memory.result") == 0);
    
    ar__instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
}

static void test_send_instruction_parser__parse_send_with_expression_args(void) {
    printf("Testing send with expression arguments...\n");
    
    // Given a send with complex arguments and a parser
    const char *instruction = "send(memory.agent_id, \"Count: \" + memory.count)";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse the expressions as arguments
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_SEND);
    
    // Verify arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    void **own_items = ar__list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "memory.agent_id") == 0);
    assert(strcmp((const char*)own_items[1], "\"Count: \" + memory.count") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
}

static void test_send_instruction_parser__parse_error_missing_args(void) {
    printf("Testing error handling - missing arguments...\n");
    
    // Given a send with missing arguments and a parser
    const char *instruction = "send()";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    // And error information should be available
    const char *error = ar_send_instruction_parser__get_error(own_parser);
    assert(error != NULL);
    
    ar_send_instruction_parser__destroy(own_parser);
}

static void test_send_instruction_parser__parse_error_invalid_syntax(void) {
    printf("Testing error handling - invalid syntax...\n");
    
    // Given invalid send syntax and a parser
    const char *instruction = "send(1,)";  // Missing second argument
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    ar_send_instruction_parser__destroy(own_parser);
}

// TODO: Function calls in expressions not supported yet
/*
static void test_send_instruction_parser__parse_nested_parentheses(void) {
    printf("Testing send with nested parentheses...\n");
    
    // Given a send with nested function calls in arguments and a parser
    const char *instruction = "send(0, build(\"Hello {0}\", memory.name))";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should handle nested parentheses correctly
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_SEND);
    
    // Verify arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    void **own_items = ar__list__items(own_args);
    assert(own_items != NULL);
    assert(strcmp((const char*)own_items[0], "0") == 0);
    assert(strcmp((const char*)own_items[1], "build(\"Hello {0}\", memory.name)") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
}
*/

static void test_send_instruction_parser__reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a parser instance
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing multiple instructions with the same parser
    const char *instruction1 = "send(0, \"First\")";
    const char *instruction2 = "send(1, \"Second\")";
    
    instruction_ast_t *own_ast1 = ar_send_instruction_parser__parse(own_parser, instruction1, NULL);
    instruction_ast_t *own_ast2 = ar_send_instruction_parser__parse(own_parser, instruction2, NULL);
    
    // Then both should parse successfully
    assert(own_ast1 != NULL);
    assert(own_ast2 != NULL);
    
    // Verify first instruction
    list_t *own_args1 = ar__instruction_ast__get_function_args(own_ast1);
    assert(own_args1 != NULL);
    void **own_items1 = ar__list__items(own_args1);
    assert(strcmp((const char*)own_items1[0], "0") == 0);
    assert(strcmp((const char*)own_items1[1], "\"First\"") == 0);
    AR__HEAP__FREE(own_items1);
    ar__list__destroy(own_args1);
    
    // Verify second instruction
    list_t *own_args2 = ar__instruction_ast__get_function_args(own_ast2);
    assert(own_args2 != NULL);
    void **own_items2 = ar__list__items(own_args2);
    assert(strcmp((const char*)own_items2[0], "1") == 0);
    assert(strcmp((const char*)own_items2[1], "\"Second\"") == 0);
    AR__HEAP__FREE(own_items2);
    ar__list__destroy(own_args2);
    
    ar__instruction_ast__destroy(own_ast1);
    ar__instruction_ast__destroy(own_ast2);
    ar_send_instruction_parser__destroy(own_parser);
}

static void test_send_instruction_parser__parse_with_expression_asts(void) {
    printf("Testing send parsing with expression ASTs...\n");
    
    // Given a send function call with integer and string arguments
    const char *instruction = "send(42, \"Hello World\")";
    ar_send_instruction_parser_t *own_parser = ar_send_instruction_parser__create();
    assert(own_parser != NULL);
    
    // When parsing the instruction
    instruction_ast_t *own_ast = ar_send_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_SEND);
    
    // And the arguments should be available as expression ASTs
    const list_t *ref_arg_asts = ar__instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar__list__count(ref_arg_asts) == 2);
    
    // First argument should be an integer literal AST
    void **items = ar__list__items(ref_arg_asts);
    assert(items != NULL);
    const expression_ast_t *ref_first_arg = (const expression_ast_t*)items[0];
    assert(ref_first_arg != NULL);
    assert(ar__expression_ast__get_type(ref_first_arg) == EXPR_AST_LITERAL_INT);
    assert(ar__expression_ast__get_int_value(ref_first_arg) == 42);
    
    // Second argument should be a string literal AST
    const expression_ast_t *ref_second_arg = (const expression_ast_t*)items[1];
    assert(ref_second_arg != NULL);
    assert(ar__expression_ast__get_type(ref_second_arg) == EXPR_AST_LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(ref_second_arg), "Hello World") == 0);
    
    AR__HEAP__FREE(items);
    ar__instruction_ast__destroy(own_ast);
    ar_send_instruction_parser__destroy(own_parser);
}

int main(void) {
    printf("Running send instruction parser tests...\n\n");
    
    // Basic functionality
    test_send_instruction_parser__create_destroy();
    test_send_instruction_parser__parse_simple_send();
    test_send_instruction_parser__parse_send_with_assignment();
    test_send_instruction_parser__parse_send_with_expression_args();
    
    // Edge cases and error handling
    test_send_instruction_parser__parse_error_missing_args();
    test_send_instruction_parser__parse_error_invalid_syntax();
    // test_send_instruction_parser__parse_nested_parentheses(); // TODO: Function calls in expressions not supported
    test_send_instruction_parser__reusability();
    
    // Expression AST integration
    test_send_instruction_parser__parse_with_expression_asts();
    
    printf("\nAll send_instruction_parser tests passed!\n");
    return 0;
}