Systematically analyze and fix errors in the whitelist file to reduce technical debt.

## MANDATORY KB Consultation

Before starting error analysis:
1. Search: `grep "whitelist\|error\|systematic\|checkpoint" kb/README.md`
2. Must read these specific articles:
   - systematic-whitelist-error-resolution
   - systematic-error-whitelist-reduction
   - checkpoint-conditional-flow-pattern - Understanding conditional skipping in checkpoint workflows
3. Apply the systematic approach from these articles

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/fix-errors-whitelisted` workflow is already in progress:

```bash
make checkpoint-status CMD=fix-errors-whitelisted VERBOSE=--verbose
# Resume: make checkpoint-update CMD=fix-errors-whitelisted STEP=N
# Or reset: make checkpoint-cleanup CMD=fix-errors-whitelisted && make checkpoint-init CMD=fix-errors-whitelisted STEPS='"Count Errors" "Group by Test" "Find Patterns" "Select Target" "Analyze Root Cause" "Choose Strategy" "Plan Implementation" "Verify Current State" "Apply Fix" "Test Fix" "Remove Whitelist Entries" "Update Whitelist Total" "Update TODO.md" "Update CHANGELOG" "Final Verification" "Create Commit"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/fix_errors_whitelisted-progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=fix-errors-whitelisted STEPS='"Count Errors" "Group by Test" "Find Patterns" "Select Target" "Analyze Root Cause" "Choose Strategy" "Plan Implementation" "Verify Current State" "Apply Fix" "Test Fix" "Remove Whitelist Entries" "Update Whitelist Total" "Update TODO.md" "Update CHANGELOG" "Final Verification" "Create Commit"'
else
  make checkpoint-status CMD=fix-errors-whitelisted
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/fix_errors_whitelisted-progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Fix Whitelisted Errors
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic error fixing with minimum requirements. The process has 16 checkpoints across 4 phases.

### Initialize Tracking
```bash
# Start the error fixing process
make checkpoint-init CMD=fix-errors-whitelisted STEPS='"Count Errors" "Group by Test" "Find Patterns" "Select Target" "Analyze Root Cause" "Choose Strategy" "Plan Implementation" "Verify Current State" "Apply Fix" "Test Fix" "Remove Whitelist Entries" "Update Whitelist Total" "Update TODO.md" "Update CHANGELOG" "Final Verification" "Create Commit"'
```

**Expected output:**
```
📍 Starting: fix-errors-whitelisted (16 steps)
📁 Tracking: /tmp/fix-errors-whitelisted-progress.txt
→ Run: make checkpoint-update CMD=fix-errors-whitelisted STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=fix-errors-whitelisted
```

