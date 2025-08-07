# Test Standardization Retroactive Application

## Learning
When discovering better testing patterns, systematically apply them to all existing tests, not just new ones, to maintain consistency and quality across the entire test suite.

## Importance
Inconsistent test patterns create maintenance burden, confuse developers, and lead to copy-paste of outdated patterns. Retroactive standardization ensures all tests benefit from improvements and serve as good examples for future development.

## Example
```c
// Session progression showing retroactive application:

// 1. Discovered AR_ASSERT in Cycle 3
static void test_no_auto_saving_on_shutdown(void) {
    // New test using AR_ASSERT
    AR_ASSERT(!methodology_exists, "methodology.agerun should NOT have been saved");
}

// 2. User feedback: "Let's also make sure the tests of previous cycles follow these guidelines"

// 3. Went back to update Cycle 2 test
static void test_no_auto_loading_on_init(void) {
    // Before: Used plain assert()
    // assert(original_stdout != -1);
    
    // After: Updated to AR_ASSERT
    AR_ASSERT(original_stdout != -1, "Could not duplicate stdout for capture");
    AR_ASSERT(!loading_attempted, "System should NOT attempt to load files during init");
}

// 4. Also updated Cycle 1 test
static void test_single_session(void) {
    // Before: Mixed assert() calls
    // assert(getcwd(cwd, sizeof(cwd)) != NULL);
    
    // After: Consistent AR_ASSERT usage
    AR_ASSERT(getcwd(cwd, sizeof(cwd)) != NULL, "Should be able to get current directory");
    AR_ASSERT(!found_second_session, "Executable should only have one session, not two");
}
```

## Generalization
Retroactive standardization process:
1. **Identify the improvement** - New pattern, tool, or practice
2. **Find all affected code** - Use grep to locate old patterns
3. **Update systematically** - Work through files methodically
4. **Verify each change** - Run tests after each update
5. **Commit as refactoring** - "Standardize X across all tests"

## Implementation
```bash
# Step 1: Find all files using old pattern
grep -r "assert(" modules/*_tests.c | cut -d: -f1 | sort -u

# Step 2: For each file, update systematically
# Example: Converting assert() to AR_ASSERT()

# Before:
assert(ptr != NULL);
assert(count > 0);
assert(status == 0);

# After:
AR_ASSERT(ptr != NULL, "Pointer should be valid after allocation");
AR_ASSERT(count > 0, "Count should be positive after insertion");
AR_ASSERT(status == 0, "Operation should succeed");

# Step 3: Update includes if needed
#include "ar_assert.h"  // Add if not present

# Step 4: Run tests after each file
make test_name 2>&1

# Step 5: Commit the standardization
git commit -m "refactor(tests): standardize assertions across all test files"
```

Common patterns to retroactively apply:
- **Assertion macros**: assert() → AR_ASSERT()
- **BDD structure**: Add Given/When/Then comments
- **Error reporting**: Add diagnostic printf before assertions
- **Resource cleanup**: Ensure consistent cleanup patterns
- **Naming conventions**: Standardize test function names

## Related Patterns
- [AR_ASSERT for Descriptive Failures](ar-assert-descriptive-failures.md)
- [BDD Test Structure](bdd-test-structure.md)
- [Test Signal Reporting Practice](test-signal-reporting-practice.md)
- [Systematic Function Renaming Pattern](systematic-function-renaming-pattern.md)