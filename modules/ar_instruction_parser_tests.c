#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ar_instruction_parser.h"
#include "ar_instruction_ast.h"
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
    instruction_parser_t *parser = ar_instruction_parser__create(log);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_instruction_parser__create_destroy_with_parsers(void) {
    printf("Testing instruction parser creation with all specialized parsers...\n");
    
    // When creating an instruction parser
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    
    // Then it should successfully create with all specialized parsers initialized
    assert(own_parser != NULL);
    
    // And when destroying it
    ar_instruction_parser__destroy(own_parser);
    
    // Then it should clean up without memory leaks (verified by heap tracking)
}

static void test_instruction_parser__parse_assignment(void) {
    printf("Testing unified parse method for assignments...\n");
    
    // Given an assignment instruction and a parser with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "memory.x := 42";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully as an assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__ASSIGNMENT);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_ast), "memory.x") == 0);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_ast), "42") == 0);
    
    // And no errors should be logged
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_instruction_parser__parse_send(void) {
    printf("Testing unified parse method for send instruction...\n");
    
    // Given a send instruction
    const char *instruction = "send(1, \"Hello\")";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully as a send
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__SEND);
    
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    void **items = ar_list__items(own_args);
    assert(strcmp((const char*)items[0], "1") == 0);
    assert(strcmp((const char*)items[1], "\"Hello\"") == 0);
    AR__HEAP__FREE(items);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_send_with_assignment(void) {
    printf("Testing unified parse method for send with assignment...\n");
    
    // Given a send instruction with assignment
    const char *instruction = "memory.result := send(1, \"Hello\")";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully as a send instruction with result assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__SEND);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    assert(strcmp(ar_instruction_ast__get_function_result_path(own_ast), "memory.result") == 0);
    
    // Verify arguments
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(own_args != NULL);
    assert(ar_list__count(own_args) == 2);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_if(void) {
    printf("Testing unified parse method for if instruction...\n");
    
    // Given an if instruction
    const char *instruction = "if(memory.x > 0, 1, 0)";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully as an if
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__IF);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_parse(void) {
    printf("Testing unified parse method for parse instruction...\n");
    
    // Given a parse instruction
    const char *instruction = "parse(\"{x}\", \"x=42\")";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__PARSE);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_build(void) {
    printf("Testing unified parse method for build instruction...\n");
    
    // Given a build instruction
    const char *instruction = "build(\"{x}\", memory.vars)";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_method(void) {
    printf("Testing unified parse method for method instruction...\n");
    
    // Given a method instruction
    const char *instruction = "method(\"test\", \"send(1, message)\", \"1.0.0\")";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_agent(void) {
    printf("Testing unified parse method for agent instruction...\n");
    
    // Given an agent instruction
    const char *instruction = "agent(\"echo\", \"1.0.0\")";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__AGENT);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_destroy_agent(void) {
    printf("Testing unified parse method for destroy agent instruction...\n");
    
    // Given a destroy agent instruction
    const char *instruction = "destroy(1)";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__DESTROY_AGENT);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_destroy_method(void) {
    printf("Testing unified parse method for destroy method instruction...\n");
    
    // Given a destroy method instruction
    const char *instruction = "destroy(\"echo\", \"1.0.0\")";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__DESTROY_METHOD);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_unknown(void) {
    printf("Testing unified parse method for unknown instruction...\n");
    
    // Given an unknown instruction and a parser with log
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    const char *instruction = "unknown()";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(log);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should fail
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    assert(strstr(ar_log__get_last_error_message(log), "Unknown") != NULL);
    
    ar_instruction_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_instruction_parser__parse_empty(void) {
    printf("Testing unified parse method for empty instruction...\n");
    
    // Given an empty instruction
    const char *instruction = "";
    
    // When creating a parser and parsing via unified method
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should fail
    assert(own_ast == NULL);
    
    ar_instruction_parser__destroy(own_parser);
}

static void test_instruction_parser__parse_reusability(void) {
    printf("Testing unified parser reusability...\n");
    
    // Given a parser instance
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    // When parsing multiple instructions
    const char *inst1 = "memory.x := 42";
    ar_instruction_ast_t *own_ast1 = ar_instruction_parser__parse(own_parser, inst1);
    assert(own_ast1 != NULL);
    ar_instruction_ast__destroy(own_ast1);
    
    const char *inst2 = "send(1, \"Hello\")";
    ar_instruction_ast_t *own_ast2 = ar_instruction_parser__parse(own_parser, inst2);
    assert(own_ast2 != NULL);
    ar_instruction_ast__destroy(own_ast2);
    
    // Then both should succeed
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_simple_assignment(void) {
    printf("Testing simple assignment parsing...\n");
    
    // Given an assignment instruction
    const char *instruction = "memory.x := 42";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse successfully as an assignment
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__ASSIGNMENT);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_ast), "memory.x") == 0);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_ast), "42") == 0);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_string_assignment(void) {
    printf("Testing string assignment parsing...\n");
    
    // Given a string assignment
    const char *instruction = "memory.greeting := \"Hello, World!\"";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse the string correctly
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__ASSIGNMENT);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_ast), "memory.greeting") == 0);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_ast), "\"Hello, World!\"") == 0);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_nested_assignment(void) {
    printf("Testing nested assignment parsing...\n");
    
    // Given a nested path assignment
    const char *instruction = "memory.user.name := \"John\"";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse the nested path
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__ASSIGNMENT);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_ast), "memory.user.name") == 0);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_ast), "\"John\"") == 0);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_expression_assignment(void) {
    printf("Testing expression assignment parsing...\n");
    
    // Given an expression assignment
    const char *instruction = "memory.result := 2 + 3 * 4";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should preserve the full expression
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__ASSIGNMENT);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_ast), "2 + 3 * 4") == 0);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

