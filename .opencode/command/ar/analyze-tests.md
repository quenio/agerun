Run static analysis on the test code.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/analyze-tests` workflow is already in progress:

```bash
# Check current progress
make checkpoint-status CMD=analyze-tests VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=analyze-tests STEP=N

# Or reset and start over
./scripts/init-checkpoint.sh analyze-tests '"Build Tests" "Run Static Analysis" "Report Results"'
```

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of static analysis.

### Checkpoint Wrapper Scripts

The `run-analyze-tests.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-analyze-tests.sh
```

This script handles all stages of the test static analysis process:

### What the Script Does

1. **Build Tests**: Compiles the test executable if needed
2. **Run Static Analysis**: Executes static analysis tools on the test code
3. **Report Results**: Presents analysis findings and code quality metrics
4. **Checkpoint Completion**: Marks the workflow as complete

### Manual Checkpoint Control

If you need to manually check progress or resume a workflow:

```bash
# Check current progress
make checkpoint-status CMD=analyze-tests VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=analyze-tests STEP=N

# Reset and start over using the wrapper script
./scripts/init-checkpoint.sh analyze-tests '"Build Tests" "Run Static Analysis" "Report Results"'

# Verify checkpoint before running workflow
./scripts/require-checkpoint.sh analyze-tests

# Show completion and cleanup
./scripts/complete-checkpoint.sh analyze-tests
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
./scripts/gate-checkpoint.sh analyze-tests "Ready" "1"
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
make checkpoint-update CMD=analyze-tests STEP=2
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



## Key Points

- **Clang Static Analyzer** checks for bugs without running code
- **Common issues**: NULL dereferences, resource leaks, dead code
- **Test-specific checks**: Ensures test quality and correctness
- **Non-blocking warnings** but bugs should be fixed
- **Always use `2>&1`** to capture all analyzer output

## Test Quality Patterns

Beyond static analysis, ensure test quality by following these patterns:

- **Test complexity warning**: If tests have retry loops or complex state tracking, see [Test Complexity as Code Smell](../../../kb/test-complexity-as-code-smell.md)
- **Mock at the right level**: Avoid mocking system calls when function-level mocking suffices. See [Mock at Right Level Pattern](../../../kb/mock-at-right-level-pattern.md)
- **Check existing solutions**: Before creating complex test infrastructure, see how other tests solve similar problems. See [Check Existing Solutions First](../../../kb/check-existing-solutions-first.md)
- **Strong assertions**: Use AR_ASSERT with descriptive messages. See [Test Assertion Strength Patterns](../../../kb/test-assertion-strength-patterns.md)
- **Test effectiveness**: Verify tests actually catch failures. See [Test Effectiveness Verification](../../../kb/test-effectiveness-verification.md)