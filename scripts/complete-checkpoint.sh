#!/bin/bash
# Complete checkpoint workflow: show status and cleanup
# Usage: complete-checkpoint.sh <command>
# Returns: 0 on success, 1 on error
# Replaces 4-5 line completion pattern (status + cleanup) across all commands

set -e
set -o pipefail

COMMAND="${1:-}"

if [ -z "$COMMAND" ]; then
  echo "❌ ERROR: Command name required"
  echo "Usage: $0 <command>"
  echo "Example: $0 compact-changes"
  exit 1
fi

echo ""
echo "========================================"
echo "   CHECKPOINT COMPLETION SUMMARY"
echo "========================================"
echo ""

# Show final status
make checkpoint-status CMD="$COMMAND"

echo ""
echo "Cleaning up checkpoint tracking..."

# Clean up tracking
make checkpoint-cleanup CMD="$COMMAND"

echo "✅ Checkpoint workflow complete"
exit 0
