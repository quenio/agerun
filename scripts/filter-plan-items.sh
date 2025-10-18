#!/bin/bash
# Filter and extract plan iterations by status
# Usage: filter-plan-items.sh <plan-file> <status> [format]
# Returns: 0 if items found, 1 if none found
# Output: List of matching items in specified format

set -e
set -o pipefail

PLAN_FILE="${1:-}"
STATUS="${2:-}"
FORMAT="${3:-count}"  # count, list, table, or markdown

if [ -z "$PLAN_FILE" ] || [ -z "$STATUS" ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <plan-file> <status> [format]"
  echo ""
  echo "Parameters:"
  echo "  <plan-file>  - Path to TDD plan markdown file"
  echo "  <status>     - Status to filter for (e.g., 'REVIEWED', 'PENDING REVIEW', 'IMPLEMENTED')"
  echo "  [format]     - Output format: count, list, table, markdown (default: count)"
  echo ""
  echo "Example:"
  echo "  $0 plans/myplan.md 'REVIEWED' list"
  echo "  $0 plans/myplan.md 'PENDING REVIEW' count"
  exit 1
fi

# Validate file exists
if [ ! -f "$PLAN_FILE" ]; then
  echo "❌ ERROR: Plan file not found: $PLAN_FILE"
  exit 1
fi

# Count matching items
ITEM_COUNT=$(grep -c "#### Iteration.*- $STATUS" "$PLAN_FILE" || echo "0")

if [ "$ITEM_COUNT" -eq 0 ]; then
  echo "0"
  exit 1
fi

# Output based on format
case "$FORMAT" in
  count)
    echo "$ITEM_COUNT"
    ;;

  list)
    echo "Items with status: $STATUS"
    echo "=========================="
    { grep "#### Iteration.*- $STATUS" "$PLAN_FILE" || true; } | sed "s/#### Iteration \(.*\) - $STATUS/\1/" | nl
    ;;

  table)
    echo "Iteration | Status"
    echo "----------|-------"
    { grep "#### Iteration.*- $STATUS" "$PLAN_FILE" || true; } | sed "s/#### Iteration \(.*\) - $STATUS/\1 | $STATUS/"
    ;;

  markdown)
    echo ""
    echo "## Items with status: $STATUS"
    echo ""
    grep "#### Iteration.*- $STATUS" "$PLAN_FILE" || true
    echo ""
    ;;

  *)
    echo "❌ ERROR: Unknown format: $FORMAT"
    echo "Valid formats: count, list, table, markdown"
    exit 1
    ;;
esac

echo ""
echo "Total: $ITEM_COUNT"
exit 0
