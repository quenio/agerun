#!/bin/bash
# Check for existing new-learnings checkpoint and show status
# Usage: ./scripts/check-new-learnings-checkpoint.sh
#
# Purpose:
# - Detects if a new-learnings checkpoint is already in progress
# - Shows current status if checkpoint exists
# - Indicates if initialization is needed
#
# Returns:
# - Exit 0 if checkpoint exists (shows status)
# - Exit 1 if no checkpoint found (initialization needed)

set -e

if [ -f /tmp/new-learnings_progress.txt ]; then
    echo "✅ Existing checkpoint found:"
    echo ""
    ./scripts/status-checkpoint.sh new-learnings VERBOSE=--verbose
    exit 0
else
    echo "⚠️ No existing checkpoint found"
    echo ""
    echo "Initialize checkpoint with:"
    echo './scripts/init-checkpoint.sh new-learnings '"'"'Identify New Learnings" "Determine KB Article Strategy" "Knowledge Base Article Creation" "Validation Before Saving" "Update Knowledge Base Index" "Update Existing KB Articles (3-5 minimum)" "Review and Update Commands (3-4 minimum)" "Review Existing Guidelines" "Update Guidelines" "Validate No Broken Links" "Pre-Commit Integration Verification" "Automatic Commit and Push'"'"
    exit 1
fi
