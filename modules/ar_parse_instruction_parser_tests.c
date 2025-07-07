#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "ar_parse_instruction_parser.h"
#include "ar_instruction_ast.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_event.h"

static void test_create_parser_with_log(void) {
    printf("Testing parser creation with ar_log...\n");
    
    // Given an ar_log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with ar_log
    ar_parse_instruction_parser_t *parser = ar_parse_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_parse_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_parse_instruction_parser__create_destroy(void) {
    printf("Testing parse instruction parser create/destroy...\n");
    
    // Given the need for a parser
    // When creating a parser
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(NULL);
    
    // Then it should be created successfully
    assert(own_parser != NULL);
    
    // When destroying the parser
    ar_parse_instruction_parser__destroy(own_parser);
    
    // Then it should be destroyed without issues (memory leak detection will verify)
}

static void test_parse_instruction_parser__parse_simple(void) {
    printf("Testing simple parse function parsing...\n");
    
    // Given a simple parse function call and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "parse(\"name={name}\", \"name=John\")";
    
    // When parsing the instruction
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__PARSE);
    assert(strcmp(ar_instruction_ast__get_function_name(own_ast), "parse") == 0);
    
    // And it should have exactly 2 arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    
    // And the arguments should be correct
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"name={name}\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"name=John\"") == 0);
    AR__HEAP__FREE(own_items);
    
    ar_list__destroy(own_args);
    
    // And there should be no result path
    assert(ar_instruction_ast__get_function_result_path(own_ast) == NULL);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_instruction_parser__parse_with_assignment(void) {
    printf("Testing parse function with assignment...\n");
    
    // Given a parse function call with assignment and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.parsed := parse(\"name={name}\", \"name=John\")";
    
    // When parsing the instruction
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, "memory.parsed");
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__PARSE);
    
    // And it should have the result path
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.parsed") == 0);
    
    // And it should have the correct arguments
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"name={name}\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"name=John\"") == 0);
    AR__HEAP__FREE(own_items);
    
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_instruction_parser__parse_complex_template(void) {
    printf("Testing parse function with complex template...\n");
    
    // Given a parse function with multiple placeholders and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "parse(\"Hello {name}, you are {age} years old\", \"Hello Alice, you are 30 years old\")";
    
    // When parsing the instruction
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__PARSE);
    
    // And preserve the template with multiple placeholders
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Hello {name}, you are {age} years old\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"Hello Alice, you are 30 years old\"") == 0);
    AR__HEAP__FREE(own_items);
    
    ar_list__destroy(own_args);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

// TODO: Fix expression parser handling of quoted strings
/*
static void test_parse_instruction_parser__parse_with_escaped_quotes(void) {
    printf("Testing parse function with escaped quotes...\n");
    
    // Given a parse function with escaped quotes in strings
    const char *instruction = "parse(\"Say \\\"Hello {name}!\\\"\", \"Say \\\"Hello World!\\\"\")";
    
    // When parsing the instruction
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully
    if (own_ast == NULL) {
        fprintf(stderr, "DEBUG: Parse failed. Error: %s at position %zu\n", 
                ar_parse_instruction_parser__get_error(own_parser),
                ar_parse_instruction_parser__get_error_position(own_parser));
    }
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__PARSE);
    
    // And preserve the escaped quotes
    ar_list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    
    void **own_items = ar_list__items(own_args);
    assert(strcmp((const char*)own_items[0], "\"Say \\\"Hello {name}!\\\"\"") == 0);
    assert(strcmp((const char*)own_items[1], "\"Say \\\"Hello World!\\\"\"") == 0);
    AR__HEAP__FREE(own_items);
    
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_parse_instruction_parser__destroy(own_parser);
}
*/

