#!/bin/bash
# Extract plan iterations by status from a plan file
# Usage: extract-plan-iterations.sh <plan-file> [status]
# Status options: PENDING, IMPLEMENTED, REVIEWED, REVISED
# Returns: List of iteration numbers matching the status

set -e
set -o pipefail

PLAN_FILE="$1"
STATUS="${2:-PENDING}"

if [ -z "$PLAN_FILE" ] || [ ! -f "$PLAN_FILE" ]; then
  echo "❌ ERROR: Plan file not found: $PLAN_FILE"
  exit 1
fi

# Extract iterations with matching status
# Format: #### Iteration X.Y: ... - STATUS
case "$STATUS" in
  PENDING)
    grep "^#### Iteration [0-9]\+\.[0-9]\+:" "$PLAN_FILE" | grep "- PENDING" | sed 's/.*Iteration \([0-9.]*\).*/\1/'
    ;;
  IMPLEMENTED)
    grep "^#### Iteration [0-9]\+\.[0-9]\+:" "$PLAN_FILE" | grep "- IMPLEMENTED" | sed 's/.*Iteration \([0-9.]*\).*/\1/'
    ;;
  REVIEWED)
    grep "^#### Iteration [0-9]\+\.[0-9]\+:" "$PLAN_FILE" | grep "- REVIEWED" | sed 's/.*Iteration \([0-9.]*\).*/\1/'
    ;;
  REVISED)
    grep "^#### Iteration [0-9]\+\.[0-9]\+:" "$PLAN_FILE" | grep "- REVISED" | sed 's/.*Iteration \([0-9.]*\).*/\1/'
    ;;
  *)
    echo "❌ ERROR: Unknown status: $STATUS"
    echo "Valid options: PENDING, IMPLEMENTED, REVIEWED, REVISED"
    exit 1
    ;;
esac
