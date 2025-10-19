#!/bin/bash
# Merge permissions from local settings into main settings
# Usage: ./scripts/merge-permissions.sh
#
# Purpose:
# - Performs actual merge of permissions from local to main settings
# - Executes python merge_settings.py script
# - Captures and reports merge statistics
#
# Returns:
# - Exit 0 if merge successful
# - Exit 1 if merge failed
# - Updates /tmp/merge-settings-stats.txt with TOTAL_PERMS

source /tmp/merge-settings-stats.txt

echo "Merging permissions..."

if [ "$LOCAL_EXISTS" = "YES" ]; then
    echo "Merging $LOCAL_PERMS permissions from local file"

    # Perform actual merge using Python script
    if python3 ./scripts/merge_settings.py; then
        MERGED_PERMS=$(grep '"Bash(' ./.claude/settings.json 2>/dev/null | wc -l || echo "0")
        echo "TOTAL_PERMS=$MERGED_PERMS" >> /tmp/merge-settings-stats.txt
        echo "✅ Permissions merged"
        exit 0
    else
        echo "❌ Merge failed!"
        exit 1
    fi
fi

exit 0
