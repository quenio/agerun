#include "agerun_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_trim(void);
static void test_isspace(void);

static void test_trim(void) {
    printf("Testing ar_trim()...\n");
    
    // Test 1: Trim leading whitespace
    char str1[] = "   Hello";
    char *result1 = ar_trim(str1);
    assert(strcmp(result1, "Hello") == 0);
    
    // Test 2: Trim trailing whitespace
    char str2[] = "World   ";
    char *result2 = ar_trim(str2);
    assert(strcmp(result2, "World") == 0);
    
    // Test 3: Trim both leading and trailing whitespace
    char str3[] = "  Hello World  ";
    char *result3 = ar_trim(str3);
    assert(strcmp(result3, "Hello World") == 0);
    
    // Test 4: No whitespace to trim
    char str4[] = "NoWhitespace";
    char *result4 = ar_trim(str4);
    assert(strcmp(result4, "NoWhitespace") == 0);
    
    // Test 5: Empty string
    char str5[] = "";
    char *result5 = ar_trim(str5);
    assert(strcmp(result5, "") == 0);
    
    // Test 6: Only whitespace
    char str6[] = "   \t\n   ";
    char *result6 = ar_trim(str6);
    assert(strcmp(result6, "") == 0);
    
    printf("All ar_trim() tests passed!\n");
}

static void test_isspace(void) {
    printf("Testing ar_isspace()...\n");
    
    // Test standard whitespace characters
    assert(ar_isspace(' ') != 0);
    assert(ar_isspace('\t') != 0);
    assert(ar_isspace('\n') != 0);
    assert(ar_isspace('\r') != 0);
    assert(ar_isspace('\f') != 0);
    assert(ar_isspace('\v') != 0);
    
    // Test non-whitespace characters
    assert(ar_isspace('a') == 0);
    assert(ar_isspace('Z') == 0);
    assert(ar_isspace('0') == 0);
    assert(ar_isspace('_') == 0);
    assert(ar_isspace('@') == 0);
    
    // Test edge cases
    assert(ar_isspace(0) == 0);
    assert(ar_isspace(-1) == 0);
    
    printf("All ar_isspace() tests passed!\n");
}

int main(void) {
    printf("Starting String Module Tests...\n");
    
    test_isspace();
    test_trim();
    
    printf("All string tests passed!\n");
    return 0;
}
