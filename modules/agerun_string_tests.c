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
static void test_path_count_normal(void);
static void test_path_count_edge_cases(void);
static void test_path_segment_normal(void);
static void test_path_segment_edge_cases(void);

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
    char *original_ptr = str;
    
    // When the trim function is called
    char *result = ar_string_trim(str);
    
    // Then the result should have all whitespace removed
    assert(strcmp(result, "Hello World") == 0);
    
    // Then the result should point to a position within the original string (no new allocation)
    assert(result >= original_ptr && result < original_ptr + sizeof(str));
    
    // Then the result end (including null terminator) should be within the original buffer
    assert(result + strlen(result) < original_ptr + sizeof(str));
    
    // Then the original string should be modified in-place
    assert(strcmp(str + 2, "Hello World") == 0);
    
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

static void test_path_count_normal(void) {
    printf("Testing ar_string_path_count() with normal paths...\n");
    
    // Given several paths with different numbers of segments
    const char *path1 = "key";
    const char *path2 = "key.sub_key";
    const char *path3 = "key.sub_key.sub_sub_key";
    const char *path4 = "key.sub_key.sub_sub_key.final";
    
    // When the path_count function is called
    size_t count1 = ar_string_path_count(path1, '.');
    size_t count2 = ar_string_path_count(path2, '.');
    size_t count3 = ar_string_path_count(path3, '.');
    size_t count4 = ar_string_path_count(path4, '.');
    
    // Then the counts should match the expected segment counts
    assert(count1 == 1);
    assert(count2 == 2);
    assert(count3 == 3);
    assert(count4 == 4);
    
    printf("ar_string_path_count() normal paths test passed!\n");
}

static void test_path_count_edge_cases(void) {
    printf("Testing ar_string_path_count() with edge cases...\n");
    
    // Given edge case paths
    const char *empty = "";
    const char *null_ptr = NULL;
    const char *only_separators = ".....";
    const char *trailing_separator = "key.sub_key.";
    const char *leading_separator = ".key.sub_key";
    
    // When the path_count function is called
    size_t empty_count = ar_string_path_count(empty, '.');
    size_t null_count = ar_string_path_count(null_ptr, '.');
    size_t only_separators_count = ar_string_path_count(only_separators, '.');
    size_t trailing_count = ar_string_path_count(trailing_separator, '.');
    size_t leading_count = ar_string_path_count(leading_separator, '.');
    
    // Then the counts should match the expected segment counts
    assert(empty_count == 0);
    assert(null_count == 0);
    assert(only_separators_count == 6); // 5 separators = 6 segments
    assert(trailing_count == 3); // 2 keys + empty segment
    assert(leading_count == 3); // empty + 2 keys
    
    printf("ar_string_path_count() edge cases test passed!\n");
}

static void test_path_segment_normal(void) {
    printf("Testing ar_string_path_segment() with normal cases...\n");
    
    // Given a path with multiple segments
    const char *path = "key.sub_key.sub_sub_key";
    
    // When the path_segment function is called for each index
    char *segment0 = ar_string_path_segment(path, '.', 0);
    char *segment1 = ar_string_path_segment(path, '.', 1);
    char *segment2 = ar_string_path_segment(path, '.', 2);
    
    // Then each segment should match the expected value
    assert(segment0 != NULL);
    assert(segment1 != NULL);
    assert(segment2 != NULL);
    assert(strcmp(segment0, "key") == 0);
    assert(strcmp(segment1, "sub_key") == 0);
    assert(strcmp(segment2, "sub_sub_key") == 0);
    
    // Clean up allocated memory
    free(segment0);
    free(segment1);
    free(segment2);
    
    printf("ar_string_path_segment() normal cases test passed!\n");
}

static void test_path_segment_edge_cases(void) {
    printf("Testing ar_string_path_segment() with edge cases...\n");
    
    // Given edge case paths
    const char *empty = "";
    const char *null_ptr = NULL;
    const char *path_with_empties = "..key..end.";
    
    // When the path_segment function is called with various edge cases
    char *empty_result = ar_string_path_segment(empty, '.', 0);
    char *null_result = ar_string_path_segment(null_ptr, '.', 0);
    char *out_of_bounds = ar_string_path_segment("key.value", '.', 5);
    
    // Then the results should be as expected
    assert(empty_result == NULL);
    assert(null_result == NULL);
    assert(out_of_bounds == NULL);
    
    // Test empty segments in a path
    // Print the path for debugging
    printf("Test path: \"%s\"\n", path_with_empties);
    
    // Try getting just a few segments for debugging
    char *empty_seg1 = ar_string_path_segment(path_with_empties, '.', 0);
    char *empty_seg2 = ar_string_path_segment(path_with_empties, '.', 1);
    char *key_seg = ar_string_path_segment(path_with_empties, '.', 2);
    
    assert(empty_seg1 != NULL);
    assert(empty_seg2 != NULL);
    assert(key_seg != NULL);
    
    printf("Segment 0: \"%s\"\n", empty_seg1);
    printf("Segment 1: \"%s\"\n", empty_seg2);
    printf("Segment 2: \"%s\"\n", key_seg);
    
    assert(strcmp(empty_seg1, "") == 0);
    assert(strcmp(empty_seg2, "") == 0);
    assert(strcmp(key_seg, "key") == 0);
    
    // Clean up allocated memory
    free(empty_seg1);
    free(empty_seg2);
    free(key_seg);
    
    printf("ar_string_path_segment() edge cases test passed!\n");
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
    
    // Run path tests
    test_path_count_normal();
    test_path_count_edge_cases();
    test_path_segment_normal();
    test_path_segment_edge_cases();
    
    printf("All string tests passed!\n");
    return 0;
}
