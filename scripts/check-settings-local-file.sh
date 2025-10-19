#!/bin/bash
# Check if local settings file exists and collect statistics
# Usage: ./scripts/check-settings-local-file.sh
#
# Purpose:
# - Detects if ./.claude/settings.local.json exists
# - Counts permissions in local file if it exists
# - Outputs statistics for use by merge-settings workflow
#
# Returns:
# - Exit 0 if check completed
# - Outputs: LOCAL_EXISTS and LOCAL_PERMS to /tmp/merge-settings-stats.txt

echo "Checking for local settings file..."

if [ -f ./.claude/settings.local.json ]; then
    echo "✅ Local settings file found"
    LOCAL_EXISTS="YES"

    # Count permissions in local file
    LOCAL_PERMS=$(grep '"Bash(' ./.claude/settings.local.json | wc -l || echo "0")
    echo "   Local permissions to merge: $LOCAL_PERMS"
else
    echo "ℹ️ No local settings file - nothing to merge"
    LOCAL_EXISTS="NO"
    LOCAL_PERMS=0
fi

echo "LOCAL_EXISTS=$LOCAL_EXISTS" > /tmp/merge-settings-stats.txt
echo "LOCAL_PERMS=$LOCAL_PERMS" >> /tmp/merge-settings-stats.txt

exit 0
