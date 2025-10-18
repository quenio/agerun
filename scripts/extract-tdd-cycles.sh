#!/bin/bash
# Extract and list TDD cycle iterations from plan
# Usage: extract-tdd-cycles.sh <plan-file> [status-filter]
# Returns: 0 if cycles found, 1 if none found
# Output: Structured list of TDD cycles ready for execution

set -e
set -o pipefail

PLAN_FILE="${1:-}"
STATUS_FILTER="${2:-REVIEWED|REVISED|IMPLEMENTED}"

if [ -z "$PLAN_FILE" ]; then
  echo "❌ ERROR: Plan file required"
  echo "Usage: $0 <plan-file> [status-filter]"
  echo ""
  echo "Parameters:"
  echo "  <plan-file>       - Path to TDD plan markdown file"
  echo "  [status-filter]   - Grep pattern for statuses (default: REVIEWED|REVISED|IMPLEMENTED)"
  echo ""
  echo "Example:"
  echo "  $0 plans/myplan.md 'REVIEWED'"
  echo "  $0 plans/myplan.md 'REVIEWED|REVISED'"
  exit 1
fi

# Validate file
if [ ! -f "$PLAN_FILE" ]; then
  echo "❌ ERROR: Plan file not found: $PLAN_FILE"
  exit 1
fi

# Extract TDD cycles
echo ""
echo "TDD Cycles Ready for Execution"
echo "=============================="
echo ""

CYCLE_COUNT=0
while IFS= read -r iteration_line; do
  # Extract iteration info: #### Iteration X.Y: description - STATUS
  if [[ "$iteration_line" =~ ^####\ Iteration\ ([0-9]+\.[0-9]+):\ ([^-]+)\ -\ (.*)$ ]]; then
    CYCLE_NUM="${BASH_REMATCH[1]}"
    DESCRIPTION="${BASH_REMATCH[2]}"
    STATUS="${BASH_REMATCH[3]}"

    CYCLE_COUNT=$((CYCLE_COUNT + 1))

    echo "$CYCLE_COUNT. Iteration $CYCLE_NUM"
    echo "   Description: ${DESCRIPTION% }"
    echo "   Status: $STATUS"
    echo ""
  fi
done < <(grep -E "#### Iteration.*- ($STATUS_FILTER)" "$PLAN_FILE")

if [ "$CYCLE_COUNT" -eq 0 ]; then
  echo "No TDD cycles found matching status filter: $STATUS_FILTER"
  echo ""
  exit 1
fi

echo "=============================="
echo "Total TDD cycles: $CYCLE_COUNT"
echo ""
echo "Ready for TDD execution:"
echo "  - RED phase: Write failing test"
echo "  - GREEN phase: Implement minimal code to pass"
echo "  - REFACTOR phase: Improve code quality"
echo ""

exit 0
