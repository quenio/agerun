#!/bin/bash
# Count iterations by status in a TDD plan file
# Usage: ./scripts/count-plan-iterations.sh <plan-file>
set -o pipefail

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <plan-file>"
    exit 1
fi

PLAN_FILE=$1

if [ ! -f "$PLAN_FILE" ]; then
    echo "Error: Plan file not found: $PLAN_FILE"
    exit 1
fi

echo "Plan Iteration Summary: $PLAN_FILE"
echo "======================================"

# Count different statuses
PENDING=$(grep -c "PENDING REVIEW" "$PLAN_FILE" 2>/dev/null) || PENDING=0
REVIEWED=$(grep -c "- REVIEWED" "$PLAN_FILE" 2>/dev/null) || REVIEWED=0
REVISED=$(grep -c "- REVISED" "$PLAN_FILE" 2>/dev/null) || REVISED=0
IMPLEMENTED=$(grep -c "- IMPLEMENTED" "$PLAN_FILE" 2>/dev/null) || IMPLEMENTED=0
COMMITTED=$(grep -c "✅ COMMITTED" "$PLAN_FILE" 2>/dev/null) || COMMITTED=0

# Total iterations (any status marker after ###)
TOTAL=$(grep -E "^###+ Iteration.*-" "$PLAN_FILE" 2>/dev/null | wc -l | tr -d ' ')

echo "Total Iterations: $TOTAL"
echo ""
echo "Status Breakdown:"
echo "  PENDING REVIEW: $PENDING"
echo "  REVIEWED:       $REVIEWED"
echo "  REVISED:        $REVISED"
echo "  IMPLEMENTED:    $IMPLEMENTED"
echo "  ✅ COMMITTED:   $COMMITTED"
echo ""

# Calculate percentages if total > 0
if [ "$TOTAL" -gt 0 ]; then
    COMPLETE_PCT=$((COMMITTED * 100 / TOTAL))
    READY_PCT=$(( (REVIEWED + REVISED) * 100 / TOTAL))
    PENDING_PCT=$((PENDING * 100 / TOTAL))

    echo "Progress:"
    echo "  Completed:     $COMMITTED/$TOTAL ($COMPLETE_PCT%)"
    echo "  Ready to exec: $((REVIEWED + REVISED))/$TOTAL ($READY_PCT%)"
    echo "  Needs review:  $PENDING/$TOTAL ($PENDING_PCT%)"
fi

exit 0
