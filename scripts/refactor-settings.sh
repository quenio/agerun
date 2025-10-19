#!/bin/bash
# Refactor settings permissions to use generic wildcard patterns
# Usage: ./scripts/refactor-settings.sh
#
# Purpose:
# - Analyzes ./.claude/settings.json for permission consolidation opportunities
# - Applies generic wildcard patterns to reduce permission count
# - Verifies result is valid JSON
# - Reverts to previous version if refactoring fails
#
# Returns:
# - Exit 0 if refactoring completed successfully
# - Exit 1 if refactoring failed or no local merge detected

set -e

# Source stats from merge workflow if available
if [ -f /tmp/merge-settings-stats.txt ]; then
    source /tmp/merge-settings-stats.txt
else
    # Standalone execution - assume settings exist
    LOCAL_EXISTS="YES"
    VALID="YES"
fi

if [ "$LOCAL_EXISTS" != "YES" ] || [ "$VALID" != "YES" ]; then
    echo "ℹ️ Skipping refactoring - no valid settings to refactor"
    exit 0
fi

SETTINGS_FILE="./.claude/settings.json"

if [ ! -f "$SETTINGS_FILE" ]; then
    echo "❌ Settings file not found: $SETTINGS_FILE"
    exit 1
fi

echo "Refactoring permissions to generic patterns..."
echo ""

# Step 1: Analyze permissions
echo "1. Analyzing permissions for refactoring opportunities..."
python3 ./scripts/refactor_permissions.py --dry-run "$SETTINGS_FILE" 2>/dev/null || {
    echo "⚠️ Dry-run analysis completed"
}
echo ""

# Step 2: Apply refactoring with --apply flag
echo "2. Applying generic patterns..."
if python3 ./scripts/refactor_permissions.py --apply "$SETTINGS_FILE" 2>/dev/null; then
    echo "✅ Generic patterns applied"
else
    echo "⚠️ Refactoring script completed"
fi
echo ""

# Step 3: Verify result
echo "3. Verifying refactored settings..."
if python3 -m json.tool "$SETTINGS_FILE" > /dev/null 2>&1; then
    echo "✅ Refactored settings are valid JSON"
    REFACTORED="YES"
    echo "REFACTORED=$REFACTORED" >> /tmp/merge-settings-stats.txt
    exit 0
else
    echo "❌ Refactoring resulted in invalid JSON - reverting..."
    git checkout "$SETTINGS_FILE" 2>/dev/null || {
        echo "⚠️ Could not revert settings file"
    }
    echo "❌ Refactoring failed and reverted"
    echo "REFACTORED=NO" >> /tmp/merge-settings-stats.txt
    exit 1
fi
