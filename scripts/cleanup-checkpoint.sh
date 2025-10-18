#!/bin/bash
# Clean up checkpoint tracking for a command
# Usage: cleanup-checkpoint.sh <command>
# Example: cleanup-checkpoint.sh create-plan

set -e

COMMAND="${1:-}"

if [ -z "$COMMAND" ]; then
  echo "❌ ERROR: Missing required parameter"
  echo "Usage: $0 <command>"
  echo "Example: $0 create-plan"
  exit 1
fi

./scripts/checkpoint-cleanup.sh "$COMMAND"
