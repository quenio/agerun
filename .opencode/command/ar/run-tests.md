Build and run all tests.

**Important**: Always use `make run-tests` instead of running tests directly ([details](../../../kb/make-target-testing-discipline.md)). Tests run in parallel with other build checks for efficiency ([details](../../../kb/parallel-build-job-integration.md)).

**Test Isolation**: If tests fail inconsistently or depend on execution order, check for shared directory issues ([details](../../../kb/test-isolation-shared-directory-pattern.md)).


## Quick Start

```bash
./scripts/run-run-tests.sh
```

That's it! The script will handle everything automatically.

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step, you MUST:

1. **Report accomplishments with evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide evidence (file paths, command outputs, git status/diff)
   - **DO NOT** tell step-verifier what to verify - report what was done

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     -   
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for run-tests command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/run-tests.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Build Tests" - Status: pending
- Add todo item: "Verify Step 1: Build Tests" - Status: pending
- Add todo item: "Step 2: Run All Tests" - Status: pending
- Add todo item: "Verify Step 2: Run All Tests" - Status: pending
- Add todo item: "Step 3: Verify Results" - Status: pending
- Add todo item: "Verify Step 3: Verify Results" - Status: pending
- Add todo item: "Verify Complete Workflow: run-tests" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Workflow Execution

Run the complete workflow:

```bash
./scripts/run-run-tests.sh
```

This script handles all stages of test execution:

### What the Script Does

1. **Build Tests**: Compiles all test binaries
2. **Run All Tests**: Executes all tests with parallel job control
3. **Verify Results**: Confirms all tests passed
4. **Completion**: Marks the workflow as complete

## Troubleshooting

If the script fails, simply rerun it:

```bash
./scripts/run-run-tests.sh
```

### Check Progress
**Expected output (example at 33% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues

```bash
# Verify ready to execute
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

```bash
make run-tests 2>&1

# Mark execution complete
```

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

**Expected completion output:**
```
========================================
   WORKFLOW COMPLETION SUMMARY
========================================

📈 run-tests: 3/3 steps (100%)
   [████████████████████] 100%

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