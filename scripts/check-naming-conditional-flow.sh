#!/bin/bash
# Conditional flow for naming check based on violation count
# Usage: ./scripts/check-naming-conditional-flow.sh
#
# Purpose:
# - Reads violation count from /tmp/check-naming-stats.txt
# - If no violations found, skips analysis steps (2 and 3)
# - If violations found, enforces gate and continues to analysis
# - Updates checkpoint accordingly
#
# Returns:
# - Exit 0 if flow decision made successfully
# - Exit 1 if gate enforcement fails

set -e

# Source the stats file to get violation count
source /tmp/check-naming-stats.txt 2>/dev/null || VIOLATION_COUNT=0

if [ $VIOLATION_COUNT -eq 0 ]; then
    echo "✅ No violations found - skipping analysis"
    ./scripts/checkpoint-update.sh check-naming 2
    exit 0
else
    echo "⚠️ Found $VIOLATION_COUNT naming violations - running analysis"
    ./scripts/gate-checkpoint.sh check-naming "Violations Found" "1"
    exit $?
fi
