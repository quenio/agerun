#!/bin/bash
# Verify checkpoint tracking is initialized before proceeding
# Usage: checkpoint-require.sh <command>
# Returns: 0 if checkpoint exists, 1 if missing (exits with error message)

set -e
set -o pipefail

COMMAND="${1:-}"

if [ -z "$COMMAND" ]; then
  echo "❌ ERROR: Command name required"
  echo "Usage: $0 <command>"
  echo "Example: $0 compact-changes"
  exit 1
fi

# Use command name as-is (with dashes, following script naming convention)
PROGRESS_FILE="/tmp/${COMMAND}-progress.txt"

if [ ! -f "$PROGRESS_FILE" ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized for '$COMMAND'"
  echo ""
  echo "Initialize with:"
  echo "  ./scripts/checkpoint-init.sh $COMMAND '{STEPS}'"
  echo ""
  echo "Or manually with:"
  echo "  make checkpoint-init CMD=$COMMAND STEPS='{STEPS}'"
  exit 1
fi

echo "✅ Checkpoint tracking verified"
exit 0
