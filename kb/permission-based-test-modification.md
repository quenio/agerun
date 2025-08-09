# Permission-Based Test Modification

## Learning
During TDD, never modify test fixtures, test methods, or test expectations without explicit permission from the user. This is especially critical during the RED phase where tests must genuinely fail to validate that they're testing the right thing. Unauthorized test modifications break TDD discipline.

## Importance
- Maintains test integrity and TDD discipline
- Prevents circumventing failing tests with workarounds
- Ensures tests actually validate the intended behavior
- Preserves the RED→GREEN→REFACTOR cycle integrity
- Forces proper problem analysis instead of quick fixes

## Example
```c
// Scenario: Test is failing during RED phase of TDD

// WRONG: Changing test method without permission
// Original test method:
const char *method_body = "memory.got_wake := if(message = \"__wake__\", 1, 0)";

// Test fails with "Failed to create execution frame"
// Assistant changes to:
const char *method_body = "memory.ran := 1";  // UNAUTHORIZED CHANGE!

// User response: "You are changing the method again without asking my permission :("

// RIGHT: Ask for permission before any test modifications
// "The test is failing with 'Failed to create execution frame'. 
//  This appears to be because the method syntax is incorrect.
//  May I change the method from '=' to ':=' for proper assignment syntax?"

// After permission granted:
const char *method_body = "memory.got_wake := if(message = \"__wake__\", 1, 0)";  // Authorized fix

// BETTER: Investigate root cause instead of changing test
// "The test is failing with frame creation error. Let me investigate why
//  ar_frame__create is failing rather than changing the test method."
// Discovery: ar_system is passing NULL context to agents
// Solution: Fix ar_system instead of modifying test
```

## Generalization
Test modification protocol during TDD:
1. Test fails → Investigate root cause first
2. If test itself is wrong → Ask permission with justification
3. If code under test is wrong → Fix the code, not the test
4. Document reason for any authorized test changes
5. Never modify tests to make them pass artificially

## Implementation
```c
// Pattern for handling test failures:

void handle_test_failure(const char *error_message) {
    // Step 1: Investigate root cause
    printf("Test failed with: %s\n", error_message);
    printf("Investigating root cause...\n");
    
    // Step 2: Determine if test or code needs fixing
    bool is_test_problem = analyze_failure_source(error_message);
    
    if (is_test_problem) {
        // Step 3: Request permission for test modification
        printf("Test appears to have issue: [specific issue]\n");
        printf("Requesting permission to modify test: [specific change]\n");
        
        if (get_user_permission()) {
            // Step 4: Make authorized change
            modify_test_with_justification();
        }
    } else {
        // Step 5: Fix the actual code, not the test
        fix_code_under_test();
    }
}
```

## Related Patterns
- [Task Authorization Pattern](task-authorization-pattern.md)
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [Test First Verification Practice](test-first-verification-practice.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)