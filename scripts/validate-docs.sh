#!/bin/bash
set -e

# Step 1: Initial documentation validation
# Usage: ./scripts/check-docs-initial.sh
# Purpose: Run make check-docs and save error count for conditional flow
# Returns: Exit 0 if validation runs, saves ERROR_COUNT to /tmp/check-docs-stats.txt

echo ""
echo "========== STAGE 1: Initial Check =========="
echo ""

echo "Running documentation validation..."
ERROR_COUNT=0

if make check-docs > /tmp/check-docs-output.txt 2>&1
then
  cat /tmp/check-docs-output.txt
  echo "✅ No documentation errors found!"
  echo "ERROR_COUNT=0" > /tmp/check-docs-stats.txt
else
  cat /tmp/check-docs-output.txt
  ERROR_COUNT=$(grep -E "ERROR|FAIL" /tmp/check-docs-output.txt | wc -l || echo "0")
  echo "⚠️ Found $ERROR_COUNT documentation errors"
  echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt
fi

./scripts/update-checkpoint.sh check-docs STEP=1
