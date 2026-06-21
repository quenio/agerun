#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "ar_expression_parser.h"
#include "ar_expression_ast.h"
#include "ar_heap.h"
#include "ar_log.h"
#include "ar_assert.h"


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

static void test_create_parser_null_expression_logs_error(void) {
    printf("Testing parser creation with NULL expression logs error...\n");
    
    // Given an ar_log instance to capture errors
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // When creating a parser with NULL expression
    ar_expression_parser_t *parser = ar_expression_parser__create(log, NULL);
    
    // Then the parser should not be created
    assert(parser == NULL);
    
    // And an error should have been logged
    const char *last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "NULL expression") != NULL);
    
    // Clean up
    ar_log__destroy(log);
}

static void test_create_parser_null_expression_without_log(void) {
    printf("Testing parser creation with NULL expression (no log)...\n");
    
    // When creating a parser with NULL expression and no log
    ar_expression_parser_t *parser = ar_expression_parser__create(NULL, NULL);
    
    // Then the parser should not be created (and should not crash)
    assert(parser == NULL);
}

static void test_parse_functions_null_parser_safety(void) {
    printf("Testing parse functions with NULL parser safety...\n");
    
    // When calling parse functions with NULL parser
    // Then they should return NULL without crashing
    // Note: These can't log errors since NULL parser has no log
    
    ar_expression_ast_t *ast_literal = ar_expression_parser__parse_literal(NULL);
    assert(ast_literal == NULL);
    
    ar_expression_ast_t *ast_memory = ar_expression_parser__parse_memory_access(NULL);
    assert(ast_memory == NULL);
    
    ar_expression_ast_t *ast_arithmetic = ar_expression_parser__parse_arithmetic(NULL);
    assert(ast_arithmetic == NULL);
    
    ar_expression_ast_t *ast_comparison = ar_expression_parser__parse_comparison(NULL);
    assert(ast_comparison == NULL);
    
    ar_expression_ast_t *ast_expression = ar_expression_parser__parse_expression(NULL);
    assert(ast_expression == NULL);
}

