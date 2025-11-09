Execute comprehensive build verification with minimal output and check for hidden issues.

**Note**: The build runs all checks in parallel for efficiency ([details](../../../kb/parallel-build-job-integration.md)). Always use `make build` rather than running scripts directly ([details](../../../kb/make-target-testing-discipline.md)). All command documentation must achieve 90%+ scores or the build fails ([details](../../../kb/command-documentation-excellence-gate.md)). This workflow uses the command orchestrator pattern with checkpoint separation ([details](../../../kb/command-orchestrator-checkpoint-separation.md)).

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-build.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

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

After completing each step (before calling `checkpoint-update.sh`), you MUST:

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
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
  
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
Prompt: "Verify Step N: [Step Title] completion for build command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/build.md
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
- Add todo item: "Step 1: Compile Code" - Status: pending
- Add todo item: "Verify Step 1: Compile Code" - Status: pending
- Add todo item: "Step 2: Run Checks" - Status: pending
- Add todo item: "Verify Step 2: Run Checks" - Status: pending
- Add todo item: "Step 3: Verify Build" - Status: pending
- Add todo item: "Verify Step 3: Verify Build" - Status: pending
- Add todo item: "Verify Complete Workflow: build" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.


## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of the build process.

### Checkpoint Wrapper Scripts

The `run-build.sh` script uses the following standardized wrapper scripts:

- **`./scripts/checkpoint-init.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/checkpoint-require.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/checkpoint-gate.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/checkpoint-complete.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-build.sh
```

This script handles all stages of the comprehensive build process:

### What the Script Does

1. **Compile Code**: Compiles all modules and binaries in parallel
2. **Run Checks**: Executes documentation, naming, and quality checks
3. **Verify Build**: Confirms all artifacts and checks passed
4. **Checkpoint Completion**: Marks the workflow as complete

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
./scripts/checkpoint-status.sh build --verbose

# Resume from a specific step (only if you know it's stuck)
./scripts/checkpoint-update.sh build N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/build-progress.txt
./scripts/run-build.sh
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues



#### [EXECUTION GATE]
```bash
# Verify ready to execute
./scripts/checkpoint-gate.sh build "Ready" "1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make build 2>&1 && make check-logs

# Mark execution complete
./scripts/checkpoint-update.sh build 2
```

**Note**: If build fails due to CI network timeouts, see ([details](../../../kb/ci-network-timeout-diagnosis.md))
**Important**: `check-logs` analyzes logs generated by `build` - they must run in sequence ([details](../../../kb/build-logs-relationship-principle.md))


#### [CHECKPOINT END]
## Expected Output

### Success State
```
Building library...
Running all tests...
All 68 tests passed! (1126 assertions)
Build took 1m 24s
========================================
         BUILD SUCCESSFUL! ✓
========================================
Running log checks...
✅ No errors found in logs - CI ready!
```

### Failure States

**Compilation Error:**
```
Building library...
modules/ar_data.c:123: error: expected ';' before '}' token
make: *** [build] Error 1
```

**Test Failure:**
```
Building library...
Running all tests...
FAILED: ar_string_tests
  Expected: "hello"
  Actual: "world"
67 of 68 tests passed
make: *** [build] Error 1
```

**Log Check Failure:**
```
Building library...
Running all tests...
All 68 tests passed!
Build took 1m 24s
========================================
         BUILD SUCCESSFUL! ✓
========================================
Running log checks...
❌ Found 3 errors in logs:
  - ERROR: Method evaluation failed (in test: ar_method_evaluator_tests)
  - ERROR: Assertion failed at line 45
  - ERROR: Memory leak detected
Fix these issues before pushing to CI.
```



## Key Points

- **Always use `2>&1`** to capture stderr for complete error visibility
- **Always follow with `make check-logs`** to catch hidden issues
- Logs are only updated during build, not by check-logs ([details](../../../kb/build-logs-relationship-principle.md))
- The build must pass check-logs or CI will fail ([details](../../../kb/ci-check-logs-requirement.md))
- Check-logs includes deep analysis that can flag test output ([details](../../../kb/check-logs-deep-analysis-pattern.md))
- Use systematic build → check → fix → re-build validation loops ([details](../../../kb/validation-feedback-loop-effectiveness.md))
- Build time varies: ~20s for incremental, ~1m 24s for full build
- Use compiler errors to guide refactoring ([details](../../../kb/compilation-driven-refactoring-pattern.md))
- When verifying TDD cycle completion, build is part of systematic verification ([details](../../../kb/tdd-cycle-completion-verification-pattern.md))
- After completing TDD work, update plan documents with completion status ([details](../../../kb/plan-document-completion-status-pattern.md)) 

The Makefile uses generic parameterized targets to avoid duplication ([details](../../../kb/generic-make-targets-pattern.md)). Commands should document expected outputs for clarity ([details](../../../kb/command-output-documentation-pattern.md)) and follow the concise output principle ([details](../../../kb/concise-script-output-principle.md)).

**Important**: The build system runs parallel tests with different compilers (gcc for regular tests, clang for Thread Sanitizer). Each uses isolated build directories to prevent conflicts ([details](../../../kb/compiler-output-conflict-pattern.md)).

**Exit Code Handling**: When using popen() to run processes, remember that exit codes are multiplied by 256 ([details](../../../kb/exit-code-propagation-popen.md)). Non-critical operations should follow graceful degradation patterns ([details](../../../kb/graceful-degradation-pattern.md)).

**Shell Diagnostic Troubleshooting**: When build tools report configuration issues despite working setups, investigate diagnostic detection methods ([details](../../../kb/shell-configuration-diagnostic-troubleshooting.md)). Configuration migration between shell files may resolve tool detection problems ([details](../../../kb/configuration-migration-troubleshooting-strategy.md)).

## Related Documentation

### TDD and Testing Patterns
- [TDD Cycle Completion Verification Pattern](../../../kb/tdd-cycle-completion-verification-pattern.md)
- [Plan Document Completion Status Pattern](../../../kb/plan-document-completion-status-pattern.md)
- [TDD Plan Iteration Split Pattern](../../../kb/tdd-plan-iteration-split-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)
- [Plan Review Status Tracking Pattern](../../../kb/plan-review-status-tracking.md)
- [Iterative Plan Review Protocol](../../../kb/iterative-plan-review-protocol.md)
- [Iterative Plan Refinement Pattern](../../../kb/iterative-plan-refinement-pattern.md)