#!/bin/bash
# Verify command meets excellence standards (90%+ quality score)
# Usage: verify-command-quality.sh <command-name>
# Returns: 0 if passes (>=90%), 1 if fails (<90%)

set -e
set -o pipefail

COMMAND_NAME="$1"

if [ -z "$COMMAND_NAME" ]; then
  echo "❌ ERROR: Command name is required!"
  echo "Usage: $0 <command-name>"
  exit 1
fi

echo "🎯 Verify command meets excellence standards"
echo ""

# Run check-commands validator
if make check-commands 2>&1 | tee /tmp/create-command-validation.txt; then
  echo "✅ Command passes validation"
else
  echo "⚠️  Validation issues found - review output above"
fi

# Extract score for the command
SCORE=$(grep "$COMMAND_NAME" /tmp/create-command-validation.txt | grep -oE '[0-9]+%' || echo "0%")

echo ""
echo "Command: $COMMAND_NAME"
echo "Score: $SCORE"
echo ""

if echo "$SCORE" | grep -qE '(9[0-9]|100)%'; then
  echo "✅ EXCELLENT: Meets 90%+ threshold"
  exit 0
else
  echo "❌ NEEDS IMPROVEMENT: Below 90% threshold"
  echo ""
  echo "Common issues:"
  echo "  • Missing expected output examples"
  echo "  • Missing troubleshooting section"
  echo "  • Missing minimum requirements"
  echo "  • Incomplete checkpoint tracking"
  echo ""
  echo "Review: kb/command-documentation-excellence-gate.md"
  exit 1
fi