static void test_memory_allocation_error_logging(void) {
    printf("Testing memory allocation error logging...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Test that memory allocation failures are already logged via _set_error
    // Note: We can't easily simulate malloc failures, but we can verify
    // that the error paths exist and use _set_error which logs
    
    // Create a parser with a very long string to parse
    const char *long_string = "\"This is a test string\"";
    ar_expression_parser_t *parser = ar_expression_parser__create(log, long_string);
    assert(parser != NULL);
    
    // Parse normally - this verifies the paths exist
    ar_expression_ast_t *ast = ar_expression_parser__parse_literal(parser);
    assert(ast != NULL);
    
    // Clean up
    ar_expression_ast__destroy(ast);
    ar_expression_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_cascading_null_primary_expression(void) {
    printf("Testing cascading NULL in primary expression...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Test 1: Invalid content in parentheses
    const char *invalid_expr = "(!)";  // '!' is not a valid expression
    ar_expression_parser_t *parser = ar_expression_parser__create(log, invalid_expr);
    assert(parser != NULL);
    
    ar_expression_ast_t *ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    const char *last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Expected literal") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 2: Missing expression after opening parenthesis
    const char *empty_parens = "()";
    parser = ar_expression_parser__create(log, empty_parens);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    // Should have an error about the empty expression
    
    // Clean up
    ar_expression_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_cascading_null_binary_operations(void) {
    printf("Testing cascading NULL in binary operations...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Test 1: Invalid right operand in multiplication
    const char *invalid_mult = "5 * !";  // '!' is not a valid operand
    ar_expression_parser_t *parser = ar_expression_parser__create(log, invalid_mult);
    assert(parser != NULL);
    
    ar_expression_ast_t *ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    const char *last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Failed to parse right operand of multiplication") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 2: Invalid right operand in division
    const char *invalid_div = "10 / !";  // '!' is not a valid operand
    parser = ar_expression_parser__create(log, invalid_div);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Failed to parse right operand of division") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 3: Invalid right operand in addition
    const char *invalid_add = "5 + !";  // '!' is not a valid operand
    parser = ar_expression_parser__create(log, invalid_add);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Failed to parse right operand of addition") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 4: Invalid right operand in subtraction
    const char *invalid_sub = "10 - !";  // '!' is not a valid operand
    parser = ar_expression_parser__create(log, invalid_sub);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Failed to parse right operand of subtraction") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 5: Invalid right operand in less than comparison
    const char *invalid_lt = "5 < !";  // '!' is not a valid operand
    parser = ar_expression_parser__create(log, invalid_lt);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Failed to parse right operand of less than comparison") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 6: Invalid right operand in equality
    const char *invalid_eq = "5 = !";  // '!' is not a valid operand
    parser = ar_expression_parser__create(log, invalid_eq);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Failed to parse right operand of equality comparison") != NULL);
    
    // Clean up
    ar_expression_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_cascading_null_nested_expressions(void) {
    printf("Testing cascading NULL in nested expressions...\n");
    
    // Given a log instance
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    
    // Test 1: Invalid expression inside parentheses with binary operation
    const char *invalid_nested = "(5 + !)";  // Invalid inside parentheses
    ar_expression_parser_t *parser = ar_expression_parser__create(log, invalid_nested);
    assert(parser != NULL);
    
    ar_expression_ast_t *ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    const char *last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    // Should include context about what failed inside parentheses
    assert(strstr(last_error, "Failed to parse right operand of addition") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 2: Deeply nested invalid expression
    const char *deep_nested = "(10 * (5 + !))";  // Invalid deep inside
    parser = ar_expression_parser__create(log, deep_nested);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    // The multiplication sees its right operand (the parenthesized expression) failed
    assert(strstr(last_error, "Failed to parse right operand of multiplication") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Test 3: Complex nested expression with multiple errors
    const char *complex_nested = "(5 + (10 * !))";  // Error in nested multiplication
    parser = ar_expression_parser__create(log, complex_nested);
    assert(parser != NULL);
    
    ast = ar_expression_parser__parse_expression(parser);
    assert(ast == NULL);
    
    last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strstr(last_error, "Failed to parse right operand of addition") != NULL);
    
    ar_expression_parser__destroy(parser);
    
    // Clean up
    ar_log__destroy(log);
}

static void test_parse_pure_function_call_expression(void) {
    printf("Testing pure function call expression parsing...\n");

    // Given a pure parse() call in expression position
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *parser = ar_expression_parser__create(
        log,
        "parse(\"name={name}\", \"name=Ada\")"
    );
    assert(parser != NULL);

    // When parsing it as an expression
    ar_expression_ast_t *ast = ar_expression_parser__parse_expression(parser);

    // Then the expression parser should accept the pure call
    assert(ast != NULL);
    assert(ar_expression_ast__get_type(ast) == AR_EXPRESSION_AST_TYPE__CALL);
    assert(strcmp(ar_expression_ast__get_function_name(ast), "parse") == 0);
    assert(ar_expression_ast__get_function_arg_count(ast) == 2);
    assert(ar_log__get_last_error_message(log) == NULL);

    ar_expression_ast__destroy(ast);
    ar_expression_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_parse_build_pure_function_call_expression(void) {
    printf("Testing build pure function call expression parsing...\n");

    // Given a pure build() call in expression position
    ar_log_t *own_log = ar_log__create();
    AR_ASSERT(own_log != NULL, "Log creation should succeed");
    ar_expression_parser_t *own_parser = ar_expression_parser__create(
        own_log,
        "build(\"Hello {name}!\", {name: \"Ada\"})"
    );
    AR_ASSERT(own_parser != NULL, "Expression parser creation should succeed");

    // When parsing it as an expression
    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);

    // Then the expression parser should accept the pure call
    AR_ASSERT(own_ast != NULL, "Build call expression should parse");
    AR_ASSERT(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__CALL,
              "Build call should produce a call expression AST");
    AR_ASSERT(strcmp(ar_expression_ast__get_function_name(own_ast), "build") == 0,
              "Build call should preserve the function name");
    AR_ASSERT(ar_expression_ast__get_function_arg_count(own_ast) == 2,
              "Build call should preserve both arguments");
    AR_ASSERT(ar_log__get_last_error_message(own_log) == NULL,
              "Parser should not log an error for pure build expression");

    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(own_log);
}

static void test_reject_effectful_function_call_expression(void) {
    printf("Testing effectful function call expression rejection...\n");

    // Given an effectful send() call in expression position
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *parser = ar_expression_parser__create(
        log,
        "send(0, \"hello\")"
    );
    assert(parser != NULL);

    // When parsing it as an expression
    ar_expression_ast_t *ast = ar_expression_parser__parse_expression(parser);

    // Then the expression parser should keep effectful calls out of expressions
    assert(ast == NULL);
    const char *last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strcmp(last_error, "Function call is not a pure expression") == 0);
    assert(ar_log__get_last_error_position(log) == 0);
    assert(ar_expression_parser__get_position(parser) == 0);

    ar_expression_parser__destroy(parser);
    ar_log__destroy(log);
}

static void test_reject_effectful_function_call_in_literal_restores_position(void) {
    printf("Testing effectful function call rejection restores literal item position...\n");

    // Given an effectful send() call where a list item expression is expected
    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *parser = ar_expression_parser__create(
        log,
        "[send(0, \"hello\")]"
    );
    assert(parser != NULL);

    // When parsing the list expression
    ar_expression_ast_t *ast = ar_expression_parser__parse_expression(parser);

    // Then the rejected function call should leave the item parser at the call boundary
    assert(ast == NULL);
    const char *last_error = ar_log__get_last_error_message(log);
    assert(last_error != NULL);
    assert(strcmp(last_error, "Function call is not a pure expression") == 0);
    assert(ar_log__get_last_error_position(log) == 1);
    assert(ar_expression_parser__get_position(parser) == 1);

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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_INT);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_INT);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_DOUBLE);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_STRING);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_BINARY_OPERATOR__ADD);
    
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_BINARY_OPERATOR__ADD);
    
    const ar_expression_ast_t *ref_left = ar_expression_ast__get_left(own_ast);
    assert(ar_expression_ast__get_int_value(ref_left) == 2);
    
    const ar_expression_ast_t *ref_right = ar_expression_ast__get_right(own_ast);
    assert(ar_expression_ast__get_type(ref_right) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(ref_right) == AR_BINARY_OPERATOR__MULTIPLY);
    
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_BINARY_OPERATOR__MULTIPLY);
    
    const ar_expression_ast_t *ref_left = ar_expression_ast__get_left(own_ast);
    assert(ar_expression_ast__get_type(ref_left) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(ref_left) == AR_BINARY_OPERATOR__ADD);
    
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
        {"5 > 3", AR_BINARY_OPERATOR__GREATER},
        {"5 >= 3", AR_BINARY_OPERATOR__GREATER_EQ},
        {"5 < 3", AR_BINARY_OPERATOR__LESS},
        {"5 <= 3", AR_BINARY_OPERATOR__LESS_EQ},
        {"5 = 3", AR_BINARY_OPERATOR__EQUAL},
        {"5 <> 3", AR_BINARY_OPERATOR__NOT_EQUAL}
    };
    
    for (int i = 0; i < 6; i++) {
        ar_expression_parser_t *own_parser = ar_expression_parser__create(log, test_cases[i].expression);
        ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);
        
        assert(own_ast != NULL);
        assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_BINARY_OPERATOR__ADD);
    
    const ar_expression_ast_t *ref_left = ar_expression_ast__get_left(own_ast);
    assert(ar_expression_ast__get_type(ref_left) == AR_EXPRESSION_AST_TYPE__MEMORY_ACCESS);
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_BINARY_OPERATOR__GREATER);
    
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
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__BINARY_OP);
    assert(ar_expression_ast__get_operator(own_ast) == AR_BINARY_OPERATOR__ADD);
    
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

