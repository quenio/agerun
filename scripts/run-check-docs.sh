#!/bin/bash

set -e

# Documentation Check Workflow Script
# This script runs the complete checkpoint-based documentation validation and fix workflow

# Initialize checkpoint tracking or show status if already initialized
./scripts/init-checkpoint.sh check-docs '"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'

# Verify checkpoint tracking is ready
./scripts/require-checkpoint.sh check-docs

# ============================================================================
# STAGE 1: Initial Check (Step 1)
# ============================================================================

echo ""
echo "========== STAGE 1: Initial Check =========="
echo ""

# Run initial documentation validation
echo "Running documentation validation..."
ERROR_COUNT=0

if make check-docs 2>&1 | tee /tmp/check-docs-output.txt
then
  echo "✅ No documentation errors found!"
  echo "ERROR_COUNT=0" > /tmp/check-docs-stats.txt
else
  ERROR_COUNT=$(grep -E "ERROR|FAIL" /tmp/check-docs-output.txt | wc -l || echo "0")
  echo "⚠️ Found $ERROR_COUNT documentation errors"
  echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt
fi

make checkpoint-update CMD=check-docs STEP=1

# ============================================================================
# ERROR GATE - Conditional Flow
# ============================================================================

echo ""
echo "========== ERROR GATE =========="
echo ""

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

# ============================================================================
# STAGE 2: Fix Errors (Steps 2-3)
# ============================================================================

echo ""
echo "========== STAGE 2: Fix Errors =========="
echo ""

# Step 2: Preview Fixes
echo ""
echo "--- Step 2: Preview Fixes ---"
echo ""

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

# Step 3: Apply Fixes
echo ""
echo "--- Step 3: Apply Fixes ---"
echo ""

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

# ============================================================================
# STAGE 3: Verify and Commit (Steps 4-5)
# ============================================================================

echo ""
echo "========== STAGE 3: Verify and Commit =========="
echo ""

# Step 4: Verify Resolution
echo ""
echo "--- Step 4: Verify Resolution ---"
echo ""

echo "Verifying fixes..."
if make check-docs 2>&1
then
  echo "✅ All documentation errors resolved!"
  FINAL_STATUS="PASS"
else
  REMAINING=$(make check-docs 2>&1 | grep "ERROR" | wc -l || echo "0")
  echo "⚠️ $REMAINING errors still remain"
  echo "May need manual intervention or script enhancement"
  FINAL_STATUS="PARTIAL"
fi

echo "FINAL_STATUS=$FINAL_STATUS" >> /tmp/check-docs-stats.txt
make checkpoint-update CMD=check-docs STEP=4

# Resolution Gate
echo ""
echo "========== RESOLUTION GATE =========="
echo ""

source /tmp/check-docs-stats.txt
if [ "$FINAL_STATUS" != "PASS" ]; then
  echo "⚠️ WARNING: Not all errors resolved"
  echo "Consider enhancing the batch fix script"
fi
make checkpoint-gate CMD=check-docs GATE="Resolution" REQUIRED="4"

# Step 5: Commit and Push
echo ""
echo "--- Step 5: Commit and Push ---"
echo ""

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

# ============================================================================
# Checkpoint Completion
# ============================================================================

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/complete-checkpoint.sh check-docs

# Cleanup temporary files
rm -f /tmp/check-docs-*.txt /tmp/fix-preview.txt

echo ""
echo "✅ Documentation check workflow complete!"
