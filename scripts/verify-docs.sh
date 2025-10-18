#!/bin/bash
set -e

# Step 4: Verify documentation fixes
# Usage: ./scripts/check-docs-verify.sh
# Purpose: Run make check-docs again to verify all fixes were successful
# Returns: Exit 0 after verification

echo ""
echo "========== STAGE 3: Verify and Commit =========="
echo ""

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

# Resolution Gate
echo ""
echo "========== RESOLUTION GATE =========="
echo ""

source /tmp/check-docs-stats.txt
if [ "$FINAL_STATUS" != "PASS" ]; then
  echo "⚠️ WARNING: Not all errors resolved"
  echo "Consider enhancing the batch fix script"
fi

./scripts/update-checkpoint.sh check-docs STEP=4
