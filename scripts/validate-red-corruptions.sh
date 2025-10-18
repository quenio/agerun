#!/bin/bash
# Validate RED phase corruption evidence after Step 7 (Structure RED Phases)
# Usage: validate_red_corruptions.sh <evidence-file> <expected-count>
set -o pipefail

set -e

EVIDENCE_FILE="$1"
EXPECTED_COUNT="$2"

if [ -z "$EVIDENCE_FILE" ] || [ -z "$EXPECTED_COUNT" ]; then
    echo "Usage: validate_red_corruptions.sh <evidence-file> <expected-count>"
    exit 1
fi

if [ ! -f "$EVIDENCE_FILE" ]; then
    echo "❌ ERROR: Evidence file not found: $EVIDENCE_FILE"
    echo ""
    echo "You must create $EVIDENCE_FILE with one corruption per line:"
    echo "Example format:"
    echo "  8.1: Stub returns NULL to prove test catches failures"
    echo "  8.2: Skip freeing own_allowed_path to cause leak"
    echo "  8.3: Return 'network' instead of 'file' to prove type check"
    echo ""
    exit 1
fi

echo "🔍 Validating RED corruption evidence: $EVIDENCE_FILE"
echo "Expected entries: $EXPECTED_COUNT"
echo ""

# Count non-empty, non-comment lines
ACTUAL_COUNT=$({ grep -v "^#" "$EVIDENCE_FILE" || true; } | { grep -v "^$" || true; } | wc -l | tr -d ' ')

echo "Found entries: $ACTUAL_COUNT"

if [ "$ACTUAL_COUNT" -ne "$EXPECTED_COUNT" ]; then
    echo "❌ FAIL: Expected $EXPECTED_COUNT corruption entries, found $ACTUAL_COUNT"
    exit 1
fi

echo "✅ Corruption entry count matches"

# Validate each entry has iteration number and description
ERRORS=0

echo ""
echo "Checking corruption entries..."

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

    # Check description is not too short
    DESCRIPTION=$(echo "$line" | cut -d: -f2- | xargs)
    if [ ${#DESCRIPTION} -lt 10 ]; then
        echo "❌ Description too short: $line"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    echo "  ✅ $(echo "$line" | cut -d: -f1)"
done < "$EVIDENCE_FILE"

echo ""

if [ $ERRORS -gt 0 ]; then
    echo "❌ VALIDATION FAILED: $ERRORS format errors found"
    exit 1
fi

echo "✅ All corruption entries are properly formatted"
echo ""
echo "✅ RED CORRUPTION EVIDENCE VALIDATION PASSED"
exit 0
