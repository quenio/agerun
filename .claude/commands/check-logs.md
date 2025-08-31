Check build logs for hidden issues that might not be caught by the build summary.


# Check Logs
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic log verification and issue resolution. The process has 8 checkpoints across 4 phases with critical error handling gates.

### Initialize Tracking
```bash
# Start the log checking process
make checkpoint-init CMD=check-logs STEPS='"Run Build" "Standard Checks" "Deep Analysis" "Categorize Errors" "Fix Issues" "Update Whitelist" "Re-check Logs" "Final Validation"'
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: check-logs
Tracking file: /tmp/check-logs_progress.txt
Total steps: 8

Steps to complete:
  1. Run Build
  2. Standard Checks
  3. Deep Analysis
  4. Categorize Errors
  5. Fix Issues
  6. Update Whitelist
  7. Re-check Logs
  8. Final Validation

Goal: Ensure build logs are clean for CI
```

### Check Progress
```bash
make checkpoint-status CMD=check-logs
```

**Expected output (example at 50% completion):**
```
========================================
   CHECKPOINT STATUS: check-logs
========================================

Progress: 4/8 steps (50%)

[████████████████░░░░░░░░░░░░░░░░] 50%

Current Phase: Analysis
Errors Found: 12
  Real errors: 3
  Intentional errors: 9

Next Action:
  → Step 5: Fix Issues
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Build must complete successfully
- [ ] All real errors must be fixed
- [ ] Intentional errors must be whitelisted
- [ ] Final check must pass (CI ready)

Check build logs for hidden issues that might not be caught by the build summary.
### What it does

This command performs a two-phase analysis of build log files:

### Phase 1: Standard Checks
Detects known critical issues including:
- Assertion failures
- Segmentation faults or crashes
- Test failures that might not be properly reported
- Memory errors from sanitizers
- Thread safety issues
- Deep copy support errors
- Method loading warnings
- Unexpected test behaviors (e.g., tests expecting failure that succeed)
- Method evaluation failures
- Missing AST errors

### Phase 2: Deep Analysis (if standard checks pass)
Performs additional thorough analysis to catch edge cases:
- Scans for any ERROR/WARNING patterns that might have been missed
- Verifies test output consistency
- Checks for suspicious patterns in test-related output
- Searches for failure indicators (Cannot, Unable to, Failed to)
- Provides detailed counts and examples of any anomalies found

## Phase 1: Initial Check (Steps 1-2)

#### [CHECKPOINT START - PHASE 1]

#### Checkpoint 1: Run Build

```bash
# Ensure fresh build before checking
echo "Running fresh build..."
if ! make clean build 2>&1; then
  echo "❌ Build failed - must fix build errors first"
  exit 1
fi

echo "✅ Build completed successfully"
make checkpoint-update CMD=check-logs STEP=1
```

#### Checkpoint 2: Standard Checks

```bash
# Run Phase 1 log analysis
echo "Running standard log checks..."
ERROR_COUNT=0

if make check-logs 2>&1 | tee /tmp/check-logs-output.txt; then
  echo "✅ No errors found in standard checks"
else
  ERROR_COUNT=$(grep -c "ERROR\|FAILURE\|ASSERT" /tmp/check-logs-output.txt || echo "0")
  echo "⚠️ Found $ERROR_COUNT potential issues"
fi

echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-logs-stats.txt
make checkpoint-update CMD=check-logs STEP=2
```

#### [BUILD GATE]
```bash
# Verify build is clean before deeper analysis
make checkpoint-gate CMD=check-logs GATE="Build" REQUIRED="1"
```

**Expected gate output:**
```
========================================
   GATE: Build
========================================

✅ GATE PASSED: Build verified!

Build Status:
  ✓ Clean build completed
  ✓ All modules compiled
  ✓ Tests executed

Ready for log analysis.
```

## Phase 2: Analysis (Steps 3-4)

#### [CHECKPOINT START - PHASE 2]

#### Checkpoint 3: Deep Analysis

```bash
# Run Phase 2 deep analysis if standard passed
source /tmp/check-logs-stats.txt

if [ $ERROR_COUNT -eq 0 ]; then
  echo "Running deep analysis..."
  python3 scripts/check_logs.py --deep 2>&1 | tee /tmp/deep-analysis.txt
  
  if grep -q "WARNING\|suspicious" /tmp/deep-analysis.txt; then
    echo "⚠️ Deep analysis found warnings"
  else
    echo "✅ Deep analysis clean"
  fi
else
  echo "Skipping deep analysis - standard checks found issues"
fi

make checkpoint-update CMD=check-logs STEP=3
```

#### Checkpoint 4: Categorize Errors

```bash
# Categorize errors found
source /tmp/check-logs-stats.txt

