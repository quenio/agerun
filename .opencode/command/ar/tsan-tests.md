Run thread sanitizer on all tests for detecting data races.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/tsan-tests` workflow is already in progress:

```bash
# Check current progress
make checkpoint-status CMD=tsan-tests VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=tsan-tests STEP=N

# Or reset and start over
./scripts/init-checkpoint.sh tsan-tests '"Build Tests" "Run Sanitizer" "Report Results"'
```

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution.

### Checkpoint Wrapper Scripts

The `run-tsan-tests.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-tsan-tests.sh
```

This script handles all stages of sanitizer execution:

### What the Script Does

1. **Build Tests**: Compiles with sanitizer enabled
2. **Run Sanitizer**: Executes sanitizer with checks
3. **Report Results**: Summarizes findings
4. **Checkpoint Completion**: Marks the workflow as complete

### Manual Checkpoint Control

If you need to manually check progress or resume a workflow:

```bash
# Check current progress
make checkpoint-status CMD=tsan-tests VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=tsan-tests STEP=N

# Reset and start over using the wrapper script
./scripts/init-checkpoint.sh tsan-tests '"Build Tests" "Run Sanitizer" "Report Results"'

# Verify checkpoint before running workflow
./scripts/require-checkpoint.sh tsan-tests

# Show completion and cleanup
./scripts/complete-checkpoint.sh tsan-tests
```

## Key Points

- **ThreadSanitizer (TSAN)** detects data races and deadlocks
- **Uses clang compiler** instead of gcc for better TSAN support
- **dlsym tests excluded** automatically to prevent conflicts
- **~10x slower** than normal tests but catches concurrency bugs
- **Exit code 66** indicates thread safety issues detected