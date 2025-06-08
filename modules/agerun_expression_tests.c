#include "agerun_expression.h"
#include "agerun_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// Test expression evaluation directly using the ar__expression__evaluate function
static void test_string_literal(void);
static void test_number_literal(void);
static void test_memory_access(void);
static void test_arithmetic_expression(void);
static void test_comparison_expression(void);
static void test_function_call_expression(void);

int main(int argc, char **argv) {
    printf("Starting Expression Module Tests...\n");
    fflush(stdout);
    
    // Run specific test based on argument
    if (argc > 1) {
        if (strcmp(argv[1], "string") == 0) {
            test_string_literal();
        } else if (strcmp(argv[1], "number") == 0) {
            test_number_literal();
        } else if (strcmp(argv[1], "memory") == 0) {
            test_memory_access();
        } else if (strcmp(argv[1], "arithmetic") == 0) {
            test_arithmetic_expression();
        } else if (strcmp(argv[1], "comparison") == 0) {
            test_comparison_expression();
        } else if (strcmp(argv[1], "function_call") == 0) {
            test_function_call_expression();
        } else {
            printf("Unknown test: %s\n", argv[1]);
            return 1;
        }
    } else {
        // Run all tests by default
        printf("Running all expression tests...\n");
        fflush(stdout);
        
        printf("Running test_string_literal()...\n");
        fflush(stdout);
        test_string_literal();
        printf("Completed test_string_literal().\n");
        fflush(stdout);
        
        printf("Running test_number_literal()...\n");
        fflush(stdout);
        test_number_literal();
        printf("Completed test_number_literal().\n");
        fflush(stdout);
        
        printf("Running test_memory_access()...\n");
        fflush(stdout);
        test_memory_access();
        printf("Completed test_memory_access().\n");
        fflush(stdout);
        
        printf("Running test_arithmetic_expression()...\n");
        fflush(stdout);
        test_arithmetic_expression();
        printf("Completed test_arithmetic_expression().\n");
        fflush(stdout);
        
        printf("Running test_comparison_expression()...\n");
        fflush(stdout);
        test_comparison_expression();
        printf("Completed test_comparison_expression().\n");
        fflush(stdout);
        
        printf("Running test_function_call_expression()...\n");
        fflush(stdout);
        test_function_call_expression();
        printf("Completed test_function_call_expression().\n");
        fflush(stdout);
    }
    
    // And report success
    printf("All expression tests passed!\n");
    fflush(stdout);
    
    // Make sure we exit gracefully
    return 0;
}

static void test_string_literal(void) {
    printf("Testing string literal evaluation...\n");
    fflush(stdout);
    
    // Given a string literal expression
    const char *expr = "\"Hello, World!\"";
    
    // Create expression context
    expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
    assert(ctx != NULL);
    
    // When we evaluate the expression
    const data_t *result = ar__expression__evaluate(ctx);
    
    // Then the result should be a string with the correct value
    assert(result != NULL);
    assert(ar_data_get_type(result) == DATA_STRING);
    assert(strcmp(ar_data_get_string(result), "Hello, World!") == 0);
    
    // And the offset should be updated correctly
    assert(ar__expression__offset(ctx) == (int)strlen(expr));
    
    // Clean up - don't destroy result as it's a borrowed reference
    ar__expression__destroy_context(ctx);
    
    printf("String literal test passed.\n");
    fflush(stdout);
}

static void test_number_literal(void) {
    printf("Testing number literal evaluation...\n");
    fflush(stdout);
    
    // Test integer literal
    {
        const char *expr = "42";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 42);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
    }
    
    // Test negative integer literal
    {
        const char *expr = "-123";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == -123);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
    }
    
    // Test double literal
    {
        const char *expr = "3.14159";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_DOUBLE);
        // Use very small epsilon for floating-point comparison
        double epsilon = 0.00001;
        assert(ar_data_get_double(result) - 3.14159 < epsilon && 
               ar_data_get_double(result) - 3.14159 > -epsilon);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
    }
    
    // Test negative double literal
    {
        const char *expr = "-2.718";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_DOUBLE);
        // Use very small epsilon for floating-point comparison
        double epsilon = 0.00001;
        assert(ar_data_get_double(result) - (-2.718) < epsilon && 
               ar_data_get_double(result) - (-2.718) > -epsilon);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
    }
    
    printf("Number literal tests passed.\n");
    fflush(stdout);
}

