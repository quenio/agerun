#!/bin/bash
set -e

# Documentation Check Workflow Convenience Entry Point
# This script orchestrates the checkpoint-based documentation validation and fix workflow
# by running individual focused scripts for each step

# Initialize checkpoint tracking
./scripts/init-checkpoint.sh check-docs '"Initial Check" "Preview Fixes" "Apply Fixes" "Verify Resolution" "Commit and Push"'
./scripts/require-checkpoint.sh check-docs

# Step 1: Initial Check
./scripts/check-docs-initial.sh

# Conditional Flow: Check if errors found
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

if [ $ERROR_COUNT -eq 0 ]; then
  echo "✅ No errors to fix - skipping to commit phase"
  ./scripts/update-checkpoint.sh check-docs STEP=2
  ./scripts/update-checkpoint.sh check-docs STEP=3
  ./scripts/update-checkpoint.sh check-docs STEP=4
else
  ./scripts/gate-checkpoint.sh check-docs "Errors Found" "1"

  # Steps 2-4: Fix and verify
  ./scripts/check-docs-preview-fixes.sh
  ./scripts/check-docs-apply-fixes.sh
  ./scripts/check-docs-verify.sh
  ./scripts/gate-checkpoint.sh check-docs "Resolution" "4"
fi

# Step 5: Commit and Push
./scripts/check-docs-commit.sh

# Cleanup
./scripts/complete-checkpoint.sh check-docs
rm -f /tmp/check-docs-*.txt /tmp/fix-preview.txt

echo "✅ Documentation check workflow complete!"
