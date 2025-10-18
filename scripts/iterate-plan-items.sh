#!/bin/bash
# Orchestrate iteration over plan items with checkpoint tracking
# Usage: iterate-plan-items.sh <plan-file> <status-filter> <item-processor-script> [checkpoint-cmd]
# Returns: 0 if all items processed, 1 if error or processing stopped
#
# This helper handles the nested iteration loop pattern used in execute-plan and review-plan,
# allowing processing of multiple plan items with per-item checkpoint tracking.

set -e
set -o pipefail

PLAN_FILE="${1:-}"
STATUS_FILTER="${2:-}"
PROCESSOR_SCRIPT="${3:-}"
CHECKPOINT_CMD="${4:-}"

if [ -z "$PLAN_FILE" ] || [ -z "$STATUS_FILTER" ] || [ -z "$PROCESSOR_SCRIPT" ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <plan-file> <status-filter> <processor-script> [checkpoint-cmd]"
  echo ""
  echo "Parameters:"
  echo "  <plan-file>          - Path to TDD plan markdown file"
  echo "  <status-filter>      - Iteration status to process (e.g., 'REVIEWED', 'PENDING REVIEW')"
  echo "  <processor-script>   - Script to execute for each item"
  echo "  [checkpoint-cmd]     - Optional checkpoint command name (e.g., 'execute-plan')"
  echo ""
  echo "Example:"
  echo "  $0 plans/myplan.md 'REVIEWED' ./process-iteration.sh execute-plan"
  exit 1
fi

# Validate files exist
if [ ! -f "$PLAN_FILE" ]; then
  echo "❌ ERROR: Plan file not found: $PLAN_FILE"
  exit 1
fi

if [ ! -f "$PROCESSOR_SCRIPT" ]; then
  echo "❌ ERROR: Processor script not found: $PROCESSOR_SCRIPT"
  exit 1
fi

# Extract iteration count (count of "#### Iteration X.Y:" lines matching status)
TOTAL_ITEMS=$(grep -c "#### Iteration.*- $STATUS_FILTER" "$PLAN_FILE" || echo "0")

if [ "$TOTAL_ITEMS" -eq 0 ]; then
  echo "⚠️ No items found with status: $STATUS_FILTER"
  exit 0
fi

echo ""
echo "Processing $TOTAL_ITEMS items with status: $STATUS_FILTER"
echo ""

# Initialize iteration counter
CURRENT=0
PROCESSED=0
FAILED=0

# Extract and process each matching iteration
while IFS= read -r line; do
  # Look for iteration headers
  if [[ "$line" =~ ^####\ Iteration\ [0-9]+\.[0-9]+:\ (.*)\ -\ $STATUS_FILTER ]]; then
    CURRENT=$((CURRENT + 1))
    ITERATION_DESC="${BASH_REMATCH[1]}"

    echo "[$CURRENT/$TOTAL_ITEMS] Processing: $ITERATION_DESC"

    # Call processor script with iteration information
    if "$PROCESSOR_SCRIPT" "$PLAN_FILE" "$ITERATION_DESC" "$CURRENT" "$TOTAL_ITEMS"; then
      PROCESSED=$((PROCESSED + 1))

      # Update checkpoint if provided
      if [ -n "$CHECKPOINT_CMD" ]; then
        make checkpoint-update CMD="$CHECKPOINT_CMD" STEP="$CURRENT" 2>/dev/null || true
      fi

      echo "  ✓ Completed"
    else
      FAILED=$((FAILED + 1))
      echo "  ✗ Failed (exit code: $?)"
    fi

    echo ""
  fi
done < "$PLAN_FILE"

# Summary
echo "Processing Summary:"
echo "  Total items: $TOTAL_ITEMS"
echo "  Processed: $PROCESSED"
echo "  Failed: $FAILED"

if [ "$FAILED" -gt 0 ]; then
  echo ""
  echo "❌ $FAILED item(s) failed processing"
  exit 1
fi

if [ "$PROCESSED" -eq "$TOTAL_ITEMS" ]; then
  echo "✅ All items processed successfully"
  exit 0
else
  echo "⚠️ Only $PROCESSED/$TOTAL_ITEMS items processed"
  exit 0
fi
