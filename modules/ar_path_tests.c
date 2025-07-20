/**
 * @file ar_path_tests.c
 * @brief Test suite for the ar_path module
 */

#include "ar_path.h"
#include "ar_heap.h"
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static void test_path__create_destroy(void) {
    printf("Testing ar_path create/destroy...\n");
    
    // Given a simple path string
    const char *path_str = "memory.user.name";
    
    // When creating a path with '.' separator
    ar_path_t *own_path = ar_path__create(path_str, '.');
    
    // Then the path should be created successfully
    assert(own_path != NULL);
    
    // And we can get the original string back
    const char *ref_str = ar_path__get_string(own_path);
    assert(ref_str != NULL);
    assert(strcmp(ref_str, path_str) == 0);
    
    // And we can get the separator
    char separator = ar_path__get_separator(own_path);
    assert(separator == '.');
    
    // When destroying the path
    ar_path__destroy(own_path);
    
    // Then no memory leaks should occur (verified by heap tracking)
}

static void test_path__create_variable(void) {
    printf("Testing ar_path create_variable...\n");
    
    // Given variable paths
    const char *memory_path = "memory.x.y";
    const char *context_path = "context.user.id";
    const char *message_path = "message.type";
    
    // When creating variable paths
    ar_path_t *own_mem_path = ar_path__create_variable(memory_path);
    ar_path_t *own_ctx_path = ar_path__create_variable(context_path);
    ar_path_t *own_msg_path = ar_path__create_variable(message_path);
    
    // Then all should be created successfully
    assert(own_mem_path != NULL);
    assert(own_ctx_path != NULL);
    assert(own_msg_path != NULL);
    
    // And separator should be '.' for all
    assert(ar_path__get_separator(own_mem_path) == '.');
    assert(ar_path__get_separator(own_ctx_path) == '.');
    assert(ar_path__get_separator(own_msg_path) == '.');
    
    // Clean up
    ar_path__destroy(own_mem_path);
    ar_path__destroy(own_ctx_path);
    ar_path__destroy(own_msg_path);
}

static void test_path__create_file(void) {
    printf("Testing ar_path create_file...\n");
    
    // Given a file path
    const char *file_path = "/usr/local/bin/program";
    
    // When creating a file path
    ar_path_t *own_path = ar_path__create_file(file_path);
    
    // Then it should be created successfully
    assert(own_path != NULL);
    
    // And separator should be '/'
    assert(ar_path__get_separator(own_path) == '/');
    
    // And we can get the string back
    assert(strcmp(ar_path__get_string(own_path), file_path) == 0);
    
    // Clean up
    ar_path__destroy(own_path);
}

static void test_path__create_with_null(void) {
    printf("Testing ar_path create with NULL...\n");
    
    // When creating paths with NULL strings
    ar_path_t *own_path1 = ar_path__create(NULL, '.');
    ar_path_t *own_path2 = ar_path__create_variable(NULL);
    ar_path_t *own_path3 = ar_path__create_file(NULL);
    
    // Then all should return NULL
    assert(own_path1 == NULL);
    assert(own_path2 == NULL);
    assert(own_path3 == NULL);
}

static void test_path__destroy_null(void) {
    printf("Testing ar_path destroy with NULL...\n");
    
    // When destroying NULL
    ar_path__destroy(NULL);
    
    // Then it should not crash (no assertion needed)
}

