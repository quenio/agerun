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
static void test_path_parent_normal(void);
static void test_path_parent_edge_cases(void);

static void test_trim_leading_whitespace(void) {
    printf("Testing ar_trim() with leading whitespace...\n");
    
    // Given a string with leading whitespace
    char mut_str[] = "   Hello";
    
    // When the trim function is called
    char *result = ar_string_trim(mut_str);
    
    // Then the result should have the whitespace removed
    assert(strcmp(result, "Hello") == 0);
    
    printf("ar_trim() leading whitespace test passed!\n");
}

static void test_trim_trailing_whitespace(void) {
    printf("Testing ar_trim() with trailing whitespace...\n");
    
    // Given a string with trailing whitespace
    char mut_str[] = "World   ";
    
    // When the trim function is called
    char *result = ar_string_trim(mut_str);
    
    // Then the result should have the whitespace removed
    assert(strcmp(result, "World") == 0);
    
    printf("ar_trim() trailing whitespace test passed!\n");
}

static void test_trim_both_whitespace(void) {
    printf("Testing ar_trim() with both leading and trailing whitespace...\n");
    
    // Given a string with both leading and trailing whitespace
    char mut_str[] = "  Hello World  ";
    char *original_ptr = mut_str;
    
    // When the trim function is called
    char *result = ar_string_trim(mut_str);
    
    // Then the result should have all whitespace removed
    assert(strcmp(result, "Hello World") == 0);
    
    // Then the result should point to a position within the original string (no new allocation)
    assert(result >= original_ptr && result < original_ptr + sizeof(mut_str));
    
    // Then the result end (including null terminator) should be within the original buffer
    assert(result + strlen(result) < original_ptr + sizeof(mut_str));
    
    // Then the original string should be modified in-place
    assert(strcmp(mut_str + 2, "Hello World") == 0);
    
    printf("ar_trim() both whitespace test passed!\n");
}

static void test_trim_no_whitespace(void) {
    printf("Testing ar_trim() with no whitespace...\n");
    
    // Given a string with no whitespace
    char mut_str[] = "NoWhitespace";
    
    // When the trim function is called
    char *result = ar_string_trim(mut_str);
    
    // Then the result should be unchanged
    assert(strcmp(result, "NoWhitespace") == 0);
    
    printf("ar_trim() no whitespace test passed!\n");
}

static void test_trim_empty_string(void) {
    printf("Testing ar_trim() with empty string...\n");
    
    // Given an empty string
    char mut_str[] = "";
    
    // When the trim function is called
    char *result = ar_string_trim(mut_str);
    
    // Then the result should remain an empty string
    assert(strcmp(result, "") == 0);
    
    printf("ar_trim() empty string test passed!\n");
}

static void test_trim_only_whitespace(void) {
    printf("Testing ar_trim() with only whitespace...\n");
    
    // Given a string containing only whitespace
    char mut_str[] = "   \t\n   ";
    
    // When the trim function is called
    char *result = ar_string_trim(mut_str);
    
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
    const char *ref_path1 = "key";
    const char *ref_path2 = "key.sub_key";
    const char *ref_path3 = "key.sub_key.sub_sub_key";
    const char *ref_path4 = "key.sub_key.sub_sub_key.final";
    
    // When the path_count function is called
    size_t count1 = ar_string_path_count(ref_path1, '.');
    size_t count2 = ar_string_path_count(ref_path2, '.');
    size_t count3 = ar_string_path_count(ref_path3, '.');
    size_t count4 = ar_string_path_count(ref_path4, '.');
    
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
    const char *ref_empty = "";
    const char *ref_null_ptr = NULL;
    const char *ref_only_separators = ".....";
    const char *ref_trailing_separator = "key.sub_key.";
    const char *ref_leading_separator = ".key.sub_key";
    
    // When the path_count function is called
    size_t empty_count = ar_string_path_count(ref_empty, '.');
    size_t null_count = ar_string_path_count(ref_null_ptr, '.');
    size_t only_separators_count = ar_string_path_count(ref_only_separators, '.');
    size_t trailing_count = ar_string_path_count(ref_trailing_separator, '.');
    size_t leading_count = ar_string_path_count(ref_leading_separator, '.');
    
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
    const char *ref_path = "key.sub_key.sub_sub_key";
    
    // When the path_segment function is called for each index
    char *own_segment0 = ar_string_path_segment(ref_path, '.', 0);
    char *own_segment1 = ar_string_path_segment(ref_path, '.', 1);
    char *own_segment2 = ar_string_path_segment(ref_path, '.', 2);
    
    // Then each segment should match the expected value
    assert(own_segment0 != NULL);
    assert(own_segment1 != NULL);
    assert(own_segment2 != NULL);
    assert(strcmp(own_segment0, "key") == 0);
    assert(strcmp(own_segment1, "sub_key") == 0);
    assert(strcmp(own_segment2, "sub_sub_key") == 0);
    
    // Clean up allocated memory
    free(own_segment0);
    free(own_segment1);
    free(own_segment2);
    
    printf("ar_string_path_segment() normal cases test passed!\n");
}

