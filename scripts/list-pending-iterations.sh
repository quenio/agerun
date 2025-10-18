#!/bin/bash
# List all PENDING REVIEW iterations from a TDD plan file
# Usage: ./scripts/list-pending-iterations.sh <plan-file>
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

echo "PENDING REVIEW Iterations in: $PLAN_FILE"
echo "=========================================="
echo ""

# Extract PENDING REVIEW iterations with line numbers
grep -n "PENDING REVIEW" "$PLAN_FILE" 2>/dev/null | while IFS=: read -r line_num line_content; do
    # Extract iteration number and description
    # Format: #### Iteration X.Y: Description - PENDING REVIEW
    iteration=$(echo "$line_content" | sed -E 's/^#+[:space:]*Iteration[:space:]+([0-9.]+):.*/\1/')
    description=$(echo "$line_content" | sed -E 's/^#+[:space:]*Iteration[:space:]+[0-9.]+:[:space:]*(.*)[:space:]-[:space:]*PENDING REVIEW/\1/')

    echo "[$line_num] Iteration $iteration: $description"
done

# Count total
TOTAL=$(grep -c "PENDING REVIEW" "$PLAN_FILE" 2>/dev/null) || TOTAL=0
echo ""
echo "Total PENDING REVIEW: $TOTAL iterations"

if [ "$TOTAL" -eq 0 ]; then
    echo ""
    echo "✅ No iterations pending review - plan may be fully reviewed!"
fi

exit 0
