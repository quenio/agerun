#!/bin/bash
# Validate KB links in AGENTS.md or other markdown files
# Usage: ./scripts/validate-kb-links.sh [file]
#
# If no file specified, defaults to AGENTS.md
#
# Returns:
# - Exit 0 if all links valid
# - Exit 1 if broken links found
set -o pipefail

set -e

TARGET_FILE=${1:-AGENTS.md}

if [ ! -f "$TARGET_FILE" ]; then
    echo "Error: File not found: $TARGET_FILE"
    exit 1
fi

echo "KB Link Validation: $TARGET_FILE"
echo "========================================"
echo ""

# Extract all KB links from the file
# Pattern: kb/[path].md inside markdown links
# Use sed to extract kb/ paths from markdown link syntax [text](kb/path.md)
LINKS=$(grep -o '(kb/[^)]*\.md)' "$TARGET_FILE" 2>/dev/null | sed 's/^(//' | sed 's/)$//' | sort -u || true)

if [ -z "$LINKS" ]; then
    echo "⚠️  No KB links found in $TARGET_FILE"
    exit 0
fi

TOTAL=0
BROKEN=0
BROKEN_LINKS=""

echo "Checking KB links..."
for link in $LINKS; do
    TOTAL=$((TOTAL + 1))
    if [ ! -f "$link" ]; then
        echo "  ❌ BROKEN: $link"
        BROKEN=$((BROKEN + 1))
        BROKEN_LINKS="$BROKEN_LINKS\n  - $link"
    else
        echo "  ✅ Valid:  $link"
    fi
done

echo ""
echo "========================================"
echo "Summary:"
echo "  Total links checked: $TOTAL"
echo "  Valid links: $((TOTAL - BROKEN))"
echo "  Broken links: $BROKEN"
echo ""

if [ $BROKEN -eq 0 ]; then
    echo "✅ ALL KB LINKS VALID"
    echo ""
    echo "All $TOTAL KB links in $TARGET_FILE point to existing files."
    exit 0
else
    echo "❌ BROKEN LINKS FOUND"
    echo ""
    echo "The following links are broken:"
    echo -e "$BROKEN_LINKS"
    echo ""
    echo "Fix these broken links before proceeding."
    exit 1
fi
