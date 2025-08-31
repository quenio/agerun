Execute a clean build for comprehensive build verification with minimal output and check for hidden issues.


# Clean Build
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the build clean process
make checkpoint-init CMD=build_clean STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: build_clean
Tracking file: /tmp/build_clean_progress.txt
Total steps: 3

Steps to complete:
  1. Prepare
  2. Execute
  3. Verify

Goal: Complete build clean successfully
```

### Check Progress
```bash
make checkpoint-status CMD=build_clean
```

**Expected output (example at 33% completion):**
```
========================================
   CHECKPOINT STATUS: build_clean
========================================

Progress: 1/3 steps (33%)

[██████░░░░░░░░░░░░] 33%

Current Status: Preparing...

Next Action:
  → Step 2: Execute
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues


## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make clean build 2>&1 && make check-logs
```


#### [CHECKPOINT END - EXECUTION]
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