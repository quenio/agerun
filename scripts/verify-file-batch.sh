#!/bin/bash
# Verify batch of files exist with detailed reporting
# Usage: verify-file-batch.sh <file1> [file2] [file3] ...
# Returns: 0 if all files exist, 1 if any missing
# Output: Detailed verification report with counts

set -e
set -o pipefail

if [ $# -eq 0 ]; then
  echo "❌ ERROR: No files specified"
  echo "Usage: $0 <file1> [file2] [file3] ..."
  exit 1
fi

TOTAL_FILES=$#
FOUND_COUNT=0
MISSING_COUNT=0
MISSING_FILES=()

echo "Verifying $TOTAL_FILES file(s)..."
echo ""

# Check each file
for file in "$@"; do
  if [ -f "$file" ]; then
    echo "  ✓ Found: $file"
    FOUND_COUNT=$((FOUND_COUNT + 1))
  else
    echo "  ✗ Missing: $file"
    MISSING_COUNT=$((MISSING_COUNT + 1))
    MISSING_FILES+=("$file")
  fi
done

echo ""
echo "Summary: $FOUND_COUNT/$TOTAL_FILES files found"

# Output counts for caller
echo "TOTAL_FILES=$TOTAL_FILES"
echo "FOUND_COUNT=$FOUND_COUNT"
echo "MISSING_COUNT=$MISSING_COUNT"

# Output missing files as semicolon-separated list if any
if [ "$MISSING_COUNT" -gt 0 ]; then
  MISSING_LIST=$(printf '%s;' "${MISSING_FILES[@]}")
  MISSING_LIST="${MISSING_LIST%;}"  # Remove trailing semicolon
  echo "MISSING_FILES=\"$MISSING_LIST\""
  echo ""
  echo "❌ $MISSING_COUNT file(s) missing:"
  for file in "${MISSING_FILES[@]}"; do
    echo "   - $file"
  done
  exit 1
else
  echo "✅ All files verified"
  exit 0
fi
