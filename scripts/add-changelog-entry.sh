#!/bin/bash
# Add entry to CHANGELOG.md at the top with proper formatting
# Usage: add-changelog-entry.sh <date> <summary> [details...]
# Example: add-changelog-entry.sh "2025-10-18" "feature: implemented new feature" "Description line 1" "Description line 2"

set -e
set -o pipefail

DATE="$1"
SUMMARY="$2"
shift 2
DETAILS="$@"

if [ -z "$DATE" ] || [ -z "$SUMMARY" ]; then
  echo "❌ ERROR: Missing required arguments"
  echo "Usage: $0 <date> <summary> [details...]"
  echo "Example: $0 \"2025-10-18\" \"feature: new feature\" \"Added support for X\""
  exit 1
fi

if [ ! -f "CHANGELOG.md" ]; then
  echo "❌ ERROR: CHANGELOG.md not found in current directory"
  exit 1
fi

# Create temporary file with new entry
TEMP_FILE=$(mktemp)
trap "rm -f $TEMP_FILE" EXIT

cat > "$TEMP_FILE" << EOF
## $DATE

- **$SUMMARY**
EOF

# Add details if provided
if [ -n "$DETAILS" ]; then
  while [ $# -gt 0 ]; do
    echo "  - $1" >> "$TEMP_FILE"
    shift
  done
fi

# Add blank line and existing content
echo "" >> "$TEMP_FILE"
cat CHANGELOG.md >> "$TEMP_FILE"

# Replace original
mv "$TEMP_FILE" CHANGELOG.md

echo "✅ Added CHANGELOG entry for $DATE"
echo "   Summary: $SUMMARY"
