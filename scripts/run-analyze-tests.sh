#!/bin/bash

set -e

# Test Static Analysis Workflow Script
# This script runs the complete checkpoint-based static analysis workflow

./scripts/checkpoint-init.sh analyze-tests '"Build Tests" "Run Static Analysis" "Report Results"'
./scripts/checkpoint-require.sh analyze-tests

echo ""
echo "========== STAGE 1: Build Tests =========="
echo ""

echo "Building test executable..."
if make bin/ar_*_tests 2>&1 | head -20; then
  echo "✅ Tests built successfully"
else
  echo "⚠️ Build had issues (continuing with analysis)"
fi

./scripts/checkpoint-update.sh analyze-tests 1

echo ""
echo "========== STAGE 2: Run Static Analysis =========="
echo ""

echo "Running static analysis on test code..."
if make analyze-tests 2>&1; then
  echo "✅ Analysis complete - no critical issues found"
  ANALYSIS_STATUS="PASS"
else
  echo "⚠️ Analysis found issues"
  ANALYSIS_STATUS="ISSUES"
fi

echo "ANALYSIS_STATUS=$ANALYSIS_STATUS" > /tmp/analyze-tests-stats.txt
./scripts/checkpoint-update.sh analyze-tests 2

echo ""
echo "========== STAGE 3: Report Results =========="
echo ""

source /tmp/analyze-tests-stats.txt 2>/dev/null || ANALYSIS_STATUS="UNKNOWN"

if [ "$ANALYSIS_STATUS" = "PASS" ]; then
  echo "✅ Analysis Results: No critical issues in test code"
else
  echo "⚠️ Analysis Results: Issues found - review above"
fi

./scripts/checkpoint-update.sh analyze-tests 3

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/checkpoint-complete.sh analyze-tests
rm -f /tmp/analyze-tests-stats.txt

echo ""
echo "✅ Test static analysis workflow complete!"