static void test_path_segment_edge_cases(void) {
    printf("Testing ar_string_path_segment() with edge cases...\n");
    
    // Given edge case paths
    const char *ref_empty = "";
    const char *ref_null_ptr = NULL;
    const char *ref_path_with_empties = "..key..end.";
    
    // When the path_segment function is called with various edge cases
    char *own_empty_result = ar_string_path_segment(ref_empty, '.', 0);
    char *own_null_result = ar_string_path_segment(ref_null_ptr, '.', 0);
    char *own_out_of_bounds = ar_string_path_segment("key.value", '.', 5);
    
    // Then the results should be as expected
    assert(own_empty_result == NULL);
    assert(own_null_result == NULL);
    assert(own_out_of_bounds == NULL);
    
    // Test empty segments in a path
    // Print the path for debugging
    printf("Test path: \"%s\"\n", ref_path_with_empties);
    
    // Try getting just a few segments for debugging
    char *own_empty_seg1 = ar_string_path_segment(ref_path_with_empties, '.', 0);
    char *own_empty_seg2 = ar_string_path_segment(ref_path_with_empties, '.', 1);
    char *own_key_seg = ar_string_path_segment(ref_path_with_empties, '.', 2);
    
    assert(own_empty_seg1 != NULL);
    assert(own_empty_seg2 != NULL);
    assert(own_key_seg != NULL);
    
    printf("Segment 0: \"%s\"\n", own_empty_seg1);
    printf("Segment 1: \"%s\"\n", own_empty_seg2);
    printf("Segment 2: \"%s\"\n", own_key_seg);
    
    assert(strcmp(own_empty_seg1, "") == 0);
    assert(strcmp(own_empty_seg2, "") == 0);
    assert(strcmp(own_key_seg, "key") == 0);
    
    // Clean up allocated memory
    free(own_empty_seg1);
    free(own_empty_seg2);
    free(own_key_seg);
    
    printf("ar_string_path_segment() edge cases test passed!\n");
}

static void test_path_parent_normal(void) {
    printf("Testing ar_string_path_parent() with normal paths...\n");
    
    // Given several paths with different numbers of segments
    const char *ref_path1 = "key.sub_key";
    const char *ref_path2 = "key.sub_key.sub_sub_key";
    const char *ref_path3 = "key.sub_key.sub_sub_key.final";
    
    // When the path_parent function is called
    char *own_parent1 = ar_string_path_parent(ref_path1, '.');
    char *own_parent2 = ar_string_path_parent(ref_path2, '.');
    char *own_parent3 = ar_string_path_parent(ref_path3, '.');
    
    // Then the parents should match the expected values
    assert(own_parent1 != NULL);
    assert(own_parent2 != NULL);
    assert(own_parent3 != NULL);
    assert(strcmp(own_parent1, "key") == 0);
    assert(strcmp(own_parent2, "key.sub_key") == 0);
    assert(strcmp(own_parent3, "key.sub_key.sub_sub_key") == 0);
    
    // Clean up allocated memory
    free(own_parent1);
    free(own_parent2);
    free(own_parent3);
    
    printf("ar_string_path_parent() normal paths test passed!\n");
}

static void test_path_parent_edge_cases(void) {
    printf("Testing ar_string_path_parent() with edge cases...\n");
    
    // Given edge case paths
    const char *ref_empty = "";
    const char *ref_null_ptr = NULL;
    const char *ref_single_segment = "key";
    const char *ref_leading_separator = ".key.sub_key";
    const char *ref_trailing_separator = "key.sub_key.";
    const char *ref_only_separators = ".....";
    
    // When the path_parent function is called with edge cases
    char *own_empty_result = ar_string_path_parent(ref_empty, '.');
    char *own_null_result = ar_string_path_parent(ref_null_ptr, '.');
    char *own_single_segment_result = ar_string_path_parent(ref_single_segment, '.');
    char *own_leading_separator_result = ar_string_path_parent(ref_leading_separator, '.');
    char *own_trailing_separator_result = ar_string_path_parent(ref_trailing_separator, '.');
    char *own_only_separators_result = ar_string_path_parent(ref_only_separators, '.');
    
    // Then the results should be as expected
    assert(own_empty_result == NULL);
    assert(own_null_result == NULL);
    assert(own_single_segment_result == NULL);
    
    // For a path starting with a separator like ".key.sub_key"
    // The parent should be ".key"
    assert(own_leading_separator_result != NULL);
    assert(strcmp(own_leading_separator_result, ".key") == 0);
    
    // For a path ending with a separator like "key.sub_key."
    // The parent should be "key.sub_key" (not "key" since our implementation keeps all segments)
    assert(own_trailing_separator_result != NULL);
    assert(strcmp(own_trailing_separator_result, "key.sub_key") == 0);
    
    // For a path with only separators, the parent of "....." would be "...."
    assert(own_only_separators_result != NULL);
    assert(strcmp(own_only_separators_result, "....") == 0);
    
    // Clean up allocated memory
    free(own_leading_separator_result);
    free(own_trailing_separator_result);
    free(own_only_separators_result);
    
    printf("ar_string_path_parent() edge cases test passed!\n");
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
    test_path_parent_normal();
    test_path_parent_edge_cases();
    
    printf("All string tests passed!\n");
    return 0;
}