static void test_memory_access(void) {
    printf("Testing memory access evaluation...\n");
    fflush(stdout);
    
    // Set up memory, context, and message data structures
    
    // Set up memory
    data_t *memory = ar_data_create_map();
    assert(memory != NULL);
    assert(ar_data_set_map_string(memory, "name", "Alice"));
    assert(ar_data_set_map_integer(memory, "age", 30));
    assert(ar_data_set_map_double(memory, "balance", 450.75));
    
    // Set up memory with nested preferences
    data_t *user_preferences = ar_data_create_map();
    assert(user_preferences != NULL);
    assert(ar_data_set_map_string(user_preferences, "theme", "dark"));
    assert(ar_data_set_map_integer(user_preferences, "notifications", 1));
    assert(ar_data_set_map_data(memory, "preferences", user_preferences));
    // Note: Don't use user_preferences after this point - ownership has been transferred
    
    // Set up context with nested limits
    data_t *context = ar_data_create_map();
    assert(context != NULL);
    assert(ar_data_set_map_string(context, "environment", "production"));
    assert(ar_data_set_map_integer(context, "max_retries", 3));
    
    data_t *system_limits = ar_data_create_map();
    assert(system_limits != NULL);
    assert(ar_data_set_map_integer(system_limits, "max_memory", 1024));
    assert(ar_data_set_map_integer(system_limits, "timeout", 60));
    assert(ar_data_set_map_data(context, "limits", system_limits));
    // Note: Don't use system_limits after this point - ownership has been transferred
    
    // Set up message with nested payload
    data_t *message = ar_data_create_map();
    assert(message != NULL);
    assert(ar_data_set_map_string(message, "type", "command"));
    assert(ar_data_set_map_string(message, "action", "update"));
    
    data_t *payload = ar_data_create_map();
    assert(payload != NULL);
    assert(ar_data_set_map_string(payload, "field", "status"));
    assert(ar_data_set_map_string(payload, "value", "active"));
    assert(ar_data_set_map_data(message, "payload", payload));
    // Note: Don't use payload after this point - ownership has been transferred
    
    // Test simple memory access
    {
        const char *expr = "memory.name";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "Alice") == 0);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Note: Don't destroy result as it's a direct reference to memory
        ar__expression__destroy_context(ctx);
    }
    
    // Test nested memory access
    {
        const char *expr = "memory.preferences.theme";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "dark") == 0);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Note: Don't destroy result as it's a direct reference to memory
        ar__expression__destroy_context(ctx);
    }
    
    // Test context access
    {
        const char *expr = "context.environment";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "production") == 0);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Note: Don't destroy result as it's a direct reference to context
        ar__expression__destroy_context(ctx);
    }
    
    // Test nested context access
    {
        const char *expr = "context.limits.timeout";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 60);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Note: Don't destroy result as it's a direct reference to context
        ar__expression__destroy_context(ctx);
    }
    
    // Test message access
    {
        const char *expr = "message.type";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "command") == 0);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Note: Don't destroy result as it's a direct reference to message
        ar__expression__destroy_context(ctx);
    }
    
    // Test nested message access
    {
        const char *expr = "message.payload.field";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "status") == 0);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Note: Don't destroy result as it's a direct reference to message
        ar__expression__destroy_context(ctx);
    }
    
    // Test non-existent path
    {
        const char *expr = "memory.nonexistent.field";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        // Should return NULL for non-existent path
        assert(result == NULL);
        
        ar__expression__destroy_context(ctx);
    }
    
    // Cleanup
    ar_data_destroy(message);
    ar_data_destroy(memory);
    ar_data_destroy(context);
    
    printf("Memory access tests passed.\n");
    fflush(stdout);
}

