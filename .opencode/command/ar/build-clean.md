Execute a clean build for comprehensive build verification with minimal output and check for hidden issues.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize checkpoints before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-build-clean.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

## Checkpoint Tracking

This command uses checkpoint tracking via wrapper scripts to ensure systematic execution of clean builds.

### Checkpoint Wrapper Scripts

The `run-build-clean.sh` script uses the following standardized wrapper scripts:

- **`./scripts/init-checkpoint.sh`**: Initializes or resumes checkpoint tracking
- **`./scripts/require-checkpoint.sh`**: Verifies checkpoint is ready before proceeding
- **`./scripts/gate-checkpoint.sh`**: Validates gate conditions at workflow boundaries
- **`./scripts/complete-checkpoint.sh`**: Shows completion summary and cleanup

These wrappers provide centralized checkpoint management across all commands.

## Workflow Execution

Run the complete checkpoint-based workflow:

#### [CHECKPOINT START]

```bash
./scripts/run-build-clean.sh
```

This script handles all stages of the clean build process:

### What the Script Does

1. **Clean Artifacts**: Removes all build artifacts and intermediate files
2. **Compile Code**: Compiles all modules and binaries from scratch
3. **Verify Build**: Confirms all artifacts were rebuilt correctly
4. **Checkpoint Completion**: Marks the workflow as complete

## Troubleshooting: Manual Checkpoint Control

Only use these commands if the script fails and you need to manually intervene:

```bash
# Check current progress (if workflow interrupted)
make checkpoint-status CMD=build-clean VERBOSE=--verbose

# Resume from a specific step (only if you know it's stuck)
make checkpoint-update CMD=build-clean STEP=N

# ONLY use this if you need to reset everything and start over
rm -f /tmp/build-clean-progress.txt
./scripts/run-build-clean.sh
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues




**MANDATORY**: Always run check-logs after clean build. CI will reject builds that fail log checks. ([details](../../../kb/build-verification-before-commit.md))

**IMPORTANT**: Clean builds are required when:
- Changing dependencies
- Major refactoring
- Suspicious incremental build behavior
- Before final commit

**Critical questions**:
- Did the previous incremental build behave unexpectedly?
- Have you changed any header files?
- Are you seeing strange linker errors?

For example, if you see "undefined reference" errors in incremental builds, a clean build often resolves them.

#### [EXECUTION GATE]
```bash
# Verify ready to execute
./scripts/gate-checkpoint.sh build-clean "Ready" "1"
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make clean build 2>&1 && make check-logs

# Mark execution complete
make checkpoint-update CMD=build-clean STEP=2
```


#### [CHECKPOINT END]
## Expected Output

### Success State
```
Removing build artifacts...
Cleaning logs directory...
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

**Clean Failure (rare):**
```
Removing build artifacts...
rm: cannot remove 'bin/': Permission denied
make: *** [clean] Error 1
```

**Build Failure After Clean:**
```
Removing build artifacts...
Cleaning logs directory...
Building library...
modules/ar_data.c:123: error: 'ar_data_t' undeclared
make: *** [build] Error 1
```



## Key Points

- **Clean build takes longer** (~1m 24s vs ~20s incremental)
- **Removes all artifacts** including bin/, logs/, and object files
- **Required for**: Major refactoring, dependency changes, CI verification
- **Always use `2>&1`** to capture stderr for complete error visibility
- **Always follow with `make check-logs`** to verify CI readiness

## Related Patterns
- [Build Verification Before Commit](../../../kb/build-verification-before-commit.md)
- [Script Debugging Through Isolation](../../../kb/script-debugging-through-isolation.md) - When build errors occur, isolate the failure
- [Cross-Platform Bash Script Patterns](../../../kb/cross-platform-bash-script-patterns.md) - Build scripts must work on macOS and Linux