if [ $ERROR_COUNT -gt 0 ]; then
  echo "Categorizing $ERROR_COUNT errors..."
  
  # Count real vs intentional errors
  REAL_ERRORS=0
  INTENTIONAL_ERRORS=0
  
  # Check against whitelist
  while IFS= read -r error; do
    if grep -q "$error" log_whitelist.yaml; then
      INTENTIONAL_ERRORS=$((INTENTIONAL_ERRORS + 1))
    else
      REAL_ERRORS=$((REAL_ERRORS + 1))
    fi
  done < <(grep "ERROR\|FAILURE" /tmp/check-logs-output.txt)
  
  echo "Real errors: $REAL_ERRORS"
  echo "Intentional errors needing whitelist: $INTENTIONAL_ERRORS"
  
  echo "REAL_ERRORS=$REAL_ERRORS" >> /tmp/check-logs-stats.txt
  echo "INTENTIONAL_ERRORS=$INTENTIONAL_ERRORS" >> /tmp/check-logs-stats.txt
fi

make checkpoint-update CMD=check-logs STEP=4
```

#### [CRITICAL ERROR GATE]
```bash
# ⚠️ CRITICAL: If errors found, must resolve before proceeding
source /tmp/check-logs-stats.txt
if [ ${ERROR_COUNT:-0} -gt 0 ]; then
  make checkpoint-gate CMD=check-logs GATE="Error Analysis" REQUIRED="3,4"
fi
```

**Expected gate output (when errors found):**
```
========================================
   GATE: Error Analysis
========================================

⚠️ CRITICAL: Errors detected!

Error Summary:
  Total errors: 12
  Real errors to fix: 3
  Intentional errors to whitelist: 9

✅ GATE PASSED: Analysis complete

Proceed to resolution phase.
```

## Phase 3: Resolution (Steps 5-6)

#### [CHECKPOINT START - PHASE 3]

#### Checkpoint 5: Fix Issues

```bash
# Fix real errors (manual step)
source /tmp/check-logs-stats.txt

if [ ${REAL_ERRORS:-0} -gt 0 ]; then
  echo "⚠️ Manual intervention required!"
  echo "Fix the $REAL_ERRORS real errors identified above."
  echo "After fixing, mark this step complete."
else
  echo "✅ No real errors to fix"
fi

make checkpoint-update CMD=check-logs STEP=5
```

#### Checkpoint 6: Update Whitelist

```bash
# Update whitelist for intentional errors
source /tmp/check-logs-stats.txt

if [ ${INTENTIONAL_ERRORS:-0} -gt 0 ]; then
  echo "Updating whitelist for $INTENTIONAL_ERRORS intentional errors..."
  echo "Add the following to log_whitelist.yaml:"
  
  # Show errors needing whitelist
  grep "ERROR\|FAILURE" /tmp/check-logs-output.txt | head -5
  
  echo "\n✅ Whitelist updated"
else
  echo "✅ No whitelist updates needed"
fi

make checkpoint-update CMD=check-logs STEP=6
```

## Phase 4: Verification (Steps 7-8)

#### [CHECKPOINT START - PHASE 4]

#### Checkpoint 7: Re-check Logs

```bash
# Re-run checks after fixes
echo "Re-checking logs after fixes..."

if make check-logs 2>&1; then
  echo "✅ All log checks now pass!"
  FINAL_STATUS="PASS"
else
  echo "❌ Log checks still failing"
  FINAL_STATUS="FAIL"
fi

echo "FINAL_STATUS=$FINAL_STATUS" >> /tmp/check-logs-stats.txt
make checkpoint-update CMD=check-logs STEP=7
```

#### Checkpoint 8: Final Validation

```bash
# Final CI readiness check
source /tmp/check-logs-stats.txt

if [ "$FINAL_STATUS" = "PASS" ]; then
  echo "✅ Build is CI-ready!"
  echo "All checks passed, no errors detected."
else
  echo "❌ Build is NOT CI-ready"
  echo "Fix remaining issues before pushing to CI."
  exit 1
fi

make checkpoint-update CMD=check-logs STEP=8
```

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=check-logs
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: check-logs
========================================

Progress: 8/8 steps (100%)

[████████████████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Final Status:
  Build: Clean
  Errors fixed: 3
  Whitelist updated: 9 entries
  CI Status: READY

The build is ready for CI!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=check-logs
rm -f /tmp/check-logs-*.txt /tmp/deep-analysis.txt
```

### Usage

```bash
make check-logs
```

**Or with checkpoint tracking:**
```bash
# Initialize and run through checkpoints
make checkpoint-init CMD=check-logs STEPS='...'
# Follow checkpoint steps above
```

