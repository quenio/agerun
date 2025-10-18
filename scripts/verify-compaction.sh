#!/bin/bash
# Verify TODO.md compaction results
# Usage: ./scripts/verify-compaction.sh <stats-file>
#
# Reads original stats from stats file and verifies:
# - Incomplete tasks are preserved
# - Reduction achieved
# - File size decreased
#
# Returns:
# - Exit 0 if integrity check passes
# - Exit 1 if incomplete tasks lost

set -e

STATS_FILE=${1:-/tmp/compact-tasks-stats.txt}
TODO_FILE="TODO.md"

if [ ! -f "$STATS_FILE" ]; then
    echo "Error: Stats file not found: $STATS_FILE"
    echo "Run compaction analysis first to create stats file"
    exit 1
fi

if [ ! -f "$TODO_FILE" ]; then
    echo "Error: TODO.md not found"
    exit 1
fi

echo "Compaction Verification: $TODO_FILE"
echo "========================================"
echo ""

# Load original stats
source "$STATS_FILE"

# Measure new state
NEW_LINES=$(wc -l < "$TODO_FILE" | tr -d ' ')
NEW_BYTES=$(wc -c < "$TODO_FILE" | tr -d ' ')
NEW_INCOMPLETE=$(grep -c "^- \[ \]" "$TODO_FILE" 2>/dev/null || echo "0")
NEW_COMPLETED=$(grep -c "^- \[x\]" "$TODO_FILE" 2>/dev/null || echo "0")

echo "1. Size Comparison"
echo "   Original: $ORIGINAL_LINES lines, $ORIGINAL_BYTES bytes"
echo "   New:      $NEW_LINES lines, $NEW_BYTES bytes"
echo ""

# Calculate reduction
if [ "$ORIGINAL_LINES" -gt 0 ]; then
    LINE_REDUCTION=$(( (ORIGINAL_LINES - NEW_LINES) * 100 / ORIGINAL_LINES ))
else
    LINE_REDUCTION=0
fi

if [ "$ORIGINAL_BYTES" -gt 0 ]; then
    SIZE_REDUCTION=$(( (ORIGINAL_BYTES - NEW_BYTES) * 100 / ORIGINAL_BYTES ))
else
    SIZE_REDUCTION=0
fi

echo "2. Reduction Achieved"
echo "   Line reduction:      $LINE_REDUCTION%"
echo "   File size reduction: $SIZE_REDUCTION%"
echo ""

# Task preservation verification
echo "3. Task Preservation"
echo "   Original incomplete: $INCOMPLETE"
echo "   New incomplete:      $NEW_INCOMPLETE"
echo "   Original completed:  $COMPLETED"
echo "   New completed:       $NEW_COMPLETED (some may have been merged)"
echo ""

# Verify incomplete tasks untouched
if [ "$NEW_INCOMPLETE" -eq "$INCOMPLETE" ]; then
    echo "   ✅ All $INCOMPLETE incomplete tasks preserved"
    INTEGRITY="PASS"
elif [ "$NEW_INCOMPLETE" -gt "$INCOMPLETE" ]; then
    echo "   ⚠️  Warning: More incomplete tasks than before ($NEW_INCOMPLETE > $INCOMPLETE)"
    echo "      This should not happen during compaction"
    INTEGRITY="WARN"
else
    echo "   ❌ CRITICAL: Incomplete task count decreased!"
    echo "      Original: $INCOMPLETE, New: $NEW_INCOMPLETE"
    echo "      Lost: $((INCOMPLETE - NEW_INCOMPLETE)) tasks"
    INTEGRITY="FAIL"
fi

echo ""
echo "========================================"
echo "Verification Result: $INTEGRITY"
echo ""

if [ "$INTEGRITY" = "PASS" ]; then
    echo "✅ Compaction successful!"
    echo "   - All incomplete tasks preserved"
    echo "   - Reduced by $LINE_REDUCTION% ($((ORIGINAL_LINES - NEW_LINES)) lines)"
elif [ "$INTEGRITY" = "WARN" ]; then
    echo "⚠️  Compaction completed with warnings"
    echo "   Review the changes carefully"
elif [ "$INTEGRITY" = "FAIL" ]; then
    echo "❌ Compaction FAILED integrity check"
    echo "   Do NOT commit - incomplete tasks were lost"
    echo "   Restore from backup and review compaction logic"
fi

# Output for shell sourcing
echo ""
echo "NEW_LINES=$NEW_LINES"
echo "NEW_BYTES=$NEW_BYTES"
echo "NEW_INCOMPLETE=$NEW_INCOMPLETE"
echo "LINE_REDUCTION=$LINE_REDUCTION"
echo "SIZE_REDUCTION=$SIZE_REDUCTION"
echo "INTEGRITY=$INTEGRITY"

# Exit code based on integrity
if [ "$INTEGRITY" = "FAIL" ]; then
    exit 1
else
    exit 0
fi
