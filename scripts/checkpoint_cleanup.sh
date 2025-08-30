#!/bin/bash
# Clean up checkpoint tracking files
# Usage: checkpoint_cleanup.sh <command_name>
# Example: checkpoint_cleanup.sh new-learnings

set -e

# Get command name
COMMAND_NAME=${1:-""}

if [ -z "$COMMAND_NAME" ]; then
    echo "Cleaning up all checkpoint tracking files..."
    rm -f /tmp/*_progress.txt
    echo "✓ All tracking files removed"
else
    TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"
    if [ -f "$TRACKING_FILE" ]; then
        rm -f "$TRACKING_FILE"
        echo "✓ Tracking file removed: $TRACKING_FILE"
    else
        echo "No tracking file found for: $COMMAND_NAME"
    fi
fi