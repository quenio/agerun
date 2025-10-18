#!/bin/bash
set -e

# Step 2: Preview documentation fixes (dry-run)
# Usage: ./scripts/check-docs-preview-fixes.sh
# Purpose: Show preview of fixes before applying them (conditional on errors)
# Returns: Exit 0 after preview

echo ""
echo "========== STAGE 2: Fix Errors (Preview) =========="
echo ""

# Load error count from previous step
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

echo ""
echo "--- Step 2: Preview Fixes ---"
echo ""

if [ $ERROR_COUNT -gt 0 ]; then
  echo "Previewing fixes for $ERROR_COUNT errors..."
  python3 scripts/batch_fix_docs.py --dry-run | tee /tmp/fix-preview.txt

  echo "Review the proposed changes above."
  echo "✅ Fix preview complete"
else
  echo "No errors to fix - skipping preview"
fi

./scripts/update-checkpoint.sh check-docs STEP=2
