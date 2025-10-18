#!/bin/bash
# Run naming convention check and capture violation count
# Usage: ./scripts/run-naming-check.sh
#
# Purpose:
# - Executes naming convention validation
# - Captures output and counts violations
# - Saves violation count to /tmp/check-naming-stats.txt for conditional workflows
#
# Returns:
# - Exit 0 if check passes (no violations)
# - Exit 1 if violations found
# - Creates /tmp/check-naming-output.txt with full output
# - Creates /tmp/check-naming-stats.txt with VIOLATION_COUNT

set -e
set -o pipefail

# Check if running from repo root
if [ ! -f "Makefile" ] || [ ! -d "modules" ]; then
    echo "ERROR: This script must be run from the AgeRun repository root directory."
    exit 1
fi

echo "Running naming convention check..."
echo

# Run the check and capture output
if make check-naming 2>&1 | tee /tmp/check-naming-output.txt; then
    echo "✅ All naming conventions are correct!"
    echo "VIOLATION_COUNT=0" > /tmp/check-naming-stats.txt
    exit 0
else
    # Count violations from the output
    VIOLATION_COUNT=$(grep -E "ERROR|Invalid|violation" /tmp/check-naming-output.txt | wc -l || echo "0")
    echo ""
    echo "⚠️ Found $VIOLATION_COUNT naming violations"
    echo "VIOLATION_COUNT=$VIOLATION_COUNT" > /tmp/check-naming-stats.txt
    exit 1
fi
