#!/bin/bash
# Check checkpoint progress status
# Usage: status-checkpoint.sh <command> [--verbose]
# Example: status-checkpoint.sh create-plan --verbose

set -e

COMMAND="${1:-}"
VERBOSE="${2:-}"

if [ -z "$COMMAND" ]; then
  echo "❌ ERROR: Missing required parameter"
  echo "Usage: $0 <command> [--verbose]"
  echo "Example: $0 create-plan --verbose"
  exit 1
fi

if [ -n "$VERBOSE" ]; then
  ./scripts/checkpoint-status.sh "$COMMAND" "$VERBOSE"
else
  ./scripts/checkpoint-status.sh "$COMMAND"
fi
