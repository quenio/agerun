#!/bin/bash

set -e

# Naming Conventions Check Workflow Script
# This script runs the complete checkpoint-based naming convention validation workflow

# Initialize checkpoint tracking or show status if already initialized
./scripts/init-checkpoint.sh check-naming '"Check Naming" "Analyze Violations" "Document Findings"'

# Verify checkpoint tracking is ready
./scripts/require-checkpoint.sh check-naming

# ============================================================================
# STAGE 1: Check Naming Conventions (Step 1)
# ============================================================================

echo ""
echo "========== STAGE 1: Check Naming Conventions =========="
echo ""

# Run naming convention validation
echo "Running naming convention checks..."
VIOLATION_COUNT=0

if make check-naming 2>&1 | tee /tmp/check-naming-output.txt
then
  echo "✅ All naming conventions are correct!"
  echo "VIOLATION_COUNT=0" > /tmp/check-naming-stats.txt
else
  VIOLATION_COUNT=$(grep -E "ERROR|Invalid|violation" /tmp/check-naming-output.txt | wc -l || echo "0")
  echo "⚠️ Found $VIOLATION_COUNT naming violations"
  echo "VIOLATION_COUNT=$VIOLATION_COUNT" > /tmp/check-naming-stats.txt
fi

make checkpoint-update CMD=check-naming STEP=1

# ============================================================================
# STAGE 2: Analyze Violations (Step 2)
# ============================================================================

echo ""
echo "========== STAGE 2: Analyze Violations =========="
echo ""

source /tmp/check-naming-stats.txt 2>/dev/null || VIOLATION_COUNT=0

if [ $VIOLATION_COUNT -gt 0 ]; then
  echo "Analyzing $VIOLATION_COUNT naming violations..."
  echo ""
  echo "Violations found:"
  grep -E "ERROR|Invalid" /tmp/check-naming-output.txt | head -20 || true
  echo ""
  echo "Next steps:"
  echo "1. Review the violations above"
  echo "2. Fix violations in the source code"
  echo "3. Re-run 'make check-naming' to verify fixes"
  echo ""
  echo "Naming conventions:"
  echo "  - Typedefs: ar_<module>_t"
  echo "  - Functions: ar_<module>__<function>"
  echo "  - Static functions: _<function>"
  echo "  - Test functions: test_<module>__<test>"
  echo "  - Heap macros: AR__HEAP__<OPERATION>"
else
  echo "✅ No naming violations to analyze"
fi

make checkpoint-update CMD=check-naming STEP=2

# ============================================================================
# STAGE 3: Document Findings (Step 3)
# ============================================================================

echo ""
echo "========== STAGE 3: Document Findings =========="
echo ""

source /tmp/check-naming-stats.txt 2>/dev/null || VIOLATION_COUNT=0

if [ $VIOLATION_COUNT -gt 0 ]; then
  echo "Summary of naming convention violations:"
  echo "  Total violations: $VIOLATION_COUNT"
  echo ""
  echo "See detailed report in /tmp/check-naming-output.txt"
  echo ""
  echo "⚠️ ACTION REQUIRED: Fix violations before committing"
else
  echo "✅ All naming conventions verified successfully"
fi

make checkpoint-update CMD=check-naming STEP=3

# ============================================================================
# Checkpoint Completion
# ============================================================================

echo ""
echo "========== CHECKPOINT COMPLETION =========="
echo ""

./scripts/complete-checkpoint.sh check-naming

# Cleanup temporary files
rm -f /tmp/check-naming-*.txt

echo ""
if [ $VIOLATION_COUNT -eq 0 ]; then
  echo "✅ Naming convention check workflow complete!"
else
  echo "⚠️ Naming convention check workflow complete - violations found"
  exit 1
fi
