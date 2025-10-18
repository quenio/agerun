#!/bin/bash
# Clean up checkpoint tracking files
# Usage: checkpoint-cleanup.sh <command_name>
# Example: checkpoint-cleanup.sh new-learnings

set -e

# Get command name
COMMAND_NAME=${1:-""}

if [ -z "$COMMAND_NAME" ]; then
    rm -f /tmp/*-progress.txt
    echo "✓ All tracking files removed"
else
    # Validate command name follows dash-based convention
    if [[ "$COMMAND_NAME" == *_* ]]; then
        echo "⚠️  WARNING: Command name contains underscores: '$COMMAND_NAME'"
        echo "    Based on temp file naming standardization, use dashes instead."
        echo "    Example: 'check-naming' not 'check_naming'"
        echo ""
    fi

    TRACKING_FILE="/tmp/${COMMAND_NAME}-progress.txt"
    if [ -f "$TRACKING_FILE" ]; then
        rm -f "$TRACKING_FILE"
        echo "✓ Cleaned up: $COMMAND_NAME"
    else
        echo "No tracking for: $COMMAND_NAME"
    fi
fi