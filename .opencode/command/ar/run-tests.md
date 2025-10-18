Build and run all tests.

**Important**: Always use `make run-tests` instead of running tests directly ([details](../../../kb/make-target-testing-discipline.md)). Tests run in parallel with other build checks for efficiency ([details](../../../kb/parallel-build-job-integration.md)).

**Test Isolation**: If tests fail inconsistently or depend on execution order, check for shared directory issues ([details](../../../kb/test-isolation-shared-directory-pattern.md)).

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/run-tests` workflow is already in progress:

```bash
make checkpoint-status CMD=run-tests VERBOSE=--verbose
# Resume: make checkpoint-update CMD=run-tests STEP=N
# Or reset: make checkpoint-cleanup CMD=run-tests && make checkpoint-init CMD=run-tests STEPS='"Prepare" "Execute" "Verify"'
```

### First-Time Initialization Check

```bash
./scripts/init-checkpoint.sh run-tests '"Prepare" "Execute" "Verify"'
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/require-checkpoint.sh run-tests
```

# Run Tests
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the run tests process
make checkpoint-init CMD=run-tests STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
📍 Starting: run-tests (3 steps)
📁 Tracking: /tmp/run-tests-progress.txt
→ Run: make checkpoint-update CMD=run-tests STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=run-tests
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



#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=run-tests GATE="Ready" REQUIRED="1"
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
make checkpoint-update CMD=run-tests STEP=2
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
./scripts/complete-checkpoint.sh run-tests
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