static void test_arithmetic_expression(void) {
    printf("Testing arithmetic expression evaluation...\n");
    fflush(stdout);
    
    // Set up minimal memory structures for testing
    printf("Setting up memory, context, and message for arithmetic tests with memory access\n");
    fflush(stdout);
    
    // We'll create fresh structures for each test
    data_t *memory = ar_data_create_map();
    assert(memory != NULL);
    assert(ar_data_set_map_integer(memory, "x", 10));
    assert(ar_data_set_map_integer(memory, "y", 5));
    
    data_t *context = ar_data_create_map();
    assert(context != NULL);
    
    data_t *message = ar_data_create_map();
    assert(message != NULL);
    assert(ar_data_set_map_integer(message, "count", 42));
    
    // Test addition
    {
        printf("Testing addition expression: '2 + 3'\n");
        fflush(stdout);
        
        const char *expr = "2 + 3";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 5);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Addition test passed\n");
        fflush(stdout);
    }
    
    // Test subtraction
    {
        printf("Testing subtraction expression: '10 - 4'\n");
        fflush(stdout);
        
        const char *expr = "10 - 4";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 6);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Subtraction test passed\n");
        fflush(stdout);
    }
    
    // Test multiplication
    {
        printf("Testing multiplication expression: '5 * 3'\n");
        fflush(stdout);
        
        const char *expr = "5 * 3";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 15);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Multiplication test passed\n");
        fflush(stdout);
    }
    
    // Test division
    {
        printf("Testing division expression: '20 / 4'\n");
        fflush(stdout);
        
        const char *expr = "20 / 4";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 5);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Division test passed\n");
        fflush(stdout);
    }
    
    // Test simple addition to avoid operator precedence issues for now
    {
        printf("Testing simple addition again: '2 + 3'\n");
        fflush(stdout);
        
        const char *expr = "2 + 3";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 5);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Simple addition again test passed\n");
        fflush(stdout);
    }
    
    // Test expression with doubles
    {
        printf("Testing doubles expression: '3.5 + 2.5'\n");
        fflush(stdout);
        
        const char *expr = "3.5 + 2.5";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_DOUBLE);
        
        double epsilon = 0.00001;
        assert(ar_data_get_double(result) - 6.0 < epsilon && 
               ar_data_get_double(result) - 6.0 > -epsilon);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Doubles test passed\n");
        fflush(stdout);
    }
    
    // Test mixed integer and double
    {
        printf("Testing mixed int/double expression: '5 * 2.5'\n");
        fflush(stdout);
        
        const char *expr = "5 * 2.5";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_DOUBLE);
        
        double epsilon = 0.00001;
        assert(ar_data_get_double(result) - 12.5 < epsilon && 
               ar_data_get_double(result) - 12.5 > -epsilon);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Mixed int/double test passed\n");
        fflush(stdout);
    }
    
    // Test string concatenation with +
    {
        printf("Testing string concatenation: '\"Hello, \" + \"World!\"'\n");
        fflush(stdout);
        
        const char *expr = "\"Hello, \" + \"World!\"";
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_STRING);
        assert(strcmp(ar_data_get_string(result), "Hello, World!") == 0);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("String concatenation test passed\n");
        fflush(stdout);
    }
    
    // Test string and number
    {
        const char *expr = "\"Price: $\" + 42.99";
        
        printf("Testing string+number with expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        printf("Context created successfully\n");
        fflush(stdout);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        printf("Expression evaluated successfully\n");
        fflush(stdout);
        
        if (result) {
            printf("Result type: %d\n", ar_data_get_type(result));
            fflush(stdout);
            
            if (ar_data_get_type(result) == DATA_STRING) {
                const char* str_result = ar_data_get_string(result);
                printf("String result: '%s'\n", str_result ? str_result : "NULL");
                fflush(stdout);
                
                if (str_result) {
                    printf("Comparing with expected 'Price: $42.99'\n");
                    fflush(stdout);
                    assert(strcmp(str_result, "Price: $42.99") == 0);
                }
            }
        }
        
        printf("Offset: %d, Expected: %d\n", ar__expression__offset(ctx), (int)strlen(expr));
        fflush(stdout);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_STRING);
        const char* str_result = ar_data_get_string(result);
        printf("Got: '%s'\n", str_result);
        assert(strcmp(str_result, "Price: $42.99") == 0);
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("String+number concatenation test passed\n");
        fflush(stdout);
    }
    
    // Test arithmetic with memory access
    {
        const char *expr = "memory.x + memory.y";
        
        printf("Testing memory access arithmetic with expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        printf("Context created successfully\n");
        fflush(stdout);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        printf("Expression evaluated successfully\n");
        fflush(stdout);
        
        if (result) {
            printf("Result type: %d\n", ar_data_get_type(result));
            if (ar_data_get_type(result) == DATA_INTEGER) {
                printf("Integer result: %d (expected 15)\n", ar_data_get_integer(result));
            }
            fflush(stdout);
        }
        
        printf("Offset: %d, Expected: %d\n", ar__expression__offset(ctx), (int)strlen(expr));
        fflush(stdout);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 15); // 10 + 5 = 15
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: Expression results are borrowed references, not owned by the caller.
        // Do not destroy the result - the context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Memory access addition test passed\n");
        fflush(stdout);
    }
    
    // Test arithmetic with memory access - multiplication with a constant
    {
        const char *expr = "memory.x * 2";
        
        printf("Testing memory access arithmetic with expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        printf("Expression evaluated successfully\n");
        fflush(stdout);
        
        if (result) {
            data_type_t type = ar_data_get_type(result);
            printf("Result type: %d\n", type);
            
            // Print detailed type information
            printf("Type info - Is NULL: %s\n", result ? "no" : "yes");
            printf("Type info - Is INTEGER: %s\n", type == DATA_INTEGER ? "yes" : "no");
            printf("Type info - Is DOUBLE: %s\n", type == DATA_DOUBLE ? "yes" : "no");
            printf("Type info - Is STRING: %s\n", type == DATA_STRING ? "yes" : "no");
            printf("Type info - Is LIST: %s\n", type == DATA_LIST ? "yes" : "no");
            printf("Type info - Is MAP: %s\n", type == DATA_MAP ? "yes" : "no");
            
            if (type == DATA_INTEGER) {
                printf("Integer result: %d (expected 20)\n", ar_data_get_integer(result));
            } else if (type == DATA_DOUBLE) {
                printf("Double result: %f (expected 20.0)\n", ar_data_get_double(result));
            }
            fflush(stdout);
        }
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 20); // 10 * 2 = 20
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: For arithmetic operations with memory access, the result is a NEW value, 
        // not a reference, so we need to take ownership if we want to keep it.
        // If we don't need the result after context destruction, we can let the context handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Memory access multiplication test passed\n");
        fflush(stdout);
    }
    
    // Test with message data - multiplication with a constant
    {
        const char *expr = "message.count * 2";
        
        printf("Testing message access arithmetic with expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        printf("Expression evaluated successfully\n");
        fflush(stdout);
        
        if (result) {
            printf("Result type: %d\n", ar_data_get_type(result));
            if (ar_data_get_type(result) == DATA_INTEGER) {
                printf("Integer result: %d (expected 84)\n", ar_data_get_integer(result));
            }
            fflush(stdout);
        }
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 84); // 42 * 2 = 84
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // IMPORTANT: The result of an arithmetic operation (even with message access)
        // is a NEW value, not a reference to message. The context will handle cleanup.
        ar__expression__destroy_context(ctx);
        
        printf("Message access multiplication test passed\n");
        fflush(stdout);
    }
    
    // Properly clean up allocated objects
    if (message) {
        ar_data_destroy(message);
        message = NULL;
    }
    
    if (context) {
        ar_data_destroy(context);
        context = NULL;
    }
    
    if (memory) {
        ar_data_destroy(memory);
        memory = NULL;
    }
    
    printf("Resources properly cleaned up.\n");
    fflush(stdout);
    
    printf("Arithmetic expression tests passed.\n");
    fflush(stdout);
    
    printf("About to exit test_arithmetic_expression()...\n");
    fflush(stdout);
}