// Send tests moved to agerun_send_instruction_parser_tests.c

static void test_parse_if_function(void) {
    printf("Testing if function parsing...\n");
    
    // Given an if function call
    const char *instruction = "memory.level := if(memory.count > 5, \"High\", \"Low\")";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse as an if function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__IF);
    assert(strcmp(ar_instruction_ast__get_function_name(own_ast), "if") == 0);
    
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
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_method_function(void) {
    printf("Testing method function parsing...\n");
    
    // Given a method function call (without escaped quotes that expression parser can't handle)
    const char *instruction = "method(\"greet\", \"memory.msg := 42\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse as a method function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__METHOD);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_agent_function(void) {
    printf("Testing agent function parsing...\n");
    
    // Given an agent function call
    const char *instruction = "memory.agent_id := agent(\"echo\", \"1.0.0\", memory.context)";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse as an agent function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__AGENT);
    assert(ar_instruction_ast__has_result_assignment(own_ast) == true);
    
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 3);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_destroy_one_arg(void) {
    printf("Testing destroy with one argument...\n");
    
    // Given a destroy function call with one argument
    const char *instruction = "destroy(memory.agent_id)";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse as a destroy agent function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__DESTROY_AGENT);
    
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 1);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_destroy_two_args(void) {
    printf("Testing destroy with two arguments...\n");
    
    // Given a destroy function call with two arguments
    const char *instruction = "destroy(\"calculator\", \"1.0.0\")";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse as a destroy method function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__DESTROY_METHOD);
    
    list_t *own_args = ar_instruction_ast__get_function_args(own_ast);
    assert(ar_list__count(own_args) == 2);
    ar_list__destroy(own_args);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_parse_function(void) {
    printf("Testing parse function parsing...\n");
    
    // Given a parse function call
    const char *instruction = "memory.parsed := parse(\"name={name}\", \"name=John\")";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse as a parse function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__PARSE);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_build_function(void) {
    printf("Testing build function parsing...\n");
    
    // Given a build function call
    const char *instruction = "memory.greeting := build(\"Hello {name}!\", memory.data)";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should parse as a build function
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__BUILD);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_whitespace_handling(void) {
    printf("Testing whitespace handling...\n");
    
    // Given an instruction with extra whitespace
    const char *instruction = "  memory.x  :=  42  ";
    
    // When creating a parser and parsing the instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should handle whitespace correctly
    assert(own_ast != NULL);
    assert(ar_instruction_ast__get_type(own_ast) == AR_INST__ASSIGNMENT);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_ast), "memory.x") == 0);
    assert(strcmp(ar_instruction_ast__get_assignment_expression(own_ast), "42") == 0);
    
    ar_instruction_ast__destroy(own_ast);
    ar_instruction_parser__destroy(own_parser);
}

