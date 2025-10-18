Run address sanitizer on the executable for detecting memory issues.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/sanitize-exec` workflow is already in progress:

```bash
# Check current progress
make checkpoint-status CMD=sanitize-exec VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=sanitize-exec STEP=N

# Or reset and start over
./scripts/init-checkpoint.sh sanitize-exec '"Build Executable" "Run Sanitizer" "Report Results"'
```

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

### Manual Checkpoint Control

If you need to manually check progress or resume a workflow:

```bash
# Check current progress
make checkpoint-status CMD=sanitize-exec VERBOSE=--verbose

# Resume from a specific step (if interrupted)
make checkpoint-update CMD=sanitize-exec STEP=N

# Reset and start over using the wrapper script
./scripts/init-checkpoint.sh sanitize-exec '"Build Executable" "Run Sanitizer" "Report Results"'

# Verify checkpoint before running workflow
./scripts/require-checkpoint.sh sanitize-exec

# Show completion and cleanup
./scripts/complete-checkpoint.sh sanitize-exec
```

#### [CHECKPOINT END]

## Key Points

- **Known issue**: Methodology leaks are expected (not yet fixed)
- **Critical errors**: Use-after-free and double-free stop execution
- **Always use `2>&1`** to capture sanitizer output
- **Slower execution** but catches runtime memory errors

#### [CHECKPOINT COMPLETE]