static void test_path__get_segment_count(void) {
    printf("Testing ar_path get_segment_count...\n");
    
    // Given various paths
    ar_path_t *own_path1 = ar_path__create("memory.user.name", '.');
    ar_path_t *own_path2 = ar_path__create("single", '.');
    ar_path_t *own_path3 = ar_path__create("/usr/local/bin/program", '/');
    ar_path_t *own_path4 = ar_path__create("", '.');
    ar_path_t *own_path5 = ar_path__create("a.b.c.d.e", '.');
    
    // When getting segment counts
    size_t count1 = ar_path__get_segment_count(own_path1);
    size_t count2 = ar_path__get_segment_count(own_path2);
    size_t count3 = ar_path__get_segment_count(own_path3);
    size_t count4 = ar_path__get_segment_count(own_path4);
    size_t count5 = ar_path__get_segment_count(own_path5);
    
    // Then counts should be correct
    assert(count1 == 3);  // memory, user, name
    assert(count2 == 1);  // single
    assert(count3 == 5);  // empty, usr, local, bin, program
    assert(count4 == 0);  // empty string has 0 segments
    assert(count5 == 5);  // a, b, c, d, e
    
    // Clean up
    ar_path__destroy(own_path1);
    ar_path__destroy(own_path2);
    ar_path__destroy(own_path3);
    ar_path__destroy(own_path4);
    ar_path__destroy(own_path5);
}

static void test_path__get_segment_count_edge_cases(void) {
    printf("Testing ar_path get_segment_count edge cases...\n");
    
    // Given edge case paths (matching ar_string tests)
    ar_path_t *own_only_separators = ar_path__create(".....", '.');
    ar_path_t *own_trailing_separator = ar_path__create("key.sub_key.", '.');
    ar_path_t *own_leading_separator = ar_path__create(".key.sub_key", '.');
    
    // When getting segment counts
    size_t only_separators_count = ar_path__get_segment_count(own_only_separators);
    size_t trailing_count = ar_path__get_segment_count(own_trailing_separator);
    size_t leading_count = ar_path__get_segment_count(own_leading_separator);
    
    // Then counts should match ar_string behavior
    assert(only_separators_count == 6);  // 5 separators = 6 segments
    assert(trailing_count == 3);          // 2 keys + empty segment
    assert(leading_count == 3);           // empty + 2 keys
    
    // Clean up
    ar_path__destroy(own_only_separators);
    ar_path__destroy(own_trailing_separator);
    ar_path__destroy(own_leading_separator);
}

static void test_path__get_segment(void) {
    printf("Testing ar_path get_segment...\n");
    
    // Given a path
    ar_path_t *own_path = ar_path__create("memory.user.profile.name", '.');
    
    // When getting segments
    const char *segment0 = ar_path__get_segment(own_path, 0);
    const char *segment1 = ar_path__get_segment(own_path, 1);
    const char *segment2 = ar_path__get_segment(own_path, 2);
    const char *segment3 = ar_path__get_segment(own_path, 3);
    const char *segment_invalid = ar_path__get_segment(own_path, 4);
    
    // Then segments should be correct
    assert(segment0 != NULL);
    assert(strcmp(segment0, "memory") == 0);
    assert(segment1 != NULL);
    assert(strcmp(segment1, "user") == 0);
    assert(segment2 != NULL);
    assert(strcmp(segment2, "profile") == 0);
    assert(segment3 != NULL);
    assert(strcmp(segment3, "name") == 0);
    assert(segment_invalid == NULL);  // Out of bounds
    
    // Test that segments are borrowed (not new allocations)
    const char *segment0_again = ar_path__get_segment(own_path, 0);
    assert(segment0 == segment0_again);  // Same pointer
    
    // Clean up
    ar_path__destroy(own_path);
}

static void test_path__get_segment_edge_cases(void) {
    printf("Testing ar_path get_segment edge cases...\n");
    
    // Given a path with empty segments (matching ar_string test)
    ar_path_t *own_path = ar_path__create("..key..end.", '.');
    
    // When getting segments
    const char *seg0 = ar_path__get_segment(own_path, 0);
    const char *seg1 = ar_path__get_segment(own_path, 1);
    const char *seg2 = ar_path__get_segment(own_path, 2);
    const char *seg3 = ar_path__get_segment(own_path, 3);
    const char *seg4 = ar_path__get_segment(own_path, 4);
    const char *seg5 = ar_path__get_segment(own_path, 5);
    
    // Then segments should match ar_string behavior
    assert(seg0 != NULL && strcmp(seg0, "") == 0);    // First empty
    assert(seg1 != NULL && strcmp(seg1, "") == 0);    // Second empty
    assert(seg2 != NULL && strcmp(seg2, "key") == 0); // "key"
    assert(seg3 != NULL && strcmp(seg3, "") == 0);    // Third empty
    assert(seg4 != NULL && strcmp(seg4, "end") == 0); // "end"
    assert(seg5 != NULL && strcmp(seg5, "") == 0);    // Trailing empty
    
    // Clean up
    ar_path__destroy(own_path);
}


