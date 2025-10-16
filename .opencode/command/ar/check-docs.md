Run documentation validation and fix any errors found using an iterative approach, then commit and push the fixes.

## MANDATORY KB Consultation

Before validation:
1. Search: `grep "documentation\|validation\|example\|contract" kb/README.md`
2. Must read:
   - documentation-validation-enhancement-patterns
   - validated-documentation-examples
   - yaml-implicit-contract-validation-pattern
   - validation-feedback-loop-effectiveness
3. Apply iterative validation approach
4. For YAML files: Ensure reader/writer contracts are explicit ([details](../../../kb/yaml-implicit-contract-validation-pattern.md))

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. See [Checkpoint Sequential Execution Discipline](../../../kb/checkpoint-sequential-execution-discipline.md) for important requirements about sequential ordering and work verification.

### In-Progress Workflow Detection

If a `/check-docs` workflow is already in progress:

```bash
make checkpoint-status CMD=check-docs VERBOSE=--verbose
# Resume: make checkpoint-update CMD=check-docs STEP=N
# Or reset: make checkpoint-cleanup CMD=check-docs && make checkpoint-init CMD=check-docs STEPS='"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/check_docs_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=check-docs STEPS='"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
else
  make checkpoint-status CMD=check-docs
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/check_docs_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Check Documentation
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic documentation validation and error resolution. The process has 5 checkpoints across 3 phases with error-driven gates.

### Initialize Tracking
```bash
# Start the documentation check process
make checkpoint-init CMD=check-docs STEPS='"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
```

**Expected output:**
```
📍 Starting: check-docs (5 steps)
📁 Tracking: /tmp/check-docs_progress.txt
→ Run: make checkpoint-update CMD=check-docs STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=check-docs
```

**Expected output (example at 60% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] All documentation errors fixed
- [ ] make check-docs passes
- [ ] Changes committed and pushed
- [ ] Git status shows clean tree
### Stage 1: Initial Check (Step 1)

#### [CHECKPOINT START - STAGE 1]

#### [CHECKPOINT END]

### Process

#### Step 1: Initial Check

```bash
# Run initial documentation validation
echo "Running documentation validation..."
ERROR_COUNT=0

if make check-docs 2>&1 | tee /tmp/check-docs-output.txt; then
  echo "✅ No documentation errors found!"
  echo "ERROR_COUNT=0" > /tmp/check-docs-stats.txt
else
  ERROR_COUNT=$(grep -c "ERROR\|FAIL" /tmp/check-docs-output.txt || echo "0")
  echo "⚠️ Found $ERROR_COUNT documentation errors"
  echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt
fi

make checkpoint-update CMD=check-docs STEP=1
```

1. **Initial Check**: Run `make check-docs` to identify all documentation errors

#### [ERROR GATE - Conditional Flow]
```bash
# Determine flow: skip fix steps if no errors, proceed through them if errors found
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

if [ $ERROR_COUNT -eq 0 ]; then
  echo "✅ No errors to fix - skipping to commit phase"
  make checkpoint-update CMD=check-docs STEP=2
  make checkpoint-update CMD=check-docs STEP=3
  make checkpoint-update CMD=check-docs STEP=4
else
  make checkpoint-gate CMD=check-docs GATE="Errors Found" REQUIRED="1"
fi
```

**Expected gate output (when errors found):**
```
========================================
   GATE: Errors Found
========================================

⚠️ Documentation errors detected!

Error Summary:
  Total errors: 12
  Files affected: 5

✅ GATE PASSED: Analysis complete

Proceeding to fix phase.
```

### Stage 2: Fix Errors (Steps 2-3)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

#### Step 2: Preview Fixes

```bash
# Preview what the batch fix script would change
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

if [ $ERROR_COUNT -gt 0 ]; then
  echo "Previewing fixes for $ERROR_COUNT errors..."
  python3 scripts/batch_fix_docs.py --dry-run | tee /tmp/fix-preview.txt
  
  echo "Review the proposed changes above."
  echo "✅ Fix preview complete"
else
  echo "No errors to fix - skipping preview"
fi

make checkpoint-update CMD=check-docs STEP=2
```

2. **If errors are found**, fix them iteratively:
   - First run `python3 scripts/batch_fix_docs.py --dry-run` to preview what changes would be made
   - Review the proposed changes to ensure they look correct

#### Step 3: Apply Fixes

```bash
# Apply the fixes
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

if [ $ERROR_COUNT -gt 0 ]; then
  echo "Applying fixes..."
  python3 scripts/batch_fix_docs.py
  
  FILES_FIXED=$(git diff --name-only | grep "\.md$" | wc -l)
  echo "✅ Fixed $FILES_FIXED documentation files"
