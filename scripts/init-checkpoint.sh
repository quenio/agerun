#!/bin/bash
# Initialize checkpoint tracking if needed, or show status
# Usage: init-checkpoint.sh <command> <steps-string>
# Returns: 0 if initialized successfully, 1 if error
# Replaces 7-8 line if/then/else pattern across all commands

set -e
set -o pipefail

COMMAND="${1:-}"
STEPS="${2:-}"

if [ -z "$COMMAND" ] || [ -z "$STEPS" ]; then
  echo "❌ ERROR: Missing required parameters"
  echo "Usage: $0 <command> <steps-string>"
  echo "Example: $0 compact-changes '\"Measure\" \"Analyze\" \"Compact\" \"Verify\"'"
  exit 1
fi

# Use command name as-is (with dashes, following script naming convention)
PROGRESS_FILE="/tmp/${COMMAND}-progress.txt"

# Check if already initialized
if [ -f "$PROGRESS_FILE" ]; then
  echo "Checkpoint tracking already initialized ($(wc -l < "$PROGRESS_FILE") steps)"
  make checkpoint-status CMD="$COMMAND"
else
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD="$COMMAND" STEPS="$STEPS"
fi

exit 0