static void test_parse_instruction_parser__error_wrong_function(void) {
    printf("Testing error on wrong function name...\n");
    
    // Given a function call that's not parse and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "build(\"template\", \"input\")";
    
    // When parsing the instruction
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    // And provide an error
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Expected 'parse' function") != NULL);
    
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_instruction_parser__error_wrong_arg_count(void) {
    printf("Testing error on wrong argument count...\n");
    
    // Given a parse function with wrong number of arguments and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "parse(\"template\")";  // Missing second argument
    
    // When parsing the instruction
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    // And provide an error about argument parsing
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Failed to parse parse arguments") != NULL);
    
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_instruction_parser__error_missing_parenthesis(void) {
    printf("Testing error on missing parenthesis...\n");
    
    // Given a parse function without opening parenthesis and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "parse \"template\", \"input\"";
    
    // When parsing the instruction
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    // And provide an error about missing parenthesis
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Expected '(' after 'parse'") != NULL);
    
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_instruction_parser__reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a parser instance and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing multiple instructions
    const char *instruction1 = "parse(\"name={n}\", \"name=John\")";
    ar_instruction_ast_t *own_ast1 = ar_parse_instruction_parser__parse(own_parser, instruction1, NULL);
    assert(own_ast1 != NULL);
    assert(ar_instruction_ast__get_type(own_ast1) == AR_INSTRUCTION_AST_TYPE__PARSE);
    assert(ar_log__get_last_error_message(log) == NULL);
    
    const char *instruction2 = "memory.data := parse(\"age={a}\", \"age=30\")";
    ar_instruction_ast_t *own_ast2 = ar_parse_instruction_parser__parse(own_parser, instruction2, "memory.data");
    assert(own_ast2 != NULL);
    assert(ar_instruction_ast__get_type(own_ast2) == AR_INSTRUCTION_AST_TYPE__PARSE);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast2), "memory.data") == 0);
    
    // Parse an invalid instruction
    const char *instruction3 = "invalid(\"test\")";
    ar_instruction_ast_t *own_ast3 = ar_parse_instruction_parser__parse(own_parser, instruction3, NULL);
    assert(own_ast3 == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    
    // NOTE: With shared log, errors persist across parse attempts
    // Then the parser should still work for valid instructions
    const char *instruction4 = "parse(\"x={val}\", \"x=42\")";
    ar_instruction_ast_t *own_ast4 = ar_parse_instruction_parser__parse(own_parser, instruction4, NULL);
    assert(own_ast4 != NULL);
    assert(ar_instruction_ast__get_type(own_ast4) == AR_INSTRUCTION_AST_TYPE__PARSE);
    
    // Clean up
    ar_instruction_ast__destroy(own_ast1);
    ar_instruction_ast__destroy(own_ast2);
    ar_instruction_ast__destroy(own_ast4);
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_instruction_parser__parse_with_expression_asts(void) {
    printf("Testing parse instruction with expression ASTs...\n");
    
    // Given a parse instruction with string literal arguments and a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "parse(\"User: {name}, Age: {age}\", \"User: Alice, Age: 25\")";
    ar_parse_instruction_parser_t *own_parser = ar_parse_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    // When parsing the instruction
    ar_instruction_ast_t *own_ast = ar_parse_instruction_parser__parse(own_parser, instruction, NULL);
    
    // Then it should parse successfully with argument ASTs
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INSTRUCTION_AST_TYPE__PARSE);
    
    // And the arguments should be available as expression ASTs
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(own_ast);
    assert(ref_arg_asts != NULL);
    assert(ar_list__count(ref_arg_asts) == 2);
    
    // First argument should be a string literal AST with the template
    void **items = ar_list__items(ref_arg_asts);
    assert(items != NULL);
    const ar_expression_ast_t *ref_template = (const ar_expression_ast_t*)items[0];
    assert(ref_template != NULL);
    assert(ar_expression_ast__get_type(ref_template) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_template), "User: {name}, Age: {age}") == 0);
    
    // Second argument should be a string literal AST with the input
    const ar_expression_ast_t *ref_input = (const ar_expression_ast_t*)items[1];
    assert(ref_input != NULL);
    assert(ar_expression_ast__get_type(ref_input) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(ref_input), "User: Alice, Age: 25") == 0);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    AR__HEAP__FREE(items);
    ar_instruction_ast__destroy(own_ast);
    ar_parse_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

int main(void) {
    printf("Running parse instruction parser tests...\n\n");
    
    // Test with ar_log
    test_create_parser_with_log();
    
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        size_t len = strlen(cwd);
        if (len < 4 || strcmp(cwd + len - 4, "/bin") != 0) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please cd to the bin directory and run again.\n");
            return 1;
        }
    }
    
    test_parse_instruction_parser__create_destroy();
    test_parse_instruction_parser__parse_simple();
    test_parse_instruction_parser__parse_with_assignment();
    test_parse_instruction_parser__parse_complex_template();
    // test_parse_instruction_parser__parse_with_escaped_quotes(); // TODO: Fix expression parser handling of quoted strings
    test_parse_instruction_parser__error_wrong_function();
    test_parse_instruction_parser__error_wrong_arg_count();
    test_parse_instruction_parser__error_missing_parenthesis();
    test_parse_instruction_parser__reusability();
    
    // Expression AST integration
    test_parse_instruction_parser__parse_with_expression_asts();
    
    printf("\nAll parse instruction parser tests passed!\n");
    return 0;
}