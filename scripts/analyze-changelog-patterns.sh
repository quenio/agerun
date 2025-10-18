#!/bin/bash
# Analyze CHANGELOG.md patterns for compaction opportunities
# Usage: ./scripts/analyze-changelog-patterns.sh [changelog-file]
#
# Provides:
# - Baseline measurements (lines, bytes, metrics)
# - Pattern analysis (repetitive topics, date sections)
# - Compaction opportunities
#
# Returns:
# - Exit 0 always
# - Outputs analysis and stats to stdout
set -o pipefail

set -e

CHANGELOG_FILE=${1:-CHANGELOG.md}

if [ ! -f "$CHANGELOG_FILE" ]; then
    echo "Error: CHANGELOG file not found: $CHANGELOG_FILE"
    exit 1
fi

echo "CHANGELOG Pattern Analysis: $CHANGELOG_FILE"
echo "========================================"
echo ""

# Step 1: Measure baseline
echo "1. Baseline Measurements"

ORIGINAL_LINES=$(wc -l < "$CHANGELOG_FILE" | tr -d ' ')
ORIGINAL_BYTES=$(wc -c < "$CHANGELOG_FILE" | tr -d ' ')
METRICS_COUNT=$(grep -oE "[0-9]+[%]|[0-9]+ (files|lines|occurrences|tests|modules|functions)" "$CHANGELOG_FILE" 2>/dev/null | wc -l | tr -d ' ')

echo "   Lines: $ORIGINAL_LINES"
echo "   Bytes: $ORIGINAL_BYTES"
echo "   Metrics to preserve: $METRICS_COUNT"
echo ""

# Step 2: Analyze patterns
echo "2. Structural Analysis"

# Count date sections
DATE_SECTIONS=$(grep -c "^## 2025-" "$CHANGELOG_FILE" 2>/dev/null || echo "0")
echo "   Date sections: $DATE_SECTIONS"
echo ""

# Look for repetitive patterns
echo "3. Repetitive Pattern Detection"
echo "   (Topics appearing frequently across multiple entries)"
echo ""

# Define patterns to search for
declare -a PATTERNS=(
    "System Module"
    "Parser"
    "Global API"
    "Memory"
    "Test"
    "Migration"
    "Refactor"
    "Documentation"
)

for pattern in "${PATTERNS[@]}"; do
    COUNT=$(grep -c "$pattern" "$CHANGELOG_FILE" 2>/dev/null || echo "0")
    if [ "$COUNT" -gt 5 ]; then
        printf "   - %-20s %3d occurrences\n" "$pattern:" "$COUNT"
    fi
done

echo ""

# Compaction opportunities
echo "4. Compaction Opportunities"
echo ""
echo "   Analysis questions to consider:"
echo "   • What patterns of repetitive work appear across multiple dates?"
echo "   • Which entries describe the same architectural work?"
echo "   • Can date ranges be grouped (e.g., \"2025-01-15 to 2025-01-20\")?"
echo "   • Which verbose bullet lists need condensing?"
echo ""

if [ "$DATE_SECTIONS" -gt 30 ]; then
    echo "   ⚠️  High date fragmentation ($DATE_SECTIONS sections)"
    echo "      Consider grouping related work by theme or time period"
elif [ "$DATE_SECTIONS" -gt 15 ]; then
    echo "   ℹ️  Moderate date sections ($DATE_SECTIONS)"
    echo "      Some consolidation may help"
else
    echo "   ✅ Reasonable date granularity ($DATE_SECTIONS sections)"
fi

echo ""
echo "========================================"
echo "Summary:"
echo "  Current size: $ORIGINAL_LINES lines ($ORIGINAL_BYTES bytes)"
echo "  Metrics count: $METRICS_COUNT"
echo "  Date sections: $DATE_SECTIONS"
echo ""

# Estimate compaction potential
ESTIMATED_REDUCTION=$((ORIGINAL_LINES * 20 / 100))
echo "  Estimated compaction potential: ~$ESTIMATED_REDUCTION lines (20% reduction)"

# Output for shell sourcing
echo ""
echo "ORIGINAL_LINES=$ORIGINAL_LINES"
echo "ORIGINAL_BYTES=$ORIGINAL_BYTES"
echo "METRICS_COUNT=$METRICS_COUNT"
echo "DATE_SECTIONS=$DATE_SECTIONS"

exit 0
