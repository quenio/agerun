#!/bin/bash
# List all iterations with their current status from a TDD plan file
# Usage: ./scripts/list-iteration-status.sh <plan-file> [status-filter]
#
# Arguments:
#   plan-file: Path to the plan file
#   status-filter: (Optional) Filter by status: PENDING, REVIEWED, REVISED, IMPLEMENTED, COMMITTED
set -o pipefail

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <plan-file> [status-filter]"
    echo ""
    echo "Status filters: PENDING, REVIEWED, REVISED, IMPLEMENTED, COMMITTED"
    exit 1
fi

PLAN_FILE=$1
STATUS_FILTER=${2:-""}

if [ ! -f "$PLAN_FILE" ]; then
    echo "Error: Plan file not found: $PLAN_FILE"
    exit 1
fi

if [ -n "$STATUS_FILTER" ]; then
    echo "Iterations with status: $STATUS_FILTER"
else
    echo "All Iterations in: $PLAN_FILE"
fi
echo "=========================================="
echo ""

# Extract all iteration lines with status markers
# Format: ### Iteration X.Y: Description - STATUS
grep -E "^###+ Iteration" "$PLAN_FILE" 2>/dev/null | while IFS= read -r line; do
    # Extract components
    iteration=$(echo "$line" | sed -E 's/^#+[[:space:]]*Iteration[[:space:]]+([0-9.]+):.*/\1/')

    # Extract description (everything between ": " and last " - ")
    description=$(echo "$line" | sed -E 's/^#+[[:space:]]*Iteration[[:space:]]+[0-9.]+:[[:space:]]*(.*)/\1/')
    description=$(echo "$description" | sed -E 's/[[:space:]]+-[[:space:]]*(PENDING REVIEW|REVIEWED|REVISED|IMPLEMENTED|✅ COMMITTED).*//')

    # Extract status (after last " - ")
    status=$(echo "$line" | grep -oE "(PENDING REVIEW|REVIEWED|REVISED|IMPLEMENTED|✅ COMMITTED)" | tail -1)

    # Apply filter if specified
    if [ -n "$STATUS_FILTER" ]; then
        if ! echo "$status" | grep -qi "$STATUS_FILTER"; then
            continue
        fi
    fi

    # Format status with emoji
    case "$status" in
        "PENDINGREVIEW")
            status_display="⏳ PENDING REVIEW"
            ;;
        "REVIEWED")
            status_display="✅ REVIEWED"
            ;;
        "REVISED")
            status_display="🔄 REVISED"
            ;;
        "IMPLEMENTED")
            status_display="🔨 IMPLEMENTED"
            ;;
        "✅COMMITTED")
            status_display="✅ COMMITTED"
            ;;
        *)
            status_display="$status"
            ;;
    esac

    printf "%-8s %-50s %s\n" "$iteration" "$description" "$status_display"
done

exit 0
