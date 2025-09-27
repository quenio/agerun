#!/bin/bash
# Enhanced checkpoint cleanup with audit trail archiving
# Usage: checkpoint_cleanup_enhanced.sh <command_name>
# Example: checkpoint_cleanup_enhanced.sh new-learnings

set -e

# Get command name
COMMAND_NAME=${1:-""}

if [ -z "$COMMAND_NAME" ]; then
    # Clean all tracking files and archive audit trails
    for file in /tmp/*_progress.txt; do
        if [ -f "$file" ]; then
            cmd=$(basename "$file" _progress.txt)
            archive_audit_trail "$cmd"
        fi
    done
    rm -f /tmp/*_progress.txt
    rm -f /tmp/*_audit.txt
    echo "✓ All tracking files removed"
else
    TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"
    AUDIT_FILE="/tmp/${COMMAND_NAME}_audit.txt"
    ARCHIVE_DIR="logs/checkpoint_audit"

    if [ -f "$TRACKING_FILE" ]; then
        # Archive audit trail if it exists
        if [ -f "$AUDIT_FILE" ]; then
            mkdir -p "$ARCHIVE_DIR"
            TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
            ARCHIVE_FILE="$ARCHIVE_DIR/${COMMAND_NAME}_${TIMESTAMP}.audit"

            {
                echo "=== CHECKPOINT AUDIT TRAIL ==="
                echo "Command: $COMMAND_NAME"
                echo "Completed: $(date)"
                echo "================================"
                echo ""
                cat "$AUDIT_FILE"
            } > "$ARCHIVE_FILE"

            rm -f "$AUDIT_FILE"
            echo "✓ Audit trail archived: $ARCHIVE_FILE"
        fi

        rm -f "$TRACKING_FILE"
        echo "✓ Cleaned up: $COMMAND_NAME"
    else
        echo "No tracking for: $COMMAND_NAME"
    fi
fi