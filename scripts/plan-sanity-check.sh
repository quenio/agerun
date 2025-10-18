#!/bin/bash
# Quick sanity checks for TDD plan structure
# Usage: ./scripts/plan-sanity-check.sh <plan-file>
#
# This script performs quick pattern-based checks to verify plan structure.
# For comprehensive validation, use ./scripts/validate-tdd-plan.sh
set -o pipefail

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <plan-file>"
    echo ""
    echo "Performs quick sanity checks on TDD plan structure:"
    echo "  - Assertion count per iteration"
    echo "  - FAILS comment presence"
    echo "  - Temporary corruption documentation (Lesson 7)"
    echo "  - Temporary cleanup format"
    echo "  - Real AgeRun type usage"
    echo "  - BDD structure presence"
    echo ""
    echo "For comprehensive validation, use: ./scripts/validate-tdd-plan.sh"
    exit 1
fi

PLAN_FILE=$1

if [ ! -f "$PLAN_FILE" ]; then
    echo "Error: Plan file not found: $PLAN_FILE"
    exit 1
fi

echo "Plan Sanity Check: $PLAN_FILE"
echo "========================================"
echo ""

# 1. Check for FAILS comments (should have one per RED phase)
echo "1. Checking for FAILS comments..."
FAILS_COUNT=$(grep -c "// ← FAILS" "$PLAN_FILE" 2>/dev/null) || FAILS_COUNT=0
echo "   Found: $FAILS_COUNT FAILS markers"
if [ "$FAILS_COUNT" -eq 0 ]; then
    echo "   ⚠️  WARNING: No FAILS comments found"
else
    echo "   ✅ FAILS comments present"
fi
echo ""

# 2. Check for temporary corruption/failure documentation (Lesson 7 CRITICAL)
echo "2. Checking for temporary corruption documentation (Lesson 7)..."
CORRUPTION_COUNT=$(grep -E -c "Temporary|corrupt|break|Expected RED.*FAIL" "$PLAN_FILE" 2>/dev/null) || CORRUPTION_COUNT=0
echo "   Found: $CORRUPTION_COUNT temporary corruption/failure references"
if [ "$CORRUPTION_COUNT" -eq 0 ]; then
    echo "   ❌ CRITICAL: No temporary corruption documentation found"
    echo "      RED phases MUST document how assertions will fail"
else
    echo "   ✅ Temporary corruption documentation present"
fi
echo ""

# 3. Check for temporary cleanup format
echo "3. Checking for temporary cleanup comments..."
TEMP_CLEANUP_COUNT=$(grep -c "temporary:" "$PLAN_FILE" 2>/dev/null) || TEMP_CLEANUP_COUNT=0
echo "   Found: $TEMP_CLEANUP_COUNT temporary cleanup markers"
if [ "$TEMP_CLEANUP_COUNT" -gt 0 ]; then
    echo "   ✅ Temporary cleanup present"
    # Show examples
    echo "   Examples:"
    grep "temporary:" "$PLAN_FILE" 2>/dev/null | head -2 | sed 's/^/      /'
else
    echo "   ℹ️  No temporary cleanup (may not be needed if no .1 iterations)"
fi
echo ""

# 4. Check for real AgeRun types
echo "4. Checking for real AgeRun types (ar_*_t)..."
TYPE_COUNT=$(grep -E -c "ar_[a-z_]+_t" "$PLAN_FILE" 2>/dev/null) || TYPE_COUNT=0
echo "   Found: $TYPE_COUNT AgeRun type references"
if [ "$TYPE_COUNT" -eq 0 ]; then
    echo "   ⚠️  WARNING: No AgeRun types found (placeholder types?)"
else
    echo "   ✅ Real AgeRun types present"
    # Show unique types
    echo "   Types found:"
    grep -E -o "ar_[a-z_]+_t" "$PLAN_FILE" 2>/dev/null | sort -u | head -5 | sed 's/^/      /'
fi
echo ""

# 5. Check for BDD structure
echo "5. Checking for BDD structure comments..."
GIVEN_COUNT=$(grep -c "// Given" "$PLAN_FILE" 2>/dev/null) || GIVEN_COUNT=0
WHEN_COUNT=$(grep -c "// When" "$PLAN_FILE" 2>/dev/null) || WHEN_COUNT=0
THEN_COUNT=$(grep -c "// Then" "$PLAN_FILE" 2>/dev/null) || THEN_COUNT=0
CLEANUP_COUNT=$(grep -c "// Cleanup" "$PLAN_FILE" 2>/dev/null) || CLEANUP_COUNT=0

echo "   Given:   $GIVEN_COUNT"
echo "   When:    $WHEN_COUNT"
echo "   Then:    $THEN_COUNT"
echo "   Cleanup: $CLEANUP_COUNT"

if [ "$GIVEN_COUNT" -eq 0 ] || [ "$WHEN_COUNT" -eq 0 ] || [ "$THEN_COUNT" -eq 0 ] || [ "$CLEANUP_COUNT" -eq 0 ]; then
    echo "   ⚠️  WARNING: Incomplete BDD structure"
else
    echo "   ✅ BDD structure present"
fi
echo ""

# 6. Check for status markers
echo "6. Checking for iteration status markers..."
PENDING_COUNT=$(grep -c "PENDING REVIEW" "$PLAN_FILE" 2>/dev/null) || PENDING_COUNT=0
REVIEWED_COUNT=$(grep -c "- REVIEWED" "$PLAN_FILE" 2>/dev/null) || REVIEWED_COUNT=0
REVISED_COUNT=$(grep -c "- REVISED" "$PLAN_FILE" 2>/dev/null) || REVISED_COUNT=0

echo "   PENDING REVIEW: $PENDING_COUNT"
echo "   REVIEWED:       $REVIEWED_COUNT"
echo "   REVISED:        $REVISED_COUNT"

if [ "$PENDING_COUNT" -eq 0 ] && [ "$REVIEWED_COUNT" -eq 0 ] && [ "$REVISED_COUNT" -eq 0 ]; then
    echo "   ⚠️  WARNING: No status markers found"
else
    echo "   ✅ Status markers present"
fi
echo ""

# Summary
echo "========================================"
echo "Summary:"
echo ""

WARNINGS=0
CRITICAL=0

if [ "$FAILS_COUNT" -eq 0 ]; then
    WARNINGS=$((WARNINGS + 1))
fi

if [ "$CORRUPTION_COUNT" -eq 0 ]; then
    CRITICAL=$((CRITICAL + 1))
fi

if [ "$TYPE_COUNT" -eq 0 ]; then
    WARNINGS=$((WARNINGS + 1))
fi

if [ "$CRITICAL" -gt 0 ]; then
    echo "❌ CRITICAL ISSUES: $CRITICAL"
    echo "   Plan has critical methodology violations"
    exit 1
fi

if [ "$WARNINGS" -gt 0 ]; then
    echo "⚠️  WARNINGS: $WARNINGS"
    echo "   Plan has potential issues - review recommended"
    exit 0
fi

echo "✅ Sanity checks PASSED"
echo ""
echo "Next step: Run comprehensive validation:"
echo "  ./scripts/validate-tdd-plan.sh $PLAN_FILE"

exit 0
