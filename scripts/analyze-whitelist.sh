#!/bin/bash
# Analyze log whitelist patterns
# Usage: ./scripts/analyze-whitelist.sh [whitelist-file]
#
# Provides analysis of whitelisted errors including:
# - Total count
# - Top tests with most errors
# - Most common error messages
#
# Returns:
# - Exit 0 always
# - Outputs analysis to stdout
set -o pipefail

set -e

WHITELIST_FILE=${1:-log_whitelist.yaml}

if [ ! -f "$WHITELIST_FILE" ]; then
    echo "Error: Whitelist file not found: $WHITELIST_FILE"
    exit 1
fi

echo "Whitelist Analysis: $WHITELIST_FILE"
echo "========================================"
echo ""

# Step 1: Count total whitelisted errors
TOTAL_COUNT=$(grep -c "^  -" "$WHITELIST_FILE" 2>/dev/null || echo "0")
echo "1. Total Statistics"
echo "   Total whitelisted errors: $TOTAL_COUNT"
echo ""

# Step 2: Group by test context (top 10)
echo "2. Top 10 Tests with Most Errors"
if grep -q "context:" "$WHITELIST_FILE" 2>/dev/null; then
    grep "context:" "$WHITELIST_FILE" | \
        sed 's/.*context: "//' | \
        sed 's/".*//' | \
        sort | \
        uniq -c | \
        sort -nr | \
        head -10 | \
        sed 's/^/   /'
else
    echo "   (no context entries found)"
fi
echo ""

# Step 3: Find common error patterns (top 10)
echo "3. Top 10 Most Common Error Messages"
if grep -q "message:" "$WHITELIST_FILE" 2>/dev/null; then
    grep "message:" "$WHITELIST_FILE" | \
        sed 's/.*message: "//' | \
        sed 's/".*//' | \
        sort | \
        uniq -c | \
        sort -nr | \
        head -10 | \
        sed 's/^/   /'
else
    echo "   (no message entries found)"
fi
echo ""

echo "========================================"
echo "Summary:"
echo "  Total entries: $TOTAL_COUNT"

if [ "$TOTAL_COUNT" -gt 50 ]; then
    echo "  ⚠️  High count - consider fixing root causes"
elif [ "$TOTAL_COUNT" -gt 20 ]; then
    echo "  ℹ️  Moderate count - review for duplicates"
else
    echo "  ✅ Low count - reasonable whitelist size"
fi

# Output count for shell sourcing
echo ""
echo "BEFORE_COUNT=$TOTAL_COUNT"

exit 0