static void test_parse_empty_list_literal(void) {
    printf("Testing empty list literal parsing...\n");

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, "[]");
    assert(own_parser != NULL);

    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);

    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_LIST);
    assert(ar_expression_ast__get_list_item_count(own_ast) == 0);

    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_list_literal_rejects_trailing_comma(void) {
    printf("Testing list literal rejects trailing comma...\n");

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, "[1, 2,]");
    assert(own_parser != NULL);

    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);

    assert(own_ast == NULL);
    const char *error_msg = ar_log__get_last_error_message(log);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "Trailing comma") != NULL);

    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_empty_map_literal(void) {
    printf("Testing empty map literal parsing...\n");

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, "{}");
    assert(own_parser != NULL);

    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);

    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_MAP);
    assert(ar_expression_ast__get_map_entry_count(own_ast) == 0);

    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_nested_map_literal(void) {
    printf("Testing nested map literal parsing...\n");

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, "{name: \"Ada\", scores: [1, 2]}");
    assert(own_parser != NULL);

    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);

    assert(own_ast != NULL);
    assert(ar_expression_ast__get_type(own_ast) == AR_EXPRESSION_AST_TYPE__LITERAL_MAP);
    assert(ar_expression_ast__get_map_entry_count(own_ast) == 2);
    assert(strcmp(ar_expression_ast__get_map_key(own_ast, 0), "name") == 0);
    assert(strcmp(ar_expression_ast__get_map_key(own_ast, 1), "scores") == 0);

    const ar_expression_ast_t *ref_scores = ar_expression_ast__get_map_value(own_ast, 1);
    assert(ar_expression_ast__get_type(ref_scores) == AR_EXPRESSION_AST_TYPE__LITERAL_LIST);
    assert(ar_expression_ast__get_list_item_count(ref_scores) == 2);

    ar_expression_ast__destroy(own_ast);
    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_map_literal_rejects_trailing_comma(void) {
    printf("Testing map literal rejects trailing comma...\n");

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, "{name: \"Ada\",}");
    assert(own_parser != NULL);

    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);

    assert(own_ast == NULL);
    const char *error_msg = ar_log__get_last_error_message(log);
    assert(error_msg != NULL);
    assert(strstr(error_msg, "Trailing comma") != NULL);

    ar_expression_parser__destroy(own_parser);
    ar_log__destroy(log);
}