static void test_path__get_segment_with_null(void) {
    printf("Testing ar_path get_segment with NULL...\n");
    
    // When getting segment count/segments from NULL
    size_t count = ar_path__get_segment_count(NULL);
    const char *segment = ar_path__get_segment(NULL, 0);
    
    // Then all should handle NULL gracefully
    assert(count == 0);
    assert(segment == NULL);
}

static void test_path__get_parent(void) {
    printf("Testing ar_path get_parent...\n");
    
    // Given various paths
    ar_path_t *own_path1 = ar_path__create("memory.user.profile.name", '.');
    ar_path_t *own_path2 = ar_path__create("memory.user", '.');
    ar_path_t *own_path3 = ar_path__create("memory", '.');
    ar_path_t *own_path4 = ar_path__create("/usr/local/bin", '/');
    ar_path_t *own_path5 = ar_path__create("", '.');
    
    // When getting parent paths
    ar_path_t *own_parent1 = ar_path__get_parent(own_path1);
    ar_path_t *own_parent2 = ar_path__get_parent(own_path2);
    ar_path_t *own_parent3 = ar_path__get_parent(own_path3);
    ar_path_t *own_parent4 = ar_path__get_parent(own_path4);
    ar_path_t *own_parent5 = ar_path__get_parent(own_path5);
    
    // Then parent paths should be correct
    assert(own_parent1 != NULL);
    assert(strcmp(ar_path__get_string(own_parent1), "memory.user.profile") == 0);
    assert(ar_path__get_segment_count(own_parent1) == 3);
    
    assert(own_parent2 != NULL);
    assert(strcmp(ar_path__get_string(own_parent2), "memory") == 0);
    assert(ar_path__get_segment_count(own_parent2) == 1);
    
    assert(own_parent3 == NULL);  // No parent for single segment
    assert(own_parent4 != NULL);
    assert(strcmp(ar_path__get_string(own_parent4), "/usr/local") == 0);
    assert(own_parent5 == NULL);  // No parent for empty path
    
    // Clean up
    ar_path__destroy(own_path1);
    ar_path__destroy(own_path2);
    ar_path__destroy(own_path3);
    ar_path__destroy(own_path4);
    ar_path__destroy(own_path5);
    ar_path__destroy(own_parent1);
    ar_path__destroy(own_parent2);
    ar_path__destroy(own_parent4);
}

static void test_path__get_parent_with_null(void) {
    printf("Testing ar_path get_parent with NULL...\n");
    
    // When getting parent of NULL
    ar_path_t *parent = ar_path__get_parent(NULL);
    
    // Then should return NULL
    assert(parent == NULL);
}

static void test_path__get_parent_edge_cases(void) {
    printf("Testing ar_path get_parent edge cases...\n");
    
    // Given edge case paths (matching ar_string tests)
    ar_path_t *own_leading_separator = ar_path__create(".key.sub_key", '.');
    ar_path_t *own_trailing_separator = ar_path__create("key.sub_key.", '.');
    ar_path_t *own_only_separators = ar_path__create(".....", '.');
    
    // When getting parent paths
    ar_path_t *own_parent_leading = ar_path__get_parent(own_leading_separator);
    ar_path_t *own_parent_trailing = ar_path__get_parent(own_trailing_separator);
    ar_path_t *own_parent_separators = ar_path__get_parent(own_only_separators);
    
    // Then results should match ar_string behavior
    assert(own_parent_leading != NULL);
    assert(strcmp(ar_path__get_string(own_parent_leading), ".key") == 0);
    
    assert(own_parent_trailing != NULL);
    assert(strcmp(ar_path__get_string(own_parent_trailing), "key.sub_key") == 0);
    
    assert(own_parent_separators != NULL);
    assert(strcmp(ar_path__get_string(own_parent_separators), "....") == 0);
    
    // Clean up
    ar_path__destroy(own_leading_separator);
    ar_path__destroy(own_trailing_separator);
    ar_path__destroy(own_only_separators);
    ar_path__destroy(own_parent_leading);
    ar_path__destroy(own_parent_trailing);
    ar_path__destroy(own_parent_separators);
}

