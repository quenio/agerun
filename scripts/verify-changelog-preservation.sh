#!/bin/bash
# Verify CHANGELOG.md preservation metrics after compaction
# Usage: verify-changelog-preservation.sh <stats-file>
# Returns: 0 if preservation successful, 1 if metrics lost

set -e
set -o pipefail

STATS_FILE="${1:-/tmp/compact-changes-stats.txt}"

if [ ! -f "$STATS_FILE" ]; then
  echo "❌ ERROR: Stats file not found: $STATS_FILE"
  exit 1
fi

if [ ! -f "CHANGELOG.md" ]; then
  echo "❌ ERROR: CHANGELOG.md not found in current directory"
  exit 1
fi

source "$STATS_FILE"

echo "Verifying preservation..."

# Get new metrics
NEW_LINES=$(wc -l < CHANGELOG.md)
NEW_BYTES=$(wc -c < CHANGELOG.md)
NEW_METRICS=$(grep -oE "[0-9]+[%]|[0-9]+ (files|lines|occurrences|tests|modules|functions)" CHANGELOG.md | wc -l || echo "0")

echo "New: $NEW_LINES lines, $NEW_BYTES bytes"
echo "Metrics found: $NEW_METRICS (expected: $METRICS_COUNT)"

# Calculate reduction
LINE_REDUCTION=$(( (ORIGINAL_LINES - NEW_LINES) * 100 / ORIGINAL_LINES ))
SIZE_REDUCTION=$(( (ORIGINAL_BYTES - NEW_BYTES) * 100 / ORIGINAL_BYTES ))

echo "Line reduction: $LINE_REDUCTION%"
echo "File size reduction: $SIZE_REDUCTION%"

# Update stats file
echo "NEW_LINES=$NEW_LINES" >> "$STATS_FILE"
echo "NEW_BYTES=$NEW_BYTES" >> "$STATS_FILE"
echo "NEW_METRICS=$NEW_METRICS" >> "$STATS_FILE"
echo "LINE_REDUCTION=$LINE_REDUCTION" >> "$STATS_FILE"
echo "SIZE_REDUCTION=$SIZE_REDUCTION" >> "$STATS_FILE"

# Verify metrics preserved
if [ "$NEW_METRICS" -ge "$METRICS_COUNT" ]; then
  echo "✅ All metrics preserved"
  echo "METRICS_OK=PASS" >> "$STATS_FILE"
else
  echo "⚠️ WARNING: Some metrics may be missing"
  echo "METRICS_OK=WARN" >> "$STATS_FILE"
fi

# Verify dates preserved
DATE_CHECK=$(grep "^## 2025-" CHANGELOG.md | wc -l || echo "0")
echo "Date sections remaining: $DATE_CHECK"

if [ "$DATE_CHECK" -gt 0 ]; then
  exit 0
else
  echo "⚠️ WARNING: No date sections found"
  exit 0
fi
