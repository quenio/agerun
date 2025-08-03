# Test Isolation Through Strategic Commenting

## Learning
When debugging test interactions or isolating error sources, temporarily commenting out tests can reveal dependencies and pinpoint which test is causing issues. This is a valid debugging strategy that helps identify test interference and unexpected side effects.

## Importance
Tests can interact in unexpected ways through shared state, timing issues, or resource conflicts. Strategic commenting allows systematic isolation of problematic tests without deleting code or losing test coverage permanently.

## Example
```c
// Debugging INTEGER message corruption in method_creator_tests
int main(void) {
    printf("Running method-creator method tests...\n\n");
    
    // Temporarily disabled to check if INTEGER message errors come from other tests
    // test_method_creator_create_simple();
    test_method_creator_invalid_syntax();
    
    printf("\nAll method-creator method tests passed!\n");
    return 0;
}

// After isolating the issue, uncomment and fix:
int main(void) {
    printf("Running method-creator method tests...\n\n");
    
    test_method_creator_create_simple();  // Uncommented after fixing compile() bug
    test_method_creator_invalid_syntax();
    
    printf("\nAll method-creator method tests passed!\n");
    return 0;
}
```

## Generalization
Test isolation strategy:
1. **Identify symptoms**: Note error messages and which tests are involved
2. **Comment systematically**: Disable tests one at a time or in groups
3. **Run and observe**: Check if errors disappear with specific tests disabled
4. **Narrow down**: Re-enable tests gradually to find the culprit
5. **Fix root cause**: Address the underlying issue
6. **Uncomment all tests**: Ensure full coverage before committing

## Implementation
```c
// Pattern for test isolation with clear documentation
void run_test_suite(void) {
    // Core functionality tests
    test_basic_creation();
    test_basic_destruction();
    
    // DEBUGGING: Temporarily disabled - causes memory corruption
    // TODO: Fix interaction with test_advanced_feature()
    // test_edge_case_handling();
    
    // Advanced feature tests
    test_advanced_feature();
    
    // DEBUGGING: Re-enable after fixing edge case handling
    // test_integration_scenario();
}

// Shell script for systematic test isolation
#!/bin/bash
# test_isolate.sh - Help isolate failing tests

TEST_FILE=$1
TESTS=$(grep -o "test_[a-zA-Z0-9_]*(" "$TEST_FILE" | sed 's/($//')

for test in $TESTS; do
    echo "Running with only: $test"
    # Comment out all tests except current one
    sed -i.bak "s/^[[:space:]]*\(test_.*\);/\/\/ \1;/" "$TEST_FILE"
    sed -i "s/^[[:space:]]*\/\/ \($test\);/\1;/" "$TEST_FILE"
    
    if make "${TEST_FILE%.c}" 2>&1 | grep -q "ERROR"; then
        echo "  -> FAILS"
    else
        echo "  -> PASSES"
    fi
done

# Restore original
mv "$TEST_FILE.bak" "$TEST_FILE"
```

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Test Execution Order Dependencies](test-execution-order-dependencies.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)