static void test_path__starts_with(void) {
    printf("Testing ar_path starts_with...\n");
    
    // Given various paths
    ar_path_t *own_path1 = ar_path__create_variable("memory.user.name");
    ar_path_t *own_path2 = ar_path__create_variable("context.request.id");
    ar_path_t *own_path3 = ar_path__create_file("/usr/local/bin/program");
    ar_path_t *own_path4 = ar_path__create_variable("memory");
    
    // When checking if paths start with prefixes
    bool starts1 = ar_path__starts_with(own_path1, "memory");
    bool starts2 = ar_path__starts_with(own_path1, "memory.user");
    bool starts3 = ar_path__starts_with(own_path1, "memory.user.name");
    bool starts4 = ar_path__starts_with(own_path1, "context");
    bool starts5 = ar_path__starts_with(own_path1, "memory.user.name.extra");
    
    bool starts6 = ar_path__starts_with(own_path2, "context");
    bool starts7 = ar_path__starts_with(own_path2, "context.request");
    
    bool starts8 = ar_path__starts_with(own_path3, "/usr");
    bool starts9 = ar_path__starts_with(own_path3, "/usr/local");
    bool starts10 = ar_path__starts_with(own_path3, "/opt");
    
    bool starts11 = ar_path__starts_with(own_path4, "memory");
    bool starts12 = ar_path__starts_with(own_path4, "mem");
    
    // Then results should be correct
    assert(starts1 == true);   // memory.user.name starts with memory
    assert(starts2 == true);   // memory.user.name starts with memory.user
    assert(starts3 == true);   // memory.user.name starts with memory.user.name
    assert(starts4 == false);  // memory.user.name does not start with context
    assert(starts5 == false);  // memory.user.name does not start with memory.user.name.extra
    
    assert(starts6 == true);   // context.request.id starts with context
    assert(starts7 == true);   // context.request.id starts with context.request
    
    assert(starts8 == true);   // /usr/local/bin/program starts with /usr
    assert(starts9 == true);   // /usr/local/bin/program starts with /usr/local
    assert(starts10 == false); // /usr/local/bin/program does not start with /opt
    
    assert(starts11 == true);  // memory starts with memory
    assert(starts12 == false); // memory does not start with mem (partial segment match)
    
    // Clean up
    ar_path__destroy(own_path1);
    ar_path__destroy(own_path2);
    ar_path__destroy(own_path3);
    ar_path__destroy(own_path4);
}

static void test_path__starts_with_null(void) {
    printf("Testing ar_path starts_with with NULL...\n");
    
    ar_path_t *own_path = ar_path__create_variable("memory.user");
    
    // When checking with NULL values
    bool result1 = ar_path__starts_with(NULL, "memory");
    bool result2 = ar_path__starts_with(own_path, NULL);
    bool result3 = ar_path__starts_with(NULL, NULL);
    
    // Then all should return false
    assert(result1 == false);
    assert(result2 == false);
    assert(result3 == false);
    
    ar_path__destroy(own_path);
}

