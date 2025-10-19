#!/bin/bash
# Quick checkpoint progress check without full initialization
# Usage: checkpoint-status-quick.sh <command> [verbose]
# Returns: 0 if initialized, 1 if not, 2 if error
# Output: Progress summary, step count, percentage complete

set -e
set -o pipefail

COMMAND="${1:-}"
VERBOSE="${2:-}"

if [ -z "$COMMAND" ]; then
  echo "❌ ERROR: Command name required"
  echo "Usage: $0 <command> [verbose]"
  echo "Example: $0 compact-changes"
  exit 1
fi

# Use command name directly in progress file format
PROGRESS_FILE="/tmp/${COMMAND}-progress.txt"

if [ ! -f "$PROGRESS_FILE" ]; then
  echo "⚠️ Checkpoint not initialized for '$COMMAND'"
  exit 1
fi

# Show status
if [ "$VERBOSE" = "verbose" ] || [ "$VERBOSE" = "--verbose" ]; then
  ./scripts/checkpoint-status.sh "$COMMAND" --verbose
else
  ./scripts/checkpoint-status.sh "$COMMAND"
fi

exit 0
