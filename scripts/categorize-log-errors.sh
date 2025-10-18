#!/bin/bash
# Categorize log errors as real vs intentional (whitelisted)
# Usage: ./scripts/categorize-log-errors.sh <log-output-file> <whitelist-file>
#
# Reads errors from log output, checks against whitelist, and categorizes them
#
# Returns:
# - Exit 0 always (categorization complete)
# - Outputs: REAL_ERRORS and INTENTIONAL_ERRORS counts to stdout

set -e

LOG_FILE=${1:-/tmp/check-logs-output.txt}
WHITELIST_FILE=${2:-log_whitelist.yaml}

if [ ! -f "$LOG_FILE" ]; then
    echo "Error: Log file not found: $LOG_FILE"
    exit 1
fi

if [ ! -f "$WHITELIST_FILE" ]; then
    echo "Warning: Whitelist file not found: $WHITELIST_FILE"
    echo "All errors will be categorized as REAL"
fi

# Count total errors
TOTAL_ERRORS=$(grep -c "ERROR\|FAILURE" "$LOG_FILE" 2>/dev/null || echo "0")

if [ "$TOTAL_ERRORS" -eq 0 ]; then
    echo "No errors found in $LOG_FILE"
    echo "REAL_ERRORS=0"
    echo "INTENTIONAL_ERRORS=0"
    exit 0
fi

echo "Categorizing $TOTAL_ERRORS errors from $LOG_FILE..."
echo ""

# Count real vs intentional errors
REAL_ERRORS=0
INTENTIONAL_ERRORS=0

# Check each error against whitelist
while IFS= read -r error; do
    if [ -f "$WHITELIST_FILE" ] && grep -q "$error" "$WHITELIST_FILE"; then
        INTENTIONAL_ERRORS=$((INTENTIONAL_ERRORS + 1))
    else
        REAL_ERRORS=$((REAL_ERRORS + 1))
    fi
done < <(grep "ERROR\|FAILURE" "$LOG_FILE")

echo "Categorization Results:"
echo "  Total errors: $TOTAL_ERRORS"
echo "  Real errors (need fixing): $REAL_ERRORS"
echo "  Intentional errors (whitelisted): $INTENTIONAL_ERRORS"
echo ""

# Output for shell sourcing
echo "REAL_ERRORS=$REAL_ERRORS"
echo "INTENTIONAL_ERRORS=$INTENTIONAL_ERRORS"

exit 0
