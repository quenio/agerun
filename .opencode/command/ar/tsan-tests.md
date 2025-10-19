Run thread sanitizer on all tests for detecting data races.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-tsan-tests.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution.

### Checkpoint Wrapper Scripts

The `run-tsan-tests.sh` script uses the following standardized wrapper scripts:

- **`./scripts/checkpoint-init.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/checkpoint-require.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/checkpoint-gate.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/checkpoint-complete.sh`**: Shows completion summary and cleanup

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

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
./scripts/checkpoint-status.sh tsan-tests --verbose

# Resume from a specific step (only if you know it's stuck)
./scripts/checkpoint-update.sh tsan-tests N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/tsan-tests-progress.txt
./scripts/run-tsan-tests.sh
```

#### [CHECKPOINT END]

## Key Points

- **ThreadSanitizer (TSAN)** detects data races and deadlocks
- **Uses clang compiler** instead of gcc for better TSAN support
- **dlsym tests excluded** automatically to prevent conflicts
- **~10x slower** than normal tests but catches concurrency bugs
- **Exit code 66** indicates thread safety issues detected
#### [CHECKPOINT COMPLETE]