else
  echo "No fixes needed"
fi

make checkpoint-update CMD=check-docs STEP=3
```

   - If the changes look good, run `python3 scripts/batch_fix_docs.py` to apply the fixes
   - Run `make check-docs` again to verify the fixes worked

### Stage 3: Verify and Commit (Steps 4-5)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

#### Step 4: Verify Resolution

```bash
# Verify all errors are fixed
echo "Verifying fixes..."
if make check-docs 2>&1; then
  echo "✅ All documentation errors resolved!"
  FINAL_STATUS="PASS"
else
  REMAINING=$(make check-docs 2>&1 | grep -c "ERROR" || echo "0")
  echo "⚠️ $REMAINING errors still remain"
  echo "May need manual intervention or script enhancement"
  FINAL_STATUS="PARTIAL"
fi

echo "FINAL_STATUS=$FINAL_STATUS" >> /tmp/check-docs-stats.txt
make checkpoint-update CMD=check-docs STEP=4
```

3. **Repeat if needed**: If errors remain after the first fix attempt:
   - Analyze why some errors weren't fixed
   - Run the batch fix script again (with dry-run first, then for real)
   - Continue until all errors are resolved or identify which errors need manual intervention

#### [RESOLUTION GATE]
```bash
# ⚠️ CRITICAL: Verify all errors resolved
source /tmp/check-docs-stats.txt
if [ "$FINAL_STATUS" != "PASS" ]; then
  echo "⚠️ WARNING: Not all errors resolved"
  echo "Consider enhancing the batch fix script"
fi
make checkpoint-gate CMD=check-docs GATE="Resolution" REQUIRED="4"
```

#### Step 5: Commit and Push

```bash
# Commit and push fixes if any
FILES_CHANGED=$(git diff --name-only | wc -l)

if [ $FILES_CHANGED -gt 0 ]; then
  echo "Committing documentation fixes..."
  git add -A
  git commit -m "docs: fix documentation validation errors"
  git push
  
  # Verify push
  git status
  echo "✅ Documentation fixes committed and pushed"
else
  echo "✅ No changes to commit"
fi

make checkpoint-update CMD=check-docs STEP=5
```

4. **Commit and push the fixes**: Once all documentation errors are resolved:
   - Check git status to see what files were modified
   - Stage all documentation fixes with `git add`
   - Commit with message: "docs: fix documentation validation errors"
   - Push to the remote repository
   - Verify push completed with `git status`

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=check-docs
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: check-docs
========================================

Progress: 5/5 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Final Status:
  Errors found: 12
  Errors fixed: 12
  Validation: PASS
  Changes pushed: Yes

Documentation is now valid!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=check-docs
rm -f /tmp/check-docs-*.txt /tmp/fix-preview.txt
```

### What the batch fix script handles
- **Non-existent function/type references**: Adds EXAMPLE tags or replaces with real types ([details](../../../kb/documentation-placeholder-validation-pattern.md))
- **Broken relative markdown links**: Calculates and fixes correct relative paths
- **Absolute paths**: Converts to relative paths
- **Additional contexts** (enhanced): Struct fields, function params, sizeof, type casts, variable declarations ([details](../../../kb/batch-documentation-fix-enhancement.md))

### Important notes
- Always use `--dry-run` first to preview changes before applying them
- The script only modifies `.md` files
- Some complex errors may require manual intervention ([details](../../../kb/documentation-error-type-classification.md))
- If the script can't fix all errors, enhance it rather than fixing manually ([details](../../../kb/script-enhancement-over-one-off.md))
- Use iterative validation feedback loops for systematic error resolution ([details](../../../kb/validation-feedback-loop-effectiveness.md))

This validation ensures:
- All code examples use real AgeRun types ([details](../../../kb/validated-documentation-examples.md))
- File references are valid
- Function names exist in the codebase
- Support for both C and Zig documentation
- Markdown links follow GitHub-compatible patterns ([details](../../../kb/markdown-link-resolution-patterns.md))

For manual fixing of validation errors, see:
- [Documentation Validation Error Patterns](../../../kb/documentation-validation-error-patterns.md)
- [Documentation Validation Enhancement Patterns](../../../kb/documentation-validation-enhancement-patterns.md)

For module documentation consistency:
- [Documentation Index Consistency Pattern](../../../kb/documentation-index-consistency-pattern.md) - Update modules/README.md after integration
- [Documentation Implementation Sync](../../../kb/documentation-implementation-sync.md) - Keep docs in sync with code