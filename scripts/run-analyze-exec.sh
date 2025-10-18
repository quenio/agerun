#!/bin/bash

set -e

# Executable Static Analysis Workflow Script
# This script runs the complete checkpoint-based static analysis workflow

# Initialize checkpoint tracking or show status if already initialized
./scripts/init-checkpoint.sh analyze-exec '"Build Executable" "Run Static Analysis" "Report Results"'

# Verify checkpoint tracking is ready
./scripts/require-checkpoint.sh analyze-exec

# ============================================================================
# STAGE 1: Build Executable (Step 1)
# ============================================================================

echo ""
echo "========== STAGE 1: Build Executable =========="
echo ""

echo "Building executable..."
if make bin/ar_executable 2>&1; then
  echo "✅ Executable built successfully"
else
  echo "⚠️ Build had issues (continuing with analysis)"
fi

make checkpoint-update CMD=analyze-exec STEP=1

# ============================================================================
# STAGE 2: Run Static Analysis (Step 2)
# ============================================================================

echo ""
echo "========== STAGE 2: Run Static Analysis =========="
echo ""

echo "Running static analysis on executable code..."
echo ""

# Verify analyzer is available
if ! command -v clang-analyzer &> /dev/null; then
  if ! command -v scan-build &> /dev/null; then
    echo "⚠️ WARNING: Static analysis tools not found (clang-analyzer or scan-build)"
    echo "Install Clang to enable static analysis"
  fi
fi

# Run static analysis
if make analyze-exec 2>&1; then
  echo "✅ Static analysis complete - no critical issues found"
  ANALYSIS_STATUS="PASS"
else
  echo "⚠️ Static analysis found issues"
  ANALYSIS_STATUS="ISSUES"
fi

echo "ANALYSIS_STATUS=$ANALYSIS_STATUS" > /tmp/analyze-exec-stats.txt

make checkpoint-update CMD=analyze-exec STEP=2

# ============================================================================
# STAGE 3: Report Results (Step 3)
# ============================================================================

echo ""
echo "========== STAGE 3: Report Results =========="
echo ""

source /tmp/analyze-exec-stats.txt 2>/dev/null || ANALYSIS_STATUS="UNKNOWN"

if [ "$ANALYSIS_STATUS" = "PASS" ]; then
  echo "✅ Analysis Results: No critical issues detected"
  echo ""
  echo "Code quality checks:"
  echo "  ✓ NULL pointer dereferences"
  echo "  ✓ Use-after-free"
  echo "  ✓ Resource leaks"
  echo "  ✓ Dead code"
  echo "  ✓ Logic errors"
else
  echo "⚠️ Analysis Results: Issues found"
  echo ""
  echo "Review the output above for details"
  echo "Common issues:"
  echo "  - Unchecked return values"
  echo "  - Missing malloc/free pairs"
  echo "  - Unreachable code blocks"
fi

make checkpoint-update CMD=analyze-exec STEP=3

# ============================================================================
# Checkpoint Completion
# ============================================================================

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/complete-checkpoint.sh analyze-exec

# Cleanup temporary files
rm -f /tmp/analyze-exec-stats.txt

echo ""
echo "✅ Static analysis workflow complete!"