static void test_path__variable_functions(void) {
    printf("Testing ar_path variable-specific functions...\n");
    
    // Given various paths
    ar_path_t *own_mem_path = ar_path__create_variable("memory.user.name");
    ar_path_t *own_ctx_path = ar_path__create_variable("context.request.id");
    ar_path_t *own_msg_path = ar_path__create_variable("message.type");
    ar_path_t *own_other_path = ar_path__create_variable("other.data");
    ar_path_t *own_file_path = ar_path__create_file("/usr/local/bin");
    ar_path_t *own_single = ar_path__create_variable("memory");
    
    // When getting variable roots
    const char *root1 = ar_path__get_variable_root(own_mem_path);
    const char *root2 = ar_path__get_variable_root(own_ctx_path);
    const char *root3 = ar_path__get_variable_root(own_msg_path);
    const char *root4 = ar_path__get_variable_root(own_other_path);
    const char *root5 = ar_path__get_variable_root(own_file_path);
    const char *root6 = ar_path__get_variable_root(own_single);
    
    // Then roots should be correct
    assert(root1 != NULL);
    assert(strcmp(root1, "memory") == 0);
    assert(root2 != NULL);
    assert(strcmp(root2, "context") == 0);
    assert(root3 != NULL);
    assert(strcmp(root3, "message") == 0);
    assert(root4 != NULL);
    assert(strcmp(root4, "other") == 0);
    assert(root5 != NULL);
    assert(strcmp(root5, "") == 0);  // File path has empty first segment
    assert(root6 != NULL);
    assert(strcmp(root6, "memory") == 0);
    
    // When checking path types
    bool is_mem1 = ar_path__is_memory_path(own_mem_path);
    bool is_mem2 = ar_path__is_memory_path(own_ctx_path);
    bool is_mem3 = ar_path__is_memory_path(own_single);
    
    bool is_ctx1 = ar_path__is_context_path(own_ctx_path);
    bool is_ctx2 = ar_path__is_context_path(own_mem_path);
    
    bool is_msg1 = ar_path__is_message_path(own_msg_path);
    bool is_msg2 = ar_path__is_message_path(own_mem_path);
    
    // Then type checks should be correct
    assert(is_mem1 == true);   // memory.user.name is memory path
    assert(is_mem2 == false);  // context.request.id is not memory path
    assert(is_mem3 == true);   // memory is memory path
    
    assert(is_ctx1 == true);   // context.request.id is context path
    assert(is_ctx2 == false);  // memory.user.name is not context path
    
    assert(is_msg1 == true);   // message.type is message path
    assert(is_msg2 == false);  // memory.user.name is not message path
    
    // Clean up
    ar_path__destroy(own_mem_path);
    ar_path__destroy(own_ctx_path);
    ar_path__destroy(own_msg_path);
    ar_path__destroy(own_other_path);
    ar_path__destroy(own_file_path);
    ar_path__destroy(own_single);
}

static void test_path__variable_functions_null(void) {
    printf("Testing ar_path variable functions with NULL...\n");
    
    // When calling with NULL
    const char *root = ar_path__get_variable_root(NULL);
    bool is_mem = ar_path__is_memory_path(NULL);
    bool is_ctx = ar_path__is_context_path(NULL);
    bool is_msg = ar_path__is_message_path(NULL);
    
    // Then all should handle NULL gracefully
    assert(root == NULL);
    assert(is_mem == false);
    assert(is_ctx == false);
    assert(is_msg == false);
}

