Run address sanitizer on all tests for detecting memory issues.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-sanitize-tests.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution.

### Checkpoint Wrapper Scripts

The `run-sanitize-tests.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-sanitize-tests.sh
```

This script handles all stages of sanitizer execution:

### What the Script Does

1. **Build Tests**: Compiles with sanitizer enabled
2. **Run Sanitizer**: Executes sanitizer with checks
3. **Report Results**: Summarizes findings
4. **Checkpoint Completion**: Marks the workflow as complete

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
make checkpoint-status CMD=sanitize-tests VERBOSE=--verbose

# Resume from a specific step (only if you know it's stuck)
make checkpoint-update CMD=sanitize-tests STEP=N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/sanitize-tests-progress.txt
./scripts/run-sanitize-tests.sh
```

#### [CHECKPOINT END]

## Key Points

- **AddressSanitizer (ASAN)** detects memory leaks, use-after-free, buffer overflows
- **dlsym tests excluded** automatically to prevent conflicts
- **~2x slower** than normal tests but catches critical issues
- **Exit on first error** - fix immediately before continuing

**Important**: Tests using dlsym for function interception (named `*_dlsym_tests`) are automatically excluded from sanitizer builds to prevent conflicts. These tests still run in normal builds. ([details](../../../kb/sanitizer-test-exclusion-pattern.md), [technique](../../../kb/dlsym-test-interception-technique.md))

**Test Validation**: When testing error handling, use permission-based failure testing with chmod to create predictable failures ([details](../../../kb/permission-based-failure-testing.md)). Always verify test effectiveness by temporarily breaking the implementation ([details](../../../kb/test-effectiveness-verification.md)).

**Mocking Guidelines**: When using dlsym for testing, mock at the appropriate level - function-level mocking is simpler and more maintainable than system call interception ([details](../../../kb/mock-at-right-level-pattern.md)). Complex test code with retry loops or state tracking indicates mocking at the wrong abstraction level ([details](../../../kb/test-complexity-as-code-smell.md)). Always check how other tests solve similar problems before creating new infrastructure ([details](../../../kb/check-existing-solutions-first.md)).

**Diagnostic Troubleshooting**: When sanitizer tools report configuration issues despite working setups, apply systematic diagnostic troubleshooting ([details](../../../kb/shell-configuration-diagnostic-troubleshooting.md)). Shell loading order affects tool behavior ([details](../../../kb/shell-loading-order-tool-detection.md)).
#### [CHECKPOINT COMPLETE]
