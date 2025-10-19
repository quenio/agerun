#!/bin/bash
set -e

# Step 3: Apply documentation fixes
# Usage: ./scripts/check-docs-apply-fixes.sh
# Purpose: Run batch fix script to fix all identified errors (conditional on errors)
# Returns: Exit 0 after fixes applied

echo ""
echo "========== STAGE 2: Fix Errors (Apply) =========="
echo ""

# Load error count from initial check
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

echo ""
echo "--- Step 3: Apply Fixes ---"
echo ""

if [ $ERROR_COUNT -gt 0 ]; then
  echo "Applying fixes..."
  python3 scripts/batch_fix_docs.py

  FILES_FIXED=$(git diff --name-only | grep "\.md$" | wc -l)
  echo "✅ Fixed $FILES_FIXED documentation files"
else
  echo "No fixes needed"
fi
