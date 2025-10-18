#!/bin/bash
# Update checkpoint progress to a specific step
# Usage: update-checkpoint.sh <command> <step>
# Example: update-checkpoint.sh create-plan 3

set -e

COMMAND="${1:-}"
STEP="${2:-}"

if [ -z "$COMMAND" ] || [ -z "$STEP" ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <command> <step>"
  echo "Example: $0 create-plan 3"
  exit 1
fi

./scripts/checkpoint-update.sh "$COMMAND" "$STEP"
