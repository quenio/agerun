Run address sanitizer on the executable for detecting memory issues.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-sanitize-exec.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution.

### Checkpoint Wrapper Scripts

The `run-sanitize-exec.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-sanitize-exec.sh
```

This script handles all stages of address sanitizer execution:

### What the Script Does

1. **Build Executable**: Compiles with AddressSanitizer enabled
2. **Run Sanitizer**: Executes sanitizer with memory checks
3. **Report Results**: Summarizes findings and memory issues
4. **Checkpoint Completion**: Marks the workflow as complete

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
make checkpoint-status CMD=sanitize-exec VERBOSE=--verbose

# Resume from a specific step (only if you know it's stuck)
make checkpoint-update CMD=sanitize-exec STEP=N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/sanitize-exec-progress.txt
./scripts/run-sanitize-exec.sh
```

#### [CHECKPOINT END]

## Key Points

- **Known issue**: Methodology leaks are expected (not yet fixed)
- **Critical errors**: Use-after-free and double-free stop execution
- **Always use `2>&1`** to capture sanitizer output
- **Slower execution** but catches runtime memory errors

#### [CHECKPOINT COMPLETE]