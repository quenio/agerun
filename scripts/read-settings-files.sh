#!/bin/bash
# Read both main and local settings files and collect statistics
# Usage: ./scripts/read-settings-files.sh
#
# Purpose:
# - Reads ./.claude/settings.json and ./.claude/settings.local.json
# - Counts permissions in each file
# - Updates statistics for merge workflow
#
# Returns:
# - Exit 0 if successful
# - Outputs updated stats to /tmp/merge-settings-stats.txt

source /tmp/merge-settings-stats.txt

echo "Reading settings files..."

if [ "$LOCAL_EXISTS" = "YES" ]; then
    # Check main settings file
    if [ ! -f ./.claude/settings.json ]; then
        echo "⚠️ Main settings file missing - will create"
    fi

    # Count existing permissions in main
    MAIN_PERMS=$(grep '"Bash(' ./.claude/settings.json 2>/dev/null | wc -l || echo "0")
    echo "   Main file permissions: $MAIN_PERMS"
    ./scripts/update-merge-stats.sh MAIN_PERMS "$MAIN_PERMS"

    echo "✅ Settings files read"
fi

exit 0
