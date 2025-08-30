# Test Effectiveness Verification

## Learning
Always verify that new tests actually detect failures by temporarily breaking the implementation in the way the test is designed to catch. A test that never fails provides false confidence.

## Importance
Tests exist to catch regressions and errors. If a test passes even when the implementation is broken, it's not testing what we think it is. Verification ensures tests provide real protection against future bugs.

## Example
```c
// Testing error handling for save failures
static void test_executable__continues_on_save_failure(ar_executable_fixture_t *mut_fixture) {
    // ... test setup ...
    
    // STEP 1: Write test expecting graceful failure handling
    AR_ASSERT(exit_status == 0, "Should exit successfully despite save failure");
    AR_ASSERT(found_warning, "Should see warning about save failure");
}

// STEP 2: Temporarily break implementation to verify test catches it
// Change from warning + continue:
if (ar_methodology__save_methods_with_instance(mut_methodology, METHODOLOGY_FILE_NAME)) {
    printf("Methodology saved to %s\n", METHODOLOGY_FILE_NAME);
} else {
    printf("Warning: Failed to save methodology to %s\n", METHODOLOGY_FILE_NAME);
}

// To error + exit:
if (!ar_methodology__save_methods_with_instance(mut_methodology, METHODOLOGY_FILE_NAME)) {
    printf("Error: Failed to save methodology to %s\n", METHODOLOGY_FILE_NAME);
    return 1;  // Exit with error - test should now FAIL
}

// STEP 3: Verify test fails with broken implementation
// Exit status: 512 (expecting 0)  ✗ Test correctly detects the failure!

// STEP 4: Restore correct implementation and verify test passes
```

## Generalization
For every new test, especially those testing error conditions:
1. Write the test expecting specific behavior
2. Temporarily modify implementation to violate that expectation
3. Verify the test fails with clear indication of what's wrong
4. Restore correct implementation and verify test passes
5. This meta-testing ensures test quality

## Related Patterns
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md)
- [DLSym Test Interception Technique](dlsym-test-interception-technique.md)

## Implementation
```bash
# Workflow for verifying test effectiveness
1. make test_name 2>&1  # Should pass with correct implementation
2. # Edit code to break in specific way
3. make test_name 2>&1  # Should FAIL if test is effective
4. # Check failure message is clear and specific
5. # Restore correct implementation
6. make test_name 2>&1  # Should pass again
```

## Related Patterns
- [Test Expectation Reality Alignment](test-expectation-reality-alignment.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Test Completeness Enumeration](test-completeness-enumeration.md)