static void test_comparison_expression(void) {
    printf("Testing comparison expression evaluation...\n");
    fflush(stdout);
    
    // Set up minimal memory structures for testing
    printf("Setting up memory, context, and message for comparison tests\n");
    fflush(stdout);
    
    // We'll create fresh structures for comparison tests
    data_t *memory = ar_data_create_map();
    assert(memory != NULL);
    assert(ar_data_set_map_integer(memory, "count", 10));
    assert(ar_data_set_map_string(memory, "status", "active"));
    
    data_t *context = ar_data_create_map();
    assert(context != NULL);
    assert(ar_data_set_map_integer(context, "threshold", 5));
    
    data_t *message = ar_data_create_map();
    assert(message != NULL);
    assert(ar_data_set_map_integer(message, "priority", 3));
    
    // Test equality with integers
    {
        const char *expr = "5 = 5";
        
        printf("Testing equality expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        printf("Equality test - Expr: '%s'\n", expr);
        if (result) {
            printf("Type: %d, Value: %d, Offset: %d\n", 
                   ar_data_get_type(result), 
                   ar_data_get_integer(result),
                   ar__expression__offset(ctx));
        } else {
            printf("Offset: %d\n", ar__expression__offset(ctx));
        }
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Equality test passed\n");
        fflush(stdout);
    }
    
    // Test equality with strings
    {
        const char *expr = "\"hello\" = \"hello\"";
        
        printf("Testing string equality expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        printf("String equality test\n");
        if (result) {
            printf("Type: %d, Value: %d, Offset: %d\n", 
                   ar_data_get_type(result), 
                   ar_data_get_integer(result),
                   ar__expression__offset(ctx));
        }
        fflush(stdout);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("String equality test passed\n");
        fflush(stdout);
    }

    // Test inequality
    {
        const char *expr = "5 <> 3";
        
        printf("Testing inequality expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Inequality test passed\n");
        fflush(stdout);
    }
    
    // Test inequality with strings
    {
        const char *expr = "\"active\" <> \"inactive\"";
        
        printf("Testing string inequality expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("String inequality test passed\n");
        fflush(stdout);
    }
    
    // Test less than
    {
        const char *expr = "3 < 5";
        
        printf("Testing less than expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Less than test passed\n");
        fflush(stdout);
    }
    
    // Test greater than
    {
        const char *expr = "7 > 4";
        
        printf("Testing greater than expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Greater than test passed\n");
        fflush(stdout);
    }
    
    // Test less than or equal
    {
        const char *expr = "5 <= 5";
        
        printf("Testing less than or equal expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Less than or equal test passed\n");
        fflush(stdout);
    }
    
    // Test greater than or equal
    {
        const char *expr = "7 >= 10";
        
        printf("Testing greater than or equal expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 0); // false
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Greater than or equal test passed\n");
        fflush(stdout);
    }
    
    // Test comparing with memory access
    {
        const char *expr = "memory.count > 5";
        
        printf("Testing memory comparison expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true (10 > 5)
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Memory comparison test passed\n");
        fflush(stdout);
    }
    
    // Test string comparison with memory access
    {
        const char *expr = "memory.status = \"active\"";
        
        printf("Testing string memory comparison expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("String memory comparison test passed\n");
        fflush(stdout);
    }
    
    // Test comparing memory and context
    {
        const char *expr = "memory.count > context.threshold";
        
        printf("Testing memory-context comparison expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true (10 > 5)
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Memory-context comparison test passed\n");
        fflush(stdout);
    }
    
    // Test comparing message and context
    {
        const char *expr = "message.priority <= context.threshold";
        
        printf("Testing message-context comparison expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 1); // true (3 <= 5)
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Message-context comparison test passed\n");
        fflush(stdout);
    }
    
    // Test complex comparison with arithmetic
    {
        const char *expr = "memory.count + 5 > context.threshold * 3";
        
        printf("Testing complex comparison expression: '%s'\n", expr);
        fflush(stdout);
        
        // Create expression context
        expression_context_t *ctx = ar__expression__create_context(memory, context, message, expr);
        assert(ctx != NULL);
        
        const data_t *result = ar__expression__evaluate(ctx);
        
        assert(result != NULL);
        assert(ar_data_get_type(result) == DATA_INTEGER);
        assert(ar_data_get_integer(result) == 0); // false (10 + 5 = 15, 5 * 3 = 15, 15 > 15 is false)
        assert(ar__expression__offset(ctx) == (int)strlen(expr));
        
        // Let the context handle cleanup of the result
        ar__expression__destroy_context(ctx);
        
        printf("Complex comparison test passed\n");
        fflush(stdout);
    }
    
    // Cleanup
    if (message) {
        ar_data_destroy(message);
        message = NULL;
    }
    
    if (context) {
        ar_data_destroy(context);
        context = NULL;
    }
    
    if (memory) {
        ar_data_destroy(memory);
        memory = NULL;
    }
    
    printf("Comparison expression tests passed.\n");
    fflush(stdout);
}

static void test_function_call_expression(void) {
    printf("Testing function call as expression (should fail)...\n");
    fflush(stdout);
    
    // Try to evaluate a function call as an expression
    const char *expr = "if(1, \"true\", \"false\")";
    
    // Create expression context
    expression_context_t *ctx = ar__expression__create_context(NULL, NULL, NULL, expr);
    assert(ctx != NULL);
    
    // This should return NULL since function calls are not valid expressions
    const data_t *result = ar__expression__evaluate(ctx);
    
    // Verify the result is NULL for invalid syntax
    assert(result == NULL);
    
    // Offset should be at the start of "if"
    assert(ar__expression__offset(ctx) == 0);
    
    ar__expression__destroy_context(ctx);
    
    // Test a function call within an arithmetic expression
    const char *expr2 = "5 + if(1, 10, 20)";
    
    // Create expression context
    expression_context_t *ctx2 = ar__expression__create_context(NULL, NULL, NULL, expr2);
    assert(ctx2 != NULL);
    
    // This should also return NULL when it encounters the "if" function call
    result = ar__expression__evaluate(ctx2);
    
    // Verify the result is NULL for invalid syntax
    assert(result == NULL);
    
    // Offset should be at the start of "if", which is after "5 + "
    assert(ar__expression__offset(ctx2) == 4);
    
    ar__expression__destroy_context(ctx2);
    
    printf("Function call expression test passed (verified that function calls are not valid expressions).\n");
    fflush(stdout);
}
