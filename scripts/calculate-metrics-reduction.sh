#!/bin/bash
# Calculate metrics reduction percentages (lines/bytes)
# Usage: calculate-metrics-reduction.sh <before_lines> <before_bytes> <after_lines> <after_bytes>
# Returns: 0 if successful, 1 if invalid input
# Output: Sets LINE_REDUCTION and SIZE_REDUCTION environment variables

set -e
set -o pipefail

BEFORE_LINES="${1:-0}"
BEFORE_BYTES="${2:-0}"
AFTER_LINES="${3:-0}"
AFTER_BYTES="${4:-0}"

# Validate inputs are numbers
if ! [[ "$BEFORE_LINES" =~ ^[0-9]+$ ]] || ! [[ "$BEFORE_BYTES" =~ ^[0-9]+$ ]] || \
   ! [[ "$AFTER_LINES" =~ ^[0-9]+$ ]] || ! [[ "$AFTER_BYTES" =~ ^[0-9]+$ ]]; then
  echo "❌ ERROR: All parameters must be positive integers"
  echo "Usage: $0 <before_lines> <before_bytes> <after_lines> <after_bytes>"
  exit 1
fi

# Validate that 'before' values are greater than 'after' values
if [ "$BEFORE_LINES" -lt "$AFTER_LINES" ] || [ "$BEFORE_BYTES" -lt "$AFTER_BYTES" ]; then
  echo "❌ ERROR: Before values must be >= after values"
  echo "  Lines: before=$BEFORE_LINES, after=$AFTER_LINES"
  echo "  Bytes: before=$BEFORE_BYTES, after=$AFTER_BYTES"
  exit 1
fi

# Prevent division by zero
if [ "$BEFORE_LINES" -eq 0 ] || [ "$BEFORE_BYTES" -eq 0 ]; then
  echo "❌ ERROR: Before values cannot be zero"
  exit 1
fi

# Calculate line reduction
LINE_REDUCTION=$(( (BEFORE_LINES - AFTER_LINES) * 100 / BEFORE_LINES ))

# Calculate size reduction
SIZE_REDUCTION=$(( (BEFORE_BYTES - AFTER_BYTES) * 100 / BEFORE_BYTES ))

# Output for consumption by calling script
echo "LINE_REDUCTION=$LINE_REDUCTION"
echo "SIZE_REDUCTION=$SIZE_REDUCTION"
echo "BEFORE_LINES=$BEFORE_LINES"
echo "BEFORE_BYTES=$BEFORE_BYTES"
echo "AFTER_LINES=$AFTER_LINES"
echo "AFTER_BYTES=$AFTER_BYTES"

# Display results
echo ""
echo "Reduction Summary:"
echo "  Lines: $BEFORE_LINES → $AFTER_LINES (${LINE_REDUCTION}% reduction)"
echo "  Bytes: $BEFORE_BYTES → $AFTER_BYTES (${SIZE_REDUCTION}% reduction)"

exit 0
