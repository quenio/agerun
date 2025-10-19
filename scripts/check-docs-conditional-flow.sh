#!/bin/bash
# Conditional flow for docs check based on error count
# Usage: ./scripts/check-docs-conditional-flow.sh
#
# Purpose:
# - Reads error count from /tmp/check-docs-stats.txt
# - If no errors found, skips fix stages (2, 3, 4)
# - If errors found, enforces gate and continues to fixing
# - Updates checkpoint accordingly
#
# Returns:
# - Exit 0 if flow decision made successfully
# - Exit 1 if gate enforcement fails

set -e

# Source the stats file to get error count
source /tmp/check-docs-stats.txt 2>/dev/null || ERROR_COUNT=0

if [ $ERROR_COUNT -eq 0 ]; then
    echo "✅ No documentation errors found - skipping fix stages"
    ./scripts/checkpoint-update.sh check-docs 2
    ./scripts/checkpoint-update.sh check-docs 3
    ./scripts/checkpoint-update.sh check-docs 4
    exit 0
else
    echo "⚠️ Found $ERROR_COUNT documentation error(s) - running fixes"
    ./scripts/gate-checkpoint.sh check-docs "Errors Found" "1"
    exit $?
fi
