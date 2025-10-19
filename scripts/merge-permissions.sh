#!/bin/bash
# Merge permissions from local settings into main settings
# Usage: ./scripts/merge-permissions.sh
#
# Purpose:
# - Performs actual merge of permissions from local to main settings
# - Executes python merge_settings.py script
# - Captures and reports merge statistics
# - Validates that merge actually occurred and modified settings.json
#
# Returns:
# - Exit 0 if merge successful or no local file to merge
# - Exit 1 if merge failed or validation failed
# - Updates /tmp/merge-settings-stats.txt with TOTAL_PERMS

source /tmp/merge-settings-stats.txt

echo "Merging permissions..."

if [ "$LOCAL_EXISTS" = "YES" ]; then
    echo "Merging $LOCAL_PERMS permissions from local file"

    # Capture current settings hash before merge
    BEFORE_HASH=$(md5 -q ./.claude/settings.json 2>/dev/null || echo "0")

    # Perform actual merge using Python script
    if ! python3 ./scripts/merge_settings.py; then
        echo "❌ Merge failed!"
        exit 1
    fi

    # Verify that settings.json was actually modified
    AFTER_HASH=$(md5 -q ./.claude/settings.json 2>/dev/null || echo "0")

    if [ "$BEFORE_HASH" = "$AFTER_HASH" ]; then
        echo "⚠️ Warning: Settings file was not modified by merge"
        echo "This may indicate the merge produced no changes (all permissions already existed)"
    fi

    # Count merged permissions
    MERGED_PERMS=$(grep '"Bash(' ./.claude/settings.json 2>/dev/null | wc -l || echo "0")
    ./scripts/update-merge-stats.sh TOTAL_PERMS "$MERGED_PERMS"

    # Show stats
    echo "✅ Permissions merged"
    echo ""
    echo "Merge Statistics:"
    echo "  Main permissions:   $MAIN_PERMS"
    echo "  Local permissions:  $LOCAL_PERMS"
    echo "  Merged permissions: $MERGED_PERMS"
    echo "  New permissions:    $((MERGED_PERMS - MAIN_PERMS))"
    echo ""

    exit 0
else
    echo "ℹ️ No local settings file - nothing to merge"
    exit 0
fi
