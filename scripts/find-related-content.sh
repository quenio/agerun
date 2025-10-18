#!/bin/bash
# Find related KB articles and commands for cross-referencing
# Usage: ./scripts/find-related-content.sh <keyword1> [keyword2] [keyword3] ...
#
# Searches for:
# - KB articles mentioning keywords
# - Commands mentioning keywords
#
# Returns:
# - Exit 0 always
# - Outputs related content grouped by type

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <keyword1> [keyword2] [keyword3] ..."
    echo ""
    echo "Example: $0 test assertion whitelist"
    echo "  Finds KB articles and commands related to testing, assertions, and whitelisting"
    exit 1
fi

KEYWORDS="$@"

echo "Related Content Discovery"
echo "========================================"
echo "Keywords: $KEYWORDS"
echo ""

# Build grep pattern from keywords
PATTERN=$(echo "$KEYWORDS" | sed 's/ /\\|/g')

# Step 1: Find related KB articles
echo "1. Related KB Articles"
echo "   Searching for: $PATTERN"
echo ""

KB_ARTICLES=$(grep -l "$PATTERN" kb/*.md 2>/dev/null | head -10 || true)

if [ -n "$KB_ARTICLES" ]; then
    echo "$KB_ARTICLES" | while read -r article; do
        # Get first line (title) from article
        TITLE=$(head -1 "$article" | sed 's/^# //')
        printf "   %-50s %s\n" "$article" "$TITLE"
    done

    KB_COUNT=$(echo "$KB_ARTICLES" | wc -l | tr -d ' ')
else
    echo "   (no KB articles found)"
    KB_COUNT=0
fi

echo ""
echo "   Total KB articles: $KB_COUNT"
echo ""

# Step 2: Find related commands
echo "2. Related Commands"
echo "   Searching across multiple keywords..."
echo ""

declare -a SEARCH_KEYWORDS=(
    "test"
    "build"
    "check"
    "log"
    "error"
    "whitelist"
    "assert"
)

# Use provided keywords if any, otherwise use default set
if [ $# -gt 0 ]; then
    SEARCH_KEYWORDS=("$@")
fi

COMMAND_COUNT=0

for keyword in "${SEARCH_KEYWORDS[@]}"; do
    COMMANDS=$(grep -l "$keyword" .opencode/command/ar/*.md 2>/dev/null | head -5 || true)

    if [ -n "$COMMANDS" ]; then
        echo "   === Commands mentioning '$keyword' ==="
        echo "$COMMANDS" | sed 's/^/     /'
        echo ""

        COUNT=$(echo "$COMMANDS" | wc -l | tr -d ' ')
        COMMAND_COUNT=$((COMMAND_COUNT + COUNT))
    fi
done

if [ "$COMMAND_COUNT" -eq 0 ]; then
    echo "   (no commands found)"
fi

echo "   Total command matches: $COMMAND_COUNT"
echo ""

# Step 3: Priority commands to always check
echo "3. Priority Commands to Review"
echo "   (These frequently need updates)"
echo ""

PRIORITY_COMMANDS=(
    ".opencode/command/ar/commit.md"
    ".opencode/command/ar/build.md"
    ".opencode/command/ar/run-tests.md"
    ".opencode/command/ar/sanitize-tests.md"
    ".opencode/command/ar/fix-errors-whitelisted.md"
    ".opencode/command/ar/check-logs.md"
)

for cmd in "${PRIORITY_COMMANDS[@]}"; do
    if [ -f "$cmd" ]; then
        printf "   %-50s ✅\n" "$cmd"
    else
        printf "   %-50s ❌ (not found)\n" "$cmd"
    fi
done

echo ""

# Summary
echo "========================================"
echo "Summary:"
echo "  KB articles found:    $KB_COUNT"
echo "  Command matches:      $COMMAND_COUNT"
echo "  Priority commands:    ${#PRIORITY_COMMANDS[@]}"
echo ""

if [ "$KB_COUNT" -gt 0 ] || [ "$COMMAND_COUNT" -gt 0 ]; then
    echo "✅ Found related content for cross-referencing"
    echo ""
    echo "Next steps:"
    echo "  1. Add cross-references to related KB articles"
    echo "  2. Update relevant commands with new KB references"
    echo "  3. Verify minimum requirements (3+ KB, 3+ commands)"
else
    echo "⚠️  No related content found"
    echo "   Consider broader keywords or check search paths"
fi

# Output for shell sourcing
echo ""
echo "KB_ARTICLES_FOUND=$KB_COUNT"
echo "COMMANDS_FOUND=$COMMAND_COUNT"

exit 0
