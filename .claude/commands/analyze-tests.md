Run static analysis on the test code.


# Analyze Tests
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the analyze tests process
make checkpoint-init CMD=analyze_tests STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: analyze_tests (3 steps)
📁 Tracking: /tmp/analyze_tests_progress.txt
→ Run: make checkpoint-update CMD=analyze_tests STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=analyze_tests
```

**Expected output (example at 33% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues




**CRITICAL**: Test code quality matters as much as production code. Poor test quality leads to false confidence.

**Before running**: Review test coverage to identify gaps. The analyzer checks for:
- Memory leaks in test fixtures
- Improper cleanup
- Test logic errors
- Resource management issues

**Questions to consider**:
- Do all tests properly clean up resources?
- Are test assertions actually testing the right things?
- Is there dead code in the tests?

#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=analyze_tests GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make analyze-tests 2>&1

# Mark execution complete
make checkpoint-update CMD=analyze_tests STEP=2
```


#### [CHECKPOINT END - EXECUTION]
## Expected Output

### Success State
```
Running static analysis on tests...
Analyzing ar_agent_tests.c...
Analyzing ar_agency_tests.c...
Analyzing ar_data_tests.c...
... (65 more test files)

Static analysis complete!
No issues found in test code.
```

### Warning States

**Unused Variables:**
```
Running static analysis on tests...
Analyzing ar_data_tests.c...
ar_data_tests.c:45:10: warning: unused variable 'result' [-Wunused-variable]
    int result = 42;
        ^~~~~~

Analyzing ar_string_tests.c...
ar_string_tests.c:123:15: warning: unused variable 'buffer' [-Wunused-variable]
    char buffer[256];
         ^~~~~~

Static analysis complete!
Found 2 warnings in test code.
```

**Potential NULL Dereference:**
```
Running static analysis on tests...
Analyzing ar_agent_tests.c...
ar_agent_tests.c:78:5: warning: Access to field 'id' results in a dereference of a null pointer
    agent->id = 123;
    ^~~~~~~~~

Static analysis complete!
Found 1 potential bug in test code.
make: *** [analyze-tests] Error 1
```

**Resource Leak:**
```
Running static analysis on tests...
Analyzing ar_data_tests.c...
ar_data_tests.c:156:1: warning: Potential leak of memory pointed to by 'data'
}
^

Static analysis complete!
Found 1 potential leak in test code.
```


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=analyze_tests
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: analyze_tests
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Prepare: ✓ Complete
  Execute: ✓ Complete  
  Verify: ✓ Complete

The analyze tests completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=analyze_tests
```

## Key Points

- **Clang Static Analyzer** checks for bugs without running code
- **Common issues**: NULL dereferences, resource leaks, dead code
- **Test-specific checks**: Ensures test quality and correctness
- **Non-blocking warnings** but bugs should be fixed
- **Always use `2>&1`** to capture all analyzer output

## Test Quality Patterns

Beyond static analysis, ensure test quality by following these patterns:

- **Test complexity warning**: If tests have retry loops or complex state tracking, see [Test Complexity as Code Smell](../../kb/test-complexity-as-code-smell.md)
- **Mock at the right level**: Avoid mocking system calls when function-level mocking suffices. See [Mock at Right Level Pattern](../../kb/mock-at-right-level-pattern.md)
- **Check existing solutions**: Before creating complex test infrastructure, see how other tests solve similar problems. See [Check Existing Solutions First](../../kb/check-existing-solutions-first.md)
- **Strong assertions**: Use AR_ASSERT with descriptive messages. See [Test Assertion Strength Patterns](../../kb/test-assertion-strength-patterns.md)
- **Test effectiveness**: Verify tests actually catch failures. See [Test Effectiveness Verification](../../kb/test-effectiveness-verification.md)