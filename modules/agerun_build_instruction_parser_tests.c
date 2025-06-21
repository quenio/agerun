#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "agerun_build_instruction_parser.h"
#include "agerun_instruction_ast.h"
#include "agerun_list.h"
#include "agerun_heap.h"

static void test_build_instruction_parser__create_destroy(void) {
    printf("Testing build instruction parser create/destroy...\n");
    
    // When creating a parser
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_build_instruction_parser__destroy(own_parser);
    // Then no crash occurs
}

static void test_build_instruction_parser__parse_simple(void) {
    printf("Testing simple build function parsing...\n");
    
    // Given a build function call
    const char *instruction = "build(\"Hello {name}!\", memory.data)";
    
    // When creating a parser and parsing the instruction
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully as a build function
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_BUILD);
    assert(strcmp(ar__instruction_ast__get_function_name(own_ast), "build") == 0);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == false);
    
    // Verify arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    void **own_items = ar__list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Hello {name}!\"") == 0);
    assert(strcmp((const char*)own_items[1], "memory.data") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
}

static void test_build_instruction_parser__parse_with_assignment(void) {
    printf("Testing build function with assignment...\n");
    
    // Given a build function call with assignment
    const char *instruction = "memory.greeting := build(\"Hello {name}!\", memory.values)";
    
    // When creating a parser and parsing the instruction
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, "memory.greeting");
    
    // Then it should parse successfully with assignment
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_BUILD);
    assert(ar__instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar__instruction_ast__get_function_result_path(own_ast), "memory.greeting") == 0);
    
    // Verify arguments
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    void **own_items = ar__list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Hello {name}!\"") == 0);
    assert(strcmp((const char*)own_items[1], "memory.values") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
}

static void test_build_instruction_parser__parse_multiple_placeholders(void) {
    printf("Testing build with multiple placeholders...\n");
    
    // Given a build function with multiple placeholders
    const char *instruction = "build(\"User: {firstName} {lastName}, Role: {role}\", memory.user)";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_BUILD);
    
    // Verify template with multiple placeholders
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar__list__count(own_args) == 2);
    void **own_items = ar__list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"User: {firstName} {lastName}, Role: {role}\"") == 0);
    assert(strcmp((const char*)own_items[1], "memory.user") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
}

static void test_build_instruction_parser__parse_escaped_quotes(void) {
    printf("Testing build with escaped quotes...\n");
    
    // Given a build function with escaped quotes in template
    const char *instruction = "build(\"Say \\\"Hello {name}!\\\"\", memory.data)";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_BUILD);
    
    // Verify escaped quotes are preserved
    list_t *own_args = ar__instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    void **own_items = ar__list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Say \\\"Hello {name}!\\\"\"") == 0);
    AR__HEAP__FREE(own_items);
    ar__list__destroy(own_args);
    
    ar__instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
}

static void test_build_instruction_parser__parse_whitespace_handling(void) {
    printf("Testing build with whitespace variations...\n");
    
    // Given a build function with extra whitespace
    const char *instruction = "  build  (  \"Hello {name}!\"  ,  memory.data  )  ";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully, handling whitespace
    assert(own_ast != NULL);
    assert(ar__instruction_ast__get_type(own_ast) == INST_AST_BUILD);
    
    ar__instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
}

static void test_build_instruction_parser__parse_error_wrong_function(void) {
    printf("Testing error on wrong function name...\n");
    
    // Given a non-build function
    const char *instruction = "notbuild(\"template\", memory.data)";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_build_instruction_parser__get_error(own_parser) != NULL);
    assert(ar_build_instruction_parser__get_error_position(own_parser) == 0);
    
    ar_build_instruction_parser__destroy(own_parser);
}

static void test_build_instruction_parser__parse_error_missing_parenthesis(void) {
    printf("Testing error on missing parenthesis...\n");
    
    // Given a build without opening parenthesis
    const char *instruction = "build \"template\", memory.data";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_build_instruction_parser__get_error(own_parser) != NULL);
    assert(ar_build_instruction_parser__get_error_position(own_parser) == 6);  // After "build "
    
    ar_build_instruction_parser__destroy(own_parser);
}

static void test_build_instruction_parser__parse_error_wrong_arg_count(void) {
    printf("Testing error on wrong argument count...\n");
    
    // Test with 1 argument (needs 2)
    {
        const char *instruction = "build(\"template\")";
        ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
        assert(own_parser != NULL);
        
        instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
        assert(own_ast == NULL);
        assert(ar_build_instruction_parser__get_error(own_parser) != NULL);
        
        ar_build_instruction_parser__destroy(own_parser);
    }
    
    // Test with 3 arguments (needs 2)
    {
        const char *instruction = "build(\"template\", memory.data, \"extra\")";
        ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
        assert(own_parser != NULL);
        
        instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
        assert(own_ast == NULL);
        assert(ar_build_instruction_parser__get_error(own_parser) != NULL);
        
        ar_build_instruction_parser__destroy(own_parser);
    }
}

static void test_build_instruction_parser__parser_reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a parser
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create();
    assert(own_parser != NULL);
    
    // First parse
    const char *instruction1 = "build(\"Hello {name}\", memory.data1)";
    instruction_ast_t *own_ast1 = ar_build_instruction_parser__parse(own_parser, instruction1, NULL);
    assert(own_ast1 != NULL);
    
    // Verify first parse
    list_t *own_args1 = ar__instruction_ast__get_function_args(own_ast1);
    assert(own_args1 != NULL);
    void **own_items1 = ar__list__items(own_args1);
    assert(strcmp((const char*)own_items1[1], "memory.data1") == 0);
    AR__HEAP__FREE(own_items1);
    ar__list__destroy(own_args1);
    ar__instruction_ast__destroy(own_ast1);
    
    // Second parse with same parser
    const char *instruction2 = "build(\"Goodbye {name}\", memory.data2)";
    instruction_ast_t *own_ast2 = ar_build_instruction_parser__parse(own_parser, instruction2, NULL);
    assert(own_ast2 != NULL);
    
    // Verify second parse
    list_t *own_args2 = ar__instruction_ast__get_function_args(own_ast2);
    assert(own_args2 != NULL);
    void **own_items2 = ar__list__items(own_args2);
    assert(strcmp((const char*)own_items2[0], "\"Goodbye {name}\"") == 0);
    assert(strcmp((const char*)own_items2[1], "memory.data2") == 0);
    AR__HEAP__FREE(own_items2);
    ar__list__destroy(own_args2);
    ar__instruction_ast__destroy(own_ast2);
    
    ar_build_instruction_parser__destroy(own_parser);
}

int main(void) {
    printf("Running build instruction parser tests...\n\n");
    
    test_build_instruction_parser__create_destroy();
    test_build_instruction_parser__parse_simple();
    test_build_instruction_parser__parse_with_assignment();
    test_build_instruction_parser__parse_multiple_placeholders();
    test_build_instruction_parser__parse_escaped_quotes();
    test_build_instruction_parser__parse_whitespace_handling();
    test_build_instruction_parser__parse_error_wrong_function();
    test_build_instruction_parser__parse_error_missing_parenthesis();
    test_build_instruction_parser__parse_error_wrong_arg_count();
    test_build_instruction_parser__parser_reusability();
    
    printf("\nAll build instruction parser tests passed!\n");
    return 0;
}