## Why it's important:

Even when `make build` shows "SUCCESS", the logs may contain:
- Assertion failures that didn't propagate to the build script
- Warnings or errors that should be addressed
- Tests that are failing silently
- Important diagnostic output

## Recommended workflow:

1. Run `make build` first
2. Always follow up with `make check-logs` to double-check
3. If issues are found, examine the specific log files in `logs/`

## Log file locations:

- `logs/run-tests.log` - Standard test execution
- `logs/sanitize-tests.log` - Tests with AddressSanitizer
- `logs/tsan-tests.log` - Tests with ThreadSanitizer
- `logs/analyze-exec.log` - Static analysis of executable
- `logs/analyze-tests.log` - Static analysis of tests

To view a specific log: `less logs/<logname>.log`
To search all logs: `grep -r 'pattern' logs/`

This check is especially important after the critical build system issue discovered on 2025-07-28 where test failures were not being properly reported ([details](../../kb/build-system-exit-code-verification.md)).

The log checker uses context-aware filtering to distinguish between intentional test errors and real problems ([details](../../kb/intentional-test-errors-filtering.md)).

## Troubleshooting

### If check-logs keeps failing:
```bash
# Check specific log files
grep -n "ERROR\|FAILURE" logs/*.log

# Verify whitelist is being applied
python3 scripts/check_logs.py --verbose

# Check for new test errors
diff logs/run-tests.log logs/run-tests.log.previous
```

### Common issues:
1. **New test added**: May introduce intentional errors needing whitelist
2. **Whitelist syntax**: YAML formatting affects matching
3. **Environment differences**: Test names may vary (sanitizer suffixes)
4. **Real failures**: Memory leaks, assertions, segfaults need fixing

### Quick fixes:
```bash
# For intentional errors, add to whitelist:
echo '  - context: "test_name"' >> log_whitelist.yaml
echo '    message: "ERROR: message"' >> log_whitelist.yaml
echo '    comment: "Testing error case"' >> log_whitelist.yaml

# Then verify:
make check-logs
```

## Related Documentation

### Checkpoint Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../kb/multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../kb/command-thoroughness-requirements-pattern.md)

### Log Checking Patterns
- [Build System Exit Code Verification](../../kb/build-system-exit-code-verification.md)
- [Intentional Test Errors Filtering](../../kb/intentional-test-errors-filtering.md)
- [Log Format Variation Handling](../../kb/log-format-variation-handling.md)
- [YAML String Matching Pitfalls](../../kb/yaml-string-matching-pitfalls.md)
- [Whitelist Simplification Pattern](../../kb/whitelist-simplification-pattern.md)
- [Systematic Error Whitelist Reduction](../../kb/systematic-error-whitelist-reduction.md)
- [Uniform Filtering Application](../../kb/uniform-filtering-application.md)

## Managing Intentional Errors

If `make check-logs` reports errors that are intentional (e.g., testing error handling), you should add them to the whitelist:

1. Look at the error output from `check_logs.py` to identify:
   - The test context (shown as "in test: test_name")
   - The error message (without timestamp)

2. Add an entry to `log_whitelist.yaml`:
   ```yaml
   - context: "ar_method_evaluator_tests"  # The test where error occurs
     message: "ERROR: Method evaluation failed"
     comment: "Testing error handling for invalid method"
   ```

3. Key points about the simplified whitelist:
   - `context`: The exact test name or "executable" for non-test contexts
   - `message`: The error/warning message to match (timestamps automatically stripped)
   - `comment`: Optional description of why this is whitelisted
   - No more before/after matching - context + message is sufficient ([details](../../kb/whitelist-simplification-pattern.md))
   - Consider fixing root causes instead of whitelisting ([details](../../kb/systematic-error-whitelist-reduction.md))

Example workflow:
```bash
# Run tests and check logs
make build 2>&1
make check-logs

# If intentional errors are found, examine the output:
# logs/run-tests.log:123:ERROR: Expected literal (string or number) (at position 0) (in test: ar_expression_parser_tests)

# Add to log_whitelist.yaml:
ignored_errors:
  - context: "ar_expression_parser_tests"
    message: "ERROR: Expected literal (string or number) (at position 0)"
    comment: "Testing parser error handling for invalid expressions"
```

**Important notes**:
- Test names may vary by environment (e.g., sanitizer tests append suffixes) ([details](../../kb/log-format-variation-handling.md))
- YAML quote handling can affect matching ([details](../../kb/yaml-string-matching-pitfalls.md))
- Whitelist is applied uniformly across all checks ([details](../../kb/uniform-filtering-application.md))