**Expected output (example at 50% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Fix at least 5 whitelist errors
- [ ] Verify each fix with check-logs
- [ ] Update all documentation
- [ ] Ensure build remains clean

### Process Overview

This command guides you through:
1. Analyzing the whitelist for patterns
2. Identifying high-impact fixes
3. Implementing solutions
4. Removing whitelist entries
5. Tracking progress

### Important Context

The whitelist contains both intentional test errors AND success messages that check-logs flags. Not all entries represent problems - many are necessary for testing error handling ([details](../../../kb/whitelist-success-message-management.md)). Classify errors by type to determine appropriate fixing strategies ([details](../../../kb/documentation-error-type-classification.md)). Check-logs failures will block CI ([details](../../../kb/ci-check-logs-requirement.md)). Some errors may come from redundant test cleanup that can be removed ([details](../../../kb/redundant-test-cleanup-anti-pattern.md)).

### Stage 1: Analysis (Steps 1-3)

#### [CHECKPOINT START - STAGE 1]

#### [CHECKPOINT END]

#### Step 1: Analyze Current Whitelist

#### Steps 1-3: Analyze Whitelist

Run analysis using helper script:

```bash
# Analyze whitelist and save counts
./scripts/analyze-whitelist.sh log_whitelist.yaml | tee /tmp/fix-errors-whitelisted-counts.txt

# Mark all analysis steps complete
make checkpoint-update CMD=fix-errors-whitelisted STEP=1
make checkpoint-update CMD=fix-errors-whitelisted STEP=2
make checkpoint-update CMD=fix-errors-whitelisted STEP=3
```

The script provides:
1. Total count of whitelisted errors
2. Top 10 tests with most errors
3. Top 10 most common error messages

#### [ANALYSIS GATE]
```bash
# MANDATORY: Must complete analysis before proceeding
make checkpoint-gate CMD=fix-errors-whitelisted GATE="Analysis" REQUIRED="1,2,3"
```

**Expected gate output:**
```
✅ GATE 'Analysis' - PASSED
   Verified: Steps 1,2,3
```

### Stage 2: Strategy Development (Steps 4-7)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

#### Step 2: Identify Target Pattern

#### Step 4: Select Target

Based on the analysis, identify which errors to fix. Look for:
- **Similar errors** across multiple tests (e.g., wake message field access)
- **High frequency** patterns that affect many tests
- **Clear root causes** with known solutions

Example patterns to look for:
```bash
# Wake message field access errors
grep -B1 -A1 "Cannot access field.*on STRING value.*__wake__" log_whitelist.yaml | grep -E "context:|message:"

# Method not found errors
grep -B1 -A1 "has no method" log_whitelist.yaml | grep -E "context:|message:"

# Intentional test errors
grep -B1 -A1 "Intentional.*error" log_whitelist.yaml | grep -E "context:|message:"

# After selecting target pattern
make checkpoint-update CMD=fix-errors-whitelisted STEP=4
```

**IMPORTANT**: Select errors that affect at least 5 whitelist entries to meet minimum requirements.

#### Step 5: Analyze Root Cause

Document the root cause of the selected error pattern:
```bash
echo "Root cause analysis:"
echo "1. Why does this error occur?"
echo "2. What is the underlying issue?"
echo "3. How many entries are affected?"

make checkpoint-update CMD=fix-errors-whitelisted STEP=5
```

#### Step 3: Develop Fix Strategy

#### Step 6: Choose Strategy

Based on the pattern identified, determine the fix approach:


### For Test Ownership Issues ([details](../../../kb/test-fixture-message-ownership.md))
```
1. Identify tests bypassing system flow
2. Add ownership management before execution
3. Clean up with destroy_if_owned
4. Test and remove whitelist entries
```

### For CI Network Timeouts ([details](../../../kb/ci-network-timeout-diagnosis.md))
```
1. Check if action is deprecated
2. Find recommended replacement
3. Verify maintainer credibility ([details](../../kb/tool-maintainer-verification-pattern.md))
4. Update to latest version ([details](../../kb/tool-version-selection-due-diligence.md))
```

### For Overly Broad Patterns ([details](../../../kb/whitelist-specificity-pattern.md))
```
1. Make error messages more specific
2. Use unique field names in tests
3. Update whitelist with specific patterns
4. Verify only intended errors are whitelisted
```

```bash
# Document chosen strategy
make checkpoint-update CMD=fix-errors-whitelisted STEP=6
```

#### Step 7: Plan Implementation

Create detailed implementation plan:
```bash
echo "Implementation plan:"
echo "1. Files to modify:"
echo "2. Changes to make:"
echo "3. Expected outcome:"
echo "4. Entries to remove: [count]"

make checkpoint-update CMD=fix-errors-whitelisted STEP=7
```

#### [STRATEGY GATE]
```bash
# MANDATORY: Must have clear strategy before implementation
make checkpoint-gate CMD=fix-errors-whitelisted GATE="Strategy" REQUIRED="4,5,6,7"
```

**Expected gate output:**
```
✅ GATE 'Strategy' - PASSED
   Verified: Steps 4,5,6,7
```

### Stage 3: Implementation (Steps 8-11)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

#### [CRITICAL IMPLEMENTATION GATE]
```bash
# ⚠️ CRITICAL: Final check before modifying code
make checkpoint-gate CMD=fix-errors-whitelisted GATE="Implementation Ready" REQUIRED="1,2,3,4,5,6,7"
```

**Expected gate output:**
```
✅ GATE 'Implementation Ready' - PASSED
   Verified: Steps 1,2,3,4,5,6,7
```

#### Step 4: Implement Fixes

#### Step 8: Verify Current State

For each identified issue:

```bash
# 1. Verify current errors
make clean build 2>&1
make check-logs | grep -A5 "context_name"

make checkpoint-update CMD=fix-errors-whitelisted STEP=8
```

#### Step 9: Apply Fix

```bash
# 2. Apply fix to the source file
# (Edit the method/test file with the fix pattern)

echo "Files modified:"
git diff --name-only

make checkpoint-update CMD=fix-errors-whitelisted STEP=9
```

#### Step 10: Test Fix

```bash
# 3. Verify fix worked
make clean build 2>&1
if make check-logs | grep -A5 "context_name"; then
  echo "❌ Errors still present - fix may not be complete"
else
  echo "✅ Errors resolved!"
fi

make checkpoint-update CMD=fix-errors-whitelisted STEP=10
```

#### Step 11: Remove Whitelist Entries

```bash
# 4. Remove whitelist entries
# Edit log_whitelist.yaml to remove the fixed entries

# Count removed entries
source /tmp/fix-errors-whitelisted-counts.txt
AFTER_COUNT=$(grep "^  -" log_whitelist.yaml | wc -l)
REMOVED=$((BEFORE_COUNT - AFTER_COUNT))

echo "Whitelist entries removed: $REMOVED"
if [ $REMOVED -lt 5 ]; then
  echo "⚠️ WARNING: Only removed $REMOVED entries (minimum: 5)"
  echo "Continue fixing more errors to meet requirement."
else
  echo "✅ Minimum requirement met: $REMOVED entries removed"
fi

make checkpoint-update CMD=fix-errors-whitelisted STEP=11
```

### Stage 4: Documentation & Commit (Steps 12-16)

#### [CHECKPOINT START - STAGE 4]

#### [CHECKPOINT END]

#### Step 5: Update Documentation

#### Step 12: Update Whitelist Total

After fixing errors:

1. **Update whitelist total**:
   ```yaml
   # Total entries: [new count]
   ```
   ```bash
   # Verify count matches
   ACTUAL=$(grep "^  -" log_whitelist.yaml | wc -l)
   echo "Updated whitelist total to: $ACTUAL entries"
   
   make checkpoint-update CMD=fix-errors-whitelisted STEP=12
   ```

#### Step 13: Update TODO.md

2. **Update TODO.md**:
   ```markdown
   ### [Description] Error Fix (Completed YYYY-MM-DD)
   - [x] Fixed [error type] in [component]; removed [N] whitelist entries ([new total] remaining)
   ```
   ```bash
   make checkpoint-update CMD=fix-errors-whitelisted STEP=13
   ```

#### Step 14: Update CHANGELOG.md

3. **Update CHANGELOG.md** if significant (>10 entries removed):
   ```bash
   source /tmp/fix-errors-whitelisted-counts.txt
   AFTER_COUNT=$(grep "^  -" log_whitelist.yaml | wc -l)
   REMOVED=$((BEFORE_COUNT - AFTER_COUNT))
   
   if [ $REMOVED -ge 10 ]; then
     echo "📝 Update CHANGELOG.md - removed $REMOVED entries (significant)"
   else
     echo "ℹ️ CHANGELOG update optional - only $REMOVED entries removed"
   fi
   
   make checkpoint-update CMD=fix-errors-whitelisted STEP=14
   ```

#### [DOCUMENTATION GATE]
```bash
# MANDATORY: Ensure all documentation is updated
make checkpoint-gate CMD=fix-errors-whitelisted GATE="Documentation" REQUIRED="12,13,14"
```

**Expected gate output:**
```
✅ GATE 'Documentation' - PASSED
   Verified: Steps 12,13,14
```

#### Step 6: Verify and Commit

#### Step 15: Final Verification

```bash
# Final verification
make clean build 2>&1
if ! make check-logs; then
  echo "❌ Build verification failed - fix issues before committing"
  exit 1
fi

echo "✅ Build verification passed"
make checkpoint-update CMD=fix-errors-whitelisted STEP=15
```

#### Step 16: Create Commit

```bash
# Review changes
git diff log_whitelist.yaml
git diff [modified files]

# Verify minimum requirement met
source /tmp/fix-errors-whitelisted-counts.txt
AFTER_COUNT=$(grep "^  -" log_whitelist.yaml | wc -l)
REMOVED=$((BEFORE_COUNT - AFTER_COUNT))

if [ $REMOVED -lt 5 ]; then
  echo "❌ Cannot commit: Only removed $REMOVED entries (minimum: 5)"
  exit 1
fi

# Commit
git add -A
git commit -m "fix: resolve [error type] in [component]

Removed $REMOVED whitelist entries by [explanation of fix].
Whitelist reduced from $BEFORE_COUNT to $AFTER_COUNT entries."

make checkpoint-update CMD=fix-errors-whitelisted STEP=16
```

#### [CHECKPOINT COMPLETE]
```bash
./scripts/complete-checkpoint.sh fix-errors-whitelisted
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 fix-errors-whitelisted: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
rm -f /tmp/fix-errors-whitelisted-counts.txt
```

## Common Fix Patterns


### Test Message Ownership
```c
ar_data__take_ownership(message, fixture);
// ... execute test ...
ar_data__destroy_if_owned(message, fixture);
```

### Error Specificity
```c
// Instead of generic field names
"message.method_name"
// Use unique identifiers
"message.type_mismatch_test_field"
```

## Progress Tracking

Keep track of your whitelist reduction:
- Document each fix in TODO.md
- Note patterns discovered for future use
- Consider creating KB articles for new patterns
- Track total count reduction over time

## Troubleshooting

### If checkpoint tracking gets stuck:
```bash
# Check current status
make checkpoint-status CMD=fix-errors-whitelisted

# Reset if needed
make checkpoint-cleanup CMD=fix-errors-whitelisted
```

### If you've already done analysis:
```bash
# Skip to implementation phase
make checkpoint-init CMD=fix-errors-whitelisted STEPS='...'
for i in {1..7}; do
  make checkpoint-update CMD=fix-errors-whitelisted STEP=$i
done
```

### If minimum requirement not met:
Continue fixing more errors of the same type or select an additional pattern that affects more entries.

## Tips for Success

1. **Start small**: Fix one error type at a time
2. **Test thoroughly**: Ensure fixes don't break tests
3. **Document patterns**: Create KB articles for reusable solutions
4. **Be specific**: Make whitelist entries as specific as possible
5. **Track progress**: Celebrate each reduction in the count
6. **Meet minimums**: Always fix at least 5 errors per session

## Related Documentation

### Checkpoint Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Error Fixing Patterns
- [Systematic Whitelist Error Resolution](../../../kb/systematic-whitelist-error-resolution.md)
- [Test Fixture Message Ownership](../../../kb/test-fixture-message-ownership.md)
- [Whitelist Specificity Pattern](../../../kb/whitelist-specificity-pattern.md)
- [Systematic Error Whitelist Reduction](../../../kb/systematic-error-whitelist-reduction.md)
- [Check-Logs Deep Analysis Pattern](../../../kb/check-logs-deep-analysis-pattern.md)
- [Whitelist vs Pattern Filtering](../../../kb/whitelist-vs-pattern-filtering.md)

Remember: Every fixed error improves code quality and reduces technical debt!