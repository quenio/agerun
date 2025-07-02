#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "ar_expression_parser.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_log.h"


static void test_create_parser_with_log(void) {
    printf("Testing parser creation with ar_log...\n");
    
    // Given an ar_log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with ar_log
    const char *expression = "42";
    ar_expression_parser_t *parser = ar_expression_parser__create(log, expression);
    
    // Then the parser should be created successfully
    assert(parser != NULL);
    
    // Clean up
    ar_expression_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_parse_integer_literal(void) {
    printf("Testing integer literal parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with an integer literal
    const char *expression = "42";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully as an integer literal
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_INT);
    assert(ar_expression_ast__get_int_value(own_ast) == 42);
    assert(ar_log__get_last_error_message(log) == NULL);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_negative_integer(void) {
    printf("Testing negative integer parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with a negative integer
    const char *expression = "-123";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_INT);
    assert(ar_expression_ast__get_int_value(own_ast) == -123);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_double_literal(void) {
    printf("Testing double literal parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with a double literal
    const char *expression = "3.14159";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully as a double literal
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_DOUBLE);
    assert(fabs(ar_expression_ast__get_double_value(own_ast) - 3.14159) < 0.00001);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_string_literal(void) {
    printf("Testing string literal parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with a string literal
    const char *expression = "\"hello world\"";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully as a string literal
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(own_ast), "hello world") == 0);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_empty_string(void) {
    printf("Testing empty string parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with an empty string
    const char *expression = "\"\"";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar_expression_ast__get_string_value(own_ast), "") == 0);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_unterminated_string(void) {
    printf("Testing unterminated string error...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with an unterminated string
    const char *expression = "\"hello";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should fail with an error
    assert(own_ast == NULL);
    const char *error_msg = ar_log__get_last_error_message(log);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "Unterminated string") != NULL);
    
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_simple_memory_access(void) {
    printf("Testing simple memory access parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with memory access
    const char *expression = "memory";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar_expression_ast__get_memory_base(own_ast), "memory") == 0);
    
    size_t path_count = 0;
    char **own_path = ar_expression_ast__get_memory_path(own_ast, &path_count);
    assert(path_count == 0);
    assert(own_path == NULL);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_memory_access_with_path(void) {
    printf("Testing memory access with path parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with memory access and path
    const char *expression = "message.user.name";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully with the correct path
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar_expression_ast__get_memory_base(own_ast), "message") == 0);
    
    size_t path_count = 0;
    char **own_path = ar_expression_ast__get_memory_path(own_ast, &path_count);
    assert(path_count == 2);
    assert(own_path != NULL);
    assert(strcmp(own_path[0], "user") == 0);
    assert(strcmp(own_path[1], "name") == 0);
    AR__HEAP__FREE(own_path);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_context_access(void) {
    printf("Testing context access parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with context access
    const char *expression = "context.request_id";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar_expression_ast__get_memory_base(own_ast), "context") == 0);
    
    size_t path_count = 0;
    char **own_path = ar_expression_ast__get_memory_path(own_ast, &path_count);
    assert(path_count == 1);
    assert(strcmp(own_path[0], "request_id") == 0);
    AR__HEAP__FREE(own_path);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_simple_addition(void) {
    printf("Testing simple addition parsing...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with addition
    const char *expression = "2 + 3";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    const ar_expression_ast_t *ref_left = ar_expression_ast__get_left(own_ast);
    const ar_expression_ast_t *ref_right = ar_expression_ast__get_right(own_ast);
    assert(ar_expression_ast__get_int_value(ref_left) == 2);
    assert(ar_expression_ast__get_int_value(ref_right) == 3);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_arithmetic_precedence(void) {
    printf("Testing arithmetic precedence...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with mixed operators
    const char *expression = "2 + 3 * 4";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then multiplication should have higher precedence
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    const ar_expression_ast_t *ref_left = ar_expression_ast__get_left(own_ast);
    assert(ar_expression_ast__get_int_value(ref_left) == 2);
    
    const ar_expression_ast_t *ref_right = ar_expression_ast__get_right(own_ast);
    assert(ar_expression_ast__get_type(ref_right) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(ref_right) == AR_OP__MULTIPLY);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_parenthesized_expression(void) {
    printf("Testing parenthesized expression...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with parentheses
    const char *expression = "(2 + 3) * 4";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Check for error
    if (own_ast == NULL) {
        const char *error_msg = ar_log__get_last_error_message(log);
        printf("Parse error: %s\n", error_msg ? error_msg : "Unknown error");
        ar_expression_parser__destroy(own_parser);
        ar_log__destroy(log);
        assert(false && "Failed to parse parenthesized expression");
    }
    
    // Then parentheses should override precedence
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_OP__MULTIPLY);
    
    const ar_expression_ast_t *ref_left = ar_expression_ast__get_left(own_ast);
    assert(ar_expression_ast__get_type(ref_left) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(ref_left) == AR_OP__ADD);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_comparison_operators(void) {
    printf("Testing comparison operators...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Test all comparison operators
    struct {
        const char *expression;
        ar_binary_operator_t expected_op;
    } test_cases[] = {
        {"5 > 3", AR_OP__GREATER},
        {"5 >= 3", AR_OP__GREATER_EQ},
        {"5 < 3", AR_OP__LESS},
        {"5 <= 3", AR_OP__LESS_EQ},
        {"5 = 3", AR_OP__EQUAL},
        {"5 <> 3", AR_OP__NOT_EQUAL}
    };
    
    for (int i = 0; i < 6; i++) {
        ar_expression_parser_t *own_parser = ar_expression_parser__create(log, test_cases[i].expression);
        ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
        
        assert(own_ast != NULL);
        assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
        assert(ar_expression_ast__get_operator(own_ast) == test_cases[i].expected_op);
        
        ar_expression_ast__destroy(own_ast);
        ar_expression_parser__destroy(own_parser);
    }
    ar_log__destroy(log);
}

static void test_parse_memory_in_arithmetic(void) {
    printf("Testing memory access in arithmetic...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with memory access in arithmetic
    const char *expression = "memory.x + 5";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse correctly
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    const ar_expression_ast_t *ref_left = ar_expression_ast__get_left(own_ast);
    assert(ar_expression_ast__get_type(ref_left) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar_expression_ast__get_memory_base(ref_left), "memory") == 0);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_complex_expression(void) {
    printf("Testing complex expression...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a complex expression
    const char *expression = "(memory.count + 1) * 2 > message.limit";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse the entire structure correctly
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_OP__GREATER);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_whitespace_handling(void) {
    printf("Testing whitespace handling...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with various whitespace
    const char *expression = "  42   +   \t\n  8  ";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should parse correctly ignoring whitespace
    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_invalid_expression(void) {
    printf("Testing invalid expression error...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an invalid expression
    const char *expression = "2 + + 3";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should fail with an error
    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);
    
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_trailing_characters(void) {
    printf("Testing trailing characters error...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given an expression with trailing characters
    const char *expression = "42 abc";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
    
    // Then it should fail with an error
    assert(own_ast == NULL);
    const char *error_msg = ar_log__get_last_error_message(log);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "Unexpected characters") != NULL);
    
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_position_tracking(void) {
    printf("Testing position tracking...\n");
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Given a parser
    const char *expression = "42 + 3";
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, expression);
    
    // When checking initial position
    assert(ar_expression_parser__get_position(own_parser) == 0);
    
    // When parsing a literal
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_literal(own_parser);
    assert(own_ast != NULL);
    assert(ar_expression_parser__get_position(own_parser) == 2); // After "42"
    
    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_null_safety(void) {
    printf("Testing NULL safety...\n");
    
    // Test NULL expression
    ar_expression_parser_t *own_parser = ar_expression_parser__create(NULL, NULL);
    assert(own_parser == NULL);
    
    // Test NULL parser parameter
    assert(ar_expression_parser__parse_expression(NULL) == NULL);
    assert(ar_expression_parser__get_position(NULL) == 0);
    // Note: get_error test removed - will be removed from API
    
    // Test destroy with NULL
    ar_expression_parser__destroy(NULL);
}

int main(void) {
    printf("Running expression parser tests...\n\n");
    
    test_create_parser_with_log();
    test_parse_integer_literal();
    test_parse_negative_integer();
    test_parse_double_literal();
    test_parse_string_literal();
    test_parse_empty_string();
    test_parse_unterminated_string();
    test_parse_simple_memory_access();
    test_parse_memory_access_with_path();
    test_parse_context_access();
    test_parse_simple_addition();
    test_parse_arithmetic_precedence();
    test_parse_parenthesized_expression();
    test_parse_comparison_operators();
    test_parse_memory_in_arithmetic();
    test_parse_complex_expression();
    test_parse_whitespace_handling();
    test_parse_invalid_expression();
    test_parse_trailing_characters();
    test_parse_position_tracking();
    test_parse_null_safety();
    
    printf("\nAll expression parser tests passed!\n");
    return 0;
}