static void test_path__join(void) {
    printf("Testing ar_path join...\n");
    
    // Given various paths
    ar_path_t *own_base1 = ar_path__create_variable("memory.user");
    ar_path_t *own_base2 = ar_path__create_file("/usr/local");
    ar_path_t *own_base3 = ar_path__create_variable("");
    ar_path_t *own_base4 = ar_path__create_variable("memory");
    
    // When joining paths
    ar_path_t *own_joined1 = ar_path__join(own_base1, "profile.name");
    ar_path_t *own_joined2 = ar_path__join(own_base2, "bin/program");
    ar_path_t *own_joined3 = ar_path__join(own_base3, "memory.user");
    ar_path_t *own_joined4 = ar_path__join(own_base4, "");
    ar_path_t *own_joined5 = ar_path__join(own_base1, "");
    
    // Then joined paths should be correct
    assert(own_joined1 != NULL);
    assert(strcmp(ar_path__get_string(own_joined1), "memory.user.profile.name") == 0);
    assert(ar_path__get_segment_count(own_joined1) == 4);
    
    assert(own_joined2 != NULL);
    assert(strcmp(ar_path__get_string(own_joined2), "/usr/local/bin/program") == 0);
    assert(ar_path__get_segment_count(own_joined2) == 5);
    
    assert(own_joined3 != NULL);
    assert(strcmp(ar_path__get_string(own_joined3), "memory.user") == 0);
    
    assert(own_joined4 != NULL);
    assert(strcmp(ar_path__get_string(own_joined4), "memory") == 0);
    
    assert(own_joined5 != NULL);
    assert(strcmp(ar_path__get_string(own_joined5), "memory.user") == 0);
    
    // Clean up
    ar_path__destroy(own_base1);
    ar_path__destroy(own_base2);
    ar_path__destroy(own_base3);
    ar_path__destroy(own_base4);
    ar_path__destroy(own_joined1);
    ar_path__destroy(own_joined2);
    ar_path__destroy(own_joined3);
    ar_path__destroy(own_joined4);
    ar_path__destroy(own_joined5);
}

static void test_path__join_with_null(void) {
    printf("Testing ar_path join with NULL...\n");
    
    ar_path_t *own_base = ar_path__create_variable("memory");
    
    // When joining with NULL
    ar_path_t *joined1 = ar_path__join(NULL, "user");
    ar_path_t *joined2 = ar_path__join(own_base, NULL);
    ar_path_t *joined3 = ar_path__join(NULL, NULL);
    
    // Then all should return NULL
    assert(joined1 == NULL);
    assert(joined2 == NULL);
    assert(joined3 == NULL);
    
    ar_path__destroy(own_base);
}

static void test_path__normalize(void) {
    printf("Testing ar_path normalize...\n");
    
    // Given paths with redundant separators
    ar_path_t *own_path1 = ar_path__create("memory..user...name", '.');
    ar_path_t *own_path2 = ar_path__create("//usr///local//bin/", '/');
    ar_path_t *own_path3 = ar_path__create(".", '.');
    ar_path_t *own_path4 = ar_path__create("memory.", '.');
    
    // When normalizing paths
    ar_path_t *own_norm1 = ar_path__normalize(own_path1);
    ar_path_t *own_norm2 = ar_path__normalize(own_path2);
    ar_path_t *own_norm3 = ar_path__normalize(own_path3);
    ar_path_t *own_norm4 = ar_path__normalize(own_path4);
    
    // Then normalized paths should have empty segments removed
    assert(own_norm1 != NULL);
    assert(strcmp(ar_path__get_string(own_norm1), "memory.user.name") == 0);
    assert(ar_path__get_segment_count(own_norm1) == 3);
    
    assert(own_norm2 != NULL);
    assert(strcmp(ar_path__get_string(own_norm2), "/usr/local/bin") == 0);
    assert(ar_path__get_segment_count(own_norm2) == 4);  // Leading / creates empty first segment
    
    assert(own_norm3 != NULL);
    assert(strcmp(ar_path__get_string(own_norm3), "") == 0);
    assert(ar_path__get_segment_count(own_norm3) == 0);
    
    assert(own_norm4 != NULL);
    assert(strcmp(ar_path__get_string(own_norm4), "memory") == 0);
    assert(ar_path__get_segment_count(own_norm4) == 1);
    
    // Clean up
    ar_path__destroy(own_path1);
    ar_path__destroy(own_path2);
    ar_path__destroy(own_path3);
    ar_path__destroy(own_path4);
    ar_path__destroy(own_norm1);
    ar_path__destroy(own_norm2);
    ar_path__destroy(own_norm3);
    ar_path__destroy(own_norm4);
}

