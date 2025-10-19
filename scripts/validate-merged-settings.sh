#!/bin/bash
# Validate merged settings JSON syntax
# Usage: ./scripts/validate-merged-settings.sh
#
# Purpose:
# - Validates that merged settings.json is valid JSON
# - Updates statistics with validation result
#
# Returns:
# - Exit 0 if validation passes
# - Exit 1 if validation fails
# - Updates /tmp/merge-settings-stats.txt with VALID flag

source /tmp/merge-settings-stats.txt

echo "Validating merged settings..."

if [ "$LOCAL_EXISTS" = "YES" ]; then
    # Validate JSON syntax
    if python3 -m json.tool ./.claude/settings.json > /dev/null 2>&1; then
        echo "✅ Merged settings are valid JSON"
        VALID="YES"
    else
        echo "❌ Invalid JSON after merge!"
        VALID="NO"
        echo "VALID=$VALID" >> /tmp/merge-settings-stats.txt
        exit 1
    fi

    echo "VALID=$VALID" >> /tmp/merge-settings-stats.txt
fi

exit 0
