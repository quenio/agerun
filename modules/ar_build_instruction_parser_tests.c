#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_build_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_heap.h"
#include "ar_log.h"

static void test_create_parser_with_log(void) {
    printf("Testing parser creation with ar_log...\n");
    
    // Given an ar_log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with ar_log
    ar_build_instruction_parser_t *parser = ar_build_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_build_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_build_instruction_parser__create_destroy(void) {
    printf("Testing build instruction parser create/destroy...\n");
    
    // When creating a parser
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(NULL);
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_build_instruction_parser__destroy(own_parser);
    // Then no crash occurs
}

static void test_build_instruction_parser__parse_simple(void) {
    printf("Testing simple build function parsing...\n");
    
    // Given a build function call and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "build(\"Hello {name}!\", memory.data)";
    
    // When creating a parser and parsing the instruction
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully as a build function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    assert(strcmp(ar_instruction_ast__get_function_name(own_ast), "build") == 0);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == false);
    
    // Verify arguments
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Hello {name}!\"") == 0);
    assert(strcmp((const char*)own_items[1], "memory.data") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_build_instruction_parser__parse_with_assignment(void) {
    printf("Testing build function with assignment...\n");
    
    // Given a build function call with assignment and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.greeting := build(\"Hello {name}!\", memory.values)";
    
    // When creating a parser and parsing the instruction
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, "memory.greeting");
    
    // Then it should parse successfully with assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.greeting") == 0);
    
    // Verify arguments
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Hello {name}!\"") == 0);
    assert(strcmp((const char*)own_items[1], "memory.values") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_build_instruction_parser__parse_multiple_placeholders(void) {
    printf("Testing build with multiple placeholders...\n");
    
    // Given a build function with multiple placeholders and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "build(\"User: {firstName} {lastName}, Role: {role}\", memory.user)";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    
    // Verify template with multiple placeholders
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"User: {firstName} {lastName}, Role: {role}\"") == 0);
    assert(strcmp((const char*)own_items[1], "memory.user") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

// TODO: Fix expression parser handling of quoted strings
/*
static void test_build_instruction_parser__parse_escaped_quotes(void) {
    printf("Testing build with escaped quotes...\n");
    
    // Given a build function with escaped quotes in template
    const char *instruction = "build(\"Say \\\"Hello {name}!\\\"\", memory.data)";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    
    // Verify escaped quotes are preserved
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Say \\\"Hello {name}!\\\"\"") == 0);
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
}
*/

static void test_build_instruction_parser__parse_whitespace_handling(void) {
    printf("Testing build with whitespace variations...\n");
    
    // Given a build function with extra whitespace and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "  build  (  \"Hello {name}!\"  ,  memory.data  )  ";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully, handling whitespace
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_build_instruction_parser__parse_error_wrong_function(void) {
    printf("Testing error on wrong function name...\n");
    
    // Given a non-build function and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "notbuild(\"template\", memory.data)";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(ar_log__get_last_error_position(log) == 0);
    
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_build_instruction_parser__parse_error_missing_parenthesis(void) {
    printf("Testing error on missing parenthesis...\n");
    
    // Given a build without opening parenthesis and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "build \"template\", memory.data";
    
    // When parsing
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(ar_log__get_last_error_position(log) == 6);  // After "build "
    
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_build_instruction_parser__parse_error_wrong_arg_count(void) {
    printf("Testing error on wrong argument count...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Test with 1 argument (needs 2)
    {
        const char *instruction = "build(\"template\")";
        ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
        assert(own_parser != NULL);
        
        ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
        assert(own_ast == NULL);
        assert(ar_log__get_last_error_message(log) != NULL);
        
        ar_build_instruction_parser__destroy(own_parser);
    }
    
    // Test with 3 arguments (needs 2)
    {
        const char *instruction = "build(\"template\", memory.data, \"extra\")";
        ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
        assert(own_parser != NULL);
        
        ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
        assert(own_ast == NULL);
        assert(ar_log__get_last_error_message(log) != NULL);
        
        ar_build_instruction_parser__destroy(own_parser);
    }
    
    ar_log__destroy(log);
}

static void test_build_instruction_parser__parser_reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a log instance and a parser
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // First parse
    const char *instruction1 = "build(\"Hello {name}\", memory.data1)";
    ar_instruction_ast_t *own_ast1 = ar_build_instruction_parser__parse(own_parser, instruction1, NULL);
    assert(own_ast1 != NULL);
    
    // Verify first parse
    list_t *own_args1 = ar_instruction_ast__get_function_args(own_ast1);
    assert(own_args1 != NULL);
    void **own_items1 = ar_list__items(own_args1);
    assert(strcmp((const char*)own_items1[1], "memory.data1") == 0);
    AR__HEAP__FREE(own_items1);
    ar_list__destroy(own_args1);
    ar_instruction_ast__destroy(own_ast1);
    
    // Second parse with same parser
    const char *instruction2 = "build(\"Goodbye {name}\", memory.data2)";
    ar_instruction_ast_t *own_ast2 = ar_build_instruction_parser__parse(own_parser, instruction2, NULL);
    assert(own_ast2 != NULL);
    
    // Verify second parse
    list_t *own_args2 = ar_instruction_ast__get_function_args(own_ast2);
    assert(own_args2 != NULL);
    void **own_items2 = ar_list__items(own_args2);
    assert(strcmp((const char*)own_items2[0], "\"Goodbye {name}\"") == 0);
    assert(strcmp((const char*)own_items2[1], "memory.data2") == 0);
    AR__HEAP__FREE(own_items2);
    ar_list__destroy(own_args2);
    ar_instruction_ast__destroy(own_ast2);
    
    // Note: We can't check for NULL here as the log may contain events from both parses
    
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_build_instruction_parser__parse_with_expression_asts(void) {
    printf("Testing build instruction with expression ASTs...\n");
    
    // Given a build instruction with a string literal template and memory access for map, and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "build(\"User: {name}, Age: {age}\", memory.userdata)";
    ar_build_instruction_parser_t *own_parser = ar_build_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_build_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    
    // And the arguments should be available as expression ASTs
    const list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 2);
    
    // First argument should be a string literal AST with the template
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    const ar_expression_ast_t *ref_template = (const ar_expression_ast_t*)items[0];
    assert(ref_template != NULL);
    assert(ar_expression_ast__get_type(ref_template) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_template), "User: {name}, Age: {age}") == 0);
    
    // Second argument should be a memory access AST
    const ar_expression_ast_t *ref_map_expr = (const ar_expression_ast_t*)items[1];
    assert(ref_map_expr != NULL);
    assert(ar_expression_ast__get_type(ref_map_expr) == AR_EXPR__MEMORY_ACCESS);
    // Verify memory path
    size_t path_count = 0;
    char **path_components = ar_expression_ast__get_memory_path(ref_map_expr, &path_count);
    assert(path_components != NULL);
    assert(path_count == 1);
    assert(strcmp(path_components[0], "userdata") == 0);
    AR__HEAP__FREE(path_components);
    
    AR__HEAP__FREE(items);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_build_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

int main(void) {
    printf("Running build instruction parser tests...\n\n");
    
    // Test with ar_log
    test_create_parser_with_log();
    
    test_build_instruction_parser__create_destroy();
    test_build_instruction_parser__parse_simple();
    test_build_instruction_parser__parse_with_assignment();
    test_build_instruction_parser__parse_multiple_placeholders();
    // test_build_instruction_parser__parse_escaped_quotes(); // TODO: Fix expression parser handling of quoted strings
    test_build_instruction_parser__parse_whitespace_handling();
    test_build_instruction_parser__parse_error_wrong_function();
    test_build_instruction_parser__parse_error_missing_parenthesis();
    test_build_instruction_parser__parse_error_wrong_arg_count();
    test_build_instruction_parser__parser_reusability();
    
    // Expression AST integration
    test_build_instruction_parser__parse_with_expression_asts();
    
    printf("\nAll build instruction parser tests passed!\n");
    return 0;
}