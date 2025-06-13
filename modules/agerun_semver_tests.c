#include "agerun_semver.h"
#include <stdio.h>
#include <assert.h>

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s\n", message); \
            assert(condition); \
        } \
    } while(0)

/* Function prototypes */
static void test_semver_parse(void);
static void test_semver_compare(void);
static void test_semver_are_compatible(void);
static void test_semver_matches_pattern(void);
static void test_semver_find_latest_matching(void);

static void test_semver_parse(void) {
    printf("Testing semver parsing...\n");
    
    // Full version tests
    int major, minor, patch;
    
    // Test 1: Parse full version
    TEST_ASSERT(ar__semver__parse("1.2.3", &major, &minor, &patch), "Failed to parse valid version");
    TEST_ASSERT(major == 1, "Major version parsing failed");
    TEST_ASSERT(minor == 2, "Minor version parsing failed");
    TEST_ASSERT(patch == 3, "Patch version parsing failed");
    
    // Test 2: Parse major only
    TEST_ASSERT(ar__semver__parse("5", &major, &minor, &patch), "Failed to parse major-only version");
    TEST_ASSERT(major == 5, "Major-only version parsing failed");
    TEST_ASSERT(minor == 0, "Minor should default to 0");
    TEST_ASSERT(patch == 0, "Patch should default to 0");
    
    // Test 3: Parse major.minor
    TEST_ASSERT(ar__semver__parse("2.10", &major, &minor, &patch), "Failed to parse major.minor version");
    TEST_ASSERT(major == 2, "Major version parsing failed for major.minor");
    TEST_ASSERT(minor == 10, "Minor version parsing failed for major.minor");
    TEST_ASSERT(patch == 0, "Patch should default to 0 for major.minor");
    
    // Test 4: Handle invalid version
    TEST_ASSERT(!ar__semver__parse("invalid", &major, &minor, &patch), "Should fail on invalid input");
    TEST_ASSERT(!ar__semver__parse("1.2.3.4", &major, &minor, &patch), "Should handle extra components");
    TEST_ASSERT(!ar__semver__parse("", &major, &minor, &patch), "Should handle empty string");
    TEST_ASSERT(!ar__semver__parse(NULL, &major, &minor, &patch), "Should handle NULL input");
    
    // Test 5: Handle negative numbers
    TEST_ASSERT(!ar__semver__parse("-1.2.3", &major, &minor, &patch), "Should reject negative major");
    
    printf("Semver parsing tests passed.\n");
}

static void test_semver_compare(void) {
    printf("Testing semver comparison...\n");
    
    // Test 1: Equal versions
    TEST_ASSERT(ar__semver__compare("1.2.3", "1.2.3") == 0, "Equal versions should return 0");
    
    // Test 2: Compare major versions
    TEST_ASSERT(ar__semver__compare("2.0.0", "1.9.9") > 0, "2.0.0 should be > 1.9.9");
    TEST_ASSERT(ar__semver__compare("1.0.0", "2.0.0") < 0, "1.0.0 should be < 2.0.0");
    
    // Test 3: Compare minor versions (same major)
    TEST_ASSERT(ar__semver__compare("1.2.0", "1.1.9") > 0, "1.2.0 should be > 1.1.9");
    TEST_ASSERT(ar__semver__compare("1.1.0", "1.2.0") < 0, "1.1.0 should be < 1.2.0");
    
    // Test 4: Compare patch versions (same major.minor)
    TEST_ASSERT(ar__semver__compare("1.2.3", "1.2.2") > 0, "1.2.3 should be > 1.2.2");
    TEST_ASSERT(ar__semver__compare("1.2.2", "1.2.3") < 0, "1.2.2 should be < 1.2.3");
    
    // Test 5: Compare partial versions
    TEST_ASSERT(ar__semver__compare("1.2", "1.1") > 0, "1.2 should be > 1.1");
    TEST_ASSERT(ar__semver__compare("1", "2") < 0, "1 should be < 2");
    
    // Test 6: Special cases
    TEST_ASSERT(ar__semver__compare(NULL, NULL) == 0, "NULL == NULL should be true");
    TEST_ASSERT(ar__semver__compare("1.0.0", NULL) > 0, "Any version > NULL");
    TEST_ASSERT(ar__semver__compare(NULL, "1.0.0") < 0, "NULL < Any version");
    
    // Test 7: Invalid versions
    TEST_ASSERT(ar__semver__compare("invalid", "also_invalid") != 0, "Invalid versions should use string comparison");
    TEST_ASSERT(ar__semver__compare("1.0.0", "invalid") > 0, "Valid > Invalid");
    TEST_ASSERT(ar__semver__compare("invalid", "1.0.0") < 0, "Invalid < Valid");
    
    printf("Semver comparison tests passed.\n");
}