static void test_path__normalize_with_null(void) {
    printf("Testing ar_path normalize with NULL...\n");
    
    // When normalizing NULL
    ar_path_t *normalized = ar_path__normalize(NULL);
    
    // Then should return NULL
    assert(normalized == NULL);
}

static void test_path__get_suffix_after_root(void) {
    printf("Testing ar_path get_suffix_after_root...\n");
    
    // Test 1: Simple case "memory.x" → "x"
    {
        ar_path_t *own_path = ar_path__create_variable("memory.x");
        assert(own_path != NULL);
        
        const char *suffix = ar_path__get_suffix_after_root(own_path);
        assert(suffix != NULL);
        assert(strcmp(suffix, "x") == 0);
        
        ar_path__destroy(own_path);
    }
    
    // Test 2: Nested case "memory.user.name" → "user.name"
    {
        ar_path_t *own_path = ar_path__create_variable("memory.user.name");
        assert(own_path != NULL);
        
        const char *suffix = ar_path__get_suffix_after_root(own_path);
        assert(suffix != NULL);
        assert(strcmp(suffix, "user.name") == 0);
        
        ar_path__destroy(own_path);
    }
    
    // Test 3: Different root "context.value" → "value"
    {
        ar_path_t *own_path = ar_path__create_variable("context.value");
        assert(own_path != NULL);
        
        const char *suffix = ar_path__get_suffix_after_root(own_path);
        assert(suffix != NULL);
        assert(strcmp(suffix, "value") == 0);
        
        ar_path__destroy(own_path);
    }
}

static void test_path__get_suffix_after_root_edge_cases(void) {
    printf("Testing ar_path get_suffix_after_root edge cases...\n");
    
    // Test 1: Single segment "memory" → NULL
    {
        ar_path_t *own_path = ar_path__create_variable("memory");
        assert(own_path != NULL);
        
        const char *suffix = ar_path__get_suffix_after_root(own_path);
        assert(suffix == NULL);
        
        ar_path__destroy(own_path);
    }
    
    // Test 2: Empty string → NULL
    {
        ar_path_t *own_path = ar_path__create_variable("");
        assert(own_path != NULL);
        
        const char *suffix = ar_path__get_suffix_after_root(own_path);
        assert(suffix == NULL);
        
        ar_path__destroy(own_path);
    }
    
    // Test 3: NULL path → NULL
    {
        const char *suffix = ar_path__get_suffix_after_root(NULL);
        assert(suffix == NULL);
    }
    
    // Test 4: Path with trailing dot "memory." → NULL
    {
        ar_path_t *own_path = ar_path__create_variable("memory.");
        assert(own_path != NULL);
        
        const char *suffix = ar_path__get_suffix_after_root(own_path);
        assert(suffix == NULL);
        
        ar_path__destroy(own_path);
    }
}

int main(void) {
    // Directory check
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if (!strstr(cwd, "/bin/") && !strstr(cwd, "/bin")) {
            fprintf(stderr, "ERROR: Tests must be run from the bin directory!\n");
            fprintf(stderr, "Current directory: %s\n", cwd);
            fprintf(stderr, "Please run from a bin directory\n");
            return 1;
        }
    }
    
    // Run tests
    test_path__create_destroy();
    test_path__create_variable();
    test_path__create_file();
    test_path__create_with_null();
    test_path__destroy_null();
    test_path__get_segment_count();
    test_path__get_segment_count_edge_cases();
    test_path__get_segment();
    test_path__get_segment_edge_cases();
    test_path__get_segment_with_null();
    test_path__get_parent();
    test_path__get_parent_with_null();
    test_path__get_parent_edge_cases();
    test_path__starts_with();
    test_path__starts_with_null();
    test_path__variable_functions();
    test_path__variable_functions_null();
    test_path__join();
    test_path__join_with_null();
    test_path__normalize();
    test_path__normalize_with_null();
    test_path__get_suffix_after_root();
    test_path__get_suffix_after_root_edge_cases();
    
    printf("All ar_path tests passed!\n");
    
    ar_heap__memory_report();
    return 0;
}