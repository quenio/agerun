Execute comprehensive build verification with minimal output and check for hidden issues.

**Note**: The build runs all checks in parallel for efficiency ([details](../../kb/parallel-build-job-integration.md)). Always use `make build` rather than running scripts directly ([details](../../kb/make-target-testing-discipline.md)). All command documentation must achieve 90%+ scores or the build fails ([details](../../kb/command-documentation-excellence-gate.md)).

# Build
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the build process
make checkpoint-init CMD=build STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: build
Tracking file: /tmp/build_progress.txt
Total steps: 3

Steps to complete:
  1. Prepare
  2. Execute
  3. Verify

Goal: Complete build successfully
```

### Check Progress
```bash
make checkpoint-status CMD=build
```

**Expected output (example at 33% completion):**
```
========================================
   CHECKPOINT STATUS: build
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



#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD=build GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
========================================
   GATE: Ready
========================================

✅ GATE PASSED: Ready to execute!

Prerequisites verified:
  ✓ Environment prepared
  ✓ Dependencies available
  
Proceed with execution.
```

## Command

#### [CHECKPOINT START - EXECUTION]

```bash
make build 2>&1 && make check-logs

# Mark execution complete
make checkpoint-update CMD=build STEP=2
```

**Note**: If build fails due to CI network timeouts, see ([details](../../kb/ci-network-timeout-diagnosis.md))


#### [CHECKPOINT END - EXECUTION]
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


#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=build
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: build
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Preparation: ✓ Complete
  Execution: ✓ Complete  
  Verification: ✓ Complete

The build completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=build
```

## Key Points

- **Always use `2>&1`** to capture stderr for complete error visibility
- **Always follow with `make check-logs`** to catch hidden issues
- Logs are only updated during build, not by check-logs ([details](../../kb/build-logs-relationship-principle.md))
- The build must pass check-logs or CI will fail ([details](../../kb/ci-check-logs-requirement.md))
- Check-logs includes deep analysis that can flag test output ([details](../../kb/check-logs-deep-analysis-pattern.md))
- Build time varies: ~20s for incremental, ~1m 24s for full build 

The Makefile uses generic parameterized targets to avoid duplication ([details](../../kb/generic-make-targets-pattern.md)). Commands should document expected outputs for clarity ([details](../../kb/command-output-documentation-pattern.md)).

**Important**: The build system runs parallel tests with different compilers (gcc for regular tests, clang for Thread Sanitizer). Each uses isolated build directories to prevent conflicts ([details](../../kb/compiler-output-conflict-pattern.md)).

**Exit Code Handling**: When using popen() to run processes, remember that exit codes are multiplied by 256 ([details](../../kb/exit-code-propagation-popen.md)). Non-critical operations should follow graceful degradation patterns ([details](../../kb/graceful-degradation-pattern.md)).