#include "agerun_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_trim_leading_whitespace(void);
static void test_trim_trailing_whitespace(void);
static void test_trim_both_whitespace(void);
static void test_trim_no_whitespace(void);
static void test_trim_empty_string(void);
static void test_trim_only_whitespace(void);
static void test_isspace_standard_whitespace(void);
static void test_isspace_non_whitespace(void);
static void test_isspace_edge_cases(void);

static void test_trim_leading_whitespace(void) {
    printf("Testing ar_trim() with leading whitespace...\n");
    
    // Given a string with leading whitespace
    char str[] = "   Hello";
    
    // When the trim function is called
    char *result = ar_string_trim(str);
    
    // Then the result should have the whitespace removed
    assert(strcmp(result, "Hello") == 0);
    
    printf("ar_trim() leading whitespace test passed!\n");
}

static void test_trim_trailing_whitespace(void) {
    printf("Testing ar_trim() with trailing whitespace...\n");
    
    // Given a string with trailing whitespace
    char str[] = "World   ";
    
    // When the trim function is called
    char *result = ar_string_trim(str);
    
    // Then the result should have the whitespace removed
    assert(strcmp(result, "World") == 0);
    
    printf("ar_trim() trailing whitespace test passed!\n");
}

static void test_trim_both_whitespace(void) {
    printf("Testing ar_trim() with both leading and trailing whitespace...\n");
    
    // Given a string with both leading and trailing whitespace
    char str[] = "  Hello World  ";
    
    // When the trim function is called
    char *result = ar_string_trim(str);
    
    // Then the result should have all whitespace removed
    assert(strcmp(result, "Hello World") == 0);
    
    printf("ar_trim() both whitespace test passed!\n");
}

static void test_trim_no_whitespace(void) {
    printf("Testing ar_trim() with no whitespace...\n");
    
    // Given a string with no whitespace
    char str[] = "NoWhitespace";
    
    // When the trim function is called
    char *result = ar_string_trim(str);
    
    // Then the result should be unchanged
    assert(strcmp(result, "NoWhitespace") == 0);
    
    printf("ar_trim() no whitespace test passed!\n");
}

static void test_trim_empty_string(void) {
    printf("Testing ar_trim() with empty string...\n");
    
    // Given an empty string
    char str[] = "";
    
    // When the trim function is called
    char *result = ar_string_trim(str);
    
    // Then the result should remain an empty string
    assert(strcmp(result, "") == 0);
    
    printf("ar_trim() empty string test passed!\n");
}

static void test_trim_only_whitespace(void) {
    printf("Testing ar_trim() with only whitespace...\n");
    
    // Given a string containing only whitespace
    char str[] = "   \t\n   ";
    
    // When the trim function is called
    char *result = ar_string_trim(str);
    
    // Then the result should be an empty string
    assert(strcmp(result, "") == 0);
    
    printf("ar_trim() only whitespace test passed!\n");
}

static void test_isspace_standard_whitespace(void) {
    printf("Testing ar_isspace() with standard whitespace characters...\n");
    
    // Given standard whitespace characters
    
    // When ar_isspace is called on each character
    // Then it should return non-zero for each whitespace character
    assert(ar_string_isspace(' ') != 0);
    assert(ar_string_isspace('\t') != 0);
    assert(ar_string_isspace('\n') != 0);
    assert(ar_string_isspace('\r') != 0);
    assert(ar_string_isspace('\f') != 0);
    assert(ar_string_isspace('\v') != 0);
    
    printf("ar_isspace() standard whitespace test passed!\n");
}

static void test_isspace_non_whitespace(void) {
    printf("Testing ar_isspace() with non-whitespace characters...\n");
    
    // Given non-whitespace characters
    
    // When ar_isspace is called on each character
    // Then it should return zero for each non-whitespace character
    assert(ar_string_isspace('a') == 0);
    assert(ar_string_isspace('Z') == 0);
    assert(ar_string_isspace('0') == 0);
    assert(ar_string_isspace('_') == 0);
    assert(ar_string_isspace('@') == 0);
    
    printf("ar_isspace() non-whitespace test passed!\n");
}

static void test_isspace_edge_cases(void) {
    printf("Testing ar_isspace() with edge cases...\n");
    
    // Given edge case values
    
    // When ar_isspace is called with these values
    // Then it should return zero for non-whitespace values
    assert(ar_string_isspace(0) == 0);
    assert(ar_string_isspace(-1) == 0);
    
    printf("ar_isspace() edge cases test passed!\n");
}

int main(void) {
    printf("Starting String Module Tests...\n");
    
    // Run isspace tests
    test_isspace_standard_whitespace();
    test_isspace_non_whitespace();
    test_isspace_edge_cases();
    
    // Run trim tests
    test_trim_leading_whitespace();
    test_trim_trailing_whitespace();
    test_trim_both_whitespace();
    test_trim_no_whitespace();
    test_trim_empty_string();
    test_trim_only_whitespace();
    
    printf("All string tests passed!\n");
    return 0;
}
