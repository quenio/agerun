#!/bin/bash
# Validate TDD plan structure after Step 6 (Plan Iterations)
# Usage: validate_plan_structure.sh <plan-file> <expected-iteration-count>
set -o pipefail

set -e

PLAN_FILE="$1"
EXPECTED_COUNT="$2"

if [ -z "$PLAN_FILE" ] || [ -z "$EXPECTED_COUNT" ]; then
    echo "Usage: validate_plan_structure.sh <plan-file> <expected-iteration-count>"
    exit 1
fi

if [ ! -f "$PLAN_FILE" ]; then
    echo "❌ ERROR: Plan file not found: $PLAN_FILE"
    exit 1
fi

echo "🔍 Validating plan structure: $PLAN_FILE"
echo "Expected iterations: $EXPECTED_COUNT"
echo ""

# Count iterations with PENDING REVIEW markers (only iteration headings, not status lines)
ACTUAL_COUNT=$(grep "^#### Iteration.*- PENDING REVIEW$" "$PLAN_FILE" | wc -l | tr -d ' ')

echo "Found iterations: $ACTUAL_COUNT"

if [ "$ACTUAL_COUNT" -ne "$EXPECTED_COUNT" ]; then
    echo "❌ FAIL: Expected $EXPECTED_COUNT iterations, found $ACTUAL_COUNT"
    exit 1
fi

echo "✅ Iteration count matches"

# Validate each iteration has required sections
ERRORS=0

echo ""
echo "Checking iteration structure..."

# Extract iteration numbers (only from iteration headings)
ITERATIONS=$(grep "^#### Iteration.*- PENDING REVIEW$" "$PLAN_FILE" | sed 's/^#### Iteration \([0-9.]*\):.*/\1/')

for iter in $ITERATIONS; do
    echo -n "  Iteration $iter: "

    # Check for required sections (using grep -A to look ahead)
    if ! grep -q "#### Iteration $iter:.*PENDING REVIEW" "$PLAN_FILE"; then
        echo "❌ Missing iteration header"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    if ! { grep -A 50 "#### Iteration $iter:" "$PLAN_FILE" || true; } | grep -q "^\*\*Objective\*\*:"; then
        echo "❌ Missing Objective"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    if ! { grep -A 100 "#### Iteration $iter:" "$PLAN_FILE" || true; } | grep -q "^\*\*RED Phase:\*\*"; then
        echo "❌ Missing RED Phase"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    if ! { grep -A 150 "#### Iteration $iter:" "$PLAN_FILE" || true; } | grep -q "^\*\*GREEN Phase:\*\*"; then
        echo "❌ Missing GREEN Phase"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    if ! { grep -A 200 "#### Iteration $iter:" "$PLAN_FILE" || true; } | grep -q "^\*\*Verification:\*\*"; then
        echo "❌ Missing Verification"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    # Check for dual-goal pattern in RED phase
    if ! { grep -A 100 "#### Iteration $iter:" "$PLAN_FILE" || true; } | grep -q "GOAL 1:"; then
        echo "❌ Missing GOAL 1 in RED phase"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    if ! { grep -A 100 "#### Iteration $iter:" "$PLAN_FILE" || true; } | grep -q "GOAL 2:"; then
        echo "❌ Missing GOAL 2 in RED phase"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    echo "✅"
done

echo ""

if [ $ERRORS -gt 0 ]; then
    echo "❌ VALIDATION FAILED: $ERRORS structural errors found"
    exit 1
fi

echo "✅ All iterations have required structure"
echo ""
echo "✅ PLAN STRUCTURE VALIDATION PASSED"
exit 0