static void test_parse_error_handling(void) {
    printf("Testing error handling...\n");
    
    // Test 1: Invalid assignment operator
    {
        ar_log_t *log = ar_log__create();
        assert(log != NULL);
        const char *instruction = "memory.x = 42";  // Should be :=
        instruction_parser_t *own_parser = ar_instruction_parser__create(log);
        assert(own_parser != NULL);
        
        ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
        assert(own_ast == NULL);
        
        assert(ar_log__get_last_error_message(log) != NULL);
        assert(ar_log__get_last_error_position(log) > 0);
        
        ar_instruction_parser__destroy(own_parser);
        ar_log__destroy(log);
    }
    
    // Test 2: Invalid memory path
    {
        ar_log_t *log = ar_log__create();
        assert(log != NULL);
        const char *instruction = "x := 42";  // Missing memory prefix
        instruction_parser_t *own_parser = ar_instruction_parser__create(log);
        assert(own_parser != NULL);
        
        ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
        assert(own_ast == NULL);
        
        ar_instruction_parser__destroy(own_parser);
        ar_log__destroy(log);
    }
    
    // Test 3: Send error tests moved to agerun_send_instruction_parser_tests.c
}

static void test_parse_empty_instruction(void) {
    printf("Testing empty instruction...\n");
    
    // Given an empty instruction
    const char *instruction = "";
    
    // When creating a parser and trying to parse empty instruction
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    ar_instruction_ast_t *own_ast = ar_instruction_parser__parse(own_parser, instruction);
    
    // Then it should return NULL
    assert(own_ast == NULL);
    
    ar_instruction_parser__destroy(own_parser);
}

static void test_parser_reusability(void) {
    printf("Testing parser reusability...\n");
    
    // Given a parser
    instruction_parser_t *own_parser = ar_instruction_parser__create(NULL);
    assert(own_parser != NULL);
    
    // First parse
    const char *instruction1 = "memory.x := 42";
    ar_instruction_ast_t *own_ast1 = ar_instruction_parser__parse(own_parser, instruction1);
    assert(own_ast1 != NULL);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_ast1), "memory.x") == 0);
    ar_instruction_ast__destroy(own_ast1);
    
    // Second parse with same parser
    const char *instruction2 = "memory.y := 100";
    ar_instruction_ast_t *own_ast2 = ar_instruction_parser__parse(own_parser, instruction2);
    assert(own_ast2 != NULL);
    assert(strcmp(ar_instruction_ast__get_assignment_path(own_ast2), "memory.y") == 0);
    ar_instruction_ast__destroy(own_ast2);
    
    ar_instruction_parser__destroy(own_parser);
}

int main(void) {
    printf("Running instruction parser tests...\n\n");
    
    // Test with ar_log
    test_create_parser_with_log();
    
    // Parser lifecycle tests
    test_instruction_parser__create_destroy_with_parsers();
    
    // Unified parser tests
    test_instruction_parser__parse_assignment();
    test_instruction_parser__parse_send();
    test_instruction_parser__parse_send_with_assignment();
    test_instruction_parser__parse_if();
    test_instruction_parser__parse_parse();
    test_instruction_parser__parse_build();
    test_instruction_parser__parse_method();
    test_instruction_parser__parse_agent();
    test_instruction_parser__parse_destroy_agent();
    test_instruction_parser__parse_destroy_method();
    test_instruction_parser__parse_unknown();
    test_instruction_parser__parse_empty();
    test_instruction_parser__parse_reusability();
    
    // Assignment tests
    test_parse_simple_assignment();
    test_parse_string_assignment();
    test_parse_nested_assignment();
    test_parse_expression_assignment();
    
    // Function call tests
    // Send tests moved to agerun_send_instruction_parser_tests.c
    test_parse_if_function();
    test_parse_method_function();
    test_parse_agent_function();
    test_parse_destroy_one_arg();
    test_parse_destroy_two_args();
    test_parse_parse_function();
    test_parse_build_function();
    
    // Edge cases
    test_parse_whitespace_handling();
    test_parse_error_handling();
    test_parse_empty_instruction();
    test_parser_reusability();
    
    printf("\nAll instruction_parser tests passed!\n");
    return 0;
}