static void test_semver_are_compatible(void) {
    printf("Testing semver compatibility...\n");
    
    // Test 1: Same major version
    TEST_ASSERT(ar__semver__are_compatible("1.0.0", "1.9.9"), "1.0.0 should be compatible with 1.9.9");
    TEST_ASSERT(ar__semver__are_compatible("1.2.3", "1.0.0"), "1.2.3 should be compatible with 1.0.0");
    
    // Test 2: Different major versions
    TEST_ASSERT(!ar__semver__are_compatible("1.0.0", "2.0.0"), "1.0.0 should not be compatible with 2.0.0");
    TEST_ASSERT(!ar__semver__are_compatible("2.0.0", "1.0.0"), "2.0.0 should not be compatible with 1.0.0");
    
    // Test 3: Partial versions
    TEST_ASSERT(ar__semver__are_compatible("1", "1.2.3"), "1 should be compatible with 1.2.3");
    TEST_ASSERT(ar__semver__are_compatible("1.2", "1.2.3"), "1.2 should be compatible with 1.2.3");
    
    // Test 4: Special cases
    TEST_ASSERT(!ar__semver__are_compatible(NULL, "1.0.0"), "NULL should not be compatible with any version");
    TEST_ASSERT(!ar__semver__are_compatible("1.0.0", NULL), "Any version should not be compatible with NULL");
    TEST_ASSERT(!ar__semver__are_compatible(NULL, NULL), "NULL should not be compatible with NULL");
    
    // Test 5: Invalid versions
    TEST_ASSERT(!ar__semver__are_compatible("invalid", "1.0.0"), "Invalid version should not be compatible");
    TEST_ASSERT(!ar__semver__are_compatible("1.0.0", "invalid"), "Invalid version should not be compatible");
    
    printf("Semver compatibility tests passed.\n");
}

static void test_semver_matches_pattern(void) {
    printf("Testing semver pattern matching...\n");
    
    // Test 1: Exact matches
    TEST_ASSERT(ar__semver__matches_pattern("1.2.3", "1.2.3"), "1.2.3 should match 1.2.3");
    
    // Test 2: Partial patterns
    TEST_ASSERT(ar__semver__matches_pattern("1.2.3", "1"), "1.2.3 should match pattern 1");
    TEST_ASSERT(ar__semver__matches_pattern("1.2.3", "1.2"), "1.2.3 should match pattern 1.2");
    TEST_ASSERT(!ar__semver__matches_pattern("1.2.3", "2"), "1.2.3 should not match pattern 2");
    TEST_ASSERT(!ar__semver__matches_pattern("1.2.3", "1.3"), "1.2.3 should not match pattern 1.3");
    
    // Test 3: Special cases
    TEST_ASSERT(!ar__semver__matches_pattern(NULL, "1"), "NULL should not match any pattern");
    TEST_ASSERT(!ar__semver__matches_pattern("1.2.3", NULL), "Any version should not match NULL pattern");
    TEST_ASSERT(!ar__semver__matches_pattern(NULL, NULL), "NULL should not match NULL pattern");
    
    // Test 4: Invalid versions
    TEST_ASSERT(!ar__semver__matches_pattern("invalid", "1"), "Invalid version should not match any pattern");
    TEST_ASSERT(!ar__semver__matches_pattern("1.2.3", "invalid"), "Any version should not match invalid pattern");
    
    printf("Semver pattern matching tests passed.\n");
}

static void test_semver_find_latest_matching(void) {
    printf("Testing semver find latest matching...\n");
    
    // Test 1: Find latest matching version
    const char *versions[] = {"1.0.0", "1.1.0", "1.2.0", "2.0.0", "2.1.0"};
    int count = sizeof(versions) / sizeof(versions[0]);
    
    TEST_ASSERT(ar__semver__find_latest_matching(versions, count, "1") == 2, "Should find 1.2.0 as latest 1.x.x");
    TEST_ASSERT(ar__semver__find_latest_matching(versions, count, "2") == 4, "Should find 2.1.0 as latest 2.x.x");
    TEST_ASSERT(ar__semver__find_latest_matching(versions, count, "1.1") == 1, "Should find 1.1.0 as latest 1.1.x");
    
    // Test 2: No matching versions
    TEST_ASSERT(ar__semver__find_latest_matching(versions, count, "3") == -1, "Should return -1 for no matches");
    
    // Test 3: Handle NULL entries
    const char *with_nulls[] = {"1.0.0", NULL, "1.2.0", "2.0.0", NULL};
    int with_nulls_count = sizeof(with_nulls) / sizeof(with_nulls[0]);
    
    TEST_ASSERT(ar__semver__find_latest_matching(with_nulls, with_nulls_count, "1") == 2, "Should skip NULL entries");
    
    // Test 4: Special cases
    TEST_ASSERT(ar__semver__find_latest_matching(NULL, 5, "1") == -1, "Should handle NULL array");
    TEST_ASSERT(ar__semver__find_latest_matching(versions, 0, "1") == -1, "Should handle zero count");
    TEST_ASSERT(ar__semver__find_latest_matching(versions, count, NULL) == -1, "Should handle NULL pattern");
    
    printf("Semver find latest matching tests passed.\n");
}

int main(void) {
    printf("Starting semver tests...\n");
    
    test_semver_parse();
    test_semver_compare();
    test_semver_are_compatible();
    test_semver_matches_pattern();
    test_semver_find_latest_matching();
    
    printf("All semver tests passed!\n");
    return 0;
}
