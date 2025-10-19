#!/bin/bash
# Initialize checkpoint tracking if needed, or show status
# Usage: init-checkpoint.sh <command> <step1> <step2> ...
# Returns: 0 if initialized successfully, 1 if error
# Replaces 7-8 line if/then/else pattern across all commands

set -e
set -o pipefail

COMMAND="${1:-}"
shift

if [ -z "$COMMAND" ] || [ $# -eq 0 ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <command> <step1> <step2> ..."
  echo "Example: $0 check-docs 'Validate Docs' 'Preview Fixes' 'Apply Fixes' 'Verify Resolution' 'Commit and Push'"
  exit 1
fi

# Use command name as-is (with dashes, following script naming convention)
PROGRESS_FILE="/tmp/${COMMAND}-progress.txt"

# Check if already initialized
if [ -f "$PROGRESS_FILE" ]; then
  echo "Checkpoint tracking already initialized ($(wc -l < "$PROGRESS_FILE") steps)"
  bash scripts/checkpoint-status.sh "$COMMAND"
else
  echo "⚠️  Initializing checkpoint tracking..."
  bash scripts/checkpoint-init.sh "$COMMAND" "$@"
fi

exit 0