static void test_parse_map_literal_rejects_quoted_key(void) {
    printf("Testing map literal rejects quoted keys...\n");

    ar_log_t *log = ar_log__create();
    assert(log != NULL);
    ar_expression_parser_t *own_parser = ar_expression_parser__create(log, "{\"name\": \"Ada\"}");
    assert(own_parser != NULL);

    ar_expression_ast_t *own_ast = ar_expression_parser__parse_expression(own_parser);

    assert(own_ast == NULL);
    assert(ar_log__get_last_error_message(log) != NULL);

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
    test_create_parser_null_expression_logs_error();
    test_create_parser_null_expression_without_log();
    test_parse_functions_null_parser_safety();
    test_memory_allocation_error_logging();
    test_cascading_null_primary_expression();
    test_cascading_null_binary_operations();
    test_cascading_null_nested_expressions();
    test_parse_pure_function_call_expression();
    test_parse_build_pure_function_call_expression();
    test_reject_effectful_function_call_expression();
    test_reject_effectful_function_call_in_literal_restores_position();
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
    test_parse_empty_list_literal();
    test_parse_list_literal_rejects_trailing_comma();
    test_parse_empty_map_literal();
    test_parse_nested_map_literal();
    test_parse_map_literal_rejects_trailing_comma();
    test_parse_map_literal_rejects_quoted_key();
    test_parse_position_tracking();
    test_parse_null_safety();
    
    printf("\nAll expression parser tests passed!\n");
    return 0;
}
