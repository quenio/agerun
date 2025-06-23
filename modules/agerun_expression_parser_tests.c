#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_heap.h"

static void test_parse_integer_literal(void) {
    printf("Testing integer literal parsing...\n");
    
    // Given an expression with an integer literal
    const char *expression = "42";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully as an integer literal
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_INT);
    assert(ar__expression_ast__get_int_value(own_ast) == 42);
    assert(ar__expression_parser__get_error(own_parser) == NULL);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_negative_integer(void) {
    printf("Testing negative integer parsing...\n");
    
    // Given an expression with a negative integer
    const char *expression = "-123";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_INT);
    assert(ar__expression_ast__get_int_value(own_ast) == -123);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_double_literal(void) {
    printf("Testing double literal parsing...\n");
    
    // Given an expression with a double literal
    const char *expression = "3.14159";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully as a double literal
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_DOUBLE);
    assert(fabs(ar__expression_ast__get_double_value(own_ast) - 3.14159) < 0.00001);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_string_literal(void) {
    printf("Testing string literal parsing...\n");
    
    // Given an expression with a string literal
    const char *expression = "\"hello world\"";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully as a string literal
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(own_ast), "hello world") == 0);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_empty_string(void) {
    printf("Testing empty string parsing...\n");
    
    // Given an expression with an empty string
    const char *expression = "\"\"";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__LITERAL_STRING);
    assert(strcmp(ar__expression_ast__get_string_value(own_ast), "") == 0);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_unterminated_string(void) {
    printf("Testing unterminated string error...\n");
    
    // Given an expression with an unterminated string
    const char *expression = "\"hello";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should fail with an error
    assert(own_ast == NULL);
    assert(ar__expression_parser__get_error(own_parser) != NULL);
    assert(strstr(ar__expression_parser__get_error(own_parser), "Unterminated string") != NULL);
    
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_simple_memory_access(void) {
    printf("Testing simple memory access parsing...\n");
    
    // Given an expression with memory access
    const char *expression = "memory";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar__expression_ast__get_memory_base(own_ast), "memory") == 0);
    
    size_t path_count = 0;
    char **own_path = ar__expression_ast__get_memory_path(own_ast, &path_count);
    assert(path_count == 0);
    assert(own_path == NULL);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_memory_access_with_path(void) {
    printf("Testing memory access with path parsing...\n");
    
    // Given an expression with memory access and path
    const char *expression = "message.user.name";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully with the correct path
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar__expression_ast__get_memory_base(own_ast), "message") == 0);
    
    size_t path_count = 0;
    char **own_path = ar__expression_ast__get_memory_path(own_ast, &path_count);
    assert(path_count == 2);
    assert(own_path != NULL);
    assert(strcmp(own_path[0], "user") == 0);
    assert(strcmp(own_path[1], "name") == 0);
    AR__HEAP__FREE(own_path);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_context_access(void) {
    printf("Testing context access parsing...\n");
    
    // Given an expression with context access
    const char *expression = "context.request_id";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar__expression_ast__get_memory_base(own_ast), "context") == 0);
    
    size_t path_count = 0;
    char **own_path = ar__expression_ast__get_memory_path(own_ast, &path_count);
    assert(path_count == 1);
    assert(strcmp(own_path[0], "request_id") == 0);
    AR__HEAP__FREE(own_path);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_simple_addition(void) {
    printf("Testing simple addition parsing...\n");
    
    // Given an expression with addition
    const char *expression = "2 + 3";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse successfully
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_ast);
    const ar_expression_ast_t *ref_right = ar__expression_ast__get_right(own_ast);
    assert(ar__expression_ast__get_int_value(ref_left) == 2);
    assert(ar__expression_ast__get_int_value(ref_right) == 3);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_arithmetic_precedence(void) {
    printf("Testing arithmetic precedence...\n");
    
    // Given an expression with mixed operators
    const char *expression = "2 + 3 * 4";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then multiplication should have higher precedence
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_ast);
    assert(ar__expression_ast__get_int_value(ref_left) == 2);
    
    const ar_expression_ast_t *ref_right = ar__expression_ast__get_right(own_ast);
    assert(ar__expression_ast__get_type(ref_right) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(ref_right) == AR_OP__MULTIPLY);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_parenthesized_expression(void) {
    printf("Testing parenthesized expression...\n");
    
    // Given an expression with parentheses
    const char *expression = "(2 + 3) * 4";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Check for error
    if (own_ast == NULL) {
        printf("Parse error: %s\n", ar__expression_parser__get_error(own_parser));
        ar__expression_parser__destroy(own_parser);
        assert(false && "Failed to parse parenthesized expression");
    }
    
    // Then parentheses should override precedence
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_ast) == AR_OP__MULTIPLY);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_ast);
    assert(ar__expression_ast__get_type(ref_left) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(ref_left) == AR_OP__ADD);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_comparison_operators(void) {
    printf("Testing comparison operators...\n");
    
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
        ar_expression_parser_t *own_parser = ar__expression_parser__create(test_cases[i].expression);
        ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
        
        assert(own_ast != NULL);
        assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
        assert(ar__expression_ast__get_operator(own_ast) == test_cases[i].expected_op);
        
        ar__expression_ast__destroy(own_ast);
        ar__expression_parser__destroy(own_parser);
    }
}

