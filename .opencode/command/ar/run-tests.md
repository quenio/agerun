Build and run all tests.

**Important**: Always use `make run-tests` instead of running tests directly ([details](../../../kb/make-target-testing-discipline.md)). Tests run in parallel with other build checks for efficiency ([details](../../../kb/parallel-build-job-integration.md)).

**Test Isolation**: If tests fail inconsistently or depend on execution order, check for shared directory issues ([details](../../../kb/test-isolation-shared-directory-pattern.md)).

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-run-tests.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of tests.

### Checkpoint Wrapper Scripts

The `run-run-tests.sh` script uses the following standardized wrapper scripts:

- **`./scripts/checkpoint-init.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/checkpoint-require.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/checkpoint-gate.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/checkpoint-complete.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-run-tests.sh
```

This script handles all stages of test execution:

### What the Script Does

1. **Build Tests**: Compiles all test binaries
2. **Run All Tests**: Executes all tests with parallel job control
3. **Verify Results**: Confirms all tests passed
4. **Checkpoint Completion**: Marks the workflow as complete

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
./scripts/checkpoint-status.sh run-tests --verbose

# Resume from a specific step (only if you know it's stuck)
./scripts/checkpoint-update.sh run-tests N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/run-tests-progress.txt
./scripts/run-run-tests.sh
```

### Check Progress
```bash
./scripts/checkpoint-status.sh run-tests
```

**Expected output (example at 33% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ./scripts/checkpoint-update.sh command N
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues



#### [EXECUTION GATE]
```bash
# Verify ready to execute
./scripts/checkpoint-gate.sh run-tests "Ready" "1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make run-tests 2>&1

# Mark execution complete
./scripts/checkpoint-update.sh run-tests 2
```


#### [CHECKPOINT END - EXECUTION]
## Expected Output

### Success State
```
Building library...
Running all tests...
  ✓ ar_agent_tests
  ✓ ar_agency_tests
  ✓ ar_data_tests
  ✓ ar_expression_parser_tests
  ✓ ar_instruction_parser_tests
  ✓ ar_method_evaluator_tests
  ✓ ar_string_tests
  ... (61 more tests)
All 68 tests passed! (1126 assertions)
```

### Failure States

**Test Failure:**
```
Building library...
Running all tests...
  ✓ ar_agent_tests
  ✗ ar_data_tests
    FAILED at ar_data_tests.c:45
    Expected: 42
    Actual: 0
  ✓ ar_expression_parser_tests
  ... (65 more tests)
67 of 68 tests passed
make: *** [run-tests] Error 1
```

**Memory Leak:**
```
Building library...
Running all tests...
All 68 tests passed! (1126 assertions)

WARNING: 3 memory leaks detected!
See bin/run-tests/memory_report_*.log for details
```

**Compilation Error:**
```
Building library...
modules/ar_data.c:123: error: expected ';' before '}' token
make: *** [ar_data.o] Error 1
```


#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh run-tests
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 run-tests: 3/3 steps (100%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```

## Key Points

- **Always use `2>&1`** to capture memory leak warnings
- **Individual test reports** at `bin/run-tests/memory_report_<test>.log`
- **Zero tolerance for leaks** - any leak should be fixed immediately
- Tests run in isolated processes for safety

**Test Infrastructure**: Tests requiring shared setup/teardown should use fixture modules with opaque types and proper lifecycle management ([details](../../../kb/test-fixture-module-creation-pattern.md)). Fixtures should evolve organically by adding helper methods as needs arise ([details](../../../kb/test-fixture-evolution-pattern.md)). Avoid redundant cleanup that fixtures already handle ([details](../../../kb/redundant-test-cleanup-anti-pattern.md)).

**Test Effectiveness**: When testing error paths, temporarily break implementation to verify tests detect failures ([details](../../../kb/test-effectiveness-verification.md)). Use filesystem permissions (chmod) to create predictable failure conditions ([details](../../../kb/permission-based-failure-testing.md)). For untestable conditions, use dlsym interception ([details](../../../kb/dlsym-test-interception-technique.md)), but mock at the right level - avoid mocking system calls when function-level mocking works ([details](../../../kb/mock-at-right-level-pattern.md)).

**Test Quality**: Write strong tests that verify specific outcomes, not just success/failure ([details](../../../kb/test-assertion-strength-patterns.md)). When using TDD, plan one assertion per iteration ([details](../../../kb/tdd-iteration-planning-pattern.md)), ensure RED phase produces assertion failures not compilation errors ([details](../../../kb/tdd-red-phase-assertion-requirement.md)), and implement minimal GREEN solutions ([details](../../../kb/tdd-green-phase-minimalism.md)). Use AR_ASSERT macros with descriptive messages, never plain assert() ([details](../../../kb/standards-over-expediency-principle.md)). Always use make to run tests, never execute binaries directly ([details](../../../kb/make-only-test-execution.md)). Verify existing error coverage before adding new tests ([details](../../../kb/error-coverage-verification-before-enhancement.md)). If tests become complex with retry loops or state tracking, it's a code smell ([details](../../../kb/test-complexity-as-code-smell.md)). Check existing solutions first before creating new test infrastructure ([details](../../../kb/check-existing-solutions-first.md)). When completing TDD cycles, verify all iterations systematically ([details](../../../kb/tdd-cycle-completion-verification-pattern.md)) and update plan documents with completion status ([details](../../../kb/plan-document-completion-status-pattern.md)).

**Troubleshooting**: When test tools report configuration issues despite working setups, apply systematic diagnostic troubleshooting ([details](../../../kb/shell-configuration-diagnostic-troubleshooting.md)). Shell configuration migration may resolve tool detection problems ([details](../../../kb/configuration-migration-troubleshooting-strategy.md)).