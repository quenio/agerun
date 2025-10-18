#!/bin/bash
# Validate GREEN phase minimalism evidence after Step 8 (Structure GREEN Phases)
# Usage: validate_green_minimalism.sh <evidence-file> <expected-count>
set -o pipefail

set -e

EVIDENCE_FILE="$1"
EXPECTED_COUNT="$2"

if [ -z "$EVIDENCE_FILE" ] || [ -z "$EXPECTED_COUNT" ]; then
    echo "Usage: validate_green_minimalism.sh <evidence-file> <expected-count>"
    exit 1
fi

if [ ! -f "$EVIDENCE_FILE" ]; then
    echo "❌ ERROR: Evidence file not found: $EVIDENCE_FILE"
    echo ""
    echo "You must create $EVIDENCE_FILE with minimalism verification:"
    echo "Example format:"
    echo "  8.1: Hardcoded return - minimal"
    echo "  8.2: Only frees resources - minimal"
    echo "  9.1: Returns NULL - hardcoded"
    echo "  9.2: Reads file - forced by test (not over-implemented)"
    echo ""
    exit 1
fi

echo "🔍 Validating GREEN minimalism evidence: $EVIDENCE_FILE"
echo "Expected entries: $EXPECTED_COUNT"
echo ""

# Count non-empty, non-comment lines
ACTUAL_COUNT=$({ grep -v "^#" "$EVIDENCE_FILE" || true; } | { grep -v "^$" || true; } | wc -l | tr -d ' ')

echo "Found entries: $ACTUAL_COUNT"

if [ "$ACTUAL_COUNT" -ne "$EXPECTED_COUNT" ]; then
    echo "❌ FAIL: Expected $EXPECTED_COUNT minimalism entries, found $ACTUAL_COUNT"
    exit 1
fi

echo "✅ Minimalism entry count matches"

# Validate each entry has iteration number and justification
ERRORS=0

echo ""
echo "Checking minimalism entries..."

# Keywords that indicate proper minimalism thinking
GOOD_KEYWORDS="hardcoded|minimal|only|forced by|simple|stub|returns (NULL|false|true|0)"

while IFS= read -r line; do
    # Skip comments and empty lines
    [[ "$line" =~ ^# ]] && continue
    [ -z "$line" ] && continue

    # Check format: N.N: Description
    if ! echo "$line" | grep -q "^[0-9]\+\.[0-9]\+:"; then
        echo "❌ Invalid format (missing iteration number): $line"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    # Check description mentions minimalism concepts
    DESCRIPTION=$(echo "$line" | cut -d: -f2- | tr '[:upper:]' '[:lower:]')
    if ! echo "$DESCRIPTION" | grep -qE "$GOOD_KEYWORDS"; then
        echo "⚠️  Warning: No minimalism keywords in: $line"
        echo "    Expected keywords: hardcoded, minimal, stub, forced by test, etc."
    fi

    echo "  ✅ $(echo "$line" | cut -d: -f1)"
done < "$EVIDENCE_FILE"

echo ""

if [ $ERRORS -gt 0 ]; then
    echo "❌ VALIDATION FAILED: $ERRORS format errors found"
    exit 1
fi

echo "✅ All minimalism entries are properly formatted"
echo ""
echo "✅ GREEN MINIMALISM EVIDENCE VALIDATION PASSED"
exit 0