static void test_parse_memory_in_arithmetic(void) {
    printf("Testing memory access in arithmetic...\n");
    
    // Given an expression with memory access in arithmetic
    const char *expression = "memory.x + 5";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse correctly
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    const ar_expression_ast_t *ref_left = ar__expression_ast__get_left(own_ast);
    assert(ar__expression_ast__get_type(ref_left) == AR_EXPR__MEMORY_ACCESS);
    assert(strcmp(ar__expression_ast__get_memory_base(ref_left), "memory") == 0);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_complex_expression(void) {
    printf("Testing complex expression...\n");
    
    // Given a complex expression
    const char *expression = "(memory.count + 1) * 2 > message.limit";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse the entire structure correctly
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_ast) == AR_OP__GREATER);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_whitespace_handling(void) {
    printf("Testing whitespace handling...\n");
    
    // Given an expression with various whitespace
    const char *expression = "  42   +   \t\n  8  ";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should parse correctly ignoring whitespace
    assert(own_ast != NULL);
    assert(ar__expression_ast__get_type(own_ast) == AR_EXPR__BINARY_OP);
    assert(ar__expression_ast__get_operator(own_ast) == AR_OP__ADD);
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_invalid_expression(void) {
    printf("Testing invalid expression error...\n");
    
    // Given an invalid expression
    const char *expression = "2 + + 3";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should fail with an error
    assert(own_ast == NULL);
    assert(ar__expression_parser__get_error(own_parser) != NULL);
    
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_trailing_characters(void) {
    printf("Testing trailing characters error...\n");
    
    // Given an expression with trailing characters
    const char *expression = "42 abc";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When parsing the expression
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_expression(own_parser);
    
    // Then it should fail with an error
    assert(own_ast == NULL);
    assert(ar__expression_parser__get_error(own_parser) != NULL);
    assert(strstr(ar__expression_parser__get_error(own_parser), "Unexpected characters") != NULL);
    
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_position_tracking(void) {
    printf("Testing position tracking...\n");
    
    // Given a parser
    const char *expression = "42 + 3";
    ar_expression_parser_t *own_parser = ar__expression_parser__create(expression);
    
    // When checking initial position
    assert(ar__expression_parser__get_position(own_parser) == 0);
    
    // When parsing a literal
    ar_expression_ast_t *own_ast = ar__expression_parser__parse_literal(own_parser);
    assert(own_ast != NULL);
    assert(ar__expression_parser__get_position(own_parser) == 2); // After "42"
    
    ar__expression_ast__destroy(own_ast);
    ar__expression_parser__destroy(own_parser);
}

static void test_parse_null_safety(void) {
    printf("Testing NULL safety...\n");
    
    // Test NULL expression
    ar_expression_parser_t *own_parser = ar__expression_parser__create(NULL);
    assert(own_parser == NULL);
    
    // Test NULL parser parameter
    assert(ar__expression_parser__parse_expression(NULL) == NULL);
    assert(ar__expression_parser__get_position(NULL) == 0);
    assert(ar__expression_parser__get_error(NULL) == NULL);
    
    // Test destroy with NULL
    ar__expression_parser__destroy(NULL);
}

int main(void) {
    printf("Running expression parser tests...\n\n");
    
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
